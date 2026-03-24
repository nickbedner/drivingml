#include "mana/graphics/apis/apidirectx12.h"

static void api_directx_12_get_hardware_adapter(IDXGIFactory4* pFactory, IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter) {
  *ppAdapter = NULL;
  IDXGIAdapter1* adapter = NULL;
  IDXGIFactory6* factory6 = NULL;

  if (SUCCEEDED(pFactory->lpVtbl->QueryInterface(pFactory, &IID_IDXGIFactory6, (void**)&factory6))) {
    for (UINT adapter_index = 0;; adapter_index++) {
      DXGI_GPU_PREFERENCE gpuPreference = requestHighPerformanceAdapter ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED;
      if (FAILED(factory6->lpVtbl->EnumAdapterByGpuPreference(factory6, adapter_index, gpuPreference, &IID_IDXGIAdapter1, (void**)&adapter)))
        break;

      DXGI_ADAPTER_DESC1 desc;
      adapter->lpVtbl->GetDesc1(adapter, &desc);

      if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        continue;

      if (SUCCEEDED(D3D12CreateDevice((IUnknown*)adapter, D3D_FEATURE_LEVEL_12_0, &IID_ID3D12Device, NULL)))
        break;
    }
  }

  if (adapter == NULL) {
    for (UINT adapter_index = 0;; adapter_index++) {
      if (FAILED(pFactory->lpVtbl->EnumAdapters1(pFactory, adapter_index, &adapter)))
        break;

      DXGI_ADAPTER_DESC1 desc;
      adapter->lpVtbl->GetDesc1(adapter, &desc);

      if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        continue;

      if (SUCCEEDED(D3D12CreateDevice((IUnknown*)adapter, D3D_FEATURE_LEVEL_12_0, &IID_ID3D12Device, NULL)))
        break;
    }
  }

  DXGI_ADAPTER_DESC1 desc;
  adapter->lpVtbl->GetDesc1(adapter, &desc);
  log_message(LOG_SEVERITY_DEBUG, "Selected device: %ls\n", desc.Description);

  *ppAdapter = adapter;
}

static uint_fast8_t api_directx_12_init_wrapper(struct APICommon* api_common) {
  struct DirectX12API* directx_12_api = &(api_common->directx_12_api);

  UINT dxgi_factory_flags = 0;

#ifdef ENABLE_DEBUG_CONTROLLER
  if (SUCCEEDED(D3D12GetDebugInterface(&IID_ID3D12Debug, (void**)&(directx_12_api->debug_controller)))) {
    directx_12_api->debug_controller->lpVtbl->EnableDebugLayer(directx_12_api->debug_controller);
    dxgi_factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
  } else {
    log_message(LOG_SEVERITY_ERROR, "Failed to create DirectX 12 debug interface!\n");
    return DIRECTX_12_API_SETUP_DEBUG_INTERFACE_ERROR;
  }
#endif

  if (FAILED(CreateDXGIFactory2(dxgi_factory_flags, &IID_IDXGIFactory4, (void**)&(directx_12_api->factory)))) {
    log_message(LOG_SEVERITY_ERROR, "Failed to create DirectX 12 factory!\n");
    return DIRECTX_12_API_CREATE_FACTORY_ERROR;
  }

  IDXGIAdapter1* hardware_adapter;
  api_directx_12_get_hardware_adapter(directx_12_api->factory, &hardware_adapter, true);

  if (FAILED(D3D12CreateDevice((IUnknown*)hardware_adapter, D3D_FEATURE_LEVEL_12_0, &IID_ID3D12Device, (void**)&(directx_12_api->device))))
    return DIRECTX_12_API_CREATE_DEVICE_ERROR;

  D3D12_COMMAND_QUEUE_DESC queue_desc;
  memset(&queue_desc, 0, sizeof(queue_desc));
  queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

  if (FAILED(directx_12_api->device->lpVtbl->CreateCommandQueue(directx_12_api->device, &queue_desc, &IID_ID3D12CommandQueue, (void**)&(directx_12_api->command_queue))))
    return DIRECTX_12_API_CREATE_COMMAND_QUEUE_ERROR;

  if (FAILED(directx_12_api->device->lpVtbl->CreateCommandAllocator(directx_12_api->device, D3D12_COMMAND_LIST_TYPE_DIRECT, &IID_ID3D12CommandAllocator, (void**)&(directx_12_api->command_allocator))))
    return 1;

  if (FAILED(directx_12_api->device->lpVtbl->CreateCommandList(directx_12_api->device, 0, D3D12_COMMAND_LIST_TYPE_DIRECT, directx_12_api->command_allocator, NULL, &IID_ID3D12GraphicsCommandList, (void**)&(directx_12_api->command_list))))
    return 1;

  if (FAILED(directx_12_api->command_list->lpVtbl->Close(directx_12_api->command_list)))
    return 1;

  HRESULT hr = directx_12_api->device->lpVtbl->CreateFence(directx_12_api->device, 0, D3D12_FENCE_FLAG_NONE, &IID_ID3D12Fence, (void**)&directx_12_api->fence);
  if (FAILED(hr)) {
    log_message(LOG_SEVERITY_ERROR, "Failed to create fence");
    return 1;
  }

  directx_12_api->fence_value = 0;

  directx_12_api->fence_event = CreateEvent(NULL, FALSE, FALSE, NULL);
  if (directx_12_api->fence_event == NULL) {
    log_message(LOG_SEVERITY_ERROR, "Failed to create event handle");
    return 1;
  }

  return DIRECTX_12_API_SUCCESS;
}

