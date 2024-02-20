/*
module.cpp - C++ module configuration file.
Copyright (C) 2023  Makonede

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/


#define MODULE_NAME "Link Cannon"
constexpr auto MODULE_NAME_SIZE = sizeof MODULE_NAME;

[[gnu::section(".bss")]]
char __nx_module_runtime[0xd0uz];

struct ModuleName {
  [[maybe_unused]] int unused;
  int nameSize;
  char name[MODULE_NAME_SIZE];
};

[[gnu::section(".nx-module-name")]]
constexpr ModuleName moduleName{
  .nameSize = static_cast<int>(MODULE_NAME_SIZE), .name = MODULE_NAME
};
