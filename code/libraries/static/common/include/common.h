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
using u8 = unsigned char;
using u32 = unsigned int;
using s32 = int32_t;


#endif // !UTIL_NATIVE_DEFINES_H
