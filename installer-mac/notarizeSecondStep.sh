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
VOLUME_NAME="ODLA_Installer-"$GIT_VERSION
echo "Stapling app... $OUTPUT_PATH/$PACKAGE_NAME"
xcrun stapler staple $OUTPUT_PATH/$PACKAGE_NAME
xcrun stapler validate $OUTPUT_PATH/$PACKAGE_NAME


rm $DMG_FILE

echo "Create notarized dmg"
hdiutil create -fs HFS+ -srcfolder $OUTPUT_PATH/$PACKAGE_NAME -volname $VOLUME_NAME $DMG_FILE

