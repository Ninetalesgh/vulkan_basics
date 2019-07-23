#ifndef CORE_WINDOW_H
#define CORE_WINDOW_H

namespace bs::platform
{

  class IWindow
  {
  public:
    virtual void* GetHandle() = 0;
  };

};

#endif