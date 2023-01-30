set VERSION=0.1
set TID=01007EF00011E000
set BID=8E9978D50BDD20B4
set RELEASE_PATH=release\atmosphere\contents\%TID%

make -C build -f ..\Makefile
rd /s /q release
md %RELEASE_PATH%\exefs
md %RELEASE_PATH%\romfs\System
md %RELEASE_PATH%\cheats
echo 1.6.0-LC%VERSION% > %RELEASE_PATH%\romfs\System\Version.txt
copy build\linkcannon.nso %RELEASE_PATH%\exefs\subsdk9
copy build\app.npdm %RELEASE_PATH%\exefs\main.npdm
robocopy romfs %RELEASE_PATH%\romfs /e /mt
copy cheat.txt %RELEASE_PATH%\cheats\%BID%.txt

@echo off
echo.
echo Build complete. To install, copy the contents of the release folder onto the
echo root of your microSD card.
pause
