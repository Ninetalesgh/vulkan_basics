#include "renderer_dx12.h"
#include "fence.h"
#include "util_dx12.h"

#include <platform/window.h>

#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <directx12/d3dx12.h>

#include <wincodec.h>
#include <codecvt>

#include <iostream>

__declspec(align(16)) struct constant_buffer_per_object
{
  float4x4 wvp;
};

HRESULT CompileShader(const WCHAR* _filename, const char* _entrypoint, const char* _profile, ID3DBlob** _out_blob)
{
  UINT compile_flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
  ID3D10Blob* error;
  HRESULT hresult = D3DCompileFromFile(_filename, 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, _entrypoint, _profile, compile_flags, 0, _out_blob, &error);
  if (hresult != S_OK)
  {
    char temp[1024];
    sprintf_s(temp, sizeof(temp), "%.*s", (int)error->GetBufferSize(), (char*)error->GetBufferPointer());
    OutputDebugStringA(temp);
  }
  return hresult;
}


void bs::dx12::RendererDx12::Init(platform::IWindow* _window)
{
  HRESULT hresult;

  //TODO
  HWND* window = (HWND*)_window->GetHandle();

  hresult = D3D12GetDebugInterface(IID_PPV_ARGS(&m_debug_interface));
  Check(hresult);
  m_debug_interface->EnableDebugLayer();


  //create factory
  hresult = CreateDXGIFactory(IID_PPV_ARGS(&m_factory));
  Check(hresult);

  //find best adapter
  std::vector<IDXGIAdapter1*> adapter;
  int adapter_index = 0;
  bool adapter_found = false;
  IDXGIAdapter1* tmp_adapter = nullptr;

  while (m_factory->EnumAdapters1(adapter_index++, &tmp_adapter) != DXGI_ERROR_NOT_FOUND)
  {
    adapter.push_back(tmp_adapter);
  }
  size_t highest_memory = 0;

  for (auto a : adapter)
  {
    DXGI_ADAPTER_DESC1 adapter_descriptor;
    a->GetDesc1(&adapter_descriptor);
    if (adapter_descriptor.Flags & DXGI_ADAPTER_FLAG_SOFTWARE || adapter_descriptor.Flags & D3D_DRIVER_TYPE_SOFTWARE || adapter_descriptor.Flags & D3D_DRIVER_TYPE_WARP || adapter_descriptor.DeviceId == 0x8c)
    {
      continue;
    }

    if (adapter_descriptor.DedicatedVideoMemory > highest_memory)
    {
      highest_memory = adapter_descriptor.DedicatedVideoMemory;
      tmp_adapter = a;
    }
  }

  for (auto a : adapter)
  {
    if (a == tmp_adapter)continue;
    SAFE_RELEASE(a);
  }

  //setup debug device
  void** device_tmp = nullptr;
#ifdef _DEBUG
  device_tmp = (void**)& m_debug_device;
#else
  device_tmp = (void**)& m_device;
#endif

  //create device
  hresult = D3D12CreateDevice(tmp_adapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), device_tmp);
  Check(hresult);

#ifdef _DEBUG
  hresult = m_debug_device->QueryInterface(IID_PPV_ARGS(&m_device));
  Check(hresult);
