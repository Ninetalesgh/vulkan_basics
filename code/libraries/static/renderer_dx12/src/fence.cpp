#include "fence.h"
#include "util_dx12.h"

bs::dx12::Fence::Fence()
{

}

bs::dx12::Fence::Fence(ID3D12Device* _device, IDXGISwapChain3* _swap_chain, ID3D12CommandQueue* _command_queue)
{
  initialize(_device, _swap_chain, _command_queue);
}

bs::dx12::Fence::~Fence()
{
  SAFE_RELEASE(m_fence);
}

void bs::dx12::Fence::initialize(ID3D12Device* _device, IDXGISwapChain3* _swap_chain, ID3D12CommandQueue* _command_queue)
{
  m_command_queue = _command_queue;
  m_swap_chain = _swap_chain;
  HRESULT hresult;
  hresult = _device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
  Check(hresult);
  m_fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);

  m_fence_value = 1;

  current_frame = m_swap_chain->GetCurrentBackBufferIndex();
  previous_frame = 1 - current_frame; //TODO
}

void bs::dx12::Fence::signal()
{
  HRESULT hresult;
  m_frame_fence_values[current_frame] = m_fence_value++;
  hresult = m_command_queue->Signal(m_fence, m_frame_fence_values[current_frame]);
  Check(hresult);
}

void bs::dx12::Fence::synchronize()
{
  HRESULT hresult;
  previous_frame = current_frame;
  current_frame = m_swap_chain->GetCurrentBackBufferIndex();

  {
    UINT64 test = m_fence->GetCompletedValue();
    if (test < m_frame_fence_values[current_frame])
    {
      hresult = m_fence->SetEventOnCompletion(m_frame_fence_values[current_frame], m_fence_event);
      Check(hresult);
      WaitForSingleObject(m_fence_event, INFINITE);
    }
  }
}
