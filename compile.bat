set VERSION=0.1
set TID=01007EF00011E000
set BID=8E9978D50BDD20B4
set RELEASE_PATH=release\atmosphere\contents\%TID%

make -C build -f ..\Makefile || exit /b
rd /s /q release || exit /b
md %RELEASE_PATH%\exefs || exit /b
md %RELEASE_PATH%\romfs\System || exit /b
md %RELEASE_PATH%\cheats || exit /b
echo 1.6.0-LC%VERSION% > %RELEASE_PATH%\romfs\System\Version.txt || exit /b
copy build\linkcannon.nso %RELEASE_PATH%\exefs\subsdk9 || exit /b
copy build\app.npdm %RELEASE_PATH%\exefs\main.npdm || exit /b
robocopy romfs %RELEASE_PATH%\romfs /e /mt
copy cheat.txt %RELEASE_PATH%\cheats\%BID%.txt || exit /b

@echo off
echo. || exit /b
echo Build complete. To install, copy the contents of the release folder onto the || exit /b
echo root of your microSD card. || exit /b
pause || exit /b
