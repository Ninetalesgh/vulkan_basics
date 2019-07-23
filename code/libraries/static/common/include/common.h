#ifndef COMMON_COMMON_H
#define COMMON_COMMON_H

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

using int8  = int8_t;
using uint8 = uint8_t;

using int16  = int16_t;
using uint16 = uint16_t;

using int32  = int32_t;
using uint32 = uint32_t;

using int64  = int64_t;
using uint64 = uint64_t;


#endif // !UTIL_NATIVE_DEFINES_H
