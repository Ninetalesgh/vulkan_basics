#pragma once

#include <common.h>
#include <malloc.h>
#include <cstring>

namespace bs
{
  template <u32 size>
  struct String_FixedSize
  {

    char data[size];
  };

  struct String
  {
    String() 
    {
      static char const nullchar = '\0';
      data = &nullchar;
    }

    String( char const* string )
    {
      char const* runner = string;
      while ( runner++ != '\0' ) {}
      u32 size = runner - string;
      data = (char const*)malloc( size );
      memcpy( (void*)data, string, size );
    }

      operator char const* ( ) { return data; }
  private:
    char const* data;
  };

  using String8 = String_FixedSize<8>;
  using String16 = String_FixedSize<16>;
  using String32 = String_FixedSize<32>;

}