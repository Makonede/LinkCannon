# setup_build.ps1 - Build setup PowerShell script for Windows.
# Copyright (C) 2023  Makonede
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

$PSNativeCommandUseErrorActionPreference = $true
$ErrorActionPreference = 'Stop'


New-Item dump\Pack -ItemType Directory -Force
New-Item dump\System\Resource -ItemType Directory -Force

if (Test-Path gcm.cache) { Remove-Item gcm.cache\* -Recurse -Force }

try {
    & "$Env:DEVKITPRO\devkitA64\bin\aarch64-none-elf-g++" -c -fmodules-ts -x `
        c++-system-header -std=gnu++2b algorithm any array atomic barrier bit `
        bitset cassert cctype cerrno cfenv cfloat charconv chrono cinttypes `
        climits clocale cmath compare complex concepts condition_variable `
        coroutine csetjmp csignal cstdarg cstddef cstdint cstdio cstdlib `
        cstring ctime cuchar cwchar cwctype deque exception execution expected `
        filesystem flat_map flat_set format forward_list fstream functional `
        future generator hazard_pointer initializer_list iomanip ios iosfwd `
        iostream istream iterator latch limits list locale map mdspan memory `
        memory_resource mutex new numbers numeric optional ostream print queue `
        random ranges ratio rcu regex scoped_allocator semaphore set `
        shared_mutex source_location span spanstream sstream stack stacktrace `
        stdexcept stdfloat stop_token streambuf string string_view syncstream `
        system_error text_encoding thread tuple type_traits typeindex typeinfo `
        unordered_map unordered_set utility valarray variant vector version
}
catch [System.SystemException] {}

if (!(Test-Path .venv)) { py -3 -m venv .venv }
.venv\Scripts\Activate.ps1
py -3 -m pip install -U pip
py -3 -m pip install -U wheel
py -3 -m pip install -U setuptools
py -3 -m pip install -Ur tools\botw-link\requirements.txt
py -3 -m pip install -U vidua
