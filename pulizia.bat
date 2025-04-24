@echo off
setlocal enabledelayedexpansion

:: Definisci la cartella src
set "SRC_DIR=src"

:: Crea un elenco temporaneo dei file da mantenere
set "FILES_TO_KEEP="
for %%F in (
    app.cpp appcustom.cpp appdorico.cpp appmusescore3.cpp appmusescore4.cpp button.cpp buttoncommand.cpp buttoninsertion.cpp buttonnumber.cpp buttontoggle.cpp database.cpp keyboard.cpp main.cpp menu.cpp menuinsertion.cpp menustandard.cpp menuvcenter.cpp metadata.cpp panel.cpp pthread_barrier.c voiceover.cpp
) do (
    set "FILES_TO_KEEP=!FILES_TO_KEEP! %%F"
)

:: Aggiungi anche i file header (.h)
for %%F in (
    app.h appcustom.h appdorico.h appmusescore3.h appmusescore4.h button.h buttoncommand.h buttoninsertion.h buttonnumber.h buttontoggle.h database.h keyboard.h menu.h menuinsertion.h menustandard.h menuvcenter.h metadata.h panel.h pthread_barrier.h hidapi.h voiceover.h
) do (
    set "FILES_TO_KEEP=!FILES_TO_KEEP! %%F"
)

:: Cancella tutti i file nella cartella src tranne quelli nell'elenco
pushd "%SRC_DIR%"
for %%F in (*) do (
    set "KEEP_FILE=0"
    for %%K in (%FILES_TO_KEEP%) do (
        if /i "%%F"=="%%K" set "KEEP_FILE=1"
    )
    if "!KEEP_FILE!"=="0" (
        echo Deleting: %%F
        del "%%F"
    ) else (
        echo Keeping: %%F
    )
)
popd

echo Done.
pause