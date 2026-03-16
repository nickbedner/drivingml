#include "mana/graphics/utilities/texture/texturedirectx12.h"

static void texture_directx_12_upload_texture_data(struct APICommon* api_common, ID3D12GraphicsCommandList* command_list, ID3D12Resource* texture_resource, DXGI_FORMAT format, void* data, uint32_t width, uint32_t height, uint8_t bytes_per_channel, uint8_t channels) {
  uint32_t bytes_per_pixel = bytes_per_channel * channels;
  uint32_t row_pitch = width * bytes_per_pixel;
  uint32_t aligned_row_pitch = (row_pitch + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1);
  uint64_t buffer_size = (uint64_t)aligned_row_pitch * height;

  D3D12_RESOURCE_DESC buffer_desc = {0};
  buffer_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
  buffer_desc.Alignment = 0;
  buffer_desc.Width = buffer_size;
  buffer_desc.Height = 1;
  buffer_desc.DepthOrArraySize = 1;
  buffer_desc.MipLevels = 1;
  buffer_desc.Format = DXGI_FORMAT_UNKNOWN;
  buffer_desc.SampleDesc.Count = 1;
  buffer_desc.SampleDesc.Quality = 0;
  buffer_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
  buffer_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

  ID3D12Resource* upload_heap = NULL;

  D3D12_HEAP_PROPERTIES heap_properties = {0};
  heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
  heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
  heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
  heap_properties.CreationNodeMask = 1;
  heap_properties.VisibleNodeMask = 1;

  if (FAILED(api_common->directx_12_api.device->lpVtbl->CreateCommittedResource(api_common->directx_12_api.device, &heap_properties, D3D12_HEAP_FLAG_NONE, &buffer_desc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, &IID_ID3D12Resource, (void**)&upload_heap)))
    return;

  uint8_t* mapped_data = NULL;
  D3D12_RANGE range = {0, buffer_size};
  HRESULT hr = upload_heap->lpVtbl->Map(upload_heap, 0, &range, (void**)&mapped_data);
  if (FAILED(hr)) {
    upload_heap->lpVtbl->Release(upload_heap);
    return;
  }

  uint8_t* src_data = (uint8_t*)data;
  uint32_t row_bytes = width * channels * bytes_per_channel;

  for (uint32_t y = 0; y < height; ++y) {
    memcpy(mapped_data, src_data, row_bytes);
    src_data += row_bytes;
    mapped_data += aligned_row_pitch;
  }

  upload_heap->lpVtbl->Unmap(upload_heap, 0, NULL);

  D3D12_TEXTURE_COPY_LOCATION src = {0};
  src.pResource = upload_heap;
  src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
  src.PlacedFootprint.Offset = 0;
  src.PlacedFootprint.Footprint.Format = format;
  src.PlacedFootprint.Footprint.Width = width;
  src.PlacedFootprint.Footprint.Height = height;
  src.PlacedFootprint.Footprint.Depth = 1;
  src.PlacedFootprint.Footprint.RowPitch = aligned_row_pitch;

  D3D12_TEXTURE_COPY_LOCATION dst = {0};
  dst.pResource = texture_resource;
  dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
  dst.SubresourceIndex = 0;

  hr = api_common->directx_12_api.command_allocator->lpVtbl->Reset(api_common->directx_12_api.command_allocator);
  if (FAILED(hr)) {
    log_message(LOG_SEVERITY_ERROR, "Failed to reset command allocator");
    upload_heap->lpVtbl->Release(upload_heap);
    return;
  }

  hr = api_common->directx_12_api.command_list->lpVtbl->Reset(api_common->directx_12_api.command_list, api_common->directx_12_api.command_allocator, NULL);
  if (FAILED(hr)) {
    log_message(LOG_SEVERITY_ERROR, "Failed to reset command list");
    upload_heap->lpVtbl->Release(upload_heap);
    return;
  }

  command_list->lpVtbl->CopyTextureRegion(command_list, &dst, 0, 0, 0, &src, NULL);

  D3D12_RESOURCE_BARRIER barrier = {0};
  barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  barrier.Transition.pResource = texture_resource;
  barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
  barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
  barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  command_list->lpVtbl->ResourceBarrier(command_list, 1, &barrier);

  hr = api_common->directx_12_api.command_list->lpVtbl->Close(api_common->directx_12_api.command_list);
  if (FAILED(hr)) {
    log_message(LOG_SEVERITY_ERROR, "Failed to close command list");
    upload_heap->lpVtbl->Release(upload_heap);
    return;
  }

  ID3D12CommandList* pp_command_lists[] = {(ID3D12CommandList*)api_common->directx_12_api.command_list};
  api_common->directx_12_api.command_queue->lpVtbl->ExecuteCommandLists(api_common->directx_12_api.command_queue, 1, pp_command_lists);

  api_common->directx_12_api.fence_value++;
  api_common->directx_12_api.command_queue->lpVtbl->Signal(api_common->directx_12_api.command_queue, api_common->directx_12_api.fence, api_common->directx_12_api.fence_value);

  if (api_common->directx_12_api.fence->lpVtbl->GetCompletedValue(api_common->directx_12_api.fence) < api_common->directx_12_api.fence_value) {
    api_common->directx_12_api.fence->lpVtbl->SetEventOnCompletion(api_common->directx_12_api.fence, api_common->directx_12_api.fence_value, api_common->directx_12_api.fence_event);
    WaitForSingleObject(api_common->directx_12_api.fence_event, INFINITE);
  }

  upload_heap->lpVtbl->Release(upload_heap);
}

