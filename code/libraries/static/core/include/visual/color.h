#ifndef CORE_COLOR_H
#define CORE_COLOR_H

#include "common.h"


namespace bs::core::visual
{

  struct Color32Bit
  {
    Color32Bit()
      : a(1)
      , r(0)
      , g(0)
      , b(0)
    {}

    Color32Bit(uint8 _a, uint8 _r, uint8 _g, uint8 _b)
      : a(_a)
      , r(_r)
      , g(_g)
      , b(_b)
    {}

    Color32Bit(uint32 _argb)
      : argb(_argb)
    {
      
    }

    union
    {
      uint32 argb;
      struct
      {
        //TODO endianness
        uint8 b, g, r, a;
      };
    };

    INLINE operator uint32() { return argb; }
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
      uint8 a8 = static_cast<uint8>( a * 255.0f );
      uint8 r8 = static_cast<uint8>( r * 255.0f );
      uint8 g8 = static_cast<uint8>( g * 255.0f );
      uint8 b8 = static_cast<uint8>( b * 255.0f );
      return Color32Bit{ a8, r8, g8, b8 };
    }

    float a, r, g, b;
  };

};
  
#endif
