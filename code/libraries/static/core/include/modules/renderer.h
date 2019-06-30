#ifndef CORE_RENDERER_H
#define CORE_RENDERER_H

namespace bs::core::modules
{
  class RenderObject;

  class IRenderer
  {
  public:
    virtual void Init() = 0;
    virtual void RegisterRenderObject(RenderObject*) = 0;
    
    virtual void Render() = 0;

    virtual void UnregisterRenderObject(RenderObject*) = 0;
    virtual void End() = 0;
  };
};

#endif