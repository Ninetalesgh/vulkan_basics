#ifndef CORE_RENDERER_H
#define CORE_RENDERER_H

namespace bs::core::modules
{
  class IRenderer
  {
  public:
    virtual void Init() = 0;
    virtual void RegisterRenderable() = 0;
    
    virtual void Render() = 0;


    virtual void UnregisterRenderable() = 0;
    virtual void End() = 0;
  };
};

#endif