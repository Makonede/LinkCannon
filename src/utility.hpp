/*
utility.hpp - Header file for commonly used utilities.
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


#pragma once

#include <nn/os.h>


// Yield the thread and sleep for 1 ds (default)
// This allows other threads to work and prevents busy waiting while loops from
// eating CPU cycles.
inline auto Yield(unsigned long long time = 100'000'000ull) noexcept {
  nn::os::SleepThread(nn::TimeSpan::FromNanoSeconds(time));
}
