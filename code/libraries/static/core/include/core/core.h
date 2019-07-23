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

  class Core
  {
    static unique<Core> Create();

    void Init();
    void RegisterApp(IApp*);
    void RegisterModule(modules::IModule*); //TODO these go into app
    void Run();

    //receive window messages
    //receive delta time 

    void TickUI();
    


    void UnregisterModule(modules::IModule*);
    void UnregisterApp(IApp*);
    void End();

  };


  class ICore
  {
  public:

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