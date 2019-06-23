#ifndef UTIL_NATIVE_COLOR_H
#define UTIL_NATIVE_COLOR_H

#include "defines/defines.h"


namespace bs::util::visual
{

  struct Color32Bit
  {
    Color32Bit()
      : a(1)
      , r(0)
      , g(0)
      , b(0)
    {}

    Color32Bit(u8 _a, u8 _r, u8 _g, u8 _b)
      : a(_a)
      , r(_r)
      , g(_g)
      , b(_b)
    {}

    Color32Bit(u32 _argb)
      : argb(_argb)
    {
      
    }

    union
    {
      u32 argb;
      struct
      {
        //TODO endianness
        u8 b, g, r, a;
      };
    };

    INLINE operator u32() { return argb; }
  };

  struct Color
  {
    Color()
      : a(1.0f)
      , r(0.0f)
      , g(0.0f)
      , b(0.0f)
    {}
    Color(float _a, float _r, float _g, float _b)
      : a(_a)
      , r(_r)
      , g(_g)
      , b(_b)
    {}
    Color(Color32Bit _color)
      : a(static_cast<float>(_color.a) / 255.0f)
      , r(static_cast<float>(_color.r) / 255.0f)
      , g(static_cast<float>(_color.g) / 255.0f)
      , b(static_cast<float>(_color.b) / 255.0f)
    {}

    INLINE explicit operator Color32Bit() 
    {
      //TODO make sure rgba are 0-1
      u8 a8 = static_cast<u8>( a * 255.0f );
      u8 r8 = static_cast<u8>( r * 255.0f );
      u8 g8 = static_cast<u8>( g * 255.0f );
      u8 b8 = static_cast<u8>( b * 255.0f );
      return Color32Bit{ a8, r8, g8, b8 };
    }

    float a, r, g, b;
  };

};
  
#endif
