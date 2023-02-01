rem  compile.bat - Autobuild batch file for Windows.
rem  Copyright (C) 2023  Makonede
rem 
rem  This program is free software: you can redistribute it and/or modify
rem  it under the terms of the GNU General Public License as published by
rem  the Free Software Foundation, either version 3 of the License, or
rem  (at your option) any later version.
rem 
rem  This program is distributed in the hope that it will be useful,
rem  but WITHOUT ANY WARRANTY; without even the implied warranty of
rem  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
rem  GNU General Public License for more details.
rem 
rem  You should have received a copy of the GNU General Public License
rem  along with this program.  If not, see <https://www.gnu.org/licenses/>.

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
