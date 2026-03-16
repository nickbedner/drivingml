#include "mana/graphics/utilities/texture/texturedirectx12.h"

static uint64_t texture_directx_12_upload_texture_data(struct APICommon* api_common, ID3D12GraphicsCommandList* command_list, ID3D12Resource* texture_resource, DXGI_FORMAT format, const void* data, uint32_t base_width, uint32_t base_height, uint32_t mip_levels, uint8_t bytes_per_channel, uint8_t channels) {
  ID3D12Device* device = api_common->directx_12_api.device;

  D3D12_RESOURCE_DESC tex_desc;
  texture_resource->lpVtbl->GetDesc(texture_resource, &tex_desc);

  uint32_t bytes_per_pixel = bytes_per_channel * channels;
  UINT num_subresources = mip_levels;

  D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layouts = (D3D12_PLACED_SUBRESOURCE_FOOTPRINT*)malloc(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) * num_subresources);
  UINT* num_rows = (UINT*)malloc(sizeof(UINT) * num_subresources);
  UINT64* row_sizes = (UINT64*)malloc(sizeof(UINT64) * num_subresources);
  D3D12_SUBRESOURCE_DATA* subresources = (D3D12_SUBRESOURCE_DATA*)malloc(sizeof(D3D12_SUBRESOURCE_DATA) * num_subresources);

  if (!layouts || !num_rows || !row_sizes || !subresources) {
    free(layouts);
    free(num_rows);
    free(row_sizes);
    free(subresources);
    return 1;
  }

  UINT64 upload_size = 0;
  device->lpVtbl->GetCopyableFootprints(device, &tex_desc, 0, num_subresources, 0, layouts, num_rows, row_sizes, &upload_size);

  const uint8_t* src = (const uint8_t*)data;
  for (UINT mip = 0; mip < num_subresources; ++mip) {
    uint32_t w = base_width >> mip;
    uint32_t h = base_height >> mip;
    if (w == 0) w = 1;
    if (h == 0) h = 1;

    subresources[mip].pData = src;
    subresources[mip].RowPitch = (LONG_PTR)(w * bytes_per_pixel);
    subresources[mip].SlicePitch = (LONG_PTR)(w * h * bytes_per_pixel);

    src += (size_t)(w * h * bytes_per_pixel);
  }

  D3D12_HEAP_PROPERTIES heap_props = {0};
  heap_props.Type = D3D12_HEAP_TYPE_UPLOAD;
  heap_props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
  heap_props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
  heap_props.CreationNodeMask = 1;
  heap_props.VisibleNodeMask = 1;

  D3D12_RESOURCE_DESC upload_desc = {0};
  upload_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
  upload_desc.Alignment = 0;
  upload_desc.Width = upload_size;
  upload_desc.Height = 1;
  upload_desc.DepthOrArraySize = 1;
  upload_desc.MipLevels = 1;
  upload_desc.Format = DXGI_FORMAT_UNKNOWN;
  upload_desc.SampleDesc.Count = 1;
  upload_desc.SampleDesc.Quality = 0;
  upload_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
  upload_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

  ID3D12Resource* upload_heap = NULL;
  HRESULT hr = device->lpVtbl->CreateCommittedResource(device, &heap_props, D3D12_HEAP_FLAG_NONE, &upload_desc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, &IID_ID3D12Resource, (void**)&upload_heap);

  if (FAILED(hr)) {
    free(layouts);
    free(num_rows);
    free(row_sizes);
    free(subresources);
    return 1;
  }

  uint8_t* mapped = NULL;
  D3D12_RANGE read_range = {0, 0};
  hr = upload_heap->lpVtbl->Map(upload_heap, 0, &read_range, (void**)&mapped);
  if (FAILED(hr)) {
    upload_heap->lpVtbl->Release(upload_heap);
    free(layouts);
    free(num_rows);
    free(row_sizes);
    free(subresources);
    return 1;
  }

  for (UINT mip = 0; mip < num_subresources; ++mip) {
    const uint8_t* src_rows = (const uint8_t*)subresources[mip].pData;
    uint8_t* dst_rows = mapped + layouts[mip].Offset;
    UINT64 src_row_pitch = (UINT64)subresources[mip].RowPitch;
    UINT dst_row_pitch = layouts[mip].Footprint.RowPitch;

    for (UINT row = 0; row < num_rows[mip]; ++row)
      memcpy(dst_rows + (size_t)row * dst_row_pitch, src_rows + (size_t)row * src_row_pitch, (size_t)src_row_pitch);
  }

  upload_heap->lpVtbl->Unmap(upload_heap, 0, NULL);

  hr = api_common->directx_12_api.command_allocator->lpVtbl->Reset(api_common->directx_12_api.command_allocator);
  if (FAILED(hr)) {
    upload_heap->lpVtbl->Release(upload_heap);
    free(layouts);
    free(num_rows);
    free(row_sizes);
    free(subresources);
    return 1;
  }

  hr = command_list->lpVtbl->Reset(command_list, api_common->directx_12_api.command_allocator, NULL);
  if (FAILED(hr)) {
    upload_heap->lpVtbl->Release(upload_heap);
    free(layouts);
    free(num_rows);
    free(row_sizes);
    free(subresources);
    return 1;
  }

  for (UINT mip = 0; mip < num_subresources; ++mip) {
    D3D12_TEXTURE_COPY_LOCATION dst = {0};
    dst.pResource = texture_resource;
    dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dst.SubresourceIndex = mip;

    D3D12_TEXTURE_COPY_LOCATION src_loc = {0};
    src_loc.pResource = upload_heap;
    src_loc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    src_loc.PlacedFootprint = layouts[mip];

    command_list->lpVtbl->CopyTextureRegion(command_list, &dst, 0, 0, 0, &src_loc, NULL);
  }

  D3D12_RESOURCE_BARRIER barrier = {0};
  barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
  barrier.Transition.pResource = texture_resource;
  barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
  barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
  barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  command_list->lpVtbl->ResourceBarrier(command_list, 1, &barrier);

  hr = command_list->lpVtbl->Close(command_list);
  if (FAILED(hr)) {
    upload_heap->lpVtbl->Release(upload_heap);
    free(layouts);
    free(num_rows);
    free(row_sizes);
    free(subresources);
    return 1;
  }

  ID3D12CommandList* lists[] = {(ID3D12CommandList*)command_list};
  api_common->directx_12_api.command_queue->lpVtbl->ExecuteCommandLists(api_common->directx_12_api.command_queue, 1, lists);
  api_common->directx_12_api.fence_value++;
  api_common->directx_12_api.command_queue->lpVtbl->Signal(api_common->directx_12_api.command_queue, api_common->directx_12_api.fence, api_common->directx_12_api.fence_value);

  if (api_common->directx_12_api.fence->lpVtbl->GetCompletedValue(api_common->directx_12_api.fence) <
      api_common->directx_12_api.fence_value) {
    api_common->directx_12_api.fence->lpVtbl->SetEventOnCompletion(api_common->directx_12_api.fence, api_common->directx_12_api.fence_value, api_common->directx_12_api.fence_event);
    WaitForSingleObject(api_common->directx_12_api.fence_event, INFINITE);
  }

  upload_heap->lpVtbl->Release(upload_heap);
  free(layouts);
  free(num_rows);
  free(row_sizes);
  free(subresources);
  return 0;
}

