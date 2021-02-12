#pragma once
#include <cstdint>

#define INLINE

#ifndef INLINE
  #ifdef _WINDOWS
    #define INLINE __forceinline
  #elif _ANDROID
    #define INLINE inline
  #endif
#endif

#ifndef EXPORT
  #ifdef _WINDOWS
    #define EXPORT extern "C" __declspec(dllexport)
  #elif _ANDROID
    #define EXPORT 
  #endif
#endif


//TODO find a place for these type redefinitions 

using i8  = int8_t;
using u8 = uint8_t;

using i16  = int16_t;
using u16 = uint16_t;

using i32  = int32_t;
using s32  = int32_t;
using u32 = uint32_t;

using i64  = int64_t;
using u64 = uint64_t;