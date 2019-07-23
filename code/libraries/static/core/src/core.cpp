
#include "modules/renderer.h"
#include "core/core.h"

namespace bs::core
{
  unique<Core> Core::Create()
  {
    return std::make_unique<Core>();
    //return new Core();
  }
};
