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

    void Init(PlatformDesc*);
    void End();

    void Run();

    void RegisterApp(IApp*);
    void UnregisterApp(IApp*);

   // void RegisterModule(modules::IModule*); //TODO these go into app
   // void UnregisterModule(modules::IModule*);

    //void RegisterInput();
    //void UnregisterInput();

    //void Run();


  };

};
#endif