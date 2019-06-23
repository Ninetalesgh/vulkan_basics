#ifndef UTIL_NATIVE_ALLOCATOR_H
#define UTIL_NATIVE_ALLOCATOR_H

#include "defines/defines.h"
#include <malloc.h>

namespace bs::util::memory
{

  class Allocator
  {
  public:
    Allocator()
    {}

    //TODO templated
    void* AllocUntyped(u32 _byteSize) 
    {
      return nullptr;
    }

    template<typename T>
    INLINE static T* Alloc(u32 _count) 
    {
      void* allocation = calloc(static_cast<size_t>(_count), sizeof(T));

      return static_cast<T*>(allocation);
    }

    template<typename T>
    INLINE static void Free(T* _data)
    {
      if ( _data != nullptr )
      {
        free(_data);
      }
    }

    template<typename T, typename... Args>
    INLINE static T* NewArray(u32 _count, Args... _args)
    {
      return new T[_count](_args);
    }

    template<typename T>
    INLINE static void DeleteArray(T* _object)
    {
      if ( _object != nullptr )
      {
        delete[] _object;
      }
    }

    template<typename T, typename... Args>
    INLINE static T* New(Args... _args)
    {
      return new T(_args);
    }

    template<typename T>
    INLINE static void Delete(T* _object)
    {
      if (_object != nullptr)
      {
        delete _object;
      }
    }
  private:
    //TODO keep track of allocations

  };

};

#endif