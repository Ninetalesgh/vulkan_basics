#ifndef CORE_RENDERER_H
#define CORE_RENDERER_H

namespace bs::renderer
{
  class RenderObject;
}

namespace bs::modules
{

  class IRenderer
  {
  public:
    virtual void Init() = 0;
    virtual void RegisterRenderObject(renderer::RenderObject*) = 0;
    
    virtual void Render() = 0;

    virtual void UnregisterRenderObject(renderer::RenderObject*) = 0;
    virtual void End() = 0;
  };
};

#endif