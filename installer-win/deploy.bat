@echo off
setlocal enabledelayedexpansion

:: Configurazione iniziale
set QT_DIR=D:\Qt
set PRO_FILE=odla.pro
set BUILD_DIR=build
set BIN_DIR=bin
set DEPLOY_DIR=..\..\Binari\WIN\ODLA
set INSTALLER_SCRIPT=odla.iss
set INDEX=0
set "TOOLS_DIR=%QT_DIR%\Tools"
set "MINGW_PATHS="
set /A index=0

:: Cerca tutte le cartelle MinGW nella directory Qt\Tools
for /D %%D in ("%TOOLS_DIR%\mingw*") do (
    set /A index+=1
    set "MINGW_PATHS[!index!]=%%D"
    echo !index! - %%D
)

if %index%==0 (
    echo Nessuna installazione di MinGW trovata in %TOOLS_DIR%.
    pause
    exit /b
)

:: Chiede all'utente di scegliere la versione
set /P CHOICE="Seleziona il numero della toolchain MinGW da usare: "
if not defined MINGW_PATHS[%CHOICE%] (
    echo Scelta non valida.
    exit /b
)

set "MINGW_SELECTED=!MINGW_PATHS[%CHOICE%]!"
echo Hai scelto: %MINGW_SELECTED%

:: Imposta il PATH
set "PATH=%MINGW_SELECTED%\bin;%PATH%"
echo PATH impostato su %MINGW_SELECTED%\bin


:: Verifica esistenza file progetto
set "PRO_FILE=..\odla.pro"

:: Verifica esistenza
if not exist "%PRO_FILE%" (
    echo ERRORE CRITICO: File progetto non trovato
    echo Percorso cercato: %CD%\%PRO_FILE%
    pause
    exit /b 1
)

:: Mostra percorso completo normalizzato
for %%F in ("%PRO_FILE%") do (
    echo Trovato progetto in: %%~fF
    set "PRO_FILE=%%~fF"
)

echo Ricerca versioni di Qt in %QT_DIR%...

:: Ricerca versioni Qt (formato X.Y.Z)
for /d %%v in ("%QT_DIR%\*") do (
    set "FOLDER_NAME=%%~nxv"

    for /f "tokens=1-3 delims=." %%a in ("!FOLDER_NAME!") do (
        if "%%a" neq "" if "%%b" neq "" (
            set /a INDEX+=1
            set "QT_VERSION_!INDEX!=!FOLDER_NAME!"
        )
    )
)

if %INDEX%==0 (
    echo ERRORE: Nessuna versione Qt trovata in %QT_DIR%.
    pause
    exit /b 1
)

echo.
echo Versioni trovate:
for /L %%i in (1,1,%INDEX%) do echo %%i - Qt !QT_VERSION_%%i!

:: Selezione versione Qt
echo.
:select_version
set /p SELECT="Seleziona il numero corrispondente alla versione di Qt: "
if not defined QT_VERSION_%SELECT% (
    echo Errore: selezione non valida.
    goto select_version
)
set "QT_SELECTED=!QT_VERSION_%SELECT%!"
echo Hai selezionato Qt %QT_SELECTED%.

:: Ricerca compilatori
set COMPILER_INDEX=0
echo Ricerca compilatori in %QT_DIR%\%QT_SELECTED%...
for /d %%c in ("%QT_DIR%\%QT_SELECTED%\*") do (
    if exist "%%c\bin\qmake.exe" (
        set /a COMPILER_INDEX+=1
        set "COMPILER_!COMPILER_INDEX!=%%~nxc"
        echo Trovato compilatore: %%~nxc
    )
)

if %COMPILER_INDEX%==0 (
    echo ERRORE: Nessun compilatore trovato per Qt %QT_SELECTED%.
    pause
    exit /b 1
)

:: Selezione compilatore
echo.
echo Seleziona un compilatore:
for /L %%i in (1,1,%COMPILER_INDEX%) do echo %%i - !COMPILER_%%i!

:select_compiler
set /p COMPILER_SELECTION=Scegli il compilatore (numero): 
if not defined COMPILER_%COMPILER_SELECTION% (
    echo ERRORE: Selezione non valida.
    goto select_compiler
)
set "COMPILER_DIR=!COMPILER_%COMPILER_SELECTION%!"
echo Hai selezionato il compilatore: %COMPILER_DIR%

