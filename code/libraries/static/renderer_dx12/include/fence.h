#ifndef RENDERERDX12_FENCE_H
#define RENDERERDX12_FENCE_H


#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <directx12/d3dx12.h>

namespace bs::dx12
{

  class Fence
  {
  public:
    Fence(ID3D12Device* _device, IDXGISwapChain3* _swap_chain, ID3D12CommandQueue* _command_queue);
    ~Fence();
    void initialize(ID3D12Device* _device, IDXGISwapChain3* _swap_chain, ID3D12CommandQueue* _command_queue);

    void signal();
    void synchronize();

    int32_t current_frame;
    int32_t previous_frame;
  private:
    Fence();
    Fence(const Fence&) = delete;

    ID3D12CommandQueue* m_command_queue;
    IDXGISwapChain3* m_swap_chain;

    HANDLE m_fence_event;
    ID3D12Fence* m_fence;
    UINT64 m_fence_value;
    UINT64 m_frame_fence_values[2] = {};


  };
};

#endif