
#include "visual/bitmap.h"
#include "memory/allocator.h"


bs::util::visual::Bitmap::Bitmap(u32 _width, u32 _height)
  : m_Width(_width)
  , m_Height(_height)
  , m_Size(_width * _height)
{
  m_Data = memory::Allocator::Alloc<u32>(_width * _height);
}

bs::util::visual::Bitmap::~Bitmap()
{
  memory::Allocator::Free(m_Data);
}

void bs::util::visual::Bitmap::Fill(bs::util::visual::Color _color)
{
  for (int i = 0; i < m_Size; ++i)
  {
    m_Data[i] = static_cast<Color32Bit>(_color);
  }
}

void bs::util::visual::Bitmap::Resize(u32 _newWidth, u32 _newHeight)
{
}
