#ifndef COMMON_PTR_H
#define COMMON_PTR_H

#include <memory>

namespace bs
{
  template<typename T> using unique = std::unique_ptr<T>;
  template<typename T> using shared = std::shared_ptr<T>;
  template<typename T> using weak = std::weak_ptr<T>;

  //template<typename T, typename... Args>
  //unique<T> make_unique;

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