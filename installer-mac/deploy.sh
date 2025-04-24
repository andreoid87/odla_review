#!/bin/bash

# Set the working directory to the script's folder
cd "$(dirname "$0")" || exit 1

# Define the path to the Qt folder
QT_FOLDER=$(find ~ -type d -name "Qt" -maxdepth 2 2>/dev/null | head -n 1)

# Check if folder was found
if [[ -z "$QT_FOLDER" ]]; then
    echo "Errore: Directory Qt non trovata. Assicurati che Qt sia installato."
    exit 1
fi

echo "Directory Qt trovata in: $QT_FOLDER"

# Find all Qt version directories matching the pattern X.Y.Z
QT_VERSIONS=()
while IFS= read -r -d '' dir; do
    if [[ "$(basename "$dir")" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
        QT_VERSIONS+=("$dir")
    fi
done < <(find "$QT_FOLDER" -maxdepth 1 -type d -print0)

# Check if there are any Qt versions found
if [ ${#QT_VERSIONS[@]} -eq 0 ]; then
    echo "No Qt versions found in $QT_FOLDER."
    exit 1
fi

# Display available Qt versions to the user
echo "Available Qt versions:"
for i in "${!QT_VERSIONS[@]}"; do
    echo "$((i + 1)): ${QT_VERSIONS[i]}"
done

# Ask user for the Qt version choice
read -rp "Select a Qt version (1-${#QT_VERSIONS[@]}): " VERSION_CHOICE

# Validate user choice
if [[ "$VERSION_CHOICE" -lt 1 || "$VERSION_CHOICE" -gt "${#QT_VERSIONS[@]}" ]]; then
    echo "Invalid choice. Exiting."
    exit 1
fi

# Get the selected Qt version path
SELECTED_QT="${QT_VERSIONS[$((VERSION_CHOICE - 1))]}"

# Ask user for the deployment tool choice
echo "Which deployment tool would you like to use? (1 for macdeployqt, 2 for macdeployqt6):"
read -r TOOL_CHOICE

# Set the deployment tool path based on user choice
if [[ "$TOOL_CHOICE" == "1" ]]; then
    QT_DEPLOY="${SELECTED_QT}/*/bin/macdeployqt"
elif [[ "$TOOL_CHOICE" == "2" ]]; then
    QT_DEPLOY="${SELECTED_QT}/*/bin/macdeployqt6"
else
    echo "Invalid choice. Exiting."
    exit 1
fi

APP_PATH="../build/Odla.app"
ICON_PATH="$APP_PATH/Contents/Resources/odla.icns"
BACKGROUND_PATH="background.png"

# Execute the app deployment
echo "Executing: $QT_DEPLOY \"$APP_PATH\""
# $QT_DEPLOY "$APP_PATH"

# Check for error of macdeployqt
if [[ $? -ne 0 ]]; then
    echo "Error: $QT_DEPLOY \"$APP_PATH\" failed."
    exit 1
fi

echo "Signing .app and library files..."
read -rp "Enter your Team ID: " TEAM_ID
SIGN_ID="Developer ID Application: KEMONIA RIVER S.R.L. ($TEAM_ID)"

# Recursive signign
sign_recursive() {
    local path="$1"
    codesign --force --verify --verbose --sign "$SIGN_ID" "$path" --options runtime
    if [ $? -ne 0 ]; then
        echo "Errore nella firma di: $path"
    fi
}

# Firma dei framework
for framework in "$APP_PATH/Contents/Frameworks/"*; do
    sign_recursive "$framework"
done

# Firma dei plug-in
for plugin in "$APP_PATH/Contents/PlugIns/"*; do
    sign_recursive "$plugin"
done

# Firma dell'app principale
sign_recursive "$APP_PATH"

# Get the version from the latest git tag
VERSION=$(git -C ../ describe --tags --abbrev=0)
DMG_NAME="ODLA_${VERSION}"

# Create the DMG
create-dmg \
  --volname $DMG_NAME \
  --volicon "$ICON_PATH" \
  --window-pos 200 120 \
  --window-size 660 420 \
  --icon-size 170 \
  --icon "Odla.app" 170 170 \
  --hide-extension "Odla.app" \
  --app-drop-link 490 170 \
  --background "$BACKGROUND_PATH" \
  "$DMG_NAME.dmg" \
  "$APP_PATH"

# Chiedi all'utente se vuole procedere con la notarizzazione
read -rp "Would you like to notarize this app? (y/n): " NOTARIZE_CHOICE

# Controlla la scelta dell'utente
if [[ "$NOTARIZE_CHOICE" == "y" ]]; then
    echo "Notarizing..."
    # Inserisci qui il comando di notarizzazione
elif [[ "$NOTARIZE_CHOICE" == "n" ]]; then
    echo "GoodBye!"
    exit 1
else
    echo "Scelta non valida. Esco."
    exit 1
fi

# Sign the DMG file
echo "Signing DMG..."
codesign -vvv --deep --strict --sign "Developer ID Application: KEMONIA RIVER S.R.L. ($TEAM_ID)" --entitlements "entitlements.plist" "ODLA_${VERSION}.dmg"

# Ask for Apple credentials
read -rp "Enter your app-specific password: " APP_SPECIFIC_PASSWORD

# Notarize the DMG
echo "Uploading file for notarization..."
xcrun notarytool submit "$DMG_NAME.dmg" --apple-id accordinoandrea12@gmail.com --password "$APP_SPECIFIC_PASSWORD" --team-id "$TEAM_ID" --wait

# Ask user to confirm before proceeding to staple
echo "Wait for the notarization email confirmation. Press 1 to continue after confirmation."
read -rp "Enter 1 to continue: " CONTINUE
if [[ "$CONTINUE" != "1" ]]; then
    echo "Exiting without stapling."
    exit 1
fi

# Staple the notarized application
TEMP_FOLDER=$(mktemp -d)
cp -R "$APP_PATH" "$TEMP_FOLDER/"
xcrun stapler staple "$TEMP_FOLDER/Odla.app"
xcrun stapler validate "$TEMP_FOLDER/Odla.app"


DMG_NAME_NOTARIZED="ODLA_${VERSION}_notarized"

# Create the final notarized DMG
create-dmg \
  --volname $DMG_NAME \
  --volicon "$ICON_PATH" \
  --window-pos 200 120 \
  --window-size 660 420 \
  --icon-size 170 \
  --icon "Odla.app" 170 170 \
  --hide-extension "Odla.app" \
  --app-drop-link 490 170 \
  --background "$BACKGROUND_PATH" \
  "$DMG_NAME_NOTARIZED.dmg" \
  "$TEMP_FOLDER/Odla.app"

# Clean up temporary folder
rm -rf "$TEMP_FOLDER"
echo "Done! Notarized DMG created: $DMG_NAME_NOTARIZED.dmg"