:: Configurazione percorsi toolchain
set QT_PATH=%QT_DIR%\%QT_SELECTED%\%COMPILER_DIR%
set QMAKE="%QT_PATH%\bin\qmake.exe"
set WINDEPLOYQT="%QT_PATH%\bin\windeployqt6.exe"

:: Ricerca mingw32-make (priorità: Tools > Qt install)
set MAKE=
for /d %%t in ("%QT_DIR%\Tools\mingw*") do (
    if exist "%%t\bin\mingw32-make.exe" (
        set MAKE="%%t\bin\mingw32-make.exe"
    )
)

if not defined MAKE (
    if exist "%QT_PATH%\bin\mingw32-make.exe" (
        set MAKE="%QT_PATH%\bin\mingw32-make.exe"
    )
)

if not defined MAKE (
    echo ERRORE: mingw32-make.exe non trovato!
    pause
    exit /b 1
)

:: Configurazione spec
if "%COMPILER_DIR:~0,5%"=="mingw" (
    set COMPILER_SPEC=win32-g++
) else if "%COMPILER_DIR:~0,3%"=="msc" (
    set COMPILER_SPEC=win32-msvc
    set MAKE="nmake.exe"
)

echo.
echo --- CONFIGURAZIONE TOOLCHAIN ---
echo Qt Path:    %QT_PATH%
echo QMake:      %QMAKE%
echo Make:       %MAKE%
echo Windeploy:  %WINDEPLOYQT%
echo Compiler:   %COMPILER_SPEC%
echo.

:: Pulizia ambiente
echo Pulizia progetto...
if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
if exist "%BIN_DIR%" rmdir /s /q "%BIN_DIR%"
mkdir "%BUILD_DIR%"
mkdir "%BIN_DIR%"

:: Build progetto
echo Eseguo qmake...
cd "%BUILD_DIR%"
%QMAKE% "%PRO_FILE%" -spec %COMPILER_SPEC% "CONFIG+=qtquickcompiler" QMAKE_LFLAGS+=" -Wl,-subsystem,windows"
if %errorlevel% neq 0 (
    echo ERRORE qmake
    pause
    exit /b 1
)

echo Compilazione progetto...
%MAKE% -j12
if %errorlevel% neq 0 (
    echo ERRORE compilazione
    pause
    exit /b 1
)
cd ..

:: Ricerca eseguibile
set EXE_FOUND=0
for /r "%BUILD_DIR%" %%f in (*.exe) do (
    if /i "%%~nxf"=="odla.exe" (
        set "EXE_PATH=%%f"
        set EXE_FOUND=1
        goto :EXE_FOUND
    )
)

:EXE_FOUND
if %EXE_FOUND% equ 0 (
    echo ERRORE: Nessun eseguibile odla.exe trovato
    pause
    exit /b 1
)

echo Copio %EXE_PATH% in %BIN_DIR%
copy "%EXE_PATH%" "%BIN_DIR%\"
if %errorlevel% neq 0 (
    echo ERRORE durante copia eseguibile
    pause
    exit /b 1
)

:: Deployment
echo Eseguo windeployqt...
%WINDEPLOYQT% "%BIN_DIR%\odla.exe" --release --qmldir "..\qml" --no-translations
if %errorlevel% neq 0 (
    echo ERRORE windeployqt
    pause
    exit /b 1
)

echo Copia file in %DEPLOY_DIR%...
if not exist "%DEPLOY_DIR%" mkdir "%DEPLOY_DIR%"
xcopy /E /I /Y "%BIN_DIR%\*" "%DEPLOY_DIR%\"
if %errorlevel% neq 0 (
    echo ERRORE durante copia deployment
    pause
    exit /b 1
)

:: Build installer
if exist "%INSTALLER_SCRIPT%" (
    echo Compilazione installer...
    "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" "%INSTALLER_SCRIPT%"
    if %errorlevel% neq 0 (
        echo ERRORE compilazione installer
        pause
        exit /b 1
    )
)

echo.
echo BUILD COMPLETATO CON SUCCESSO!
pause