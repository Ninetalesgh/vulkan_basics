#ifndef CORE_BITMAP_H
#define CORE_BITMAP_H

#include "visual/color.h"
#include "common.h"

namespace bs::core
{
  class Allocator;
namespace visual
{

  class Bitmap
  {
  public:
    Bitmap()
      : m_Data(nullptr)
      , m_Width(0)
      , m_Height(0)
      , m_Size(0)
    {}
    Bitmap(u32 _width, u32 _height);
    ~Bitmap();


    void Fill(Color _color);
    void Resize(u32 _newWidth, u32 _newHeight);

  private:
    u32* m_Data;
    u32  m_Width;
    u32  m_Height;
    u32  m_Size;
  };
  
}};


#endif