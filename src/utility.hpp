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

#include <memory>

#include <cstdlib>

#include <nn/os.h>

typedef void (*ThreadFunc)(void *);


// Create a thread
template <typename T = void>
inline constexpr auto StartThread(ThreadFunc func, T *arg = nullptr) {
  auto thread = std::make_unique<nn::os::ThreadType>();
  constexpr auto STACK_SIZE = 0x80000uz;
  constexpr auto ALIGNMENT = 0x1000uz;
  auto *stack = std::aligned_alloc(ALIGNMENT, STACK_SIZE);

  if (stack == nullptr) [[unlikely]] {
    return false;
  }

  constexpr auto PRIORITY = 16;

  if (nn::os::CreateThread(
    thread.get(), func, static_cast<void *>(arg), stack,
    static_cast<unsigned long long>(STACK_SIZE), PRIORITY
  ).IsFailure()) [[unlikely]] {
    std::free(stack);
    return false;
  }

  nn::os::StartThread(thread.release());
  return true;
}


// Yield the thread and sleep for 1 ds (default)
// This allows other threads to work and prevents busy waiting while loops from
// eating CPU cycles.
inline constexpr auto Yield(unsigned long long time = 100'000'000ull) noexcept {
  nn::os::SleepThread(nn::TimeSpan::FromNanoSeconds(time));
}
