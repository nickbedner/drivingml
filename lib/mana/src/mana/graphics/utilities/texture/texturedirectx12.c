#include "mana/graphics/utilities/texture/texturedirectx12.h"

static void texture_directx_12_upload_texture_data(struct APICommon *api_common, ID3D12GraphicsCommandList *command_list, ID3D12Resource *texture_resource, DXGI_FORMAT format, void *data, uint32_t width, uint32_t height, uint8_t bytes_per_channel, uint8_t channels) {
  // uint32_t aligned_row_pitch = (width * channels + 255) & ~(uint32_t)255; // Round up to the nearest multiple of 256
  uint32_t bytes_per_pixel = bytes_per_channel * channels;
  uint32_t row_pitch = width * bytes_per_pixel;
  uint32_t aligned_row_pitch = (row_pitch + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1);
  uint64_t buffer_size = (uint64_t)aligned_row_pitch * height * bytes_per_channel;

  // Create an upload heap buffer, not a texture
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

  ID3D12Resource *upload_heap;

  D3D12_HEAP_PROPERTIES heap_properties;
  heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;  // Or any other value of D3D12_HEAP_TYPE
  heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
  heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
  heap_properties.CreationNodeMask = 1;
  heap_properties.VisibleNodeMask = 1;

  if (FAILED(api_common->directx_12_api.device->lpVtbl->CreateCommittedResource(api_common->directx_12_api.device, &heap_properties, D3D12_HEAP_FLAG_NONE, &buffer_desc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, &IID_ID3D12Resource, (void **)&upload_heap)))
    return;

  // Map and copy the data to the upload heap buffer
  uint8_t *mapped_data;
  D3D12_RANGE range = {0, buffer_size};
  HRESULT hr = upload_heap->lpVtbl->Map(upload_heap, 0, &range, (void **)&mapped_data);
  if (SUCCEEDED(hr)) {
    uint8_t *src_data = (uint8_t *)data;                        // Use a byte pointer for source data
    uint32_t row_bytes = width * channels * bytes_per_channel;  // Bytes per row

    for (uint32_t y = 0; y < height; ++y) {
      memcpy(mapped_data, src_data, row_bytes);
      src_data += row_bytes;
      mapped_data += aligned_row_pitch;
    }
    upload_heap->lpVtbl->Unmap(upload_heap, 0, NULL);
  }

  // Copy data from upload heap buffer to the texture
  D3D12_TEXTURE_COPY_LOCATION src = {0};
  src.pResource = upload_heap;
  src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
  src.PlacedFootprint.Footprint.Format = format;
  src.PlacedFootprint.Footprint.Width = width;
  src.PlacedFootprint.Footprint.Height = height;
  src.PlacedFootprint.Footprint.Depth = 1;
  src.PlacedFootprint.Footprint.RowPitch = aligned_row_pitch;

  D3D12_TEXTURE_COPY_LOCATION dst = {0};
  dst.pResource = texture_resource;
  dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
  dst.SubresourceIndex = 0;

  // Resetting the command list before recording
  hr = api_common->directx_12_api.command_list->lpVtbl->Reset(api_common->directx_12_api.command_list, api_common->directx_12_api.command_allocator, NULL);
  if (FAILED(hr)) {
    log_message(LOG_SEVERITY_ERROR, "Failed to reset command allocator");
    return;
  }

  command_list->lpVtbl->CopyTextureRegion(command_list, &dst, 0, 0, 0, &src, NULL);

  // Add commands to transition the texture resource to be a shader resource
  D3D12_RESOURCE_BARRIER barrier = {0};
  barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrier.Transition.pResource = texture_resource;
  barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
  barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
  barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

  command_list->lpVtbl->ResourceBarrier(command_list, 1, &barrier);

  hr = api_common->directx_12_api.command_list->lpVtbl->Close(api_common->directx_12_api.command_list);
  if (FAILED(hr)) {
    log_message(LOG_SEVERITY_ERROR, "Failed to close command list");
    return;
  }

  ID3D12CommandList *pp_command_lists[] = {(ID3D12CommandList *)api_common->directx_12_api.command_list};
  api_common->directx_12_api.command_queue->lpVtbl->ExecuteCommandLists(api_common->directx_12_api.command_queue, 1, pp_command_lists);

  // Increment the fence value.
  api_common->directx_12_api.fence_value++;

  // Signal the fence when GPU work is done.
  api_common->directx_12_api.command_queue->lpVtbl->Signal(api_common->directx_12_api.command_queue, api_common->directx_12_api.fence, api_common->directx_12_api.fence_value);

  // Check the current fence value. If it's less than our fence value, wait for the GPU.
  if (api_common->directx_12_api.fence->lpVtbl->GetCompletedValue(api_common->directx_12_api.fence) < api_common->directx_12_api.fence_value) {
    // Make the fence signal the event handle when GPU work is done.
    api_common->directx_12_api.fence->lpVtbl->SetEventOnCompletion(api_common->directx_12_api.fence, api_common->directx_12_api.fence_value, api_common->directx_12_api.fence_event);

    // Wait for the event to be signaled.
    WaitForSingleObject(api_common->directx_12_api.fence_event, INFINITE);
  }
}

