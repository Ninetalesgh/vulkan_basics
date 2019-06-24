#ifndef CORE_PTR_H
#define CORE_PTR_H

#include "common.h"
#include <memory>

namespace bs
{
  template<typename T> using unique = std::unique_ptr<T>;
  template<typename T> using shared = std::shared_ptr<T>;
  template<typename T> using weak = std::weak_ptr<T>;

  //template<typename T>
  //class unique
  //{
  //public:
  // 

  //  INLINE T* operator ->() { return m_Object; }
  //private:
  //  T* m_Object;
  //};



  //template<typename T>
  //class shared
  //{

  //};
}


#endif