#pragma once

#include <math/seadMatrix.hpp>
#include <prim/seadSafeString.hpp>


typedef bool (*CALL_EVENT)(
  void *, sead::SafeString *, sead::SafeString *, sead::Matrix34<float> *, bool,
  bool
);

[[maybe_unused]] extern bool callEvent(
  void *, sead::SafeString *, sead::SafeString *, sead::Matrix34<float> *, bool,
  bool
);

extern const bool triggerAddr;