uint8_t texture_directx_12_init(struct TextureCommon *texture_common, struct TextureManagerCommon *texture_manager_common, struct APICommon *api_common, struct TextureSettings *texture_settings, void *pixels) {
  D3D12_TEXTURE_ADDRESS_MODE mode;
  switch (texture_settings->mode_type) {
    case (MODE_REPEAT): {
      mode = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
      break;
    }
    case (MODE_MIRRORED_REPEAT): {
      mode = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
      break;
    }
    case (MODE_CLAMP_TO_EDGE): {
      mode = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
      break;
    }
    case (MODE_CLAMP_TO_BORDER): {
      mode = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
      break;
    }
  }

  uint8_t bytes_per_channel = 1;
  uint8_t channels = 4;
  DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
  switch (texture_settings->format_type) {
    case (FORMAT_R8_UNORM): {
      format = DXGI_FORMAT_R8_UNORM;
      bytes_per_channel = 1;
      channels = 1;
      break;
    }
    case (FORMAT_R8G8B8A8_UNORM): {
      format = DXGI_FORMAT_R8G8B8A8_UNORM;
      bytes_per_channel = 1;
      channels = 4;
      break;
    }
    case (FORMAT_R16_UNORM): {
      format = DXGI_FORMAT_R16_UNORM;
      bytes_per_channel = 2;
      channels = 1;
      break;
    }
    case (FORMAT_R16G16B16A16_UNORM): {
      format = DXGI_FORMAT_R16G16B16A16_UNORM;
      bytes_per_channel = 2;
      channels = 4;
      break;
    }
    case (FORMAT_R32_SFLOAT): {
      format = DXGI_FORMAT_R32_FLOAT;
      bytes_per_channel = 4;
      channels = 1;
      break;
    }
    case (FORMAT_R32G32B32A32_SFLOAT): {
      format = DXGI_FORMAT_R32G32B32A32_FLOAT;
      bytes_per_channel = 4;
      channels = 4;
      break;
    }
  }

  D3D12_RESOURCE_DESC texture_desc = {0};
  texture_desc.MipLevels = 1;
  texture_desc.Format = format;
  texture_desc.Width = texture_common->width;
  texture_desc.Height = texture_common->height;
  texture_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
  texture_desc.DepthOrArraySize = 1;
  texture_desc.SampleDesc.Count = 1;
  texture_desc.SampleDesc.Quality = 0;
  texture_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

  D3D12_HEAP_PROPERTIES heap_properties;
  heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;  // Or any other value of D3D12_HEAP_TYPE
  heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
  heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
  heap_properties.CreationNodeMask = 1;
  heap_properties.VisibleNodeMask = 1;

  if (FAILED(api_common->directx_12_api.device->lpVtbl->CreateCommittedResource(api_common->directx_12_api.device, &heap_properties, D3D12_HEAP_FLAG_NONE, &texture_desc, D3D12_RESOURCE_STATE_COPY_DEST, NULL, &IID_ID3D12Resource, (void **)&(texture_common->texture_directx12.texture_resource))))
    return 1;

  texture_directx_12_upload_texture_data(api_common, api_common->directx_12_api.command_list, texture_common->texture_directx12.texture_resource, format, pixels, texture_common->width, texture_common->height, bytes_per_channel, channels);

  D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {0};
  srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  srv_desc.Format = format;
  srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
  srv_desc.Texture2D.MipLevels = 1;

  // Get SRV heap from texture_manager_common and then move down pointer by numerical id in texture_common to get correct cpu/gpu handle for this texture
  texture_common->texture_directx12.cpu_heap_handle.ptr = texture_common->texture_manager_common->texture_manager_directx12.cpu_heap_handle.ptr + (texture_common->id * texture_common->texture_manager_common->texture_manager_directx12.descriptor_size);
  texture_common->texture_directx12.gpu_heap_handle.ptr = texture_common->texture_manager_common->texture_manager_directx12.gpu_heap_handle.ptr + (texture_common->id * texture_common->texture_manager_common->texture_manager_directx12.descriptor_size);

  api_common->directx_12_api.device->lpVtbl->CreateShaderResourceView(api_common->directx_12_api.device, texture_common->texture_directx12.texture_resource, &srv_desc, texture_common->texture_directx12.cpu_heap_handle);

  return 0;
}

void texture_directx_12_delete(struct TextureCommon *texture_common, struct APICommon *api_common) {
}