uint_fast8_t api_directx_12_init(struct APICommon* api_common) {
  const uint_fast8_t api_directx_12_error = api_directx_12_init_wrapper(api_common);
  switch (api_directx_12_error) {
    case (DIRECTX_12_API_SUCCESS): {
      break;
    }
    case (DIRECTX_12_API_SETUP_DEBUG_INTERFACE_ERROR): {
      log_message(LOG_SEVERITY_ERROR, "Failed to set up DirectX 12 debug interface!\n");
      return API_ERROR;
    }
    case (DIRECTX_12_API_CREATE_FACTORY_ERROR): {
      log_message(LOG_SEVERITY_ERROR, "Failed to create DirectX 12 factory!\n");
      return API_ERROR;
    }
    case (DIRECTX_12_API_CREATE_DEVICE_ERROR): {
      log_message(LOG_SEVERITY_ERROR, "Failed to create DirectX 12 device!\n");
      return API_ERROR;
    }
    case (DIRECTX_12_API_CREATE_COMMAND_QUEUE_ERROR): {
      log_message(LOG_SEVERITY_ERROR, "Failed to create DirectX 12 command queue!\n");
      return API_ERROR;
    }
    default: {
      log_message(LOG_SEVERITY_CRITICAL, "Unknown DirectX 12 state error! Error code: %d\n", api_directx_12_error);
      return API_ERROR;
    }
  }
  return api_directx_12_error;
}

void api_directx_12_delete(struct APICommon* api_common) {
  struct DirectX12API* directx_12_api = &(api_common->directx_12_api);

  directx_12_api->command_queue->lpVtbl->Release(directx_12_api->command_queue);
  directx_12_api->device->lpVtbl->Release(directx_12_api->device);
  directx_12_api->factory->lpVtbl->Release(directx_12_api->factory);
#ifdef ENABLE_DEBUG_CONTROLLER
  directx_12_api->debug_controller->lpVtbl->Release(directx_12_api->debug_controller);
#endif
}

