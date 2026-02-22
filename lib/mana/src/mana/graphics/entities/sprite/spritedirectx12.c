#include "mana/graphics/entities/sprite/spritedirectx12.h"

uint_fast8_t sprite_directx_12_init(struct SpriteCommon *sprite_common, struct APICommon *api_common, struct Shader *shader, struct Texture *texture, size_t num) {
  // Set up the vertex buffer
  directx_12_graphics_utils_setup_vertex_buffer(&(api_common->directx_12_api), sprite_common->image_mesh->mesh_common.vertices, &sprite_common->sprite_directx12.vertex_buffer);

  // Create the vertex buffer view
  sprite_common->sprite_directx12.vertex_buffer_view.BufferLocation = sprite_common->sprite_directx12.vertex_buffer->lpVtbl->GetGPUVirtualAddress(sprite_common->sprite_directx12.vertex_buffer);
  sprite_common->sprite_directx12.vertex_buffer_view.StrideInBytes = sprite_common->image_mesh->mesh_common.mesh_memory_size;
  sprite_common->sprite_directx12.vertex_buffer_view.SizeInBytes = (UINT)(vector_size(sprite_common->image_mesh->mesh_common.vertices) * sprite_common->image_mesh->mesh_common.mesh_memory_size);

  // Set up the index buffer
  directx_12_graphics_utils_setup_index_buffer(&(api_common->directx_12_api), sprite_common->image_mesh->mesh_common.indices, &sprite_common->sprite_directx12.index_buffer);

  // Create the index buffer view
  sprite_common->sprite_directx12.index_buffer_view.BufferLocation = sprite_common->sprite_directx12.index_buffer->lpVtbl->GetGPUVirtualAddress(sprite_common->sprite_directx12.index_buffer);
  sprite_common->sprite_directx12.index_buffer_view.Format = DXGI_FORMAT_R32_UINT;
  sprite_common->sprite_directx12.index_buffer_view.SizeInBytes = (UINT)(vector_size(sprite_common->image_mesh->mesh_common.indices) * sprite_common->image_mesh->mesh_common.indices->memory_size);  // Assuming an IndexType for indices

  // Set up the constant buffer
  UINT64 constant_buffer_size = sizeof(struct SpriteUniformBufferObject);
  directx_12_graphics_utils_setup_constant_buffer(&(api_common->directx_12_api), constant_buffer_size, &(sprite_common->sprite_directx12.constant_buffer));

  return 0;
}

void sprite_directx_12_delete(struct SpriteCommon *sprite_common, struct APICommon *api_common) {
  if (sprite_common->sprite_directx12.vertex_buffer) {
    sprite_common->sprite_directx12.vertex_buffer->lpVtbl->Release(sprite_common->sprite_directx12.vertex_buffer);
    sprite_common->sprite_directx12.vertex_buffer = NULL;
  }

  if (sprite_common->sprite_directx12.index_buffer) {
    sprite_common->sprite_directx12.index_buffer->lpVtbl->Release(sprite_common->sprite_directx12.index_buffer);
    sprite_common->sprite_directx12.index_buffer = NULL;
  }

  if (sprite_common->sprite_directx12.constant_buffer) {
    sprite_common->sprite_directx12.constant_buffer->lpVtbl->Release(sprite_common->sprite_directx12.constant_buffer);
    sprite_common->sprite_directx12.constant_buffer = NULL;
  }
}