#endif

  D3D12_COMMAND_QUEUE_DESC command_queue_desc = {};
  command_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  hresult = m_device->CreateCommandQueue(&command_queue_desc, IID_PPV_ARGS(&m_command_queue));
  Check(hresult);
  SETNAME_STRING(m_command_queue, "main command queue");

  //TODO compute command queue code
  /*D3D12_COMMAND_QUEUE_DESC command_queue_cs_desc = {};
  command_queue_cs_desc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
  hresult = m_device->CreateCommandQueue(&command_queue_cs_desc, IID_PPV_ARGS(&m_compute_command_queue));
  Check(hresult);
  SETNAME_STRING(m_compute_command_queue, "compute command queue");*/

  //create swap chain
  DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
  swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swap_chain_desc.SampleDesc.Count = 1;
  swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swap_chain_desc.BufferCount = 2;
  swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

  IDXGISwapChain1* tmp_swap_chain;
  hresult = m_factory->CreateSwapChainForHwnd(m_command_queue, *m_window, &swap_chain_desc, 0, 0, &tmp_swap_chain);
  Check(hresult);

  hresult = tmp_swap_chain->QueryInterface(IID_PPV_ARGS(&m_swap_chain));
  Check(hresult);

  for (int i = 0; i < 2; ++i)
  {
    hresult = m_swap_chain->GetBuffer(UINT(i), IID_PPV_ARGS(&m_render_target[i]));
    Check(hresult);
  }

  //TODO use smart pointers
  m_fence = new Fence(m_device, m_swap_chain, m_command_queue);
  m_fence_cs = new Fence(m_device, m_swap_chain, m_compute_command_queue);

  D3D12_DESCRIPTOR_HEAP_DESC render_target_view_descriptor_heap_desc = {};
  render_target_view_descriptor_heap_desc.NumDescriptors = 2;
  render_target_view_descriptor_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  hresult = m_device->CreateDescriptorHeap(&render_target_view_descriptor_heap_desc, IID_PPV_ARGS(&m_rtv_descriptor_heap));
  Check(hresult);
  SETNAME_STRING(m_rtv_descriptor_heap, "render target view descriptor heap");

  auto render_target_view_descriptor = m_rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart();

  for (size_t i = 0; i < 2; ++i)
  {
    D3D12_RENDER_TARGET_VIEW_DESC render_target_view_desc = {};
    hresult = m_swap_chain->GetBuffer(UINT(i), IID_PPV_ARGS(&m_render_target[i]));
    Check(hresult);

    m_device->CreateRenderTargetView(m_render_target[i], 0, render_target_view_descriptor);
    render_target_view_descriptor.ptr += m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  }

  D3D12_DESCRIPTOR_HEAP_DESC dsv_heap_desc = {};
  dsv_heap_desc.NumDescriptors = 1;
  dsv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
  dsv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  hresult = m_device->CreateDescriptorHeap(&dsv_heap_desc, IID_PPV_ARGS(&m_dsv_descriptor_heap));
  Check(hresult);
  SETNAME_STRING(m_dsv_descriptor_heap, "depth stencil descriptor heap");

  D3D12_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc = {};
  depth_stencil_view_desc.Format = DXGI_FORMAT_D32_FLOAT;
  depth_stencil_view_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
  depth_stencil_view_desc.Flags = D3D12_DSV_FLAG_NONE;

  D3D12_CLEAR_VALUE depth_optimized_clear_value = {};
  depth_optimized_clear_value.Format = DXGI_FORMAT_D32_FLOAT;
  depth_optimized_clear_value.DepthStencil.Depth = 1.0f;
  depth_optimized_clear_value.DepthStencil.Stencil = 0;

  int32_t width, height;
  RECT rect;
  if (GetWindowRect(*m_window, &rect))
  {
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;
  }


  hresult = m_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
    D3D12_HEAP_FLAG_NONE,
    &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R32_TYPELESS, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
    D3D12_RESOURCE_STATE_DEPTH_WRITE,
    &depth_optimized_clear_value, IID_PPV_ARGS(&m_depth_stencil));
  Check(hresult);
  SETNAME_STRING(m_depth_stencil, "depth stencil buffer");

  m_device->CreateDepthStencilView(m_depth_stencil,
    &depth_stencil_view_desc,
    m_dsv_descriptor_heap->GetCPUDescriptorHandleForHeapStart());

  //TODO this is for finite entity count. albeit.. a lot.
  hresult = m_device->CreateCommittedResource(
    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
    D3D12_HEAP_FLAG_NONE,
    &CD3DX12_RESOURCE_DESC::Buffer(1024 * 64),
    D3D12_RESOURCE_STATE_GENERIC_READ,
    nullptr,
    IID_PPV_ARGS(&m_cb_upload));
  SETNAME_STRING(m_cb_upload, "constant buffer per object");
  Check(hresult);

  hresult = m_cb_upload->Map(0, &CD3DX12_RANGE(0, 0), reinterpret_cast<void**>(&m_cbv_gpu_address));
  Check(hresult);

  m_viewport = new D3D12_VIEWPORT();
  m_viewport->TopLeftX = 0;
  m_viewport->TopLeftY = 0;
  m_viewport->Width = (float)width;
  m_viewport->Height = (float)height;
  m_viewport->MinDepth = 0.0f;
  m_viewport->MaxDepth = 1.0f;

  m_scissor_rect = new RECT();
  m_scissor_rect->left = 0;
  m_scissor_rect->top = 0;
  m_scissor_rect->right = width;
  m_scissor_rect->bottom = height;

  for (int i = 0; i < 2; ++i)
  {
    hresult = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_command_allocator[i]));
    Check(hresult);
  }
  SETNAME_STRING(m_command_allocator[0], "command allocator 0");
  SETNAME_STRING(m_command_allocator[1], "command allocator 1");

  hresult = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_command_allocator[0], 0, IID_PPV_ARGS(&m_command_list_resource));
  m_command_list_resource->Close();
  SETNAME_STRING(m_command_list_resource, "resource command list");

  D3D12_DESCRIPTOR_HEAP_DESC srv_heap_desc = {};
  srv_heap_desc.NumDescriptors = 512;
  srv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  srv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

  hresult = m_device->CreateDescriptorHeap(&srv_heap_desc, IID_PPV_ARGS(&m_srv_descriptor_heap));
  Check(hresult);
  SETNAME_STRING(m_srv_descriptor_heap, "srv descriptor heap");


  ID3DBlob* vertex_shader;
  ID3DBlob* pixel_shader;
  hresult = CompileShader(L"assets/shader/main.ps.hlsl", "main", "ps_5_1", &pixel_shader);
  Check(hresult);
  hresult = CompileShader(L"assets/shader/main.vs.hlsl", "main", "vs_5_1", &vertex_shader);
  Check(hresult);

  ID3DBlob* root_signature_blob;
  hresult = D3DGetBlobPart(pixel_shader->GetBufferPointer(), pixel_shader->GetBufferSize(), D3D_BLOB_ROOT_SIGNATURE, 0, &root_signature_blob);
  Check(hresult);

  hresult = m_device->CreateRootSignature(0, root_signature_blob->GetBufferPointer(), root_signature_blob->GetBufferSize(), IID_PPV_ARGS(&m_root_signature_main));
  Check(hresult);
  SETNAME_STRING(m_root_signature_main, "main root signature");

  //TODO rework input layout
  D3D12_INPUT_ELEMENT_DESC input_layout[] =
  {
    { "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0, 0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
    { "UV"      ,0,DXGI_FORMAT_R32G32_FLOAT   ,0,12,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 }
  };

  D3D12_INPUT_LAYOUT_DESC input_layout_desc = {};
  input_layout_desc.NumElements = sizeof(input_layout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
  input_layout_desc.pInputElementDescs = input_layout;

  D3D12_BLEND_DESC blend_desc = {};
  const D3D12_RENDER_TARGET_BLEND_DESC rt_blend_desc =
  {
    TRUE,FALSE,
    D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_INV_SRC_ALPHA, D3D12_BLEND_OP_ADD,
    D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
    D3D12_LOGIC_OP_NOOP,
    D3D12_COLOR_WRITE_ENABLE_ALL
  };
  for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
    blend_desc.RenderTarget[i] = rt_blend_desc;

  D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeline_state_desc = { 0 };
  pipeline_state_desc.InputLayout = input_layout_desc;
  pipeline_state_desc.pRootSignature = m_root_signature_main;

  pipeline_state_desc.VS = CD3DX12_SHADER_BYTECODE(vertex_shader);
  pipeline_state_desc.PS = CD3DX12_SHADER_BYTECODE(pixel_shader);
  pipeline_state_desc.BlendState = blend_desc; // CD3DX12_BLEND_DESC(D3D12_DEFAULT);
  pipeline_state_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
  pipeline_state_desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
  pipeline_state_desc.SampleMask = UINT_MAX;
  pipeline_state_desc.NumRenderTargets = 1;
  pipeline_state_desc.SampleDesc.Count = 1;
  pipeline_state_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
  pipeline_state_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
  pipeline_state_desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

  D3D12_DEPTH_STENCIL_DESC depth_stencil_desc = {};
  depth_stencil_desc.DepthEnable = TRUE;
  depth_stencil_desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
  depth_stencil_desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
  depth_stencil_desc.StencilEnable = FALSE;

  pipeline_state_desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
  pipeline_state_desc.DepthStencilState = depth_stencil_desc;
  hresult = m_device->CreateGraphicsPipelineState(&pipeline_state_desc, IID_PPV_ARGS(&m_pipeline_state_main));
  Check(hresult);
  SETNAME_STRING(m_pipeline_state_main, "main pso");

  hresult = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_command_allocator[0], 0, IID_PPV_ARGS(&m_command_list_main));
  m_command_list_main->Close();
  SETNAME_STRING(m_command_list_main, "main command list");

  vertex v_list[4];
  v_list[0].pos = float3(-1.0f, 1.0f, 0.5f);
  v_list[1].pos = float3(1.0f, 1.0f, 0.5f);
  v_list[2].pos = float3(1.0f, -1.0f, 0.5f);
  v_list[3].pos = float3(-1.0f, -1.0f, 0.5f);

  v_list[0].uv = float2(0.0f, 0.0f);
  v_list[1].uv = float2(1.0f, 0.0f);
  v_list[2].uv = float2(1.0f, 1.0f);
  v_list[3].uv = float2(0.0f, 1.0f);

  DWORD i_list[] = { 0,1,2,0,2,3 };

  int vbuffer_size = sizeof(v_list);
  int ibuffer_size = sizeof(i_list);

  m_device->CreateCommittedResource(
    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
    D3D12_HEAP_FLAG_NONE,
    &CD3DX12_RESOURCE_DESC::Buffer(vbuffer_size),
    D3D12_RESOURCE_STATE_GENERIC_READ,
    nullptr,
    IID_PPV_ARGS(&m_vertex_buffer));
  SETNAME(m_vertex_buffer);

  UINT8* dest = 0;
  m_vertex_buffer->Map(0, &CD3DX12_RANGE(0, 0), (void**)& dest);
  memcpy(dest, &v_list[0], vbuffer_size);

  m_vb_view.BufferLocation = m_vertex_buffer->GetGPUVirtualAddress();
  m_vb_view.StrideInBytes = sizeof(vertex);
  m_vb_view.SizeInBytes = vbuffer_size;

  m_device->CreateCommittedResource(
    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
    D3D12_HEAP_FLAG_NONE,
    &CD3DX12_RESOURCE_DESC::Buffer(ibuffer_size),
    D3D12_RESOURCE_STATE_GENERIC_READ,
    nullptr,
    IID_PPV_ARGS(&m_index_buffer));

  SETNAME(m_index_buffer);
  dest = 0;
  m_index_buffer->Map(0, &CD3DX12_RANGE(0, 0), (void**)& dest);
  memcpy(dest, &i_list[0], ibuffer_size);

  m_ib_view.BufferLocation = m_index_buffer->GetGPUVirtualAddress();
  m_ib_view.Format = DXGI_FORMAT_R32_UINT;
  m_ib_view.SizeInBytes = ibuffer_size;


  printf("dx12 initialized.\n");
}