uint8_t texture_directx_12_init(struct TextureCommon* texture_common, struct TextureManagerCommon* texture_manager_common, struct APICommon* api_common, void* pixels) {
  struct TextureSettings texture_settings = texture_common->texture_settings;

  D3D12_TEXTURE_ADDRESS_MODE mode = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
  switch (texture_settings.mode_type) {
    case MODE_REPEAT:
      mode = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
      break;
    case MODE_MIRRORED_REPEAT:
      mode = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
      break;
    case MODE_CLAMP_TO_EDGE:
      mode = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
      break;
    case MODE_CLAMP_TO_BORDER:
      mode = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
      break;
    default:
      mode = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
      break;
  }

  uint8_t bytes_per_channel = 1;
  uint8_t channels = 4;
  DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
  switch (texture_settings.format_type) {
    case FORMAT_R8_UNORM:
      format = DXGI_FORMAT_R8_UNORM;
      bytes_per_channel = 1;
      channels = 1;
      break;
    case FORMAT_R8G8B8A8_UNORM:
      format = DXGI_FORMAT_R8G8B8A8_UNORM;
      bytes_per_channel = 1;
      channels = 4;
      break;
    case FORMAT_R16_UNORM:
      format = DXGI_FORMAT_R16_UNORM;
      bytes_per_channel = 2;
      channels = 1;
      break;
    case FORMAT_R16G16B16A16_UNORM:
      format = DXGI_FORMAT_R16G16B16A16_UNORM;
      bytes_per_channel = 2;
      channels = 4;
      break;
    case FORMAT_R32_SFLOAT:
      format = DXGI_FORMAT_R32_FLOAT;
      bytes_per_channel = 4;
      channels = 1;
      break;
    case FORMAT_R32G32B32A32_SFLOAT:
      format = DXGI_FORMAT_R32G32B32A32_FLOAT;
      bytes_per_channel = 4;
      channels = 4;
      break;
    default:
      format = DXGI_FORMAT_R8G8B8A8_UNORM;
      bytes_per_channel = 1;
      channels = 4;
      break;
  }

  uint32_t mip_levels = 1;
  if (texture_settings.mip_type == MIP_GENERATE) {
    uint32_t max_dim = texture_common->width > texture_common->height ? texture_common->width : texture_common->height;
    mip_levels = (uint32_t)(floor(log2((double)max_dim))) + 1;
  } else if (texture_settings.mip_type == MIP_CUSTOM) {
    mip_levels = texture_settings.mip_count;
  }

  if (texture_settings.mip_type == MIP_NONE)
    mip_levels = 1;

  D3D12_FILTER filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
  switch (texture_settings.filter_type) {
    case FILTER_NEAREST:
      filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
      break;
    case FILTER_BILINEAR:
      filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
      break;
    case FILTER_TRILINEAR:
      filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
      break;
    case FILTER_ANISOTROPIC:
      filter = D3D12_FILTER_ANISOTROPIC;
      break;
    default:
      filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
      break;
  }

  texture_common->texture_directx12.sampler_cpu_handle.ptr = texture_common->texture_manager_common->texture_manager_directx12.sampler_cpu_heap_handle.ptr + (texture_common->id * texture_common->texture_manager_common->texture_manager_directx12.sampler_descriptor_size);
  texture_common->texture_directx12.sampler_gpu_handle.ptr = texture_common->texture_manager_common->texture_manager_directx12.sampler_gpu_heap_handle.ptr + (texture_common->id * texture_common->texture_manager_common->texture_manager_directx12.sampler_descriptor_size);

  D3D12_SAMPLER_DESC sampler_desc = {0};
  sampler_desc.Filter = filter;
  sampler_desc.AddressU = mode;
  sampler_desc.AddressV = mode;
  sampler_desc.AddressW = mode;
  sampler_desc.MipLODBias = 0.0f;
  sampler_desc.MaxAnisotropy = (filter == D3D12_FILTER_ANISOTROPIC) ? (UINT)texture_settings.max_anisotropy : 1;
  sampler_desc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
  sampler_desc.MinLOD = 0.0f;
  sampler_desc.MaxLOD = (FLOAT)(mip_levels - 1);
  sampler_desc.BorderColor[0] = 0.0f;
  sampler_desc.BorderColor[1] = 0.0f;
  sampler_desc.BorderColor[2] = 0.0f;
  sampler_desc.BorderColor[3] = 0.0f;

  api_common->directx_12_api.device->lpVtbl->CreateSampler(api_common->directx_12_api.device, &sampler_desc, texture_common->texture_directx12.sampler_cpu_handle);

  D3D12_RESOURCE_DESC texture_desc = {0};
  texture_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  texture_desc.Alignment = 0;
  texture_desc.Width = texture_common->width;
  texture_desc.Height = texture_common->height;
  texture_desc.DepthOrArraySize = 1;
  texture_desc.MipLevels = (UINT16)mip_levels;
  texture_desc.Format = format;
  texture_desc.SampleDesc.Count = 1;
  texture_desc.SampleDesc.Quality = 0;
  texture_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
  texture_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

  D3D12_HEAP_PROPERTIES heap_properties = {0};
  heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;
  heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
  heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
  heap_properties.CreationNodeMask = 1;
  heap_properties.VisibleNodeMask = 1;

  if (FAILED(api_common->directx_12_api.device->lpVtbl->CreateCommittedResource(api_common->directx_12_api.device, &heap_properties, D3D12_HEAP_FLAG_NONE, &texture_desc, D3D12_RESOURCE_STATE_COPY_DEST, NULL, &IID_ID3D12Resource, (void**)&(texture_common->texture_directx12.texture_resource))))
    return 1;

  texture_directx_12_upload_texture_data(api_common, api_common->directx_12_api.command_list, texture_common->texture_directx12.texture_resource, format, pixels, texture_common->width, texture_common->height, bytes_per_channel, channels);

  D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {0};
  srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  srv_desc.Format = format;
  srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
  srv_desc.Texture2D.MostDetailedMip = 0;
  srv_desc.Texture2D.MipLevels = (UINT)mip_levels;
  srv_desc.Texture2D.PlaneSlice = 0;
  srv_desc.Texture2D.ResourceMinLODClamp = 0.0f;

  texture_common->texture_directx12.srv_cpu_handle.ptr = texture_common->texture_manager_common->texture_manager_directx12.srv_cpu_heap_handle.ptr + (texture_common->id * texture_common->texture_manager_common->texture_manager_directx12.srv_descriptor_size);
  texture_common->texture_directx12.srv_gpu_handle.ptr = texture_common->texture_manager_common->texture_manager_directx12.srv_gpu_heap_handle.ptr + (texture_common->id * texture_common->texture_manager_common->texture_manager_directx12.srv_descriptor_size);

  api_common->directx_12_api.device->lpVtbl->CreateShaderResourceView(api_common->directx_12_api.device, texture_common->texture_directx12.texture_resource, &srv_desc, texture_common->texture_directx12.srv_cpu_handle);

  return 0;
}

void texture_directx_12_delete(struct TextureCommon* texture_common, struct APICommon* api_common) {
}