void sprite_directx_12_render(struct SpriteCommon *sprite_common, struct GBufferCommon *gbuffer_common) {
  gbuffer_common->gbuffer_directx12.command_list->lpVtbl->RSSetViewports(gbuffer_common->gbuffer_directx12.command_list, 1, &(sprite_common->shader->shader_common.shader_directx12.viewport));
  gbuffer_common->gbuffer_directx12.command_list->lpVtbl->RSSetScissorRects(gbuffer_common->gbuffer_directx12.command_list, 1, &(sprite_common->shader->shader_common.shader_directx12.scissor_rect));

  // Bind the pipeline state object (PSO) - which contains the shader stages, blend state, etc.
  gbuffer_common->gbuffer_directx12.command_list->lpVtbl->SetPipelineState(gbuffer_common->gbuffer_directx12.command_list, sprite_common->shader->shader_common.shader_directx12.pipeline_state);

  // Bind the root signature
  gbuffer_common->gbuffer_directx12.command_list->lpVtbl->SetGraphicsRootSignature(gbuffer_common->gbuffer_directx12.command_list, sprite_common->shader->shader_common.shader_directx12.root_signature);

  // Set the constant buffer view for the sprite.
  // You might need to handle this differently depending on how your root signature and shader are set up.
  D3D12_GPU_VIRTUAL_ADDRESS cbv_address = sprite_common->sprite_directx12.constant_buffer->lpVtbl->GetGPUVirtualAddress(sprite_common->sprite_directx12.constant_buffer);
  gbuffer_common->gbuffer_directx12.command_list->lpVtbl->SetGraphicsRootConstantBufferView(gbuffer_common->gbuffer_directx12.command_list, 0, cbv_address);  // Assuming the CBV is bound at the first (0th) root parameter

  // Set the descriptor heap
  // ID3D12DescriptorHeap *heaps[] = {sprite_common->image_texture->texture_common.texture_directx12.srv_heap};
  // gbuffer_common->gbuffer_directx12.command_list->lpVtbl->SetDescriptorHeaps(gbuffer_common->gbuffer_directx12.command_list, 1, heaps);
  ID3D12DescriptorHeap *descriptor_heaps[] = {sprite_common->image_texture->texture_common.texture_manager_common->texture_manager_directx12.srv_heap, sprite_common->shader->shader_common.shader_directx12.sampler_heap};
  gbuffer_common->gbuffer_directx12.command_list->lpVtbl->SetDescriptorHeaps(gbuffer_common->gbuffer_directx12.command_list, _countof(descriptor_heaps), descriptor_heaps);

  // Bind the SRV for the texture
  gbuffer_common->gbuffer_directx12.command_list->lpVtbl->SetGraphicsRootDescriptorTable(gbuffer_common->gbuffer_directx12.command_list, 1, sprite_common->image_texture->texture_common.texture_directx12.gpu_heap_handle);

  // Bind the sampler for the texture
  gbuffer_common->gbuffer_directx12.command_list->lpVtbl->SetGraphicsRootDescriptorTable(gbuffer_common->gbuffer_directx12.command_list, 2, sprite_common->shader->shader_common.shader_directx12.sampler_handle_gpu);

  // Bind the vertex buffer.
  D3D12_VERTEX_BUFFER_VIEW vbv = sprite_common->sprite_directx12.vertex_buffer_view;
  gbuffer_common->gbuffer_directx12.command_list->lpVtbl->IASetVertexBuffers(gbuffer_common->gbuffer_directx12.command_list, 0, 1, &vbv);

  // Bind the index buffer.
  D3D12_INDEX_BUFFER_VIEW ibv = sprite_common->sprite_directx12.index_buffer_view;
  gbuffer_common->gbuffer_directx12.command_list->lpVtbl->IASetIndexBuffer(gbuffer_common->gbuffer_directx12.command_list, &ibv);

  // Set the primitive topology
  gbuffer_common->gbuffer_directx12.command_list->lpVtbl->IASetPrimitiveTopology(gbuffer_common->gbuffer_directx12.command_list, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);  // Assuming triangles, adjust as necessary

  // Draw indexed
  gbuffer_common->gbuffer_directx12.command_list->lpVtbl->DrawIndexedInstanced(gbuffer_common->gbuffer_directx12.command_list, (UINT)sprite_common->image_mesh->mesh_common.indices->size, 1, 0, 0, 0);
}

void sprite_directx_12_update_uniforms(struct SpriteCommon *sprite_common, struct APICommon *api_common, struct GBufferCommon *gbuffer_common) {
  struct SpriteUniformBufferObject ubos = {0};
  ubos.proj = gbuffer_common->projection_matrix;
  ubos.proj = mat4_transpose(ubos.proj);

  ubos.view = gbuffer_common->view_matrix;
  ubos.view = mat4_transpose(ubos.view);

  ubos.model = mat4_translate(MAT4_IDENTITY, sprite_common->position);
  ubos.model = mat4_mul(ubos.model, quaternion_to_mat4(quaternion_normalise(sprite_common->rotation)));
  ubos.model = mat4_scale(ubos.model, sprite_common->scale);
  ubos.model = mat4_transpose(ubos.model);

  // Map the constant buffer to update it
  void *data;
  HRESULT hr = sprite_common->sprite_directx12.constant_buffer->lpVtbl->Map(sprite_common->sprite_directx12.constant_buffer, 0, NULL, &data);
  if (SUCCEEDED(hr)) {
    memcpy(data, &ubos, sizeof(struct SpriteUniformBufferObject));
    sprite_common->sprite_directx12.constant_buffer->lpVtbl->Unmap(sprite_common->sprite_directx12.constant_buffer, 0, NULL);
  } else
    log_message(LOG_SEVERITY_ERROR, "Failed to map constant buffer.\n");
}
