#include "mana/graphics/render/swapchain/swapchaindirectx12.h"

// Note: This function if here because it is used by both init and resize
static inline uint_fast8_t swap_chain_directx_12_init_common(struct SwapChainCommon* swap_chain_common, struct APICommon* api_common) {
  HRESULT hr;
  for (uint_fast8_t frame = 0; frame < MAX_SWAP_CHAIN_FRAMES; frame++) {
    swap_chain_common->swap_chain_directx12.frame_index[frame] = swap_chain_common->swap_chain_directx12.swap_chain->lpVtbl->GetCurrentBackBufferIndex(swap_chain_common->swap_chain_directx12.swap_chain);

    // Create descriptor heaps.
    // Describe and create a render target view (RTV) descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc;
    memset(&rtv_heap_desc, 0, sizeof(rtv_heap_desc));
    rtv_heap_desc.NumDescriptors = MAX_SWAP_CHAIN_FRAMES;
    rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    hr = api_common->directx_12_api.device->lpVtbl->CreateDescriptorHeap(api_common->directx_12_api.device, &rtv_heap_desc, &IID_ID3D12DescriptorHeap, (void**)&(swap_chain_common->swap_chain_directx12.rtv_descriptor_heap[frame]));
    if (FAILED(hr))
      return 1;

    swap_chain_common->swap_chain_directx12.rtv_descriptor_size[frame] = api_common->directx_12_api.device->lpVtbl->GetDescriptorHandleIncrementSize(api_common->directx_12_api.device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // Create frame resources.
    swap_chain_common->swap_chain_directx12.rtv_descriptor_heap[frame]->lpVtbl->GetCPUDescriptorHandleForHeapStart(swap_chain_common->swap_chain_directx12.rtv_descriptor_heap[frame], &(swap_chain_common->swap_chain_directx12.rtv_handle[frame]));

    // Create a RTV for each frame.
    D3D12_CPU_DESCRIPTOR_HANDLE current_rtv_handle = swap_chain_common->swap_chain_directx12.rtv_handle[frame];
    hr = swap_chain_common->swap_chain_directx12.swap_chain->lpVtbl->GetBuffer(swap_chain_common->swap_chain_directx12.swap_chain, frame, &IID_ID3D12Resource, (void**)&(swap_chain_common->swap_chain_directx12.render_targets[frame]));
    if (FAILED(hr))
      return 1;

    api_common->directx_12_api.device->lpVtbl->CreateRenderTargetView(api_common->directx_12_api.device, swap_chain_common->swap_chain_directx12.render_targets[frame], NULL, current_rtv_handle);

    current_rtv_handle.ptr += swap_chain_common->swap_chain_directx12.rtv_descriptor_size[frame];

    hr = api_common->directx_12_api.device->lpVtbl->CreateCommandAllocator(api_common->directx_12_api.device, D3D12_COMMAND_LIST_TYPE_DIRECT, &IID_ID3D12CommandAllocator, (void**)&(swap_chain_common->swap_chain_directx12.command_allocator[frame]));
    if (FAILED(hr))
      return 1;

    // Note: Assets
    // Create the command list.
    hr = api_common->directx_12_api.device->lpVtbl->CreateCommandList(api_common->directx_12_api.device, 0, D3D12_COMMAND_LIST_TYPE_DIRECT, swap_chain_common->swap_chain_directx12.command_allocator[frame], NULL, &IID_ID3D12CommandList, (void**)&(swap_chain_common->swap_chain_directx12.command_list[frame]));
    if (FAILED(hr))
      return 1;

    // Command lists are created in the recording state, but there is nothing to record yet. The main loop expects it to be closed, so close it now.
    hr = swap_chain_common->swap_chain_directx12.command_list[frame]->lpVtbl->Close(swap_chain_common->swap_chain_directx12.command_list[frame]);
    if (FAILED(hr))
      return 1;
  }

  for (uint_fast8_t frame = 0; frame < MAX_FRAMES_IN_FLIGHT; frame++) {
    // Create synchronization objects.

    hr = api_common->directx_12_api.device->lpVtbl->CreateFence(api_common->directx_12_api.device, 0, D3D12_FENCE_FLAG_NONE, &IID_ID3D12Fence, (void**)&(swap_chain_common->swap_chain_directx12.fence[frame]));
    if (FAILED(hr))
      return 1;
    swap_chain_common->swap_chain_directx12.fence_value[frame] = 1;

    // Create an event handle to use for frame synchronization.
    swap_chain_common->swap_chain_directx12.fence_event[frame] = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (swap_chain_common->swap_chain_directx12.fence_event[frame] == NULL) {
      hr = HRESULT_FROM_WIN32(GetLastError());
      if (FAILED(hr))
        return 1;
    }
  }

  return 0;
}

static inline void swap_chain_directx_12_update_constant_buffer(struct SwapChainCommon* swap_chain_common, struct APICommon* api_common, uint_fast32_t width, uint_fast32_t height) {
  struct BlitUniformBufferObject ubos = {0};
  ubos.screen_size = (vec2){.x = (float)width, .y = (float)height};

  // Map the constant buffer to update it
  void* data;
  HRESULT hr = swap_chain_common->swap_chain_directx12.constant_buffer->lpVtbl->Map(swap_chain_common->swap_chain_directx12.constant_buffer, 0, NULL, &data);
  if (SUCCEEDED(hr)) {
    memcpy(data, &ubos, sizeof(struct BlitUniformBufferObject));
    swap_chain_common->swap_chain_directx12.constant_buffer->lpVtbl->Unmap(swap_chain_common->swap_chain_directx12.constant_buffer, 0, NULL);
  } else
    log_message(LOG_SEVERITY_ERROR, "Failed to map constant buffer.\n");
}

uint_fast8_t swap_chain_directx_12_init(struct SwapChainCommon* swap_chain_common, struct APICommon* api_common, uint_fast32_t width, uint_fast32_t height, bool vsync, void* extra_data) {
  DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {0};
  swap_chain_desc.BufferCount = MAX_SWAP_CHAIN_FRAMES;
  swap_chain_desc.Width = width;
  swap_chain_desc.Height = height;
  swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  swap_chain_desc.SampleDesc.Count = 1;

  HRESULT hr;
  hr = api_common->directx_12_api.factory->lpVtbl->CreateSwapChainForHwnd(api_common->directx_12_api.factory, (IUnknown*)api_common->directx_12_api.command_queue, swap_chain_common->hwnd, &swap_chain_desc, NULL, NULL, (IDXGISwapChain1**)&(swap_chain_common->swap_chain_directx12.swap_chain));
  if (FAILED(hr))
    return 2;

  hr = api_common->directx_12_api.factory->lpVtbl->MakeWindowAssociation(api_common->directx_12_api.factory, swap_chain_common->hwnd, DXGI_MWA_NO_ALT_ENTER);
  if (FAILED(hr))
    return 1;

  swap_chain_directx_12_init_common(swap_chain_common, api_common);

  // TODO: There should be some kinda error checking here
  // ThrowIfFailed(factory->MakeWindowAssociation(Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER));
  hr = api_common->directx_12_api.factory->lpVtbl->MakeWindowAssociation(api_common->directx_12_api.factory, (HWND)extra_data, 0);
  if (FAILED(hr))
    return 1;

  // Set up the constant buffer
  UINT64 constant_buffer_size = sizeof(struct BlitUniformBufferObject);
  directx_12_graphics_utils_setup_constant_buffer(&(api_common->directx_12_api), constant_buffer_size, &(swap_chain_common->swap_chain_directx12.constant_buffer));

  swap_chain_directx_12_update_constant_buffer(swap_chain_common, api_common, width, height);

  return 0;
}

// Note: This function if here because it is used by both init and resize
static inline void swap_chain_directx_12_delete_common(struct SwapChainCommon* swap_chain_common, struct APICommon* api_common) {
  // Release render targets, descriptor heaps, and related resources.
  for (UINT frame = 0; frame < MAX_SWAP_CHAIN_FRAMES; frame++) {
    if (swap_chain_common->swap_chain_directx12.rtv_descriptor_heap[frame]) {
      swap_chain_common->swap_chain_directx12.rtv_descriptor_heap[frame]->lpVtbl->Release(swap_chain_common->swap_chain_directx12.rtv_descriptor_heap[frame]);
      swap_chain_common->swap_chain_directx12.rtv_descriptor_heap[frame] = NULL;
    }

    if (swap_chain_common->swap_chain_directx12.render_targets[frame]) {
      swap_chain_common->swap_chain_directx12.render_targets[frame]->lpVtbl->Release(swap_chain_common->swap_chain_directx12.render_targets[frame]);
      swap_chain_common->swap_chain_directx12.render_targets[frame] = NULL;
    }

    if (swap_chain_common->swap_chain_directx12.command_allocator[frame]) {
      swap_chain_common->swap_chain_directx12.command_allocator[frame]->lpVtbl->Release(swap_chain_common->swap_chain_directx12.command_allocator[frame]);
      swap_chain_common->swap_chain_directx12.command_allocator[frame] = NULL;
    }

    if (swap_chain_common->swap_chain_directx12.command_list[frame]) {
      swap_chain_common->swap_chain_directx12.command_list[frame]->lpVtbl->Release(swap_chain_common->swap_chain_directx12.command_list[frame]);
      swap_chain_common->swap_chain_directx12.command_list[frame] = NULL;
    }
  }

  // Release fences and fence events.
  for (UINT frame = 0; frame < MAX_FRAMES_IN_FLIGHT; frame++) {
    if (swap_chain_common->swap_chain_directx12.fence[frame]) {
      swap_chain_common->swap_chain_directx12.fence[frame]->lpVtbl->Release(swap_chain_common->swap_chain_directx12.fence[frame]);
      swap_chain_common->swap_chain_directx12.fence[frame] = NULL;
    }

    if (swap_chain_common->swap_chain_directx12.fence_event[frame]) {
      CloseHandle(swap_chain_common->swap_chain_directx12.fence_event[frame]);
      swap_chain_common->swap_chain_directx12.fence_event[frame] = NULL;
    }
  }
}

void swap_chain_directx_12_delete(struct SwapChainCommon* swap_chain_common, struct APICommon* api_common) {
  swap_chain_directx_12_delete_common(swap_chain_common, api_common);

  // Release the swap chain itself.
  if (swap_chain_common->swap_chain_directx12.swap_chain) {
    ULONG result = swap_chain_common->swap_chain_directx12.swap_chain->lpVtbl->Release(swap_chain_common->swap_chain_directx12.swap_chain);
    if (result > 0)
      log_message(LOG_SEVERITY_ERROR, "Swap chain has %d references left over.\n", result);
    swap_chain_common->swap_chain_directx12.swap_chain = NULL;
  }

  // Reset fence values.
  memset(swap_chain_common->swap_chain_directx12.fence_value, 0, sizeof(swap_chain_common->swap_chain_directx12.fence_value));
}

uint_fast8_t swap_chain_directx_12_resize(struct SwapChainCommon* swap_chain_common, struct APICommon* api_common) {
  for (uint_fast8_t frame_num = 0; frame_num < MAX_FRAMES_IN_FLIGHT; frame_num++)
    swap_chain_directx_12_wait_for_fences(swap_chain_common, api_common, frame_num);

  swap_chain_directx_12_delete_common(swap_chain_common, api_common);

  DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {0};
  swap_chain_desc.BufferCount = MAX_SWAP_CHAIN_FRAMES;
  swap_chain_desc.Width = swap_chain_common->swap_chain_extent.width;
  swap_chain_desc.Height = swap_chain_common->swap_chain_extent.height;
  swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  swap_chain_desc.SampleDesc.Count = 1;

  if (FAILED(swap_chain_common->swap_chain_directx12.swap_chain->lpVtbl->ResizeBuffers(swap_chain_common->swap_chain_directx12.swap_chain, swap_chain_desc.BufferCount, swap_chain_desc.Width, swap_chain_desc.Height, swap_chain_desc.Format, swap_chain_desc.Flags)))
    return 1;

  if (swap_chain_directx_12_init_common(swap_chain_common, api_common) != 0)
    return 1;

  swap_chain_directx_12_update_constant_buffer(swap_chain_common, api_common, swap_chain_common->swap_chain_extent.width, swap_chain_common->swap_chain_extent.height);

  return 0;
}

void swap_chain_directx_12_prepare_delete(struct SwapChainCommon* swap_chain_common, struct APICommon* api_common) {
}

static uint_fast8_t swap_chain_directx_12_blit_init_wrapper(struct SwapChainCommon* swap_chain_common, struct APICommon* api_common, struct PostProcessCommon* post_process_common) {
  if (directx_12_graphics_utils_setup_vertex_buffer(&(api_common->directx_12_api), swap_chain_common->blit_fullscreen_triangle.mesh_common.vertices, &(swap_chain_common->swap_chain_directx12.vertex_buffer)))
    return 1;
  if (directx_12_graphics_utils_setup_index_buffer(&(api_common->directx_12_api), swap_chain_common->blit_fullscreen_triangle.mesh_common.indices, &(swap_chain_common->swap_chain_directx12.index_buffer)))
    return 1;

  return 0;
}

uint_fast8_t swap_chain_directx_12_blit_init(struct SwapChainCommon* swap_chain_common, struct APICommon* api_common, struct PostProcessCommon* post_process_common) {
  if (swap_chain_directx_12_blit_init_wrapper(swap_chain_common, api_common, post_process_common)) {
    directx_12_graphics_utils_poll_debug_messages(&(api_common->directx_12_api));
    return 1;
  }

  return 0;
}

uint_fast8_t swap_chain_directx_12_blit_update(struct SwapChainCommon* swap_chain_common, struct APICommon* api_common, struct PostProcessCommon* post_process_common) {
  return 0;
}

uint_fast8_t swap_chain_directx_12_blit_render(struct SwapChainCommon* swap_chain_common, struct PostProcessCommon* post_process_common, uint_fast8_t swap_chain_num) {
  // Reset the command allocator and command list for the given swap chain.
  swap_chain_common->swap_chain_directx12.command_allocator[swap_chain_num]->lpVtbl->Reset(swap_chain_common->swap_chain_directx12.command_allocator[swap_chain_num]);
  swap_chain_common->swap_chain_directx12.command_list[swap_chain_num]->lpVtbl->Reset(swap_chain_common->swap_chain_directx12.command_list[swap_chain_num], swap_chain_common->swap_chain_directx12.command_allocator[swap_chain_num], NULL);

  // Set the necessary state.
  swap_chain_common->swap_chain_directx12.command_list[swap_chain_num]->lpVtbl->RSSetViewports(swap_chain_common->swap_chain_directx12.command_list[swap_chain_num], 1, &(swap_chain_common->blit_shader->shader.shader_common.shader_directx12.viewport));
  swap_chain_common->swap_chain_directx12.command_list[swap_chain_num]->lpVtbl->RSSetScissorRects(swap_chain_common->swap_chain_directx12.command_list[swap_chain_num], 1, &(swap_chain_common->blit_shader->shader.shader_common.shader_directx12.scissor_rect));

  // Insert the resource barrier transition.
  ID3D12Resource* back_buffer;
  swap_chain_common->swap_chain_directx12.swap_chain->lpVtbl->GetBuffer(swap_chain_common->swap_chain_directx12.swap_chain, swap_chain_num, &IID_ID3D12Resource, (void**)&back_buffer);

  D3D12_RESOURCE_BARRIER barrier;
  memset(&barrier, 0, sizeof(barrier));
  barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrier.Transition.pResource = back_buffer;
  barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
  barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
  barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  swap_chain_common->swap_chain_directx12.command_list[swap_chain_num]->lpVtbl->ResourceBarrier(swap_chain_common->swap_chain_directx12.command_list[swap_chain_num], 1, &barrier);

  // Now set and clear the render target.
  // FLOAT clear_color[4] = {0.0f, 0.0f, 0.0f, 0.0f};
  swap_chain_common->swap_chain_directx12.command_list[swap_chain_num]->lpVtbl->OMSetRenderTargets(swap_chain_common->swap_chain_directx12.command_list[swap_chain_num], 1, &(swap_chain_common->swap_chain_directx12.rtv_handle[swap_chain_num]), FALSE, NULL);
  // swap_chain_common->swap_chain_directx12.command_list[swap_chain_num]->lpVtbl->ClearRenderTargetView(swap_chain_common->swap_chain_directx12.command_list[swap_chain_num], swap_chain_common->swap_chain_directx12.rtv_handle[swap_chain_num], clear_color, 0, NULL);

  ///////////////////

  swap_chain_common->swap_chain_directx12.command_list[swap_chain_num]->lpVtbl->SetPipelineState(swap_chain_common->swap_chain_directx12.command_list[swap_chain_num], swap_chain_common->blit_shader->shader.shader_common.shader_directx12.pipeline_state);

  swap_chain_common->swap_chain_directx12.vertex_buffer_view.BufferLocation = swap_chain_common->swap_chain_directx12.vertex_buffer->lpVtbl->GetGPUVirtualAddress(swap_chain_common->swap_chain_directx12.vertex_buffer);
  swap_chain_common->swap_chain_directx12.vertex_buffer_view.StrideInBytes = swap_chain_common->blit_fullscreen_triangle.mesh_common.mesh_memory_size;
  swap_chain_common->swap_chain_directx12.vertex_buffer_view.SizeInBytes = (UINT)(swap_chain_common->blit_fullscreen_triangle.mesh_common.mesh_memory_size * swap_chain_common->blit_fullscreen_triangle.mesh_common.vertices->size);
  swap_chain_common->swap_chain_directx12.command_list[swap_chain_num]->lpVtbl->IASetVertexBuffers(swap_chain_common->swap_chain_directx12.command_list[swap_chain_num], 0, 1, &(swap_chain_common->swap_chain_directx12.vertex_buffer_view));

  swap_chain_common->swap_chain_directx12.index_buffer_view.BufferLocation = swap_chain_common->swap_chain_directx12.index_buffer->lpVtbl->GetGPUVirtualAddress(swap_chain_common->swap_chain_directx12.index_buffer);
  swap_chain_common->swap_chain_directx12.index_buffer_view.Format = DXGI_FORMAT_R32_UINT;
  swap_chain_common->swap_chain_directx12.index_buffer_view.SizeInBytes = (UINT)(sizeof(uint32_t) * swap_chain_common->blit_fullscreen_triangle.mesh_common.indices->size);
  swap_chain_common->swap_chain_directx12.command_list[swap_chain_num]->lpVtbl->IASetIndexBuffer(swap_chain_common->swap_chain_directx12.command_list[swap_chain_num], &(swap_chain_common->swap_chain_directx12.index_buffer_view));

  // ID3D12DescriptorHeap *descriptor_heaps[] = {post_process_common->post_process_directx12.srv_heap[post_process_common->ping_pong]};
  // swap_chain_common->swap_chain_directx12.command_list[swap_chain_num]->lpVtbl->SetDescriptorHeaps(swap_chain_common->swap_chain_directx12.command_list[swap_chain_num], _countof(descriptor_heaps), descriptor_heaps);
  ID3D12DescriptorHeap* descriptor_heaps[] = {post_process_common->post_process_directx12.srv_heap[post_process_common->ping_pong], swap_chain_common->blit_shader->shader.shader_common.shader_directx12.sampler_heap};
  swap_chain_common->swap_chain_directx12.command_list[swap_chain_num]->lpVtbl->SetDescriptorHeaps(swap_chain_common->swap_chain_directx12.command_list[swap_chain_num], _countof(descriptor_heaps), descriptor_heaps);
  swap_chain_common->swap_chain_directx12.command_list[swap_chain_num]->lpVtbl->SetGraphicsRootSignature(swap_chain_common->swap_chain_directx12.command_list[swap_chain_num], swap_chain_common->blit_shader->shader.shader_common.shader_directx12.root_signature);
  D3D12_GPU_VIRTUAL_ADDRESS cbv_address = swap_chain_common->swap_chain_directx12.constant_buffer->lpVtbl->GetGPUVirtualAddress(swap_chain_common->swap_chain_directx12.constant_buffer);
  swap_chain_common->swap_chain_directx12.command_list[swap_chain_num]->lpVtbl->SetGraphicsRootConstantBufferView(swap_chain_common->swap_chain_directx12.command_list[swap_chain_num], 0, cbv_address);  // Assuming the CBV is bound at the first (0th) root parameter
  swap_chain_common->swap_chain_directx12.command_list[swap_chain_num]->lpVtbl->SetGraphicsRootDescriptorTable(swap_chain_common->swap_chain_directx12.command_list[swap_chain_num], 1, post_process_common->post_process_directx12.srv_gpu_handle[post_process_common->ping_pong]);
  swap_chain_common->swap_chain_directx12.command_list[swap_chain_num]->lpVtbl->SetGraphicsRootDescriptorTable(swap_chain_common->swap_chain_directx12.command_list[swap_chain_num], 2, swap_chain_common->blit_shader->shader.shader_common.shader_directx12.sampler_handle_gpu);
  // swap_chain_common->swap_chain_directx12.command_list[swap_chain_num]->lpVtbl->SetGraphicsRootDescriptorTable(swap_chain_common->swap_chain_directx12.command_list[swap_chain_num], 0, post_process_common->post_process_directx12.srv_gpu_handle[post_process_common->ping_pong]);
  swap_chain_common->swap_chain_directx12.command_list[swap_chain_num]->lpVtbl->IASetPrimitiveTopology(swap_chain_common->swap_chain_directx12.command_list[swap_chain_num], D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  swap_chain_common->swap_chain_directx12.command_list[swap_chain_num]->lpVtbl->DrawIndexedInstanced(swap_chain_common->swap_chain_directx12.command_list[swap_chain_num], (uint32_t)swap_chain_common->blit_fullscreen_triangle.mesh_common.indices->size, 1, 0, 0, 0);

  D3D12_RESOURCE_BARRIER to_present_barrier;
  memset(&to_present_barrier, 0, sizeof(D3D12_RESOURCE_BARRIER));
  to_present_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  to_present_barrier.Transition.pResource = back_buffer;
  to_present_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
  to_present_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
  to_present_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  swap_chain_common->swap_chain_directx12.command_list[swap_chain_num]->lpVtbl->ResourceBarrier(swap_chain_common->swap_chain_directx12.command_list[swap_chain_num], 1, &to_present_barrier);

  ///////////////////

  swap_chain_common->swap_chain_directx12.command_list[swap_chain_num]->lpVtbl->Close(swap_chain_common->swap_chain_directx12.command_list[swap_chain_num]);

  back_buffer->lpVtbl->Release(back_buffer);

  return 0;
}

bool swap_chain_directx_12_wait_for_fences(struct SwapChainCommon* swap_chain_common, struct APICommon* api_common, size_t frame) {
  // Wait for the current frame's fence.
  ID3D12Fence* current_fence = swap_chain_common->swap_chain_directx12.fence[frame];
  HANDLE current_fence_event = swap_chain_common->swap_chain_directx12.fence_event[frame];
  UINT64 current_fence_value = swap_chain_common->swap_chain_directx12.fence_value[frame];

  // Increment our fence value for the current frame.
  current_fence_value++;

  // Signal the fence with the updated value from the GPU side.
  api_common->directx_12_api.command_queue->lpVtbl->Signal(api_common->directx_12_api.command_queue, current_fence, current_fence_value);

  // If the fence's completed value is less than our fence value, then we know the GPU hasn't reached this fence yet.
  // So, we need to wait.
  if (current_fence->lpVtbl->GetCompletedValue(current_fence) < current_fence_value) {
    // Set an event to be triggered when the fence reaches the currentFenceValue.
    current_fence->lpVtbl->SetEventOnCompletion(current_fence, current_fence_value, current_fence_event);

    // Wait for the event to be triggered, indicating the GPU has finished.
    WaitForSingleObject(current_fence_event, INFINITE);
  }

  // Store the updated fence value for the next frame.
  swap_chain_common->swap_chain_directx12.fence_value[frame] = current_fence_value;

  // Get the current back buffer index. This is often done here because of double or triple buffering.
  swap_chain_common->image_index = swap_chain_common->swap_chain_directx12.swap_chain->lpVtbl->GetCurrentBackBufferIndex(swap_chain_common->swap_chain_directx12.swap_chain);

  return false;
}

uint_fast8_t swap_chain_directx_12_end_frame(struct SwapChainCommon* swap_chain_common, struct PostProcessCommon* post_process_common, struct APICommon* api_common) {
  // Execute the command list
  ID3D12CommandList* pp_command_lists[] = {(ID3D12CommandList*)swap_chain_common->swap_chain_directx12.command_list[swap_chain_common->image_index]};
  api_common->directx_12_api.command_queue->lpVtbl->ExecuteCommandLists(api_common->directx_12_api.command_queue, 1, pp_command_lists);

  // Present the frame
  // swap_chain_common->swap_chain_directx12.swap_chain->lpVtbl->Present(swap_chain_common->swap_chain_directx12.swap_chain, 0, 0); // No Vsync and no flags.
  swap_chain_common->swap_chain_directx12.swap_chain->lpVtbl->Present(swap_chain_common->swap_chain_directx12.swap_chain, 1, 0);  // Vsync and no flags.

  // Signal fence for the current frame.
  const UINT64 fence_value = swap_chain_common->swap_chain_directx12.fence_value[swap_chain_common->current_frame];
  api_common->directx_12_api.command_queue->lpVtbl->Signal(api_common->directx_12_api.command_queue, api_common->directx_12_api.fence, fence_value);

  return 0;  // Assuming no need for framebuffer updates or other responses for now.
}
