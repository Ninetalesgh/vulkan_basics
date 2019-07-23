

#include "platform/renderer.h"
#include "core/core.h"
#include <ctime>

namespace bs::core
{
  unique<Core> Core::Create()
  {
    return std::make_unique<Core>();
  }

  void Core::Init(PlatformDesc*)
  {

  }

  void Core::End()
  {

  }

  void Core::Run()
  {

  }

  void Core::RegisterApp(IApp*)
  {
  }
  void Core::UnregisterApp(IApp*)
  {
  }
};
