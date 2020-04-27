#pragma once

#include <common.h>
#include <malloc.h>
#include <cstring>

namespace bs
{
  template <u32 CAPACITY>
  struct String_FixedSize
  {
    String_FixedSize( char const* string )
    {
      char const* runner = string;
      while ( runner++ != '\0' ) {}
      
      u32 size = runner - string;
      //data = (char const*)malloc( size );
      memcpy( (void*)data, string, size );
    }

    char data[CAPACITY];
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
  using String64 = String_FixedSize<64>;
}