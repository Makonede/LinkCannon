/*
main.cpp - Main C++ file.
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


#include <botw.hpp>

#include <nn/os.h>
#include <prim/seadSafeString.hpp>

#include <cstdlib>


// Main loop thread
// The function is named _main in order to not be treated as the main function.
#if __has_cpp_attribute(noreturn)
#if __has_cpp_attribute(maybe_unused)
[[noreturn]] void _main([[maybe_unused]] void *unused) {
#else  // __has_cpp_attribute(maybe_unused)
[[noreturn]] void _main(void *unused) {
  static_cast<void>(unused);

#endif  // __has_cpp_attribute(maybe_unused)
#else  // __has_cpp_attribute(noreturn)
#if __has_cpp_attribute(maybe_unused)
void _main([[maybe_unused]] void *unused) {
#else  // __has_cpp_attribute(maybe_unused)
void _main(void *unused) {
  static_cast<void>(unused);

#endif  // __has_cpp_attribute(maybe_unused)
#endif  // __has_cpp_attribute(noreturn)
  auto trigger = &triggerAddr;
  sead::SafeString eventName("LinkCannon");

  while (true) {
    // Wait until trigger points to true. This will happen when the button
    // combination is pressed.
    while (!*trigger) {}
    *trigger = false;
    callEvent(nullptr, &eventName, &eventName, nullptr, true, false);
  }
}


// Initialization function (entrypoint)
extern "C" void init() {
  static nn::os::ThreadType mainThread;

  // Allocate memory for the main thread stack
  constexpr auto stackSize = 0x80000uz;
  auto stack = aligned_alloc(0x1000uz, stackSize);

  // Attempt to create the main thread
  if (nn::os::CreateThread(
    &mainThread, _main, nullptr, stack, stackSize, 16, 0
#if __has_cpp_attribute(unlikely)
  ).IsFailure()) [[unlikely]] {
#else
  ).IsFailure()) {
#endif  // __has_cpp_attribute(unlikely)
    // Free the thread stack if it fails
    free(stack);
    return;
  }

  // Start the thread
  nn::os::StartThread(&mainThread);
}


// Finalization function
extern "C" void fini() {}
