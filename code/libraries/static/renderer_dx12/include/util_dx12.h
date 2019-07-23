#ifndef RENDERERDX12_UTILDX12_H
#define RENDERERDX12_UTILDX12_H

template<typename T>
void SAFE_RELEASE(T*& _p)
{
	if (_p)
		_p->Release();
	_p = nullptr;
}

#define ASSERT(x) \
	if(!(x)) {MessageBoxA(0,#x, "assertion failed", MB_OK); __debugbreak();}

#define Check(hresult) \
	ASSERT(SUCCEEDED(hresult))

#define SETNAME_STRING(x,y) x->SetName(L#y)
#define SETNAME(x) x->SetName(L#x)

#endif