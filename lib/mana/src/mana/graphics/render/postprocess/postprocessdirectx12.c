#include "mana/graphics/render/postprocess/postprocessdirectx12.h"

static inline uint_fast8_t post_process_directx_12_init_common(struct PostProcessCommon* post_process_common, struct APICommon* api_common, struct SwapChainCommon* swap_chain_common) {
  D3D12_HEAP_PROPERTIES heap_props;
  heap_props.Type = D3D12_HEAP_TYPE_DEFAULT;
  heap_props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
  heap_props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
  heap_props.CreationNodeMask = 1;
  heap_props.VisibleNodeMask = 1;

  D3D12_RESOURCE_DESC resource_desc;
  memset(&resource_desc, 0, sizeof(resource_desc));
  resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  resource_desc.Width = swap_chain_common->swap_chain_extent.width;
  resource_desc.Height = swap_chain_common->swap_chain_extent.height;
  resource_desc.DepthOrArraySize = 1;
  resource_desc.MipLevels = 1;
  resource_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  // Adjust format as needed
  resource_desc.SampleDesc.Count = 1;
  resource_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

  D3D12_CLEAR_VALUE init_clear_value;
  memset(&init_clear_value, 0, sizeof(init_clear_value));
  init_clear_value.Format = resource_desc.Format;
  init_clear_value.Color[0] = 0.0f;
  init_clear_value.Color[1] = 0.0f;
  init_clear_value.Color[2] = 0.0f;
  init_clear_value.Color[3] = 0.0f;

  for (uint32_t num = 0; num < POST_PROCESS_PING_PONG; num++) {
    HRESULT hr;
    hr = api_common->directx_12_api.device->lpVtbl->CreateCommittedResource(api_common->directx_12_api.device, &heap_props, D3D12_HEAP_FLAG_NONE, &resource_desc, D3D12_RESOURCE_STATE_RENDER_TARGET, &init_clear_value, &IID_ID3D12Resource, (void**)&(post_process_common->post_process_directx12.color_textures[num]));
    if (FAILED(hr))
      return 1;

    // 2. Create Descriptor Heaps
    D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc;
    memset(&rtv_heap_desc, 0, sizeof(rtv_heap_desc));
    rtv_heap_desc.NumDescriptors = 1;
    rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    hr = api_common->directx_12_api.device->lpVtbl->CreateDescriptorHeap(api_common->directx_12_api.device, &rtv_heap_desc, &IID_ID3D12DescriptorHeap, (void**)&(post_process_common->post_process_directx12.rtv_descriptor_heap[num]));
    if (FAILED(hr))
      return 1;

    post_process_common->post_process_directx12.rtv_descriptor_size[num] = api_common->directx_12_api.device->lpVtbl->GetDescriptorHandleIncrementSize(api_common->directx_12_api.device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    post_process_common->post_process_directx12.rtv_descriptor_heap[num]->lpVtbl->GetCPUDescriptorHandleForHeapStart(post_process_common->post_process_directx12.rtv_descriptor_heap[num], &(post_process_common->post_process_directx12.rtv_handle[num]));

    api_common->directx_12_api.device->lpVtbl->CreateRenderTargetView(api_common->directx_12_api.device, post_process_common->post_process_directx12.color_textures[num], NULL, post_process_common->post_process_directx12.rtv_handle[num]);

    // 3. Create Command Allocators and Command Lists
    hr = api_common->directx_12_api.device->lpVtbl->CreateCommandAllocator(api_common->directx_12_api.device, D3D12_COMMAND_LIST_TYPE_DIRECT, &IID_ID3D12CommandAllocator, (void**)&(post_process_common->post_process_directx12.command_allocator[num]));
    if (FAILED(hr))
      return 1;

    hr = api_common->directx_12_api.device->lpVtbl->CreateCommandList(api_common->directx_12_api.device, 0, D3D12_COMMAND_LIST_TYPE_DIRECT, post_process_common->post_process_directx12.command_allocator[num], NULL, &IID_ID3D12CommandList, (void**)&(post_process_common->post_process_directx12.command_list[num]));
    if (FAILED(hr))
      return 1;

    D3D12_RESOURCE_BARRIER barrier;
    memset(&barrier, 0, sizeof(barrier));
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = post_process_common->post_process_directx12.color_textures[num];
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    post_process_common->post_process_directx12.command_list[num]->lpVtbl->ResourceBarrier(post_process_common->post_process_directx12.command_list[num], 1, &barrier);

    hr = post_process_common->post_process_directx12.command_list[num]->lpVtbl->Close(post_process_common->post_process_directx12.command_list[num]);

    ID3D12CommandList* pp_command_lists[] = {(ID3D12CommandList*)post_process_common->post_process_directx12.command_list[num]};
    api_common->directx_12_api.command_queue->lpVtbl->ExecuteCommandLists(api_common->directx_12_api.command_queue, _countof(pp_command_lists), pp_command_lists);

    // 4. Create Fences
    hr = api_common->directx_12_api.device->lpVtbl->CreateFence(api_common->directx_12_api.device, 0, D3D12_FENCE_FLAG_NONE, &IID_ID3D12Fence, (void**)&(post_process_common->post_process_directx12.fence[num]));
    if (FAILED(hr))
      return 1;

    post_process_common->post_process_directx12.fence_value[num] = 1;

    post_process_common->post_process_directx12.fence_event[num] = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (post_process_common->post_process_directx12.fence_event[num] == NULL) {
      hr = HRESULT_FROM_WIN32(GetLastError());
      if (FAILED(hr))
        return 1;
    }
  }

  for (uint_fast8_t frame = 0; frame < POST_PROCESS_PING_PONG; frame++) {
    D3D12_DESCRIPTOR_HEAP_DESC srv_heap_desc;
    memset(&srv_heap_desc, 0, sizeof(srv_heap_desc));
    srv_heap_desc.NumDescriptors = MAX_SWAP_CHAIN_FRAMES;  // Or however many you need
    srv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    HRESULT result = api_common->directx_12_api.device->lpVtbl->CreateDescriptorHeap(api_common->directx_12_api.device, &srv_heap_desc, &IID_ID3D12DescriptorHeap, (void**)&(post_process_common->post_process_directx12.srv_heap[frame]));

    if (result != S_OK)
      return 1;

    post_process_common->post_process_directx12.srv_heap[frame]->lpVtbl->GetCPUDescriptorHandleForHeapStart(post_process_common->post_process_directx12.srv_heap[frame], &(post_process_common->post_process_directx12.srv_cpu_handle[frame]));
    post_process_common->post_process_directx12.srv_heap[frame]->lpVtbl->GetGPUDescriptorHandleForHeapStart(post_process_common->post_process_directx12.srv_heap[frame], &(post_process_common->post_process_directx12.srv_gpu_handle[frame]));

    // Create SRVs for each of your resources
    D3D12_SHADER_RESOURCE_VIEW_DESC color_srv_desc;
    memset(&color_srv_desc, 0, sizeof(color_srv_desc));
    color_srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  // Adjust this based on your resource's format
    color_srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    color_srv_desc.Texture2D.MipLevels = 1;
    color_srv_desc.Texture2D.MostDetailedMip = 0;
    color_srv_desc.Texture2D.ResourceMinLODClamp = 0.0f;
    color_srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    api_common->directx_12_api.device->lpVtbl->CreateShaderResourceView(api_common->directx_12_api.device, post_process_common->post_process_directx12.color_textures[frame], &color_srv_desc, post_process_common->post_process_directx12.srv_cpu_handle[frame]);
  }

  return 0;
}

// Note: I'm thinking only need to use supersample once when writing from color texture to post process. When post process is ping ponging it should be regular resolution
static inline void post_process_direct12_update_constant_buffer(struct PostProcessCommon* post_process_common, struct APICommon* api_common, struct SwapChainCommon* swap_chain_common) {
  struct ResolveUniformBufferObject ubos = {0};
  ubos.screen_size = (vec2){.x = (float)swap_chain_common->swap_chain_extent.width, .y = (float)swap_chain_common->swap_chain_extent.height};

  // Map the constant buffer to update it
  void* data;
  HRESULT hr = post_process_common->post_process_directx12.constant_buffer->lpVtbl->Map(post_process_common->post_process_directx12.constant_buffer, 0, NULL, &data);
  if (SUCCEEDED(hr)) {
    memcpy(data, &ubos, sizeof(struct ResolveUniformBufferObject));
    post_process_common->post_process_directx12.constant_buffer->lpVtbl->Unmap(post_process_common->post_process_directx12.constant_buffer, 0, NULL);
  } else
    log_message(LOG_SEVERITY_ERROR, "Failed to map constant buffer.\n");
}

uint_fast8_t post_process_directx_12_init(struct PostProcessCommon* post_process_common, struct APICommon* api_common, struct SwapChainCommon* swap_chain_common) {
  post_process_directx_12_init_common(post_process_common, api_common, swap_chain_common);

  // Set up the constant buffer
  UINT64 constant_buffer_size = sizeof(struct ResolveUniformBufferObject);
  directx_12_graphics_utils_setup_constant_buffer(&(api_common->directx_12_api), constant_buffer_size, &(post_process_common->post_process_directx12.constant_buffer));

  post_process_direct12_update_constant_buffer(post_process_common, api_common, swap_chain_common);

  return 0;
}

void post_process_directx_12_delete(struct PostProcessCommon* post_process_common, struct APICommon* api_common) {
  for (uint_fast8_t num = 0; num < POST_PROCESS_PING_PONG; num++) {
    // Release Color Textures
    if (post_process_common->post_process_directx12.color_textures[num]) {
      post_process_common->post_process_directx12.color_textures[num]->lpVtbl->Release(post_process_common->post_process_directx12.color_textures[num]);
      post_process_common->post_process_directx12.color_textures[num] = NULL;
    }

    // Release Descriptor Heaps
    if (post_process_common->post_process_directx12.rtv_descriptor_heap[num]) {
      post_process_common->post_process_directx12.rtv_descriptor_heap[num]->lpVtbl->Release(post_process_common->post_process_directx12.rtv_descriptor_heap[num]);
      post_process_common->post_process_directx12.rtv_descriptor_heap[num] = NULL;
    }

    // Release Command Allocators
    if (post_process_common->post_process_directx12.command_allocator[num]) {
      post_process_common->post_process_directx12.command_allocator[num]->lpVtbl->Release(post_process_common->post_process_directx12.command_allocator[num]);
      post_process_common->post_process_directx12.command_allocator[num] = NULL;
    }

    // Release Command Lists
    if (post_process_common->post_process_directx12.command_list[num]) {
      post_process_common->post_process_directx12.command_list[num]->lpVtbl->Release(post_process_common->post_process_directx12.command_list[num]);
      post_process_common->post_process_directx12.command_list[num] = NULL;
    }

    // Release Fences
    if (post_process_common->post_process_directx12.fence[num]) {
      post_process_common->post_process_directx12.fence[num]->lpVtbl->Release(post_process_common->post_process_directx12.fence[num]);
      post_process_common->post_process_directx12.fence[num] = NULL;
    }

    // Close Fence Events
    if (post_process_common->post_process_directx12.fence_event[num]) {
      CloseHandle(post_process_common->post_process_directx12.fence_event[num]);
      post_process_common->post_process_directx12.fence_event[num] = NULL;
    }

    // Release srv_heap
    if (post_process_common->post_process_directx12.srv_heap[num]) {
      post_process_common->post_process_directx12.srv_heap[num]->lpVtbl->Release(post_process_common->post_process_directx12.srv_heap[num]);
      post_process_common->post_process_directx12.srv_heap[num] = NULL;
    }
  }
}

uint_fast8_t post_process_directx_12_resize(struct PostProcessCommon* post_process_common, struct APICommon* api_common, struct SwapChainCommon* swap_chain_common) {
  post_process_directx_12_delete(post_process_common, api_common);

  if (post_process_directx_12_init_common(post_process_common, api_common, swap_chain_common) != 0)
    return 1;

  post_process_direct12_update_constant_buffer(post_process_common, api_common, swap_chain_common);

  return 0;
}

uint_fast8_t post_process_directx_12_resolve_init(struct PostProcessCommon* post_process_common, struct APICommon* api_common, struct GBuffer* gbuffer, struct SwapChainCommon* swap_chain_common) {
  directx_12_graphics_utils_setup_vertex_buffer(&(api_common->directx_12_api), post_process_common->blit_fullscreen_triangle.mesh_common.vertices, &(post_process_common->post_process_directx12.vertex_buffer));
  directx_12_graphics_utils_setup_index_buffer(&(api_common->directx_12_api), post_process_common->blit_fullscreen_triangle.mesh_common.indices, &(post_process_common->post_process_directx12.index_buffer));

  return 0;
}

uint_fast8_t post_process_directx_12_resolve_update(struct PostProcessCommon* post_process_common, struct APICommon* api_common, struct GBuffer* gbuffer, struct SwapChainCommon* swap_chain_common) {
  return 0;
}

uint_fast8_t post_process_directx_12_resolve_render(struct PostProcessCommon* post_process_common, struct APICommon* api_common, struct GBuffer* gbuffer, struct SwapChainCommon* swap_chain_common) {
  // Reset the command allocator and command list for post-processing.
  post_process_common->post_process_directx12.command_allocator[post_process_common->ping_pong]->lpVtbl->Reset(post_process_common->post_process_directx12.command_allocator[post_process_common->ping_pong]);
  post_process_common->post_process_directx12.command_list[post_process_common->ping_pong]->lpVtbl->Reset(post_process_common->post_process_directx12.command_list[post_process_common->ping_pong], post_process_common->post_process_directx12.command_allocator[post_process_common->ping_pong], NULL);

  // Set the necessary state.
  post_process_common->post_process_directx12.command_list[post_process_common->ping_pong]->lpVtbl->RSSetViewports(post_process_common->post_process_directx12.command_list[post_process_common->ping_pong], 1, &(post_process_common->resolve_shader->shader[swap_chain_common->supersample_scale - 1].shader_common.shader_directx12.viewport));
  post_process_common->post_process_directx12.command_list[post_process_common->ping_pong]->lpVtbl->RSSetScissorRects(post_process_common->post_process_directx12.command_list[post_process_common->ping_pong], 1, &(post_process_common->resolve_shader->shader[swap_chain_common->supersample_scale - 1].shader_common.shader_directx12.scissor_rect));

  // Insert the resource barrier transition.
  D3D12_RESOURCE_BARRIER barrier;
  memset(&barrier, 0, sizeof(D3D12_RESOURCE_BARRIER));
  barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  barrier.Transition.pResource = post_process_common->post_process_directx12.color_textures[post_process_common->ping_pong];
  barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
  barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
  barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  post_process_common->post_process_directx12.command_list[post_process_common->ping_pong]->lpVtbl->ResourceBarrier(post_process_common->post_process_directx12.command_list[post_process_common->ping_pong], 1, &barrier);

  // Set and clear the render target for post processing.
  // FLOAT clear_color[4] = {0.0f, 0.0f, 0.0f, 0.0f};
  post_process_common->post_process_directx12.command_list[post_process_common->ping_pong]->lpVtbl->OMSetRenderTargets(post_process_common->post_process_directx12.command_list[post_process_common->ping_pong], 1, &(post_process_common->post_process_directx12.rtv_handle[post_process_common->ping_pong]), FALSE, NULL);
  // post_process_common->post_process_directx12.command_list[post_process_common->ping_pong]->lpVtbl->ClearRenderTargetView(post_process_common->post_process_directx12.command_list[post_process_common->ping_pong], post_process_common->post_process_directx12.rtv_handle[post_process_common->ping_pong], clear_color, 0, NULL);

  // Set up and bind the appropriate pipeline state
  post_process_common->post_process_directx12.command_list[post_process_common->ping_pong]->lpVtbl->SetPipelineState(post_process_common->post_process_directx12.command_list[post_process_common->ping_pong], post_process_common->resolve_shader->shader[swap_chain_common->supersample_scale - 1].shader_common.shader_directx12.pipeline_state);

  post_process_common->post_process_directx12.vertex_buffer_view.BufferLocation = post_process_common->post_process_directx12.vertex_buffer->lpVtbl->GetGPUVirtualAddress(post_process_common->post_process_directx12.vertex_buffer);
  post_process_common->post_process_directx12.vertex_buffer_view.StrideInBytes = post_process_common->blit_fullscreen_triangle.mesh_common.mesh_memory_size;
  post_process_common->post_process_directx12.vertex_buffer_view.SizeInBytes = (UINT)(post_process_common->blit_fullscreen_triangle.mesh_common.mesh_memory_size * post_process_common->blit_fullscreen_triangle.mesh_common.vertices->size);
  post_process_common->post_process_directx12.command_list[post_process_common->ping_pong]->lpVtbl->IASetVertexBuffers(post_process_common->post_process_directx12.command_list[post_process_common->ping_pong], 0, 1, &(post_process_common->post_process_directx12.vertex_buffer_view));

  post_process_common->post_process_directx12.index_buffer_view.BufferLocation = post_process_common->post_process_directx12.index_buffer->lpVtbl->GetGPUVirtualAddress(post_process_common->post_process_directx12.index_buffer);
  post_process_common->post_process_directx12.index_buffer_view.Format = DXGI_FORMAT_R32_UINT;
  post_process_common->post_process_directx12.index_buffer_view.SizeInBytes = (UINT)(sizeof(uint32_t) * post_process_common->blit_fullscreen_triangle.mesh_common.indices->size);
  post_process_common->post_process_directx12.command_list[post_process_common->ping_pong]->lpVtbl->IASetIndexBuffer(post_process_common->post_process_directx12.command_list[post_process_common->ping_pong], &(post_process_common->post_process_directx12.index_buffer_view));

  // Bind descriptor heaps, root signatures, and draw the fullscreen triangle
  // ID3D12DescriptorHeap *descriptor_heaps[] = {gbuffer->gbuffer_common.gbuffer_directx12.srv_heap};
  ID3D12DescriptorHeap* descriptor_heaps[] = {gbuffer->gbuffer_common.gbuffer_directx12.srv_heap, post_process_common->resolve_shader->shader[swap_chain_common->supersample_scale - 1].shader_common.shader_directx12.sampler_heap};
  post_process_common->post_process_directx12.command_list[post_process_common->ping_pong]->lpVtbl->SetDescriptorHeaps(post_process_common->post_process_directx12.command_list[post_process_common->ping_pong], _countof(descriptor_heaps), descriptor_heaps);
  post_process_common->post_process_directx12.command_list[post_process_common->ping_pong]->lpVtbl->SetGraphicsRootSignature(post_process_common->post_process_directx12.command_list[post_process_common->ping_pong], post_process_common->resolve_shader->shader[swap_chain_common->supersample_scale - 1].shader_common.shader_directx12.root_signature);
  D3D12_GPU_VIRTUAL_ADDRESS cbv_address = post_process_common->post_process_directx12.constant_buffer->lpVtbl->GetGPUVirtualAddress(post_process_common->post_process_directx12.constant_buffer);
  post_process_common->post_process_directx12.command_list[post_process_common->ping_pong]->lpVtbl->SetGraphicsRootConstantBufferView(post_process_common->post_process_directx12.command_list[post_process_common->ping_pong], 0, cbv_address);  // Assuming the CBV is bound at the first (0th) root parameter

  post_process_common->post_process_directx12.command_list[post_process_common->ping_pong]->lpVtbl->SetGraphicsRootDescriptorTable(post_process_common->post_process_directx12.command_list[post_process_common->ping_pong], 1, gbuffer->gbuffer_common.gbuffer_directx12.srv_gpu_handle);
  post_process_common->post_process_directx12.command_list[post_process_common->ping_pong]->lpVtbl->SetGraphicsRootDescriptorTable(post_process_common->post_process_directx12.command_list[post_process_common->ping_pong], 2, post_process_common->resolve_shader->shader[swap_chain_common->supersample_scale - 1].shader_common.shader_directx12.sampler_handle_gpu);
  // post_process_common->post_process_directx12.command_list[post_process_common->ping_pong]->lpVtbl->SetGraphicsRootDescriptorTable(post_process_common->post_process_directx12.command_list[post_process_common->ping_pong], 0, gbuffer->gbuffer_common.gbuffer_directx12.srv_gpu_handle);
  post_process_common->post_process_directx12.command_list[post_process_common->ping_pong]->lpVtbl->IASetPrimitiveTopology(post_process_common->post_process_directx12.command_list[post_process_common->ping_pong], D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
  post_process_common->post_process_directx12.command_list[post_process_common->ping_pong]->lpVtbl->DrawIndexedInstanced(post_process_common->post_process_directx12.command_list[post_process_common->ping_pong], (uint32_t)post_process_common->blit_fullscreen_triangle.mesh_common.indices->size, 1, 0, 0, 0);
  // post_process_common->post_process_directx12.command_list[post_process_common->ping_pong]->lpVtbl->DrawInstanced(post_process_common->post_process_directx12.command_list[post_process_common->ping_pong], 4, 1, 0, 0);
  // post_process_common->post_process_directx12.command_list[post_process_common->ping_pong]->lpVtbl->IASetPrimitiveTopology(post_process_common->post_process_directx12.command_list[post_process_common->ping_pong], D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  // post_process_common->post_process_directx12.command_list[post_process_common->ping_pong]->lpVtbl->DrawInstanced(post_process_common->post_process_directx12.command_list[post_process_common->ping_pong], 6, 1, 0, 0);

  // Possibly set barriers or any other synchronization primitives required for DirectX12. This is a common step but specifics can vary based on the exact workflow.
  D3D12_RESOURCE_BARRIER to_present_barrier;
  memset(&to_present_barrier, 0, sizeof(D3D12_RESOURCE_BARRIER));
  to_present_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
  to_present_barrier.Transition.pResource = post_process_common->post_process_directx12.color_textures[post_process_common->ping_pong];
  to_present_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
  to_present_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
  to_present_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
  post_process_common->post_process_directx12.command_list[post_process_common->ping_pong]->lpVtbl->ResourceBarrier(post_process_common->post_process_directx12.command_list[post_process_common->ping_pong], 1, &to_present_barrier);

  // At this point, you would usually submit the command list to the command queue to execute the drawing, similar to the vkQueueSubmit in the Vulkan version.
  // Close the post-processing command list.
  post_process_common->post_process_directx12.command_list[post_process_common->ping_pong]->lpVtbl->Close(post_process_common->post_process_directx12.command_list[post_process_common->ping_pong]);

  // Submit the command list for rendering.
  ID3D12CommandList* command_lists[] = {(ID3D12CommandList*)post_process_common->post_process_directx12.command_list[post_process_common->ping_pong]};
  api_common->directx_12_api.command_queue->lpVtbl->ExecuteCommandLists(api_common->directx_12_api.command_queue, ARRAYSIZE(command_lists), command_lists);

  // Sync using a fence or some other synchronization mechanism as DirectX 12 doesn't provide semaphores out of the box like Vulkan.

  return 0;
}
