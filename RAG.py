import os
import sys
import sqlite3
import datetime
import shutil
import json
import subprocess
from pathlib import Path
import git
from tqdm import tqdm
# llama_index imports corretti
from llama_index.core import VectorStoreIndex, StorageContext, load_index_from_storage
from llama_index.core import SimpleDirectoryReader
from llama_index.llms.ollama import Ollama
from llama_index.llms.openai import OpenAI
from llama_index.embeddings.ollama import OllamaEmbedding
from llama_index.embeddings.openai import OpenAIEmbedding
from llama_index.core.schema import Document

# CONFIGURAZIONE
PROJECT_PATH = Path(".")
DB_PATH = PROJECT_PATH / "odla.db"
RAG_DATA_PATH = Path("./ODLA_RAG_Data")

# VARIABILI GLOBALI
LLM = None
EMBED_MODEL = None

# FUNZIONI UTILI

def create_and_write_file(file_path, content):
    try:
        # Verifica se la directory esiste, altrimenti creala
        directory = os.path.dirname(file_path) or '.'
        if not os.path.exists(directory):
            os.makedirs(directory)
            print(f"✅ Directory creata: {directory}")

        # Scrivi il contenuto nel file
        with open(file_path, "w", encoding="utf-8") as f:
            f.write(content)
        print(f"✅ File scritto correttamente: {file_path}")
        return True
    except Exception as e:
        print(f"❌ Errore durante la scrittura del file: {e}")
        return False
        
def dump_sqlite_database(return_content=False, file_path=None):
    """Esegue il dump del database SQLite."""
    try:
        print("\n=== INIZIO DUMP DATABASE ===")
        print(f"🔍 Percorso database: {DB_PATH}")

        # Verifica se il file del database esiste
        if not DB_PATH.exists():
            print("❌ ERRORE: Il file del database non esiste!")
            return None if return_content else False

        print("✅ File database trovato")

        # Connessione al database
        conn = sqlite3.connect(str(DB_PATH))
        cursor = conn.cursor()

        # Estrazione delle tabelle
        cursor.execute("SELECT name FROM sqlite_master WHERE type='table';")
        tables = cursor.fetchall()

        if not tables:
            print("⚠️ ATTENZIONE: Il database non contiene tabelle!")
            return None if return_content else False

        print(f"✅ Trovate {len(tables)} tabelle")

        # Creazione del contenuto del dump
        dump_lines = []
        for table in tables:
            table_name = table[0]
            print(f"  - Elaborazione tabella: {table_name}")
            dump_lines.append(f"--- Table: {table_name} ---")

            # Estrazione delle colonne
            cursor.execute(f"PRAGMA table_info({table_name});")
            columns = [col[1] for col in cursor.fetchall()]
            dump_lines.append(f"Columns: {', '.join(columns)}")

            # Estrazione delle righe
            cursor.execute(f"SELECT * FROM {table_name};")
            rows = cursor.fetchall()
            dump_lines.extend([f"{', '.join(map(str, row))}" for row in rows])
            dump_lines.append("")  # Separatore

        content = "\n".join(dump_lines)
        print("✅ Dump del database completato con successo")

        # Restituzione del contenuto o scrittura su file
        if return_content:
            return content
        elif file_path:
            with open(file_path, "w", encoding="utf-8") as f:
                f.write(content)
            print(f"✅ File dump scritto in: {file_path}")
            return True

    except Exception as e:
        print(f"❌ ERRORE durante il dump del database: {e}")
        return None if return_content else False

    finally:
        if 'conn' in locals() and conn:
            conn.close()
            print("✅ Connessione al database chiusa")

def list_indices():
    if not RAG_DATA_PATH.exists():
        return []
    return [d.name for d in RAG_DATA_PATH.iterdir() if d.is_dir()]

def select_models():
    global LLM, EMBED_MODEL
    print("""
    Seleziona backend da usare:
    1. Locale (Ollama - gemma3 + mxbai-embed-large)
    2. OpenAI API
    """)
    choice = input("Scelta: ")
    if choice == "1":
        LLM = Ollama(model="gemma:3b")
        EMBED_MODEL = OllamaEmbedding(model_name="mxbai-embed-large")
        print("✅ Configurato Ollama locale.")
    elif choice == "2":
        import openai
        openai.api_key = input("🔑 Inserisci la tua API Key OpenAI: ")
        models = openai.Model.list()
        model_ids = [m.id for m in models.data]
        print("Modelli disponibili:")
        for idx, m_id in enumerate(model_ids):
            print(f"{idx+1}. {m_id}")
        model_choice = int(input("Seleziona il modello LLM: ")) - 1
        embed_choice = int(input("Seleziona il modello di embedding: ")) - 1
        LLM = OpenAI(model=model_ids[model_choice])
        EMBED_MODEL = OpenAIEmbedding(model=model_ids[embed_choice])
        print(f"✅ Configurato OpenAI con modelli {model_ids[model_choice]} e {model_ids[embed_choice]}.")
    else:
        print("❌ Scelta non valida.")
        sys.exit()

