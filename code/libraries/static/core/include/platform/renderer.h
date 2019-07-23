#ifndef CORE_RENDERER_H
#define CORE_RENDERER_H

#include <math/matrix.h>

namespace bs::scene
{
  class IScene;
}

namespace bs::platform
{
  class IWindow;

  class IRenderer
  {
  public:
    virtual void Init(IWindow*) = 0;
    virtual void End() = 0;

    virtual void RegisterScene(bs::scene::IScene*) = 0;
    virtual void UnregisterScene(bs::scene::IScene*) = 0;
    
    virtual void Render() = 0;
    virtual void Present() = 0;
  };
};

#endif