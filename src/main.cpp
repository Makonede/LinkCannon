#include <botw.hpp>

#include <nn/os.h>
#include <prim/seadSafeString.hpp>

#include <cstdlib>


void _main(void *unused) {
  (unused);

  constexpr auto mainBase = -0x3483000ull;
  const auto triggerOffset = reinterpret_cast<unsigned char *>(0x2d562f0ull);

  const auto trigger = reinterpret_cast<bool *>(mainBase + triggerOffset);
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
  void *stack = aligned_alloc(0x1000uz, stackSize);

  if (nn::os::CreateThread(
    &mainThread, _main, nullptr, stack, stackSize, 16, 0
  ).IsFailure()) {
    return;
  }

  nn::os::StartThread(&mainThread);
}


extern "C" void fini() {}