void bs::dx12::RendererDx12::End()
{
  SAFE_RELEASE(m_device);
  SAFE_RELEASE(m_debug_device);
  SAFE_RELEASE(m_debug_interface);
  SAFE_RELEASE(m_factory);
  SAFE_RELEASE(m_command_queue);
  SAFE_RELEASE(m_compute_command_queue);
  SAFE_RELEASE(m_swap_chain);
  SAFE_RELEASE(m_render_target[0]);
  SAFE_RELEASE(m_render_target[1]);

  delete m_fence;
  delete m_fence_cs;

  delete m_viewport;
  delete m_scissor_rect;

  SAFE_RELEASE(m_command_allocator[0]);
  SAFE_RELEASE(m_command_allocator[1]);
  SAFE_RELEASE(m_command_list_resource);
  SAFE_RELEASE(m_srv_descriptor_heap);

  SAFE_RELEASE(m_cb_upload);

  SAFE_RELEASE(m_root_signature_main);
  SAFE_RELEASE(m_pipeline_state_main);
  SAFE_RELEASE(m_command_list_main);

  //delete m_texture_upload_wic_factory; ???
  //TODO does this not have to be deleted?

  for (auto r : m_shader_resources)
  {
    SAFE_RELEASE(r);
  }

  SAFE_RELEASE(m_vertex_buffer);
  SAFE_RELEASE(m_index_buffer);
}