uint8_t texture_directx_12_init(struct TextureCommon* texture_common, struct TextureManagerCommon* texture_manager_common, struct APICommon* api_common, void* pixels) {
  struct TextureSettings texture_settings = texture_common->texture_settings;

  void* upload_pixels = pixels;

  if (texture_settings.mip_type == MIP_CUSTOM) {
    uint32_t mip_count = texture_settings.mip_count;
    if (mip_count < 1) mip_count = 1;

    uint32_t bytes_per_channel_local = (texture_common->bit_depth == 16) ? 2 : 1;

    size_t total = 0;
    for (uint32_t level = 0; level < mip_count; ++level) {
      uint32_t w = texture_common->width >> level;
      uint32_t h = texture_common->height >> level;
      if (w == 0) w = 1;
      if (h == 0) h = 1;

      total += (size_t)w * (size_t)h *
               (size_t)texture_common->channels *
               (size_t)bytes_per_channel_local;
    }

    uint8_t* combined = (uint8_t*)malloc(total);
    if (!combined)
      return 1;

    size_t off = 0;
    size_t sz0 = (size_t)texture_common->width *
                 (size_t)texture_common->height *
                 (size_t)texture_common->channels *
                 (size_t)bytes_per_channel_local;

    memcpy(combined + off, pixels, sz0);
    off += sz0;

    for (uint32_t level = 1; level < mip_count; ++level) {
      char* mip_path = build_mip_path(texture_common->path, level);
      if (!mip_path) {
        free(combined);
        return 1;
      }

      uint32_t wl = 0, hl = 0, chl = 0;
      uint8_t bitl = 0, ctl = 0;
      void* pixelsL = texture_read_png(mip_path, api_common->asset_directory, &wl, &hl, &chl, &bitl, &ctl);
      free(mip_path);

      if (!pixelsL) {
        free(combined);
        return 1;
      }

      if (ctl == 2) chl = 4;

      if (bitl != texture_common->bit_depth || chl != texture_common->channels) {
        free(pixelsL);
        free(combined);
        return 1;
      }

      uint32_t exp_w = texture_common->width >> level;
      uint32_t exp_h = texture_common->height >> level;
      if (exp_w == 0) exp_w = 1;
      if (exp_h == 0) exp_h = 1;

      if (wl != exp_w || hl != exp_h) {
        free(pixelsL);
        free(combined);
        return 1;
      }

      size_t sz = (size_t)wl * (size_t)hl *
                  (size_t)texture_common->channels *
                  (size_t)bytes_per_channel_local;

      memcpy(combined + off, pixelsL, sz);
      off += sz;
      free(pixelsL);
    }

    upload_pixels = combined;
  }

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

  if (texture_settings.mip_type == MIP_CUSTOM) {
    mip_levels = texture_settings.mip_count;
    if (mip_levels < 1) mip_levels = 1;
  } else if (texture_settings.mip_type == MIP_NONE) {
    mip_levels = 1;
  } else if (texture_settings.mip_type == MIP_GENERATE) {
    // DX12 mip generation not implemented yet
    mip_levels = 1;
  }

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

  UINT max_aniso = 1;
  if (filter == D3D12_FILTER_ANISOTROPIC) {
    float req = texture_settings.max_anisotropy;
    if (req < 1.0f) req = 1.0f;
    if (req > 16.0f) req = 16.0f;
    max_aniso = (UINT)req;
  }

  D3D12_SAMPLER_DESC sampler_desc = {0};
  sampler_desc.Filter = filter;
  sampler_desc.AddressU = mode;
  sampler_desc.AddressV = mode;
  sampler_desc.AddressW = mode;
  sampler_desc.MipLODBias = 0.0f;
  sampler_desc.MaxAnisotropy = max_aniso;
  sampler_desc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
  sampler_desc.MinLOD = 0.0f;
  sampler_desc.MaxLOD = (mip_levels > 0) ? (FLOAT)(mip_levels - 1) : 0.0f;
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

  if (texture_directx_12_upload_texture_data(api_common, api_common->directx_12_api.command_list, texture_common->texture_directx12.texture_resource, format, upload_pixels, texture_common->width, texture_common->height, mip_levels, bytes_per_channel, channels) != 0)
    return 1;

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

  if (texture_settings.mip_type == MIP_CUSTOM)
    free(upload_pixels);

  return 0;
}

void texture_directx_12_delete(struct TextureCommon* texture_common, struct APICommon* api_common) {
  if (texture_common->texture_directx12.texture_resource)
    texture_common->texture_directx12.texture_resource->lpVtbl->Release(texture_common->texture_directx12.texture_resource);
}
