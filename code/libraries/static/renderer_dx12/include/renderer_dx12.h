#ifndef RENDERER_DX12_RENDERER_DX12_H
#define RENDERER_DX12_RENDERER_DX12_H

#include <modules/renderer.h>

namespace bs::renderer
{

  class RendererDx12 : public modules::IRenderer
  {
  public:
    // Inherited via IRenderer
    virtual void Init() override;

    virtual void RegisterRenderObject(renderer::RenderObject*) override;

    virtual void Render() override;

    virtual void UnregisterRenderObject(renderer::RenderObject*) override;

    virtual void End() override;




  };
}

#endif
