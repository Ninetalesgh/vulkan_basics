#ifndef RENDERER_DX12_RENDERER_DX12_H
#define RENDERER_DX12_RENDERER_DX12_H

#include <platform/renderer.h>

#include <d3d12.h>

#include <vector>


struct IDXGIFactory2;
struct IDXGISwapChain3;
class IWICImagingFactory;

namespace bs::dx12
{
  class Fence;

  class RendererDx12 : public platform::IRenderer
  {
  public:
    void Init(platform::IWindow*) override;
    void End() override;

    void RegisterScene(scene::IScene*) override;
    void UnregisterScene(scene::IScene*) override;

    void Render() override;
    void Present() override;

    int32 GetNewGPUConstantBufferOffset(); 
    int32 load_texture_from_path(const char* _path);
  private:

    HWND* m_window;
    ID3D12Device* m_device;
    ID3D12DebugDevice* m_debug_device;
    ID3D12Debug* m_debug_interface;
    IDXGIFactory2* m_factory;

    ID3D12CommandQueue* m_command_queue;
    ID3D12CommandQueue* m_compute_command_queue;
    IDXGISwapChain3* m_swap_chain;
    ID3D12Resource* m_render_target[2];
    ID3D12DescriptorHeap* m_rtv_descriptor_heap;

    ID3D12Resource* m_depth_stencil;
    ID3D12DescriptorHeap* m_dsv_descriptor_heap;

    Fence* m_fence;
    Fence* m_fence_cs;

    D3D12_VIEWPORT* m_viewport;
    LPRECT				m_scissor_rect;

    ID3D12CommandAllocator* m_command_allocator[2];
    ID3D12GraphicsCommandList* m_command_list_resource;
    ID3D12DescriptorHeap* m_srv_descriptor_heap;

    ID3D12Resource* m_cb_upload;
    UINT8* m_cbv_gpu_address;
    int32  m_current_constant_buffer_offset = 0;

    ID3D12RootSignature* m_root_signature_main;
    ID3D12PipelineState* m_pipeline_state_main;

    ID3D12GraphicsCommandList* m_command_list_main;

    IWICImagingFactory* m_texture_upload_wic_factory;
    std::vector<ID3D12Resource*> m_shader_resources;

    ID3D12Resource* m_vertex_buffer;
    ID3D12Resource* m_index_buffer;
    D3D12_VERTEX_BUFFER_VIEW m_vb_view;
    D3D12_INDEX_BUFFER_VIEW m_ib_view;

    scene::IScene* m_scene;
    //std::vector<low::entity*> m_draw_calls;


    // Inherited via IRenderer

  };
}

#endif
