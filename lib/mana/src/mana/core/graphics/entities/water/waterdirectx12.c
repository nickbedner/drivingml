#include "mana/core/graphics/entities/water/waterdirectx12.h"

u8 water_directx_12_init(struct WaterCommon* water_common, struct APICommon* api_common, struct Shader* shader, struct Texture* texture) {
  water_common->shader = shader;
  water_common->wave_texture = texture;

  directx_12_graphics_utils_setup_vertex_buffer(&(api_common->directx_12_api), water_common->water_mesh->mesh_common.vertices, &water_common->water_directx12.vertex_buffer);

  water_common->water_directx12.vertex_buffer_view.BufferLocation = water_common->water_directx12.vertex_buffer->lpVtbl->GetGPUVirtualAddress(water_common->water_directx12.vertex_buffer);
  water_common->water_directx12.vertex_buffer_view.StrideInBytes = water_common->water_mesh->mesh_common.mesh_memory_size;
  water_common->water_directx12.vertex_buffer_view.SizeInBytes = (UINT)(vector_size(water_common->water_mesh->mesh_common.vertices) * water_common->water_mesh->mesh_common.mesh_memory_size);

  directx_12_graphics_utils_setup_index_buffer(&(api_common->directx_12_api), water_common->water_mesh->mesh_common.indices, &water_common->water_directx12.index_buffer);

  water_common->water_directx12.index_buffer_view.BufferLocation = water_common->water_directx12.index_buffer->lpVtbl->GetGPUVirtualAddress(water_common->water_directx12.index_buffer);
  water_common->water_directx12.index_buffer_view.Format = DXGI_FORMAT_R32_UINT;
  water_common->water_directx12.index_buffer_view.SizeInBytes = (UINT)(vector_size(water_common->water_mesh->mesh_common.indices) * water_common->water_mesh->mesh_common.indices->memory_size);

  directx_12_graphics_utils_setup_constant_buffer(&(api_common->directx_12_api), sizeof(struct WaterVertexUniformBufferObject), &water_common->water_directx12.vertex_constant_buffer);
  directx_12_graphics_utils_setup_constant_buffer(&(api_common->directx_12_api), sizeof(struct WaterFragmentUniformBufferObject), &water_common->water_directx12.fragment_constant_buffer);

  return WATER_SUCCESS;
}

void water_directx_12_delete(struct WaterCommon* water_common, struct APICommon* api_common) {
  if (water_common->water_directx12.vertex_buffer) {
    water_common->water_directx12.vertex_buffer->lpVtbl->Release(water_common->water_directx12.vertex_buffer);
    water_common->water_directx12.vertex_buffer = NULL;
  }

  if (water_common->water_directx12.index_buffer) {
    water_common->water_directx12.index_buffer->lpVtbl->Release(water_common->water_directx12.index_buffer);
    water_common->water_directx12.index_buffer = NULL;
  }

  if (water_common->water_directx12.vertex_constant_buffer) {
    water_common->water_directx12.vertex_constant_buffer->lpVtbl->Release(water_common->water_directx12.vertex_constant_buffer);
    water_common->water_directx12.vertex_constant_buffer = NULL;
  }

  if (water_common->water_directx12.fragment_constant_buffer) {
    water_common->water_directx12.fragment_constant_buffer->lpVtbl->Release(water_common->water_directx12.fragment_constant_buffer);
    water_common->water_directx12.fragment_constant_buffer = NULL;
  }
}

