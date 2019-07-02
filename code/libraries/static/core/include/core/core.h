#ifndef CORE_CORE_H
#define CORE_CORE_H

#include <ptr.h>

namespace bs::modules
{
  class IModule;
};

namespace bs::core
{

  class PlatformDesc;
  class IApp;

  class ICore
  {
  public:
    static unique<ICore> Create();

    virtual void Init(PlatformDesc*) = 0;
    virtual void RegisterApp(IApp*) = 0;
    virtual void RegisterModule(modules::IModule*) = 0; //TODO these go into app
    virtual void Run() = 0;


    virtual void UnregisterModule(modules::IModule*) = 0;
    virtual void UnregisterApp(IApp*) = 0;
    virtual void End() = 0;
  };

};
#endif