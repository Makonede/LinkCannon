#include <botw.hpp>

#include <nn/os.h>
#include <prim/seadSafeString.hpp>

#include <cstdlib>


#if __has_cpp_attribute(noreturn)
#if __has_cpp_attribute(maybe_unused)
[[noreturn]] void _main([[maybe_unused]] void *unused) {
#else
[[noreturn]] void _main(void *unused) {
  static_cast<void>(unused);

#endif
#else
#if __has_cpp_attribute(maybe_unused)
void _main([[maybe_unused]] void *unused) {
#else
void _main(void *unused) {
  static_cast<void>(unused);

#endif
#endif
  auto trigger = &triggerAddr;
  sead::SafeString eventName("LinkCannon");

  while (true) {
    while (!*trigger) {}
    *trigger = false;
    callEvent(nullptr, &eventName, &eventName, nullptr, true, false);
  }
}


extern "C" void init() {
  static nn::os::ThreadType mainThread;
  constexpr auto stackSize = 0x80000uz;
  auto stack = aligned_alloc(0x1000uz, stackSize);

  if (nn::os::CreateThread(
    &mainThread, _main, nullptr, stack, stackSize, 16, 0
#if __has_cpp_attribute(unlikely)
  ).IsFailure()) [[unlikely]] {
#else
  ).IsFailure()) {
#endif
    free(stack);
    return;
  }

  nn::os::StartThread(&mainThread);
}


extern "C" void fini() {}
