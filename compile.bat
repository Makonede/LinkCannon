@echo off

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

echo on


set TID=01007EF00011E000
set RELEASE_PATH=release\atmosphere\contents\%TID%

py -m pip install -U pip || exit /b
py -m pip install -U setuptools || exit /b
py -m pip install -U wheel || exit /b
py -m pip install -Ur tools\botw-link\requirements.txt || exit /b
py -m pip install -U python-ips || exit /b
py -3 tools\botw-link config.toml -cvV 160 || exit /b
make -j8 -C build -f ..\Makefile || exit /b
rd /s /q release || exit /b
rd /s /q patched || exit /b
md %RELEASE_PATH%\exefs || exit /b
md %RELEASE_PATH%\romfs || exit /b
copy build\linkcannon.nso %RELEASE_PATH%\exefs\subsdk9 || exit /b
copy build\app.npdm %RELEASE_PATH%\exefs\main.npdm || exit /b
robocopy romfs %RELEASE_PATH%\romfs /e /mt || if errorlevel 8 exit /b else cmd /c "exit /b 0"
py -3 patch.py || exit /b
robocopy patched %RELEASE_PATH%\romfs /e /mt || if errorlevel 8 exit /b else cmd /c "exit /b 0"

@echo off
echo. || exit /b
echo Build complete. To install, copy the contents of the release folder onto the || exit /b
echo root of your microSD card. || exit /b
pause || exit /b
