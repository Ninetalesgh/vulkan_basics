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
  public:
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
};
#endif