void directx_12_graphics_utils_poll_debug_messages(struct DirectX12API* directx_12_api) {
#ifdef ENABLE_DEBUG_CONTROLLER
  ID3D12InfoQueue* info_queue;
  HRESULT hr = directx_12_api->device->lpVtbl->QueryInterface(directx_12_api->device, &IID_ID3D12InfoQueue, (void**)&info_queue);
  if (FAILED(hr)) {
    log_message(LOG_SEVERITY_ERROR, "Failed to query interface for ID3D12InfoQueue!\n");
    return;
  }

  char message_buffer[8192];
  D3D12_MESSAGE* message = (D3D12_MESSAGE*)message_buffer;

  UINT64 messageCount = info_queue->lpVtbl->GetNumStoredMessages(info_queue);

  for (UINT64 i = 0; i < messageCount; ++i) {
    SIZE_T messageLength = 8192;

    // Retrieve the message
    hr = info_queue->lpVtbl->GetMessageW(info_queue, i, message, &messageLength);
    if (SUCCEEDED(hr)) {
      // Print the message
      // printf("D3D12 Debug Message: %s\n", message->pDescription);
      log_message(LOG_SEVERITY_ERROR, "D3D12 Debug Message: %s\n", message->pDescription);
    }
  }

  // Clear stored messages
  info_queue->lpVtbl->ClearStoredMessages(info_queue);

  // Note: Checking if device is toast and what the reason for removal was
  hr = directx_12_api->device->lpVtbl->GetDeviceRemovedReason(directx_12_api->device);
  if (hr != S_OK)
    log_message(LOG_SEVERITY_CRITICAL, "Device removal reason: %d hex code: %#010x\n", hr, hr);
#endif
}

uint_fast8_t directx_12_graphics_utils_setup_vertex_buffer(struct DirectX12API* directx_12_api, struct Vector* vertices, ID3D12Resource** vertex_buffer) {
  UINT64 vertex_buffer_size = vertices->memory_size * vertices->size;

  ID3D12Resource* vertex_staging_buffer;

  // Setting up heap properties for UPLOAD
  D3D12_HEAP_PROPERTIES upload_heap_properties;
  upload_heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
  upload_heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
  upload_heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
  upload_heap_properties.CreationNodeMask = 1;  // Single GPU
  upload_heap_properties.VisibleNodeMask = 1;   // Single GPU

  // Setting up resource description for the buffer
  D3D12_RESOURCE_DESC buffer_desc;
  buffer_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
  buffer_desc.Alignment = 0;
  buffer_desc.Width = vertex_buffer_size;
  buffer_desc.Height = 1;
  buffer_desc.DepthOrArraySize = 1;
  buffer_desc.MipLevels = 1;
  buffer_desc.Format = DXGI_FORMAT_UNKNOWN;
  buffer_desc.SampleDesc.Count = 1;
  buffer_desc.SampleDesc.Quality = 0;
  buffer_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
  buffer_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

  HRESULT hr = directx_12_api->device->lpVtbl->CreateCommittedResource(directx_12_api->device, &upload_heap_properties, D3D12_HEAP_FLAG_NONE, &buffer_desc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, &IID_ID3D12Resource, (void**)&vertex_staging_buffer);
  if (FAILED(hr)) {
    log_message(LOG_SEVERITY_ERROR, "Failed to create vertex staging buffer!\n");
    return 1;
  }

  BYTE* vertex_data;
  vertex_staging_buffer->lpVtbl->Map(vertex_staging_buffer, 0, NULL, (void**)(&vertex_data));
  memcpy(vertex_data, vertices->items, vertex_buffer_size);
  vertex_staging_buffer->lpVtbl->Unmap(vertex_staging_buffer, 0, NULL);

  hr = directx_12_api->command_list->lpVtbl->Reset(directx_12_api->command_list, directx_12_api->command_allocator, NULL);
  if (FAILED(hr)) {
    log_message(LOG_SEVERITY_ERROR, "Failed to reset command allocator");
    return 1;
  }

  // Setting up heap properties for DEFAULT
  D3D12_HEAP_PROPERTIES default_heap_properties;
  default_heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;
  default_heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
  default_heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
  default_heap_properties.CreationNodeMask = 1;  // Single GPU
  default_heap_properties.VisibleNodeMask = 1;   // Single GPU

  directx_12_api->device->lpVtbl->CreateCommittedResource(directx_12_api->device, &default_heap_properties, D3D12_HEAP_FLAG_NONE, &buffer_desc, D3D12_RESOURCE_STATE_COMMON, NULL, &IID_ID3D12Resource, (void**)vertex_buffer);

  // Transition to COPY_DEST state
  D3D12_RESOURCE_BARRIER barrier = {};
  barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrier.Transition.pResource = *vertex_buffer;
  barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
  barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
  barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  directx_12_api->command_list->lpVtbl->ResourceBarrier(directx_12_api->command_list, 1, &barrier);

  directx_12_api->command_list->lpVtbl->CopyBufferRegion(directx_12_api->command_list, *vertex_buffer, 0, vertex_staging_buffer, 0, vertex_buffer_size);

  hr = directx_12_api->command_list->lpVtbl->Close(directx_12_api->command_list);
  if (FAILED(hr)) {
    log_message(LOG_SEVERITY_ERROR, "Failed to close command list");
    return 1;
  }

  ID3D12CommandList* pp_command_lists[] = {(ID3D12CommandList*)directx_12_api->command_list};
  directx_12_api->command_queue->lpVtbl->ExecuteCommandLists(directx_12_api->command_queue, 1, pp_command_lists);

  // Increment the fence value.
  directx_12_api->fence_value++;

  // Signal the fence when GPU work is done.
  directx_12_api->command_queue->lpVtbl->Signal(directx_12_api->command_queue, directx_12_api->fence, directx_12_api->fence_value);

  // Check the current fence value. If it's less than our fence value, wait for the GPU.
  if (directx_12_api->fence->lpVtbl->GetCompletedValue(directx_12_api->fence) < directx_12_api->fence_value) {
    // Make the fence signal the event handle when GPU work is done.
    directx_12_api->fence->lpVtbl->SetEventOnCompletion(directx_12_api->fence, directx_12_api->fence_value, directx_12_api->fence_event);

    // Wait for the event to be signaled.
    WaitForSingleObject(directx_12_api->fence_event, INFINITE);
  }

  vertex_staging_buffer->lpVtbl->Release(vertex_staging_buffer);

  return 0;
}

