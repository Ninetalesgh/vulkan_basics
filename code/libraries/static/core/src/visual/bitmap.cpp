
#include "visual/bitmap.h"
#include "memory.h"


bs::core::visual::Bitmap::Bitmap(uint32 _width, uint32 _height)
  : m_Width(_width)
  , m_Height(_height)
  , m_Size(_width * _height)
{
  //m_Data = memory::Allocator::Alloc<u32>(_width * _height);
}

bs::core::visual::Bitmap::~Bitmap()
{
 // memory::Allocator::Free(m_Data);
}

void bs::core::visual::Bitmap::Fill(bs::core::visual::Color _color)
{
  auto color32 = static_cast<Color32Bit>(_color);
  for (int i = 0; i < m_Size; ++i)
  {
    m_Data[i] = color32;
  }
}

void bs::core::visual::Bitmap::Resize(uint32 _newWidth, uint32 _newHeight)
{
}
