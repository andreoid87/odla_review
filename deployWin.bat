set PATH=C:\Qt\5.15.2\mingw81_64\bin;C:/Qt/Tools/mingw810_64\bin;%PATH%
cd C:\Users\accor\Desktop\Lavoro\ODLA
xcopy .\build.qtc\odla\release\Odla.exe ..\Binari\WIN\ODLA\ /K /D /H /Y
C:\Qt\5.15.2\mingw81_64\bin\windeployqt.exe --compiler-runtime ..\Binari\WIN\ODLA\Odla.exe