uint_fast8_t directx_12_graphics_utils_setup_index_buffer(struct DirectX12API* directx_12_api, struct Vector* indices, ID3D12Resource** index_buffer) {
  UINT64 index_buffer_size = indices->memory_size * indices->size;

  ID3D12Resource* index_staging_buffer;

  // Setting up heap properties for UPLOAD
  D3D12_HEAP_PROPERTIES upload_heap_properties;
  upload_heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
  upload_heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
  upload_heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
  upload_heap_properties.CreationNodeMask = 1;  // Single GPU
  upload_heap_properties.VisibleNodeMask = 1;   // Single GPU

  // Setting up resource description for the buffer
  D3D12_RESOURCE_DESC buffer_desc;
  buffer_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
  buffer_desc.Alignment = 0;
  buffer_desc.Width = index_buffer_size;
  buffer_desc.Height = 1;
  buffer_desc.DepthOrArraySize = 1;
  buffer_desc.MipLevels = 1;
  buffer_desc.Format = DXGI_FORMAT_UNKNOWN;
  buffer_desc.SampleDesc.Count = 1;
  buffer_desc.SampleDesc.Quality = 0;
  buffer_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
  buffer_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

  directx_12_api->device->lpVtbl->CreateCommittedResource(directx_12_api->device, &upload_heap_properties, D3D12_HEAP_FLAG_NONE, &buffer_desc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, &IID_ID3D12Resource, (void**)&index_staging_buffer);

  BYTE* index_data;
  index_staging_buffer->lpVtbl->Map(index_staging_buffer, 0, NULL, (void**)(&index_data));
  memcpy(index_data, indices->items, index_buffer_size);
  index_staging_buffer->lpVtbl->Unmap(index_staging_buffer, 0, NULL);

  // Resetting the command list before recording
  HRESULT hr = directx_12_api->command_list->lpVtbl->Reset(directx_12_api->command_list, directx_12_api->command_allocator, NULL);
  if (FAILED(hr)) {
    log_message(LOG_SEVERITY_ERROR, "Failed to reset command allocator");
    return 1;
  }

  // Setting up heap properties for DEFAULT
  D3D12_HEAP_PROPERTIES default_heap_properties;
  default_heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;
  default_heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
  default_heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
  default_heap_properties.CreationNodeMask = 1;  // Single GPU
  default_heap_properties.VisibleNodeMask = 1;   // Single GPU

  directx_12_api->device->lpVtbl->CreateCommittedResource(directx_12_api->device, &default_heap_properties, D3D12_HEAP_FLAG_NONE, &buffer_desc, D3D12_RESOURCE_STATE_COMMON, NULL, &IID_ID3D12Resource, (void**)index_buffer);

  // Transition to COPY_DEST state
  D3D12_RESOURCE_BARRIER barrier = {};
  barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrier.Transition.pResource = *index_buffer;
  barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
  barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
  barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  directx_12_api->command_list->lpVtbl->ResourceBarrier(directx_12_api->command_list, 1, &barrier);

  directx_12_api->command_list->lpVtbl->CopyBufferRegion(directx_12_api->command_list, *index_buffer, 0, index_staging_buffer, 0, index_buffer_size);

  // Close the command list after recording
  hr = directx_12_api->command_list->lpVtbl->Close(directx_12_api->command_list);
  if (FAILED(hr)) {
    log_message(LOG_SEVERITY_ERROR, "Failed to close command list");
    return 1;
  }

  ID3D12CommandList* pp_command_lists[] = {(ID3D12CommandList*)directx_12_api->command_list};
  directx_12_api->command_queue->lpVtbl->ExecuteCommandLists(directx_12_api->command_queue, 1, pp_command_lists);

  // Increment the fence value.
  directx_12_api->fence_value++;

  // Signal the fence when GPU work is done.
  directx_12_api->command_queue->lpVtbl->Signal(directx_12_api->command_queue, directx_12_api->fence, directx_12_api->fence_value);

  // Check the current fence value. If it's less than our fence value, wait for the GPU.
  if (directx_12_api->fence->lpVtbl->GetCompletedValue(directx_12_api->fence) < directx_12_api->fence_value) {
    // Make the fence signal the event handle when GPU work is done.
    directx_12_api->fence->lpVtbl->SetEventOnCompletion(directx_12_api->fence, directx_12_api->fence_value, directx_12_api->fence_event);

    // Wait for the event to be signaled.
    WaitForSingleObject(directx_12_api->fence_event, INFINITE);
  }

  index_staging_buffer->lpVtbl->Release(index_staging_buffer);

  return 0;
}

