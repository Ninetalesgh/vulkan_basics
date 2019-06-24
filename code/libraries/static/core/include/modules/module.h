#ifndef CORE_MODULE_H
#define CORE_MODULE_H

namespace bs::core::module
{
  class IModule
  {
  public:

    virtual void Init() = 0;

    virtual void End() = 0;
  };
};

#endif