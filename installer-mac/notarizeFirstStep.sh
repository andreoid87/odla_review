#!/bin/bash

GIT_VERSION=$(git describe --tag --abbrev=0)
echo "ODLA version detected: $GIT_VERSION"

QIFW_PATH=$(grealpath ~/Qt/Tools/QtInstallerFramework/*/bin)
echo "QT Installer framework path: $QIFW_PATH"

OUTPUT_PATH=$(grealpath ./ODLA_Installer-$GIT_VERSION)
echo "Output path: $OUTPUT_PATH"

PACKAGE_NAME=$(echo 'ODLA_Installer-'$GIT_VERSION.app)
echo "Package name: $PACKAGE_NAME"

DMG_FILE=$OUTPUT_PATH.dmg
echo "Upload file for notarization..."
xcrun altool -t osx -f $DMG_FILE --primary-bundle-id org.odlamusic.odla --notarize-app --username accordinoandrea12@gmail.com
