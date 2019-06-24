#include "common.h"


EXPORT void FetchCurrentFrame_PInvoke(u8* _outImg, u32 _size)
{
  int index = 0;
  while (index < _size)
  {
    _outImg[index++] = 0;//b
    _outImg[index++] = 255;//g
    _outImg[index++] = 0;//r
    _outImg[index++] = 255;//a
  }
}