void water_directx_12_render(struct WaterCommon* water_common, struct GBufferCommon* gbuffer_common) {
  ID3D12GraphicsCommandList* cmd = gbuffer_common->gbuffer_directx12.command_list;

  cmd->lpVtbl->RSSetViewports(cmd, 1, &water_common->shader->shader_common.shader_directx12.viewport);
  cmd->lpVtbl->RSSetScissorRects(cmd, 1, &water_common->shader->shader_common.shader_directx12.scissor_rect);

  cmd->lpVtbl->SetPipelineState(cmd, water_common->shader->shader_common.shader_directx12.pipeline_state);
  cmd->lpVtbl->SetGraphicsRootSignature(cmd, water_common->shader->shader_common.shader_directx12.root_signature);

  D3D12_GPU_VIRTUAL_ADDRESS vcbv = water_common->water_directx12.vertex_constant_buffer->lpVtbl->GetGPUVirtualAddress(water_common->water_directx12.vertex_constant_buffer);
  D3D12_GPU_VIRTUAL_ADDRESS fcbv = water_common->water_directx12.fragment_constant_buffer->lpVtbl->GetGPUVirtualAddress(water_common->water_directx12.fragment_constant_buffer);

  cmd->lpVtbl->SetGraphicsRootConstantBufferView(cmd, 0, vcbv);
  cmd->lpVtbl->SetGraphicsRootConstantBufferView(cmd, 1, fcbv);

  ID3D12DescriptorHeap* descriptor_heaps[] = {water_common->wave_texture->texture_common.texture_manager_common->texture_manager_directx12.srv_heap, water_common->wave_texture->texture_common.texture_manager_common->texture_manager_directx12.sampler_heap};
  cmd->lpVtbl->SetDescriptorHeaps(cmd, _countof(descriptor_heaps), descriptor_heaps);
  cmd->lpVtbl->SetGraphicsRootDescriptorTable(cmd, 2, water_common->wave_texture->texture_common.texture_directx12.srv_gpu_handle);
  cmd->lpVtbl->SetGraphicsRootDescriptorTable(cmd, 3, water_common->wave_texture->texture_common.texture_directx12.sampler_gpu_handle);

  D3D12_VERTEX_BUFFER_VIEW vbv = water_common->water_directx12.vertex_buffer_view;
  D3D12_INDEX_BUFFER_VIEW ibv = water_common->water_directx12.index_buffer_view;

  cmd->lpVtbl->IASetVertexBuffers(cmd, 0, 1, &vbv);
  cmd->lpVtbl->IASetIndexBuffer(cmd, &ibv);
  cmd->lpVtbl->IASetPrimitiveTopology(cmd, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  cmd->lpVtbl->DrawIndexedInstanced(cmd, (UINT)water_common->water_mesh->mesh_common.indices->size, 1, 0, 0, 0);
}

void water_directx_12_update_uniforms(struct WaterCommon* water_common, struct APICommon* api_common, struct GBufferCommon* gbuffer_common, u32 width, u32 height) {
  struct WaterVertexUniformBufferObject vubo = {0};
  vubo.proj = mat4_transpose(gbuffer_common->projection_matrix);
  vubo.view = mat4_transpose(gbuffer_common->view_matrix);

  vubo.model = mat4_translate(MAT4_IDENTITY, water_common->position);
  vubo.model = mat4_mul(vubo.model, quaternion_to_mat4(quaternion_normalise(water_common->rotation)));
  vubo.model = mat4_scale(vubo.model, water_common->scale);
  vubo.model = mat4_transpose(vubo.model);

  void* data = NULL;
  HRESULT hr = water_common->water_directx12.vertex_constant_buffer->lpVtbl->Map(water_common->water_directx12.vertex_constant_buffer, 0, NULL, &data);
  if (SUCCEEDED(hr)) {
    memcpy(data, &vubo, sizeof(vubo));
    water_common->water_directx12.vertex_constant_buffer->lpVtbl->Unmap(water_common->water_directx12.vertex_constant_buffer, 0, NULL);
  } else
    log_message(LOG_SEVERITY_ERROR, "Failed to map water vertex constant buffer.\n");

  struct WaterFragmentUniformBufferObject fubo = {0};
  fubo.params0.x = water_common->time;
  fubo.params0.y = 0.0f;
  fubo.params0.z = 0.0f;
  fubo.params0.w = 0.0f;

  fubo.params1 = (vec4){0};
  {
    r32 base_height = 548.0f;
    r32 bias = real32_log2((r32)height / base_height);
    fubo.params1.x = bias;
  }

  hr = water_common->water_directx12.fragment_constant_buffer->lpVtbl->Map(water_common->water_directx12.fragment_constant_buffer, 0, NULL, &data);
  if (SUCCEEDED(hr)) {
    memcpy(data, &fubo, sizeof(fubo));
    water_common->water_directx12.fragment_constant_buffer->lpVtbl->Unmap(water_common->water_directx12.fragment_constant_buffer, 0, NULL);
  } else
    log_message(LOG_SEVERITY_ERROR, "Failed to map water fragment constant buffer.\n");
}
