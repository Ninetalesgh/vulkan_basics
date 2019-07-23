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
    Bitmap(uint32 _width, uint32 _height);
    ~Bitmap();


    void Fill(Color _color);
    void Resize(uint32 _newWidth, uint32 _newHeight);

  private:
    uint32* m_Data;
    uint32  m_Width;
    uint32  m_Height;
    uint32  m_Size;
  };
  
}};


#endif