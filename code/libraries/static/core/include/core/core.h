#ifndef CORE_CORE_H
#define CORE_CORE_H

namespace bs::core
{
  class PlatformDesc;
  class IApp;
  class IModule;

  class ICore
  {
  public:

    virtual void Init(PlatformDesc*) = 0;
    virtual void RegisterApp(IApp*) = 0;
    virtual void RegisterModule(IModule*) = 0;
    virtual void Run() = 0;


    virtual void UnregisterModule(IModule*) = 0;
    virtual void UnregisterApp(IApp*) = 0;
    virtual void End() = 0;
  };
};

#endif