void bs::dx12::RendererDx12::RegisterScene(bs::scene::IScene*)
{
}

void bs::dx12::RendererDx12::UnregisterScene(bs::scene::IScene*)
{
}

void bs::dx12::RendererDx12::Render()
{
 // std::vector<low::entity*> new_draw_calls;
  float4x4 camera_view_matrix;
//  _rss->collect_draw_calls(new_draw_calls, camera_view_matrix);

  float4x4 camera_projection_matrix = projection_matrix(m_viewport->Width, m_viewport->Height, 45.0f * (3.14f / 180.0f), 0.1f, 1000.0f);

 // for (auto draw_call : new_draw_calls)
  {
    constant_buffer_per_object cbpo;
   // cbpo.wvp = camera_projection_matrix * camera_view_matrix * draw_call->get_world();
  //  memcpy(m_cbv_gpu_address + draw_call->get_gpu_buffer_id() * sizeof(constant_buffer_per_object), &cbpo, sizeof(constant_buffer_per_object));
  }

 // m_draw_calls.insert(m_draw_calls.end(), new_draw_calls.begin(), new_draw_calls.end());
  
}

void bs::dx12::RendererDx12::Present()
{
  m_fence->synchronize();

  ////TODO code placement
  HRESULT hresult;
  hresult = m_command_allocator[m_fence->current_frame]->Reset();
  Check(hresult);

  hresult = m_command_list_main->Reset(m_command_allocator[m_fence->current_frame], m_pipeline_state_main);
  Check(hresult);

  CD3DX12_CPU_DESCRIPTOR_HANDLE current_render_target_descriptor(m_rtv_descriptor_heap->GetCPUDescriptorHandleForHeapStart(), m_fence->current_frame, UINT(m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)));
  CD3DX12_CPU_DESCRIPTOR_HANDLE current_depth_stencil_descriptor(m_dsv_descriptor_heap->GetCPUDescriptorHandleForHeapStart());

  m_command_list_main->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_render_target[m_fence->current_frame], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

  m_command_list_main->OMSetRenderTargets(1, &current_render_target_descriptor, 0, &current_depth_stencil_descriptor);
  float clear_color[4] = { 0.0f,0.0f,0.0f,0.0f };

  m_command_list_main->ClearRenderTargetView(current_render_target_descriptor, clear_color, 0, 0);
  m_command_list_main->ClearDepthStencilView(current_depth_stencil_descriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

  m_command_list_main->SetGraphicsRootSignature(m_root_signature_main);

  ID3D12DescriptorHeap* descriptor_heaps[] = { m_srv_descriptor_heap };
  m_command_list_main->SetDescriptorHeaps(_countof(descriptor_heaps), descriptor_heaps);


  m_command_list_main->RSSetViewports(1, m_viewport);
  m_command_list_main->RSSetScissorRects(1, m_scissor_rect);
  m_command_list_main->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


  //// PER DRAW CALL
 // for (auto draw_call : m_draw_calls)
  {
    m_command_list_main->IASetVertexBuffers(0, 1, &m_vb_view);
    m_command_list_main->IASetIndexBuffer(&m_ib_view);

    auto srv_gpu_handle = m_srv_descriptor_heap->GetGPUDescriptorHandleForHeapStart();

  //  srv_gpu_handle.ptr += draw_call->get_texture_id() * m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


    m_command_list_main->SetGraphicsRootDescriptorTable(1, srv_gpu_handle);

    auto cbv_offset = m_cb_upload->GetGPUVirtualAddress();// +draw_call->m_constant_buffer_offset[m_fence->current_frame];
    m_command_list_main->SetGraphicsRootConstantBufferView(0, cbv_offset);

    m_command_list_main->DrawIndexedInstanced(6, 1, 0, 0, 0);

  }
  //////////////////////

  m_command_list_main->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_render_target[m_fence->current_frame], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

  hresult = m_command_list_main->Close();
  Check(hresult);

  {
    ID3D12CommandList* command_lists[] = { m_command_list_main };
    m_command_queue->ExecuteCommandLists(1, command_lists);
  }

  m_fence->signal();

  hresult = m_swap_chain->Present(0, 0);
  Check(hresult);

 // while (m_draw_calls.size())
  {
  //  m_draw_calls.pop_back();
  }
}