uint_fast8_t directx_12_graphics_utils_setup_constant_buffer(struct DirectX12API* directx_12_api, UINT64 buffer_size, ID3D12Resource** constant_buffer) {
  // Define heap properties for UPLOAD. Constant buffers typically reside in UPLOAD heap for efficiency.
  D3D12_HEAP_PROPERTIES upload_heap_properties;
  upload_heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
  upload_heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
  upload_heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
  upload_heap_properties.CreationNodeMask = 1;  // Single GPU
  upload_heap_properties.VisibleNodeMask = 1;   // Single GPU

  // Set up the resource description for the buffer
  D3D12_RESOURCE_DESC buffer_desc;
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

  HRESULT hr = directx_12_api->device->lpVtbl->CreateCommittedResource(directx_12_api->device, &upload_heap_properties, D3D12_HEAP_FLAG_NONE, &buffer_desc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, &IID_ID3D12Resource, (void**)constant_buffer);
  if (FAILED(hr)) {
    log_message(LOG_SEVERITY_ERROR, "Failed to create constant buffer");
    return 1;
  }

  // Note: Unlike index and vertex buffers which you might update and then copy to a default buffer,
  // constant buffers are typically kept in UPLOAD heap for its ability to allow CPU updates and efficient reads by the GPU.
  // Hence, you don't usually need the staging buffer -> default buffer approach that you took with the index buffer.
  return 0;
}
