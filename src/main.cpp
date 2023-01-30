#include "botw.hpp"

#include <prim/seadSafeString.hpp>


extern "C" void _main() {
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


extern "C" void fini() {}