def get_git_info():
    repo = git.Repo(PROJECT_PATH)
    branch = repo.active_branch.name
    commit = repo.head.commit.hexsha[:7]
    return branch, commit

def update_existing_index(index_name):
    index_path = RAG_DATA_PATH / index_name
    if not index_path.exists():
        print("❌ Index non trovato!")
        return

    # Salva il dump del database
    dump_file_path = PROJECT_PATH / "database_dump.txt"
    if not os.access(PROJECT_PATH, os.W_OK):
        print(f"❌ Permessi insufficienti per scrivere nella cartella {PROJECT_PATH}")
        sys.exit(1)

    # Dump contenuto come stringa
    dump_data = dump_sqlite_database(return_content=True)
    if not dump_data:
        print("❌ Impossibile ottenere il dump del database. Continuo con i soli file del progetto.")
        documents = []
    create_and_write_file(dump_file_path, dump_data)
    print("🔍 Raccolta documenti in corso...")

    # Carica i dati dalla cartella principale
    reader = SimpleDirectoryReader(
        input_dir=str(PROJECT_PATH),
        recursive=True,
        exclude_hidden=True,
        required_exts=[
            ".py", ".cpp", ".c", ".h", ".hpp", ".java", ".js", ".ts",
            ".qml", ".html", ".css", ".json", ".xml", ".txt", ".md", ".sql"
        ]
    )
    documents = reader.load_data()

    # Aggiungi il dump del database ai documenti
    if dump_data:
        db_document = Document(text=dump_data, metadata={"source": str(dump_file_path)})
        documents.append(db_document)
        os.remove(dump_file_path)

    # Correggi i metadati dei documenti
    processed_documents = []
    for doc in documents:
        source = doc.metadata.get("source", "unknown_file")
        if source == "unknown_file":
            tqdm.write(f"⚠️ Documento con metadati 'source' mancanti: {doc.text[:50]}...")
        doc.metadata["source"] = source
        processed_documents.append(doc)

    # Verifica se ci sono documenti da indicizzare
    if not processed_documents:
        print("❌ Nessun documento trovato da indicizzare. Operazione annullata.")
        return None

    print(f"📚 Trovati {len(processed_documents)} documenti da aggiornare nell'indice.")
    print("🔄 Aggiornamento dell'indice in corso...")

    # Carica l'indice esistente
    storage_context = StorageContext.from_defaults(persist_dir=str(index_path))
    index = load_index_from_storage(storage_context, llm=LLM, embed_model=EMBED_MODEL)

    # Aggiorna l'indice con i nuovi documenti
    total_docs = len(processed_documents)
    with tqdm(total=total_docs, desc="Documenti aggiornati", unit="doc") as pbar:
        batch_size = max(1, total_docs // 100)
        for i in range(0, total_docs, batch_size):
            batch_end = min(i + batch_size, total_docs)
            for doc in processed_documents[i:batch_end]:
                index.insert(doc)
            pbar.update(batch_end - i)

    print("💾 Salvataggio dell'indice aggiornato su disco...")
    index.storage_context.persist()
    print(f"✅ Index {index_name} aggiornato con successo.")

def create_new_index():
    try:
        print("\n" + "="*50)
        print("🚀 INIZIO CREAZIONE NUOVO INDICE")
        print("="*50 + "\n")

        # Ottieni informazioni Git
        branch, commit = get_git_info()
        timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
        index_name = f"{branch}_{commit}_{timestamp}"
        print(f"🔖 Nome indice: {index_name}")

        # Crea la directory per l'indice
        index_path = RAG_DATA_PATH / index_name
        os.makedirs(index_path, exist_ok=True)
        print(f"✅ Directory indice creata/verificata: {index_path}")

        # FASE 1: Dump del database
        print("\n=== FASE 1: DUMP DATABASE ===")
        dump_file_path = PROJECT_PATH / "database_dump.txt"
        dump_data = dump_sqlite_database(return_content=True)
        if not dump_data:
            print("⚠️ Nessun dato estratto dal database, continuo senza...")
            db_document = None
        else:
            if not create_and_write_file(dump_file_path, dump_data):
                print("⚠️ Fallita creazione dump del database, continuo senza...")
                db_document = None
            else:
                try:
                    with open(dump_file_path, "r", encoding="utf-8") as f:
                        db_dump_content = f.read()
                        db_document = Document(text=db_dump_content, metadata={"source": str(dump_file_path)})
                        print("✅ Documento database creato")
                except Exception as e:
                    print(f"❌ Errore lettura dump: {e}")
                    db_document = None
                finally:
                    if os.path.exists(dump_file_path):
                        os.remove(dump_file_path)
                        print("✅ File dump temporaneo rimosso")

        # FASE 2: Raccolta documenti
        print("\n🔍 Raccolta documenti in corso...")
        reader = SimpleDirectoryReader(
            input_dir=str(PROJECT_PATH),
            recursive=True,
            exclude_hidden=True,
            required_exts=[
                ".py", ".cpp", ".c", ".h", ".hpp", ".java", ".js", ".ts", ".sh", ".bat",
                ".iss", ".qml", ".html", ".css", ".json", ".xml", ".txt", ".md", ".sql"
            ]
        )
        documents = reader.load_data()

        # Correggi i metadati dei documenti
        processed_documents = []
        for doc in documents:
            source = doc.metadata.get("source", "unknown_file")
            if source == "unknown_file":
                tqdm.write(f"⚠️ Documento con metadati 'source' mancanti: {doc.text[:50]}...")
            doc.metadata["source"] = source
            processed_documents.append(doc)

        if db_document:
            processed_documents.append(db_document)

        # Verifica se ci sono documenti da indicizzare
        if not processed_documents:
            print("❌ Nessun documento trovato da indicizzare. Operazione annullata.")
            shutil.rmtree(index_path)
            return None

        print(f"📚 Trovati {len(processed_documents)} documenti da indicizzare.")

        # FASE 3: Creazione dell'indice
        print("\n🔄 Creazione dell'indice in corso...")
        index = VectorStoreIndex.from_documents(
            processed_documents,
            llm=LLM,
            embed_model=EMBED_MODEL,
            show_progress=True
        )

        # Salvataggio dell'indice su disco
        print("\n💾 Salvataggio dell'indice su disco...")
        index.storage_context.persist(persist_dir=str(index_path))
        print(f"✅ Nuovo indice creato con successo: {index_name}")
        return index_name

    except Exception as e:
        print(f"\n❌ ERRORE CRITICO durante la creazione dell'indice: {e}")
        if 'file_path' in locals():
            print(f"↳ File problematico: {file_path}")
        else:
            print("↳ File problematico: sconosciuto (file_path non definito)")
        return None

# MAIN MENU
def main():
    select_models()
    while True:
        print("""
        ==== RAG MANAGER ====
        1. Crea nuovo document index
        2. Aggiorna un document index esistente
        3. Esegui query RAG
        4. Modalità Codex + RAG
        5. Esci
        """)
        choice = input("Scegli un'opzione: ")
        if choice == "1":
            create_new_index()
            input("Premi Invio per tornare al menu principale...")
        elif choice == "2":
            indices = list_indices()
            if not indices:
                print("Nessun indice trovato.")
                continue
            print("Indici disponibili:")
            for i, idx in enumerate(indices):
                print(f"{i+1}. {idx}")
            sel = int(input("Seleziona l'indice da aggiornare: ")) - 1
            if 0 <= sel < len(indices):
                update_existing_index(indices[sel])
                input("Premi Invio per tornare al menu principale...")
            else:
                print("❌ Selezione non valida.")
        elif choice == "3":
            indices = list_indices()
            if not indices:
                print("Nessun indice trovato.")
                continue
            print("Indici disponibili:")
            for i, idx in enumerate(indices):
                print(f"{i+1}. {idx}")
            sel = int(input("Seleziona l'indice su cui interrogare: ")) - 1
            if 0 <= sel < len(indices):
                query_index(indices[sel])
                input("Premi Invio per tornare al menu principale...")
            else:
                print("❌ Selezione non valida.")
        elif choice == "4":
            indices = list_indices()
            if not indices:
                print("Nessun indice trovato.")
                continue
            print("Indici disponibili:")
            for i, idx in enumerate(indices):
                print(f"{i+1}. {idx}")
            sel = int(input("Seleziona l'indice da usare per Codex: ")) - 1
            if 0 <= sel < len(indices):
                codex_mode(indices[sel])
                input("Premi Invio per tornare al menu principale...")
            else:
                print("❌ Selezione non valida.")
        elif choice == "5":
            print("👋 Uscita.")
            sys.exit()
        else:
            print("❌ Scelta non valida.")

if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "--no-select-models":
        main()
    else:
        main()