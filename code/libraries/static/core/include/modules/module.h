#ifndef CORE_MODULE_H
#define CORE_MODULE_H

namespace bs::modules
{
  class IModule
  {
  public:

    virtual void Init() = 0;

    virtual void End() = 0;
  };
};

#endif