DXGI_FORMAT get_dxgi_format_from_wic_format(WICPixelFormatGUID& _wic_format)
{
  if (_wic_format == GUID_WICPixelFormat128bppRGBAFloat) return DXGI_FORMAT_R32G32B32A32_FLOAT;
  else if (_wic_format == GUID_WICPixelFormat64bppRGBAHalf) return DXGI_FORMAT_R16G16B16A16_FLOAT;
  else if (_wic_format == GUID_WICPixelFormat64bppRGBA) return DXGI_FORMAT_R16G16B16A16_UNORM;
  else if (_wic_format == GUID_WICPixelFormat32bppRGBA) return DXGI_FORMAT_R8G8B8A8_UNORM;
  else if (_wic_format == GUID_WICPixelFormat32bppBGRA) return DXGI_FORMAT_B8G8R8A8_UNORM;
  else if (_wic_format == GUID_WICPixelFormat32bppBGR) return DXGI_FORMAT_B8G8R8X8_UNORM;
  else if (_wic_format == GUID_WICPixelFormat32bppRGBA1010102XR) return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;

  else if (_wic_format == GUID_WICPixelFormat32bppRGBA1010102) return DXGI_FORMAT_R10G10B10A2_UNORM;
  else if (_wic_format == GUID_WICPixelFormat16bppBGRA5551) return DXGI_FORMAT_B5G5R5A1_UNORM;
  else if (_wic_format == GUID_WICPixelFormat16bppBGR565) return DXGI_FORMAT_B5G6R5_UNORM;
  else if (_wic_format == GUID_WICPixelFormat32bppGrayFloat) return DXGI_FORMAT_R32_FLOAT;
  else if (_wic_format == GUID_WICPixelFormat16bppGrayHalf) return DXGI_FORMAT_R16_FLOAT;
  else if (_wic_format == GUID_WICPixelFormat16bppGray) return DXGI_FORMAT_R16_UNORM;
  else if (_wic_format == GUID_WICPixelFormat8bppGray) return DXGI_FORMAT_R8_UNORM;
  else if (_wic_format == GUID_WICPixelFormat8bppAlpha) return DXGI_FORMAT_A8_UNORM;

  else return DXGI_FORMAT_UNKNOWN;
}
WICPixelFormatGUID convert_to_wic_format(WICPixelFormatGUID& _pixel_format)
{
  if (_pixel_format == GUID_WICPixelFormatBlackWhite) return GUID_WICPixelFormat8bppGray;
  else if (_pixel_format == GUID_WICPixelFormat1bppIndexed) return GUID_WICPixelFormat32bppRGBA;
  else if (_pixel_format == GUID_WICPixelFormat2bppIndexed) return GUID_WICPixelFormat32bppRGBA;
  else if (_pixel_format == GUID_WICPixelFormat4bppIndexed) return GUID_WICPixelFormat32bppRGBA;
  else if (_pixel_format == GUID_WICPixelFormat8bppIndexed) return GUID_WICPixelFormat32bppRGBA;
  else if (_pixel_format == GUID_WICPixelFormat2bppGray) return GUID_WICPixelFormat8bppGray;
  else if (_pixel_format == GUID_WICPixelFormat4bppGray) return GUID_WICPixelFormat8bppGray;
  else if (_pixel_format == GUID_WICPixelFormat16bppGrayFixedPoint) return GUID_WICPixelFormat16bppGrayHalf;
  else if (_pixel_format == GUID_WICPixelFormat32bppGrayFixedPoint) return GUID_WICPixelFormat32bppGrayFloat;
  else if (_pixel_format == GUID_WICPixelFormat16bppBGR555) return GUID_WICPixelFormat16bppBGRA5551;
  else if (_pixel_format == GUID_WICPixelFormat32bppBGR101010) return GUID_WICPixelFormat32bppRGBA1010102;
  else if (_pixel_format == GUID_WICPixelFormat24bppBGR) return GUID_WICPixelFormat32bppRGBA;
  else if (_pixel_format == GUID_WICPixelFormat24bppRGB) return GUID_WICPixelFormat32bppRGBA;
  else if (_pixel_format == GUID_WICPixelFormat32bppPBGRA) return GUID_WICPixelFormat32bppRGBA;
  else if (_pixel_format == GUID_WICPixelFormat32bppPRGBA) return GUID_WICPixelFormat32bppRGBA;
  else if (_pixel_format == GUID_WICPixelFormat48bppRGB) return GUID_WICPixelFormat64bppRGBA;
  else if (_pixel_format == GUID_WICPixelFormat48bppBGR) return GUID_WICPixelFormat64bppRGBA;
  else if (_pixel_format == GUID_WICPixelFormat64bppBGRA) return GUID_WICPixelFormat64bppRGBA;
  else if (_pixel_format == GUID_WICPixelFormat64bppPRGBA) return GUID_WICPixelFormat64bppRGBA;
  else if (_pixel_format == GUID_WICPixelFormat64bppPBGRA) return GUID_WICPixelFormat64bppRGBA;
  else if (_pixel_format == GUID_WICPixelFormat48bppRGBFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
  else if (_pixel_format == GUID_WICPixelFormat48bppBGRFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
  else if (_pixel_format == GUID_WICPixelFormat64bppRGBAFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
  else if (_pixel_format == GUID_WICPixelFormat64bppBGRAFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
  else if (_pixel_format == GUID_WICPixelFormat64bppRGBFixedPoint) return GUID_WICPixelFormat64bppRGBAHalf;
  else if (_pixel_format == GUID_WICPixelFormat64bppRGBHalf) return GUID_WICPixelFormat64bppRGBAHalf;
  else if (_pixel_format == GUID_WICPixelFormat48bppRGBHalf) return GUID_WICPixelFormat64bppRGBAHalf;
  else if (_pixel_format == GUID_WICPixelFormat128bppPRGBAFloat) return GUID_WICPixelFormat128bppRGBAFloat;
  else if (_pixel_format == GUID_WICPixelFormat128bppRGBFloat) return GUID_WICPixelFormat128bppRGBAFloat;
  else if (_pixel_format == GUID_WICPixelFormat128bppRGBAFixedPoint) return GUID_WICPixelFormat128bppRGBAFloat;
  else if (_pixel_format == GUID_WICPixelFormat128bppRGBFixedPoint) return GUID_WICPixelFormat128bppRGBAFloat;
  else if (_pixel_format == GUID_WICPixelFormat32bppRGBE) return GUID_WICPixelFormat128bppRGBAFloat;
  else if (_pixel_format == GUID_WICPixelFormat32bppCMYK) return GUID_WICPixelFormat32bppRGBA;
  else if (_pixel_format == GUID_WICPixelFormat64bppCMYK) return GUID_WICPixelFormat64bppRGBA;
  else if (_pixel_format == GUID_WICPixelFormat40bppCMYKAlpha) return GUID_WICPixelFormat64bppRGBA;
  else if (_pixel_format == GUID_WICPixelFormat80bppCMYKAlpha) return GUID_WICPixelFormat64bppRGBA;

#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8) || defined(_WIN7_PLATFORM_UPDATE)
  else if (_pixel_format == GUID_WICPixelFormat32bppRGB) return GUID_WICPixelFormat32bppRGBA;
  else if (_pixel_format == GUID_WICPixelFormat64bppRGB) return GUID_WICPixelFormat64bppRGBA;
  else if (_pixel_format == GUID_WICPixelFormat64bppPRGBAHalf) return GUID_WICPixelFormat64bppRGBAHalf;
#endif

  else return GUID_WICPixelFormatDontCare;
}
int get_dxgi_format_bits_per_pixel(DXGI_FORMAT& _dxgi_format)
{
  if (_dxgi_format == DXGI_FORMAT_R32G32B32A32_FLOAT) return 128;
  else if (_dxgi_format == DXGI_FORMAT_R16G16B16A16_FLOAT) return 64;
  else if (_dxgi_format == DXGI_FORMAT_R16G16B16A16_UNORM) return 64;
  else if (_dxgi_format == DXGI_FORMAT_R8G8B8A8_UNORM) return 32;
  else if (_dxgi_format == DXGI_FORMAT_B8G8R8A8_UNORM) return 32;
  else if (_dxgi_format == DXGI_FORMAT_B8G8R8X8_UNORM) return 32;
  else if (_dxgi_format == DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM) return 32;

  else if (_dxgi_format == DXGI_FORMAT_R10G10B10A2_UNORM) return 32;
  else if (_dxgi_format == DXGI_FORMAT_B5G5R5A1_UNORM) return 16;
  else if (_dxgi_format == DXGI_FORMAT_B5G6R5_UNORM) return 16;
  else if (_dxgi_format == DXGI_FORMAT_R32_FLOAT) return 32;
  else if (_dxgi_format == DXGI_FORMAT_R16_FLOAT) return 16;
  else if (_dxgi_format == DXGI_FORMAT_R16_UNORM) return 16;
  else if (_dxgi_format == DXGI_FORMAT_R8_UNORM) return 8;
  else if (_dxgi_format == DXGI_FORMAT_A8_UNORM) return 8;
  else return 0;
}


int32_t bs::dx12::RendererDx12::load_texture_from_path(const char* _path)
{

  HRESULT hresult;
  IWICBitmapDecoder* wic_decoder = nullptr;
  IWICBitmapFrameDecode* wic_frame = nullptr;
  IWICFormatConverter* wic_converter = nullptr;
  bool image_converted = false;
  if (!m_texture_upload_wic_factory)
  {
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    hresult = CoCreateInstance(
      CLSID_WICImagingFactory,
      NULL,
      CLSCTX_INPROC_SERVER,
      IID_PPV_ARGS(&m_texture_upload_wic_factory)
    );
    Check(hresult);
  }
  std::string tmp = _path;
  std::wstring wstr_path(tmp.size(), L' ');
  wstr_path.resize(std::mbstowcs(&wstr_path[0], tmp.c_str(), tmp.size()));
  //std::copy(tmp.begin(), tmp.end(), wstr_path.begin());
  hresult = m_texture_upload_wic_factory->CreateDecoderFromFilename(
    wstr_path.c_str(),
    NULL,
    GENERIC_READ,
    WICDecodeMetadataCacheOnLoad,
    &wic_decoder
  );
  if (hresult == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
  {
    printf("couldn't load asset: ");
    printf(_path);
    printf("\n --- file not found\n");
    return 0;
  }
  hresult = wic_decoder->GetFrame(0, &wic_frame);
  Check(hresult);
  WICPixelFormatGUID pixel_format;
  hresult = wic_frame->GetPixelFormat(&pixel_format);
  Check(hresult);
  UINT texture_width, texture_height;
  hresult = wic_frame->GetSize(&texture_width, &texture_height);
  Check(hresult);
  DXGI_FORMAT dxgi_format = get_dxgi_format_from_wic_format(pixel_format);
  if (dxgi_format == DXGI_FORMAT_UNKNOWN)
  {
    WICPixelFormatGUID converted_format = convert_to_wic_format(pixel_format);
    if (converted_format == GUID_WICPixelFormatDontCare) return 0;
    dxgi_format = get_dxgi_format_from_wic_format(converted_format);
    hresult = m_texture_upload_wic_factory->CreateFormatConverter(&wic_converter);
    Check(hresult);
    BOOL can_convert = false;
    hresult = wic_converter->CanConvert(pixel_format, converted_format, &can_convert);
    Check(hresult);
    hresult = wic_converter->Initialize(wic_frame, converted_format, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom);
    Check(hresult);
    image_converted = true;
  }
  BYTE* image_data;
  int bits_per_pixel = get_dxgi_format_bits_per_pixel(dxgi_format);
  int bytes_per_pixel = bits_per_pixel / 8;
  int bytes_per_row = texture_width * bytes_per_pixel;
  int image_size = bytes_per_row * texture_height;
  image_data = (BYTE*)malloc(image_size);
  if (image_converted)
  {
    hresult = wic_converter->CopyPixels(0, bytes_per_row, image_size, image_data);
    Check(hresult);
  }
  else
  {
    hresult = wic_frame->CopyPixels(0, bytes_per_row, image_size, image_data);
    Check(hresult);
  }

  m_fence->synchronize();

  m_command_list_resource->Reset(m_command_allocator[m_fence->current_frame], nullptr);

  D3D12_RESOURCE_DESC resource_description = CD3DX12_RESOURCE_DESC::Tex2D(dxgi_format, texture_width, texture_height, 1, 1);

  ID3D12Resource* new_texture_resource;

  hresult = m_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
    D3D12_HEAP_FLAG_NONE,
    &resource_description,
    D3D12_RESOURCE_STATE_COPY_DEST,
    nullptr,
    IID_PPV_ARGS(&new_texture_resource));
  Check(hresult);
  UINT64 upload_buffer_size = 0;
  m_device->GetCopyableFootprints(&resource_description, 0, 1, 0, nullptr, nullptr, nullptr, &upload_buffer_size);

  ID3D12Resource* upload_heap;
  hresult = m_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
    D3D12_HEAP_FLAG_NONE,
    &CD3DX12_RESOURCE_DESC::Buffer(upload_buffer_size),
    D3D12_RESOURCE_STATE_GENERIC_READ,
    nullptr,
    IID_PPV_ARGS(&upload_heap));
  Check(hresult);

  D3D12_SUBRESOURCE_DATA data = {};
  data.pData = image_data;
  data.RowPitch = bytes_per_row;
  data.SlicePitch = image_size;
  UpdateSubresources(m_command_list_resource, new_texture_resource, upload_heap, 0, 0, 1, &data);
  m_command_list_resource->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(new_texture_resource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

  D3D12_SHADER_RESOURCE_VIEW_DESC shader_resource_view_descriptor = {};
  shader_resource_view_descriptor.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  shader_resource_view_descriptor.Format = resource_description.Format;
  shader_resource_view_descriptor.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
  shader_resource_view_descriptor.Texture2D.MipLevels = 1;
  auto srv_cpu_handle = m_srv_descriptor_heap->GetCPUDescriptorHandleForHeapStart();
  srv_cpu_handle.ptr += m_shader_resources.size() * m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  m_device->CreateShaderResourceView(new_texture_resource, &shader_resource_view_descriptor, srv_cpu_handle);

  m_shader_resources.push_back(new_texture_resource);
  delete image_data;

  //TODO
  //SAFE_RELEASE(upload_heap);

  m_command_list_resource->Close();

  ID3D12CommandList* command_lists[] = { m_command_list_resource };
  m_command_queue->ExecuteCommandLists(1, command_lists);

  m_fence->signal();

  return m_shader_resources.size() - 1;
}

int32 bs::dx12::RendererDx12::GetNewGPUConstantBufferOffset()
{
  return m_current_constant_buffer_offset++;
}