
#include "modules/renderer.h"
#include "core/core.h"

namespace bs::core
{
  class Core : public ICore
  {
  public:
    void Init(PlatformDesc*) override
    {
    }

    virtual void RegisterApp(IApp*) override
    {
    }

    virtual void UnregisterApp(IApp*) override
    {
    }

    void RegisterModule(modules::IModule*) override
    {
    }

    void UnregisterModule(modules::IModule*) override
    {
    }

    void Run() override
    {
    }

    void End() override
    {
    }

  private:
    // Inherited via ICore
  };

  unique<ICore> ICore::Create()
  {
    return std::make_unique<Core>();
    //return new Core();
  }
};
