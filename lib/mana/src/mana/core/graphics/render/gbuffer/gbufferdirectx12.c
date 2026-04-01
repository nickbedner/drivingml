#include "mana/core/graphics/render/gbuffer/gbufferdirectx12.h"

// NOTE: This function is used for DirectX 12 error handling init
internal u8 gbuffer_directx_12_init_wrapper(struct GBufferCommon* gbuffer_common, struct APICommon* api_common, struct SwapChainCommon* swap_chain_common, const uint_fast32_t msaa_samples) {
  ID3D12Device* device = api_common->directx_12_api.device;
  HRESULT hr;

  u32 gbuffer_width = swap_chain_common->swap_chain_extent.width * swap_chain_common->supersample_scale;
  u32 gbuffer_height = swap_chain_common->swap_chain_extent.height * swap_chain_common->supersample_scale;

  DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;

  // Clear value
  gbuffer_common->gbuffer_directx12.color_clear_value.Format = DXGI_FORMAT_R8G8B8A8_UNORM;             // or whatever format your texture is
  gbuffer_common->gbuffer_directx12.color_clear_value.Color[0] = gbuffer_common->color_clear_value.r;  // Red
  gbuffer_common->gbuffer_directx12.color_clear_value.Color[1] = gbuffer_common->color_clear_value.g;  // Green
  gbuffer_common->gbuffer_directx12.color_clear_value.Color[2] = gbuffer_common->color_clear_value.b;  // Blue
  gbuffer_common->gbuffer_directx12.color_clear_value.Color[3] = gbuffer_common->color_clear_value.w;  // Alpha

  // Normal clear value
  gbuffer_common->gbuffer_directx12.normal_clear_value.Format = DXGI_FORMAT_R8G8B8A8_UNORM;              // or whatever format your texture is
  gbuffer_common->gbuffer_directx12.normal_clear_value.Color[0] = gbuffer_common->normal_clear_value.r;  // Red
  gbuffer_common->gbuffer_directx12.normal_clear_value.Color[1] = gbuffer_common->normal_clear_value.g;  // Green
  gbuffer_common->gbuffer_directx12.normal_clear_value.Color[2] = gbuffer_common->normal_clear_value.b;  // Blue
  gbuffer_common->gbuffer_directx12.normal_clear_value.Color[3] = gbuffer_common->normal_clear_value.w;  // Alpha

  gbuffer_common->gbuffer_directx12.viewport.TopLeftX = 0.0f;
  gbuffer_common->gbuffer_directx12.viewport.TopLeftY = 0.0f;
  gbuffer_common->gbuffer_directx12.viewport.Width = (r32)gbuffer_width;
  gbuffer_common->gbuffer_directx12.viewport.Height = (r32)gbuffer_height;
  gbuffer_common->gbuffer_directx12.viewport.MinDepth = 0.0f;  // typically 0.0f
  gbuffer_common->gbuffer_directx12.viewport.MaxDepth = 1.0f;  // typically 1.0f

  gbuffer_common->gbuffer_directx12.scissor_rect.left = 0;
  gbuffer_common->gbuffer_directx12.scissor_rect.top = 0;
  gbuffer_common->gbuffer_directx12.scissor_rect.right = (LONG)gbuffer_width;
  gbuffer_common->gbuffer_directx12.scissor_rect.bottom = (LONG)gbuffer_height;

  // Resource description for color and normal textures
  D3D12_RESOURCE_DESC texture_desc;
  memset(&texture_desc, 0, sizeof(texture_desc));
  texture_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  texture_desc.Width = gbuffer_width;
  texture_desc.Height = gbuffer_height;
  texture_desc.DepthOrArraySize = 1;
  texture_desc.MipLevels = 1;
  texture_desc.SampleDesc.Count = 1;
  texture_desc.Format = format;
  texture_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
  texture_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

  // Heap properties
  D3D12_HEAP_PROPERTIES heap_properties;
  heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;
  heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
  heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
  heap_properties.CreationNodeMask = 1;
  heap_properties.VisibleNodeMask = 1;

  // Create the color texture
  hr = device->lpVtbl->CreateCommittedResource(device, &heap_properties, D3D12_HEAP_FLAG_NONE, &texture_desc, D3D12_RESOURCE_STATE_RENDER_TARGET, &(gbuffer_common->gbuffer_directx12.color_clear_value), &IID_ID3D12Resource, (void**)&(gbuffer_common->gbuffer_directx12.color_texture));
  if (FAILED(hr))
    return 1;
  // Create the normal texture
  hr = device->lpVtbl->CreateCommittedResource(device, &heap_properties, D3D12_HEAP_FLAG_NONE, &texture_desc, D3D12_RESOURCE_STATE_RENDER_TARGET, &(gbuffer_common->gbuffer_directx12.normal_clear_value), &IID_ID3D12Resource, (void**)&(gbuffer_common->gbuffer_directx12.normal_texture));
  if (FAILED(hr))
    return 1;

  if (msaa_samples != 1) {
    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS ms_quality_levels;
    memset(&ms_quality_levels, 0, sizeof(ms_quality_levels));
    ms_quality_levels.Format = format;
    ms_quality_levels.SampleCount = msaa_samples;

    hr = device->lpVtbl->CheckFeatureSupport(device, D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &ms_quality_levels, sizeof(ms_quality_levels));
    if (FAILED(hr))
      return 1;

    UINT max_quality_level = ms_quality_levels.NumQualityLevels;
    if (max_quality_level > 0)
      max_quality_level -= 1;  // For zero-based indexing
    else
      max_quality_level = 0;

    // Resource description for multisampling
    D3D12_RESOURCE_DESC texture_desc_multisample = texture_desc;
    texture_desc_multisample.SampleDesc = (DXGI_SAMPLE_DESC){msaa_samples, max_quality_level};

    // Create the multisampled texture
    hr = device->lpVtbl->CreateCommittedResource(device, &heap_properties, D3D12_HEAP_FLAG_NONE, &texture_desc_multisample, D3D12_RESOURCE_STATE_RENDER_TARGET, &(gbuffer_common->gbuffer_directx12.color_clear_value), &IID_ID3D12Resource, (void**)&(gbuffer_common->gbuffer_directx12.multisample_color_texture));
    if (FAILED(hr))
      return 1;

    hr = device->lpVtbl->CreateCommittedResource(device, &heap_properties, D3D12_HEAP_FLAG_NONE, &texture_desc_multisample, D3D12_RESOURCE_STATE_RENDER_TARGET, &(gbuffer_common->gbuffer_directx12.normal_clear_value), &IID_ID3D12Resource, (void**)&(gbuffer_common->gbuffer_directx12.multisample_normal_texture));
    if (FAILED(hr))
      return 1;
  }

  // Create descriptor heaps
  D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc;
  memset(&rtv_heap_desc, 0, sizeof(rtv_heap_desc));
  rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  rtv_heap_desc.NumDescriptors = 2;  // Color and normal
  if (msaa_samples != 1)
    rtv_heap_desc.NumDescriptors += 2;  // Multisample color and normals
  rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  hr = device->lpVtbl->CreateDescriptorHeap(device, &rtv_heap_desc, &IID_ID3D12DescriptorHeap, (void**)&(gbuffer_common->gbuffer_directx12.rtv_descriptor_heap));
  if (FAILED(hr))
    return 1;

  D3D12_DESCRIPTOR_HEAP_DESC dsv_heap_desc;
  memset(&dsv_heap_desc, 0, sizeof(dsv_heap_desc));
  dsv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
  dsv_heap_desc.NumDescriptors = 1;  // Depth
  dsv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  hr = device->lpVtbl->CreateDescriptorHeap(device, &dsv_heap_desc, &IID_ID3D12DescriptorHeap, (void**)&(gbuffer_common->gbuffer_directx12.dsv_descriptor_heap));
  if (FAILED(hr))
    return 1;

  // Create RTV and DSV descriptors
  gbuffer_common->gbuffer_directx12.rtv_descriptor_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(gbuffer_common->gbuffer_directx12.rtv_descriptor_heap, &(gbuffer_common->gbuffer_directx12.rtv_handle_color));
  gbuffer_common->gbuffer_directx12.rtv_handle_normal = gbuffer_common->gbuffer_directx12.rtv_handle_color;
  gbuffer_common->gbuffer_directx12.rtv_handle_normal.ptr += device->lpVtbl->GetDescriptorHandleIncrementSize(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  if (msaa_samples != 1) {
    gbuffer_common->gbuffer_directx12.rtv_handle_multisample_color = gbuffer_common->gbuffer_directx12.rtv_handle_normal;
    gbuffer_common->gbuffer_directx12.rtv_handle_multisample_color.ptr += device->lpVtbl->GetDescriptorHandleIncrementSize(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    gbuffer_common->gbuffer_directx12.rtv_handle_multisample_normal = gbuffer_common->gbuffer_directx12.rtv_handle_multisample_color;
    gbuffer_common->gbuffer_directx12.rtv_handle_multisample_normal.ptr += device->lpVtbl->GetDescriptorHandleIncrementSize(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  }

  // Create descriptors for Color and Normal
  device->lpVtbl->CreateRenderTargetView(device, gbuffer_common->gbuffer_directx12.color_texture, NULL, gbuffer_common->gbuffer_directx12.rtv_handle_color);
  device->lpVtbl->CreateRenderTargetView(device, gbuffer_common->gbuffer_directx12.normal_texture, NULL, gbuffer_common->gbuffer_directx12.rtv_handle_normal);
  if (msaa_samples != 1) {
    device->lpVtbl->CreateRenderTargetView(device, gbuffer_common->gbuffer_directx12.multisample_color_texture, NULL, gbuffer_common->gbuffer_directx12.rtv_handle_multisample_color);
    device->lpVtbl->CreateRenderTargetView(device, gbuffer_common->gbuffer_directx12.multisample_normal_texture, NULL, gbuffer_common->gbuffer_directx12.rtv_handle_multisample_normal);
  }

  gbuffer_common->gbuffer_directx12.dsv_descriptor_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(gbuffer_common->gbuffer_directx12.dsv_descriptor_heap, &(gbuffer_common->gbuffer_directx12.dsv_handle));

  // Resource description for the depth buffer
  texture_desc.Format = DXGI_FORMAT_D32_FLOAT;
  // texture_desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
  texture_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
  texture_desc.SampleDesc.Count = msaa_samples;

  D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS ms_quality_levels;
  memset(&ms_quality_levels, 0, sizeof(ms_quality_levels));
  ms_quality_levels.Format = DXGI_FORMAT_D32_FLOAT;
  ms_quality_levels.SampleCount = msaa_samples;

  hr = device->lpVtbl->CheckFeatureSupport(device, D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &ms_quality_levels, sizeof(ms_quality_levels));
  if (FAILED(hr))
    return 1;

  UINT max_depth_quality_level = ms_quality_levels.NumQualityLevels;
  if (max_depth_quality_level > 0)
    max_depth_quality_level -= 1;  // For zero-based indexing
  else
    max_depth_quality_level = 0;

  texture_desc.SampleDesc.Quality = max_depth_quality_level;

  // Create the depth buffer
  gbuffer_common->gbuffer_directx12.depth_clear_value.Format = DXGI_FORMAT_D32_FLOAT;
  gbuffer_common->gbuffer_directx12.depth_clear_value.DepthStencil.Depth = gbuffer_common->depth_clear_value;
  gbuffer_common->gbuffer_directx12.depth_clear_value.DepthStencil.Stencil = 0;

  hr = device->lpVtbl->CreateCommittedResource(device, &heap_properties, D3D12_HEAP_FLAG_NONE, &texture_desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &(gbuffer_common->gbuffer_directx12.depth_clear_value), &IID_ID3D12Resource, (void**)&(gbuffer_common->gbuffer_directx12.depth_texture));
  if (FAILED(hr))
    return 1;

  // Create the descriptor for the Depth texture
  device->lpVtbl->CreateDepthStencilView(device, gbuffer_common->gbuffer_directx12.depth_texture, NULL, gbuffer_common->gbuffer_directx12.dsv_handle);

  // Create Command Allocators and Command Lists
  hr = api_common->directx_12_api.device->lpVtbl->CreateCommandAllocator(api_common->directx_12_api.device, D3D12_COMMAND_LIST_TYPE_DIRECT, &IID_ID3D12CommandAllocator, (void**)&(gbuffer_common->gbuffer_directx12.command_allocator));
  if (FAILED(hr))
    return 1;

  hr = api_common->directx_12_api.device->lpVtbl->CreateCommandList(api_common->directx_12_api.device, 0, D3D12_COMMAND_LIST_TYPE_DIRECT, gbuffer_common->gbuffer_directx12.command_allocator, NULL, &IID_ID3D12CommandList, (void**)&(gbuffer_common->gbuffer_directx12.command_list));
  if (FAILED(hr))
    return 1;

  D3D12_RESOURCE_BARRIER barriers[5];
  memset(barriers, 0, sizeof(barriers));
  barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barriers[0].Transition.pResource = gbuffer_common->gbuffer_directx12.color_texture;
  barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
  barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
  barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

  barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barriers[1].Transition.pResource = gbuffer_common->gbuffer_directx12.normal_texture;
  barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
  barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
  barriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

  barriers[2].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barriers[2].Transition.pResource = gbuffer_common->gbuffer_directx12.depth_texture;
  barriers[2].Transition.StateBefore = D3D12_RESOURCE_STATE_DEPTH_WRITE;
  barriers[2].Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_READ;
  barriers[2].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

  if (msaa_samples != 1) {
    barriers[3].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barriers[3].Transition.pResource = gbuffer_common->gbuffer_directx12.multisample_color_texture;
    barriers[3].Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barriers[3].Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barriers[3].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    barriers[4].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barriers[4].Transition.pResource = gbuffer_common->gbuffer_directx12.multisample_normal_texture;
    barriers[4].Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barriers[4].Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barriers[4].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    gbuffer_common->gbuffer_directx12.command_list->lpVtbl->ResourceBarrier(gbuffer_common->gbuffer_directx12.command_list, 5, barriers);
  } else
    gbuffer_common->gbuffer_directx12.command_list->lpVtbl->ResourceBarrier(gbuffer_common->gbuffer_directx12.command_list, 3, barriers);

  hr = gbuffer_common->gbuffer_directx12.command_list->lpVtbl->Close(gbuffer_common->gbuffer_directx12.command_list);
  if (FAILED(hr))
    return 1;

  ID3D12CommandList* pp_command_lists[] = {(ID3D12CommandList*)gbuffer_common->gbuffer_directx12.command_list};
  api_common->directx_12_api.command_queue->lpVtbl->ExecuteCommandLists(api_common->directx_12_api.command_queue, _countof(pp_command_lists), pp_command_lists);

  // Create Fences
  hr = api_common->directx_12_api.device->lpVtbl->CreateFence(api_common->directx_12_api.device, 0, D3D12_FENCE_FLAG_NONE, &IID_ID3D12Fence, (void**)&(gbuffer_common->gbuffer_directx12.fence));
  if (FAILED(hr))
    return 1;

  gbuffer_common->gbuffer_directx12.fence_value = 1;

  gbuffer_common->gbuffer_directx12.fence_event = CreateEvent(NULL, FALSE, FALSE, NULL);
  if (gbuffer_common->gbuffer_directx12.fence_event == NULL) {
    hr = HRESULT_FROM_WIN32(GetLastError());
    if (FAILED(hr))
      return 1;
  }

  D3D12_DESCRIPTOR_HEAP_DESC srv_heap_desc;
  memset(&srv_heap_desc, 0, sizeof(srv_heap_desc));
  srv_heap_desc.NumDescriptors = GBUFFER_TOTAL_ATTACHMENTS;  // Or however many you need
  srv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  srv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  hr = api_common->directx_12_api.device->lpVtbl->CreateDescriptorHeap(api_common->directx_12_api.device, &srv_heap_desc, &IID_ID3D12DescriptorHeap, (void**)&(gbuffer_common->gbuffer_directx12.srv_heap));
  if (FAILED(hr))
    return 1;

  UINT descriptor_size = api_common->directx_12_api.device->lpVtbl->GetDescriptorHandleIncrementSize(api_common->directx_12_api.device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

  gbuffer_common->gbuffer_directx12.srv_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(gbuffer_common->gbuffer_directx12.srv_heap, &(gbuffer_common->gbuffer_directx12.srv_cpu_handle));
  gbuffer_common->gbuffer_directx12.srv_heap->lpVtbl->GetGPUDescriptorHandleForHeapStart(gbuffer_common->gbuffer_directx12.srv_heap, &(gbuffer_common->gbuffer_directx12.srv_gpu_handle));

  SIZE_T ptr_handle = gbuffer_common->gbuffer_directx12.srv_cpu_handle.ptr;
  // Create SRVs for each of your resources
  D3D12_SHADER_RESOURCE_VIEW_DESC color_srv_desc;
  memset(&color_srv_desc, 0, sizeof(color_srv_desc));
  color_srv_desc.Format = format;
  color_srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
  color_srv_desc.Texture2D.MipLevels = 1;
  color_srv_desc.Texture2D.MostDetailedMip = 0;
  color_srv_desc.Texture2D.ResourceMinLODClamp = 0.0f;
  color_srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  api_common->directx_12_api.device->lpVtbl->CreateShaderResourceView(api_common->directx_12_api.device, gbuffer_common->gbuffer_directx12.color_texture, &color_srv_desc, gbuffer_common->gbuffer_directx12.srv_cpu_handle);
  gbuffer_common->gbuffer_directx12.srv_cpu_handle.ptr += descriptor_size;

  D3D12_SHADER_RESOURCE_VIEW_DESC normal_srv_desc;
  memset(&normal_srv_desc, 0, sizeof(normal_srv_desc));
  normal_srv_desc.Format = format;
  normal_srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
  normal_srv_desc.Texture2D.MipLevels = 1;
  normal_srv_desc.Texture2D.MostDetailedMip = 0;
  normal_srv_desc.Texture2D.ResourceMinLODClamp = 0.0f;
  normal_srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  api_common->directx_12_api.device->lpVtbl->CreateShaderResourceView(api_common->directx_12_api.device, gbuffer_common->gbuffer_directx12.normal_texture, &normal_srv_desc, gbuffer_common->gbuffer_directx12.srv_cpu_handle);
  gbuffer_common->gbuffer_directx12.srv_cpu_handle.ptr += descriptor_size;

  D3D12_SHADER_RESOURCE_VIEW_DESC depth_srv_desc;
  memset(&depth_srv_desc, 0, sizeof(depth_srv_desc));
  depth_srv_desc.Format = DXGI_FORMAT_R32_FLOAT;  // Adjust this based on your resource's format
  if (msaa_samples != 1)
    depth_srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
  else
    depth_srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
  depth_srv_desc.Texture2D.MipLevels = 1;
  depth_srv_desc.Texture2D.MostDetailedMip = 0;
  depth_srv_desc.Texture2D.ResourceMinLODClamp = 0.0f;
  depth_srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  api_common->directx_12_api.device->lpVtbl->CreateShaderResourceView(api_common->directx_12_api.device, gbuffer_common->gbuffer_directx12.depth_texture, &depth_srv_desc, gbuffer_common->gbuffer_directx12.srv_cpu_handle);
  gbuffer_common->gbuffer_directx12.srv_cpu_handle.ptr = ptr_handle;

  return 0;
}

u8 gbuffer_directx_12_init(struct GBufferCommon* gbuffer_common, struct APICommon* api_common, struct SwapChainCommon* swap_chain_common, const uint_fast32_t msaa_samples) {
  if (gbuffer_directx_12_init_wrapper(gbuffer_common, api_common, swap_chain_common, msaa_samples)) {
    directx_12_graphics_utils_poll_debug_messages(&(api_common->directx_12_api));
    return 1;
  }

  return 0;
}

void gbuffer_directx_12_delete(struct GBufferCommon* gbuffer_common, struct APICommon* api_common, const uint_fast32_t msaa_samples) {
  // Release color_texture
  if (gbuffer_common->gbuffer_directx12.color_texture) {
    gbuffer_common->gbuffer_directx12.color_texture->lpVtbl->Release(gbuffer_common->gbuffer_directx12.color_texture);
    gbuffer_common->gbuffer_directx12.color_texture = NULL;
  }

  // Release normal_texture
  if (gbuffer_common->gbuffer_directx12.normal_texture) {
    gbuffer_common->gbuffer_directx12.normal_texture->lpVtbl->Release(gbuffer_common->gbuffer_directx12.normal_texture);
    gbuffer_common->gbuffer_directx12.normal_texture = NULL;
  }

  // Release depth_texture
  if (gbuffer_common->gbuffer_directx12.depth_texture) {
    gbuffer_common->gbuffer_directx12.depth_texture->lpVtbl->Release(gbuffer_common->gbuffer_directx12.depth_texture);
    gbuffer_common->gbuffer_directx12.depth_texture = NULL;
  }

  // Release rtv_descriptor_heap
  if (gbuffer_common->gbuffer_directx12.rtv_descriptor_heap) {
    gbuffer_common->gbuffer_directx12.rtv_descriptor_heap->lpVtbl->Release(gbuffer_common->gbuffer_directx12.rtv_descriptor_heap);
    gbuffer_common->gbuffer_directx12.rtv_descriptor_heap = NULL;
  }

  // Release dsv_descriptor_heap
  if (gbuffer_common->gbuffer_directx12.dsv_descriptor_heap) {
    gbuffer_common->gbuffer_directx12.dsv_descriptor_heap->lpVtbl->Release(gbuffer_common->gbuffer_directx12.dsv_descriptor_heap);
    gbuffer_common->gbuffer_directx12.dsv_descriptor_heap = NULL;
  }

  // Release srv_heap
  if (gbuffer_common->gbuffer_directx12.srv_heap) {
    gbuffer_common->gbuffer_directx12.srv_heap->lpVtbl->Release(gbuffer_common->gbuffer_directx12.srv_heap);
    gbuffer_common->gbuffer_directx12.srv_heap = NULL;
  }

  // Release command_allocator
  if (gbuffer_common->gbuffer_directx12.command_allocator) {
    gbuffer_common->gbuffer_directx12.command_allocator->lpVtbl->Release(gbuffer_common->gbuffer_directx12.command_allocator);
    gbuffer_common->gbuffer_directx12.command_allocator = NULL;
  }

  // Release command_list
  if (gbuffer_common->gbuffer_directx12.command_list) {
    gbuffer_common->gbuffer_directx12.command_list->lpVtbl->Release(gbuffer_common->gbuffer_directx12.command_list);
    gbuffer_common->gbuffer_directx12.command_list = NULL;
  }

  // Release fence
  if (gbuffer_common->gbuffer_directx12.fence) {
    gbuffer_common->gbuffer_directx12.fence->lpVtbl->Release(gbuffer_common->gbuffer_directx12.fence);
    gbuffer_common->gbuffer_directx12.fence = NULL;
  }

  // Close fence_event if it's valid
  if (gbuffer_common->gbuffer_directx12.fence_event) {
    CloseHandle(gbuffer_common->gbuffer_directx12.fence_event);
    gbuffer_common->gbuffer_directx12.fence_event = NULL;
  }
}

u8 gbuffer_directx_12_resize(struct GBufferCommon* gbuffer_common, struct APICommon* api_common, struct SwapChainCommon* swap_chain_common, const uint_fast32_t msaa_samples) {
  gbuffer_directx_12_delete(gbuffer_common, api_common, msaa_samples);
  gbuffer_directx_12_init(gbuffer_common, api_common, swap_chain_common, msaa_samples);

  return 0;
}

u8 gbuffer_directx_12_start(struct GBufferCommon* gbuffer_common, struct SwapChainCommon* swap_chain_common, const uint_fast32_t msaa_samples) {
  // Reset command allocator and then the command list
  gbuffer_common->gbuffer_directx12.command_allocator->lpVtbl->Reset(gbuffer_common->gbuffer_directx12.command_allocator);
  gbuffer_common->gbuffer_directx12.command_list->lpVtbl->Reset(gbuffer_common->gbuffer_directx12.command_list, gbuffer_common->gbuffer_directx12.command_allocator, NULL);

  D3D12_RESOURCE_BARRIER barriers[5];
  memset(barriers, 0, sizeof(barriers));
  barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barriers[0].Transition.pResource = gbuffer_common->gbuffer_directx12.color_texture;
  barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
  barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
  barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

  barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barriers[1].Transition.pResource = gbuffer_common->gbuffer_directx12.normal_texture;
  barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
  barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
  barriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

  barriers[2].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barriers[2].Transition.pResource = gbuffer_common->gbuffer_directx12.depth_texture;
  barriers[2].Transition.StateBefore = D3D12_RESOURCE_STATE_DEPTH_READ;
  barriers[2].Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
  barriers[2].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

  if (msaa_samples != 1) {
    barriers[3].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barriers[3].Transition.pResource = gbuffer_common->gbuffer_directx12.multisample_color_texture;
    barriers[3].Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barriers[3].Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barriers[3].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    barriers[4].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barriers[4].Transition.pResource = gbuffer_common->gbuffer_directx12.multisample_normal_texture;
    barriers[4].Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barriers[4].Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barriers[4].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    gbuffer_common->gbuffer_directx12.command_list->lpVtbl->ResourceBarrier(gbuffer_common->gbuffer_directx12.command_list, 5, barriers);
  } else
    gbuffer_common->gbuffer_directx12.command_list->lpVtbl->ResourceBarrier(gbuffer_common->gbuffer_directx12.command_list, 3, barriers);

  // Set and clear color RTV
  D3D12_CPU_DESCRIPTOR_HANDLE rtv_handles[2];
  if (msaa_samples != 1) {
    rtv_handles[0] = gbuffer_common->gbuffer_directx12.rtv_handle_multisample_color;
    rtv_handles[1] = gbuffer_common->gbuffer_directx12.rtv_handle_multisample_normal;
  } else {
    rtv_handles[0] = gbuffer_common->gbuffer_directx12.rtv_handle_color;
    rtv_handles[1] = gbuffer_common->gbuffer_directx12.rtv_handle_normal;
  }

  // Bind both color and normals RTVs
  gbuffer_common->gbuffer_directx12.command_list->lpVtbl->OMSetRenderTargets(gbuffer_common->gbuffer_directx12.command_list, 2, rtv_handles, FALSE, &(gbuffer_common->gbuffer_directx12.dsv_handle));

  // Clear them
  if (msaa_samples != 1) {
    gbuffer_common->gbuffer_directx12.command_list->lpVtbl->ClearRenderTargetView(gbuffer_common->gbuffer_directx12.command_list, gbuffer_common->gbuffer_directx12.rtv_handle_multisample_color, gbuffer_common->gbuffer_directx12.color_clear_value.Color, 0, NULL);
    gbuffer_common->gbuffer_directx12.command_list->lpVtbl->ClearRenderTargetView(gbuffer_common->gbuffer_directx12.command_list, gbuffer_common->gbuffer_directx12.rtv_handle_multisample_normal, gbuffer_common->gbuffer_directx12.normal_clear_value.Color, 0, NULL);
  } else {
    gbuffer_common->gbuffer_directx12.command_list->lpVtbl->ClearRenderTargetView(gbuffer_common->gbuffer_directx12.command_list, gbuffer_common->gbuffer_directx12.rtv_handle_color, gbuffer_common->gbuffer_directx12.color_clear_value.Color, 0, NULL);
    gbuffer_common->gbuffer_directx12.command_list->lpVtbl->ClearRenderTargetView(gbuffer_common->gbuffer_directx12.command_list, gbuffer_common->gbuffer_directx12.rtv_handle_normal, gbuffer_common->gbuffer_directx12.normal_clear_value.Color, 0, NULL);
  }
  gbuffer_common->gbuffer_directx12.command_list->lpVtbl->ClearDepthStencilView(gbuffer_common->gbuffer_directx12.command_list, gbuffer_common->gbuffer_directx12.dsv_handle, D3D12_CLEAR_FLAG_DEPTH, gbuffer_common->gbuffer_directx12.depth_clear_value.DepthStencil.Depth, 0, 0, NULL);

  return 0;
}

u8 gbuffer_directx_12_stop(struct GBufferCommon* gbuffer_common, struct APICommon* api_common, const uint_fast32_t msaa_samples) {
  // Possibly set barriers or any other synchronization primitives required for DirectX12. This is a common step but specifics can vary based on the exact workflow.
  D3D12_RESOURCE_BARRIER barriers[5];
  memset(barriers, 0, sizeof(barriers));
  barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barriers[0].Transition.pResource = gbuffer_common->gbuffer_directx12.color_texture;
  barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
  barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
  barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

  barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barriers[1].Transition.pResource = gbuffer_common->gbuffer_directx12.normal_texture;
  barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
  barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
  barriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

  barriers[2].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barriers[2].Transition.pResource = gbuffer_common->gbuffer_directx12.depth_texture;
  barriers[2].Transition.StateBefore = D3D12_RESOURCE_STATE_DEPTH_WRITE;
  barriers[2].Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_READ;
  barriers[2].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

  if (msaa_samples != 1) {
    barriers[3].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barriers[3].Transition.pResource = gbuffer_common->gbuffer_directx12.multisample_color_texture;
    barriers[3].Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barriers[3].Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barriers[3].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    barriers[4].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barriers[4].Transition.pResource = gbuffer_common->gbuffer_directx12.multisample_normal_texture;
    barriers[4].Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barriers[4].Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barriers[4].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    gbuffer_common->gbuffer_directx12.command_list->lpVtbl->ResourceBarrier(gbuffer_common->gbuffer_directx12.command_list, 5, barriers);

    // Add additional barriers before ResolveSubresource operations
    D3D12_RESOURCE_BARRIER additionalBarriers[2];
    memset(additionalBarriers, 0, sizeof(additionalBarriers));

    // Transition multisample color texture to RESOLVE_SOURCE
    additionalBarriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    additionalBarriers[0].Transition.pResource = gbuffer_common->gbuffer_directx12.multisample_color_texture;
    additionalBarriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    additionalBarriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
    additionalBarriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    // Transition color texture to RESOLVE_DEST
    additionalBarriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    additionalBarriers[1].Transition.pResource = gbuffer_common->gbuffer_directx12.color_texture;
    additionalBarriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    additionalBarriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_RESOLVE_DEST;
    additionalBarriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    // Apply the additional barriers
    gbuffer_common->gbuffer_directx12.command_list->lpVtbl->ResourceBarrier(gbuffer_common->gbuffer_directx12.command_list, 2, additionalBarriers);

    // Perform the resolve operations
    gbuffer_common->gbuffer_directx12.command_list->lpVtbl->ResolveSubresource(gbuffer_common->gbuffer_directx12.command_list, gbuffer_common->gbuffer_directx12.color_texture, 0, gbuffer_common->gbuffer_directx12.multisample_color_texture, 0, DXGI_FORMAT_R8G8B8A8_UNORM);

    // Add additional barriers for the normal texture
    D3D12_RESOURCE_BARRIER normalBarriers[2];
    memset(normalBarriers, 0, sizeof(normalBarriers));

    // Transition multisample normal texture to RESOLVE_SOURCE
    normalBarriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    normalBarriers[0].Transition.pResource = gbuffer_common->gbuffer_directx12.multisample_normal_texture;
    normalBarriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    normalBarriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
    normalBarriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    // Transition normal texture to RESOLVE_DEST
    normalBarriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    normalBarriers[1].Transition.pResource = gbuffer_common->gbuffer_directx12.normal_texture;
    normalBarriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    normalBarriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_RESOLVE_DEST;
    normalBarriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    // Apply the normal texture barriers
    gbuffer_common->gbuffer_directx12.command_list->lpVtbl->ResourceBarrier(gbuffer_common->gbuffer_directx12.command_list, 2, normalBarriers);

    gbuffer_common->gbuffer_directx12.command_list->lpVtbl->ResolveSubresource(gbuffer_common->gbuffer_directx12.command_list, gbuffer_common->gbuffer_directx12.normal_texture, 0, gbuffer_common->gbuffer_directx12.multisample_normal_texture, 0, DXGI_FORMAT_R8G8B8A8_UNORM);

    // Add post-resolve barriers to transition resources back
    D3D12_RESOURCE_BARRIER postResolveBarriers[4];
    memset(postResolveBarriers, 0, sizeof(postResolveBarriers));

    // Transition multisample color texture back to PIXEL_SHADER_RESOURCE
    postResolveBarriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    postResolveBarriers[0].Transition.pResource = gbuffer_common->gbuffer_directx12.multisample_color_texture;
    postResolveBarriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
    postResolveBarriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    postResolveBarriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    // Transition color texture back to PIXEL_SHADER_RESOURCE
    postResolveBarriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    postResolveBarriers[1].Transition.pResource = gbuffer_common->gbuffer_directx12.color_texture;
    postResolveBarriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_RESOLVE_DEST;
    postResolveBarriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    postResolveBarriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    // Do the same for the normal textures
    postResolveBarriers[2].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    postResolveBarriers[2].Transition.pResource = gbuffer_common->gbuffer_directx12.multisample_normal_texture;
    postResolveBarriers[2].Transition.StateBefore = D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
    postResolveBarriers[2].Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    postResolveBarriers[2].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    postResolveBarriers[3].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    postResolveBarriers[3].Transition.pResource = gbuffer_common->gbuffer_directx12.normal_texture;
    postResolveBarriers[3].Transition.StateBefore = D3D12_RESOURCE_STATE_RESOLVE_DEST;
    postResolveBarriers[3].Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    postResolveBarriers[3].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    // Apply the post-resolve barriers
    gbuffer_common->gbuffer_directx12.command_list->lpVtbl->ResourceBarrier(gbuffer_common->gbuffer_directx12.command_list, 4, postResolveBarriers);
  } else
    gbuffer_common->gbuffer_directx12.command_list->lpVtbl->ResourceBarrier(gbuffer_common->gbuffer_directx12.command_list, 3, barriers);

  // Close the command list
  gbuffer_common->gbuffer_directx12.command_list->lpVtbl->Close(gbuffer_common->gbuffer_directx12.command_list);

  // Execute the command list
  ID3D12CommandList* pp_command_lists[] = {(ID3D12CommandList*)gbuffer_common->gbuffer_directx12.command_list};
  api_common->directx_12_api.command_queue->lpVtbl->ExecuteCommandLists(api_common->directx_12_api.command_queue, _countof(pp_command_lists), pp_command_lists);

  // Wait until frame commands are complete. This can be optimized by checking fence values or using a more advanced pattern.
  api_common->directx_12_api.command_queue->lpVtbl->Signal(api_common->directx_12_api.command_queue, gbuffer_common->gbuffer_directx12.fence, 1);
  if (gbuffer_common->gbuffer_directx12.fence->lpVtbl->GetCompletedValue(gbuffer_common->gbuffer_directx12.fence) < 1) {
    gbuffer_common->gbuffer_directx12.fence->lpVtbl->SetEventOnCompletion(gbuffer_common->gbuffer_directx12.fence, 1, gbuffer_common->gbuffer_directx12.fence_event);
    WaitForSingleObject(gbuffer_common->gbuffer_directx12.fence_event, INFINITE);
  }

  return 0;
}
