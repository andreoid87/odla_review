#!/bin/bash

# Verifica che il file di input esista
if [ ! -f "$1" ]; then
    echo "Errore: il file $1 non esiste."
    exit 1
fi

# Crea la cartella Icon.iconset
ICONSET_DIR="Icon.iconset"
mkdir -p "$ICONSET_DIR"

# Dimensioni delle icone
declare -A sizes
sizes=( ["16"]=16 ["32"]=32 ["128"]=128 ["256"]=256 ["512"]=512 )

# Crea le versioni dell'icona in diverse dimensioni
for size in "${!sizes[@]}"; do
    # Dimensione normale
    sips -z ${sizes[$size]} ${sizes[$size]} "$1" --out "$ICONSET_DIR/icon_${size}x${size}.png"
    
    # Versione @2x (risoluzione doppia)
    if [ "$size" -ne 16 ]; then
        sips -z $((${sizes[$size]} * 2)) $((${sizes[$size]} * 2)) "$1" --out "$ICONSET_DIR/icon_${size}x${size}@2x.png"
    fi
done

# Usa iconutil per creare il file .icns
iconutil -c icns "$ICONSET_DIR"

# Rimuove la cartella temporanea
rm -r "$ICONSET_DIR"

echo "File ICNS creato con successo: Icon.icns"
