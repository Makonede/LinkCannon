/*
crt0.s - Crt0 assembly file.
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


.section ".text.crt0", "ax"
.global __module_start
.extern __nx_module_runtime

__module_start:
    .word 0
    .word __nx_mod0 - __module_start

    .align 4

.section ".rodata.mod0", "a"
.global __nx_mod0

.align 2
__nx_mod0:
    .ascii "MOD0"
    .word  __dynamic_start__ - __nx_mod0
    .word  __bss_start__ - __nx_mod0
    .word  __bss_end__ - __nx_mod0
    .word  __eh_frame_hdr_start__ - __nx_mod0
    .word  __eh_frame_hdr_end__ - __nx_mod0
    .word  __nx_module_runtime - __nx_mod0
