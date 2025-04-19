#!/bin/bash

# Imposta la directory di lavoro alla cartella dello script
cd "$(dirname "$0")" || exit 1  # Cambia alla directory dello script

# Funzione per aggiornare il percorso relativo a percorso assoluto
get_absolute_path() {
  echo "$(realpath "$1")"
}

# Definizione delle variabili per i percorsi relativi
QT_DIR="../../../Qt"
PRO_FILE="../odla.pro"
BUILD_DIR="build"
DEPLOY_DIR="../../Binari/MAC/ODLA"


# Pulizia ambiente
echo "Pulizia progetto..."
rm -rf "$BUILD_DIR"
mkdir "$BUILD_DIR"

# Aggiorna i percorsi relativi a percorsi assoluti
QT_DIR=$(get_absolute_path "$QT_DIR")
PRO_FILE=$(get_absolute_path "$PRO_FILE")
BUILD_DIR=$(get_absolute_path "$BUILD_DIR")
DEPLOY_DIR=$(get_absolute_path "$DEPLOY_DIR")

echo "Percorsi assoluti impostati:"
echo "QT_DIR: $QT_DIR"
echo "PRO_FILE: $PRO_FILE"
echo "BUILD_DIR: $BUILD_DIR"
echo "DEPLOY_DIR: $DEPLOY_DIR"

# Seleziona la versione di Qt
QT_VERSIONS=($(find "$QT_DIR" -maxdepth 1 -type d -name "6.*"))
if [ ${#QT_VERSIONS[@]} -eq 0 ]; then
  echo "Nessuna versione di Qt trovata in $QT_DIR."
  exit 1
fi

echo "Versioni Qt trovate:"
select QT_SELECTED in "${QT_VERSIONS[@]}"; do
  if [ -n "$QT_SELECTED" ]; then
    break
  fi
  echo "Scelta non valida. Riprova."
done

# Aggiorna il percorso relativo della versione di Qt selezionata a percorso assoluto
QT_SELECTED=$(get_absolute_path "$QT_SELECTED")
echo "Versione di Qt selezionata: $QT_SELECTED"

# Seleziona il compilatore
COMPILERS=($(find "$QT_SELECTED" -maxdepth 1 -type d ! -name "$(basename "$QT_SELECTED")"))
echo "Compilatori trovati:"
select COMPILER_DIR in "${COMPILERS[@]}"; do
  if [ -n "$COMPILER_DIR" ]; then
    break
  fi
  echo "Scelta non valida. Riprova."
done

# Aggiorna il percorso del compilatore a percorso assoluto
COMPILER_DIR=$(get_absolute_path "$COMPILER_DIR")
echo "Compilatore selezionato: $COMPILER_DIR"

QT_PATH="$COMPILER_DIR"
QMAKE="$QT_PATH/bin/qmake"
MACDEPLOYQT="$QT_PATH/bin/macdeployqt6"
MAKE="make"

# Aggiorna il percorso di QMAKE e MACDEPLOYQT a percorso assoluto
QMAKE=$(get_absolute_path "$QMAKE")
MACDEPLOYQT=$(get_absolute_path "$MACDEPLOYQT")
echo "Usando qmake da: $QMAKE"
echo "Usando macdeployqt da: $MACDEPLOYQT"

# Verifica esistenza file progetto
if [ ! -f "$PRO_FILE" ]; then
  echo "ERRORE CRITICO: File progetto non trovato"
  echo "Percorso cercato: $PRO_FILE"
  exit 1
fi

echo "Trovato progetto in: $PRO_FILE"

# Aggiorna i percorsi di BUILD_DIR e a percorsi assoluti
BUILD_DIR=$(get_absolute_path "$BUILD_DIR")
echo "Build directory: $BUILD_DIR"

# Build progetto
echo "Eseguo qmake..."
cd "$BUILD_DIR" || exit 1
"$QMAKE" "$PRO_FILE" -spec macx-clang CONFIG+=qtquickcompiler
if [ $? -ne 0 ]; then
  echo "ERRORE qmake"
  exit 1
fi

echo "Compilazione progetto..."
$MAKE -j12
if [ $? -ne 0 ]; then
  echo "ERRORE compilazione"
  exit 1
fi
cd ..

# Trova Odla.app
APP_CANDIDATES=( $(find "$BUILD_DIR" -type d -name "Odla.app" -print0 | xargs -0 ls -td) )
if [ ${#APP_CANDIDATES[@]} -eq 0 ]; then
  echo "Nessuna applicazione Odla.app trovata."
  exit 1
fi

echo "Seleziona l'applicazione da usare:"
select APP_PATH in "${APP_CANDIDATES[@]}"; do
  if [ -n "$APP_PATH" ]; then
    break
  fi
  echo "Scelta non valida. Riprova."
done

# Aggiorna il percorso dell'applicazione a percorso assoluto
APP_PATH=$(get_absolute_path "$APP_PATH")
echo "Applicazione selezionata: $APP_PATH"

# Esegui il deploy dell'app
"$MACDEPLOYQT" "$APP_PATH"

# Firma digitale dell'eseguibile
CODESIGN_ID="Developer ID Application: KEMONIA RIVER S.R.L. (SWB463YGY7)"
ENTITLEMENTS="entitlements.plist"
codesign --force --options runtime --deep --sign "$CODESIGN_ID" --entitlements "$ENTITLEMENTS" "$APP_PATH"

# Ottieni la versione dal tag git
VERSION=$(git -C ../ describe --tags --abbrev=0)

# Crea il DMG
create-dmg \
  --volname "ODLA Installer $VERSION" \
  --volicon "installer.icns" \
  --window-pos 200 120 \
  --window-size 660 420 \
  --icon-size 170 \
  --icon "Odla.app" 170 170 \
  --hide-extension "Odla.app" \
  --app-drop-link 490 170 \
  --background "background.png" \
  "ODLA $VERSION.dmg" \
  "$APP_PATH"

echo "BUILD COMPLETATO CON SUCCESSO!"
