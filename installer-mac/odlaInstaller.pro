# Get system version
GIT_V = $$system(git --git-dir "$$PWD/../.git" --work-tree $$PWD/ describe --always --tags)
message("git folder: $$PWD/../.git")
GIT_V ~= s/-/"."
GIT_V_PARTS = $$split(GIT_V, ".")
GIT_VERSION = $$member(GIT_V_PARTS, 0).$$member(GIT_V_PARTS, 1).$$member(GIT_V_PARTS, 2)
message("Version detected of ODLA: "$$GIT_VERSION)

# AUX template (this file contains command but nothing to build)
# more information at https://doc.qt.io/qt-5/qmake-variable-reference.html#template
TEMPLATE = aux

# Giving the path of Qt Installer framework (binarycreator.exe)
QIFW_PATH = $$(QTDIR)/../../Tools/QtInstallerFramework/*/bin
message(Path of QtInstallerFramework $$QIFW_PATH <- CHANGE IT WHEN UPDATE!)

# This variables contain path and filename of installers (and repository?)
INSTALLER_OFFLINE = $$OUT_PWD/ODLA_Installer-$$GIT_VERSION/ODLA_Installer-$$GIT_VERSION
#INSTALLER_ONLINE =  $$OUT_PWD/OnlineInstaller
#REPO_PATH =         $$OUT_PWD/repository

# Create the target assembly Offline Installer
#offlineInstaller.input = $$PWD/config/config.xml $$PWD/packages
#offlineInstaller.output = $${INSTALLER_OFFLINE} #why with $$?
#message(Output of qmake >> $${INSTALLER_OFFLINE})
offlineInstaller.commands = $$QIFW_PATH/binarycreator --offline-only -c $$PWD/config/config.xml -p $$PWD/packages $${INSTALLER_OFFLINE}.app
#offlineInstaller.CONFIG += target_predeps no_link combine
message(installer command: >> $${offlineInstaller.commands})
QMAKE_EXTRA_TARGETS += offlineInstaller

sign.commands = codesign --deep --force -s "\"Apple Development: Andrea Accordino (K96W2662M8)\"" $${INSTALLER_OFFLINE}.app
message(sign command: >> $${sign.commands})
QMAKE_EXTRA_TARGETS += sign

createDMG.commands = hdiutil create -fs HFS+ -srcfolder $$OUT_PWD/ODLA_Installer-$$GIT_VERSION -volname ODLA_Installer-$$GIT_VERSION $${OUT_PWD}/ODLA_Installer-$${GIT_VERSION}.dmg
message(DMG command: >> $${createDMG.commands})
QMAKE_EXTRA_TARGETS += createDMG

# Create the target assembly Online Installer
#INPUT = $$PWD/config/config.xml $$PWD/packages
#onlineInstaller.input = INPUT
#onlineInstaller.output = $$INSTALLER_ONLINE
#onlineInstaller.commands = $$QIFW_PATH/binarycreator --online-only -c $$PWD/config/config.xml -p $$PWD/packages ${QMAKE_FILE_OUT}
#onlineInstaller.CONFIG += target_predeps no_link combine

#QMAKE_EXTRA_COMPILERS += onlineInstaller

#QMAKE_POST_LINK += $$QIFW_PATH/repogen -p $$PWD/packages --update $$REPO_PATH #-i it.kemoniariver.installer.odla

SOURCES += \
    config/controlscript.qs \
    packages/it.kemoniariver.musescore/meta/installscript.qs \
    packages/it.kemoniariver.musescore/meta/package.xml \
    packages/it.kemoniariver.odla/meta/installscript.qs \
    packages/it.kemoniariver.odla/meta/package.xml

TRANSLATIONS += \
    packages/it.kemoniariver.odla/meta/en.ts \
    packages/it.kemoniariver.odla/meta/it.ts

## TODO: sign odla and musescore app bundle too, see command below, WARNING: ENTITLEMENTS FILE. PLIST!!!
## TODO: edit codesign: codesign --force --options runtime --deep --sign "Developer ID Application: KEMONIA RIVER S.R.L. (SWB463YGY7)" --entitlements entitlements.plist ODLA_Installer-1.5.0.app
## TODO: notarization: xcrun altool -t osx -f ODLA_Installer-1.5.0.dmg --primary-bundle-id org.odlamusic.odla --notarize-app --username accordinoandrea12@gmail.com
## FOR notarization use app-specific password "hlti-danl-gyro-ixqb" (or use new one?)

