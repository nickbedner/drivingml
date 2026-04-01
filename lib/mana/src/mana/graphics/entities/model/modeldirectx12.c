#include "mana/graphics/entities/model/modeldirectx12.h"

void model_directx_12_clone_init(struct ModelCommon* model_common, struct APICommon* api_common) {
  // Set up the vertex buffer
  directx_12_graphics_utils_setup_vertex_buffer(&(api_common->directx_12_api), model_common->model_mesh->mesh_common.vertices, &model_common->model_directx12.vertex_buffer);

  // Create the vertex buffer view
  model_common->model_directx12.vertex_buffer_view.BufferLocation = model_common->model_directx12.vertex_buffer->lpVtbl->GetGPUVirtualAddress(model_common->model_directx12.vertex_buffer);
  model_common->model_directx12.vertex_buffer_view.StrideInBytes = model_common->model_mesh->mesh_common.mesh_memory_size;
  model_common->model_directx12.vertex_buffer_view.SizeInBytes = (UINT)(vector_size(model_common->model_mesh->mesh_common.vertices) * model_common->model_mesh->mesh_common.mesh_memory_size);

  // Set up the index buffer
  directx_12_graphics_utils_setup_index_buffer(&(api_common->directx_12_api), model_common->model_mesh->mesh_common.indices, &model_common->model_directx12.index_buffer);

  // Create the index buffer view
  model_common->model_directx12.index_buffer_view.BufferLocation = model_common->model_directx12.index_buffer->lpVtbl->GetGPUVirtualAddress(model_common->model_directx12.index_buffer);
  model_common->model_directx12.index_buffer_view.Format = DXGI_FORMAT_R32_UINT;
  model_common->model_directx12.index_buffer_view.SizeInBytes = (UINT)(vector_size(model_common->model_mesh->mesh_common.indices) * model_common->model_mesh->mesh_common.indices->memory_size);  // Assuming an IndexType for indices

  // Set up the constant buffer
  directx_12_graphics_utils_setup_constant_buffer(&(api_common->directx_12_api), sizeof(struct ModelUniformBufferObject), &(model_common->model_directx12.constant_buffer));
  directx_12_graphics_utils_setup_constant_buffer(&(api_common->directx_12_api), sizeof(struct LightingUniformBufferObject), &(model_common->model_directx12.lighting_constant_buffer));
  if (model_common->animated)
    directx_12_graphics_utils_setup_constant_buffer(&(api_common->directx_12_api), sizeof(struct ModelAnimationUniformBufferObject), &(model_common->model_directx12.animation_constant_buffer));
}

void model_directx_12_clone_delete(struct ModelCommon* model_common, struct APICommon* api_common) {
}

void model_directx_12_render(struct ModelCommon* model_common, struct GBuffer* gbuffer, r64 delta_time) {
  if (model_common->animated)
    animator_update(model_common->animator, delta_time);

  gbuffer->gbuffer_common.gbuffer_directx12.command_list->lpVtbl->SetPipelineState(gbuffer->gbuffer_common.gbuffer_directx12.command_list, model_common->shader_handle->shader_common.shader_directx12.pipeline_state);
  gbuffer->gbuffer_common.gbuffer_directx12.command_list->lpVtbl->SetGraphicsRootSignature(gbuffer->gbuffer_common.gbuffer_directx12.command_list, model_common->shader_handle->shader_common.shader_directx12.root_signature);

  D3D12_GPU_VIRTUAL_ADDRESS cbv_address = model_common->model_directx12.constant_buffer->lpVtbl->GetGPUVirtualAddress(model_common->model_directx12.constant_buffer);
  gbuffer->gbuffer_common.gbuffer_directx12.command_list->lpVtbl->SetGraphicsRootConstantBufferView(gbuffer->gbuffer_common.gbuffer_directx12.command_list, 0, cbv_address);  // Assuming the CBV is bound at the first (0th) root parameter
  D3D12_GPU_VIRTUAL_ADDRESS lighting_cbv_address = model_common->model_directx12.constant_buffer->lpVtbl->GetGPUVirtualAddress(model_common->model_directx12.lighting_constant_buffer);
  gbuffer->gbuffer_common.gbuffer_directx12.command_list->lpVtbl->SetGraphicsRootConstantBufferView(gbuffer->gbuffer_common.gbuffer_directx12.command_list, 1, lighting_cbv_address);
  if (model_common->animated) {
    D3D12_GPU_VIRTUAL_ADDRESS animation_cbv_address = model_common->model_directx12.constant_buffer->lpVtbl->GetGPUVirtualAddress(model_common->model_directx12.animation_constant_buffer);
    gbuffer->gbuffer_common.gbuffer_directx12.command_list->lpVtbl->SetGraphicsRootConstantBufferView(gbuffer->gbuffer_common.gbuffer_directx12.command_list, 7, animation_cbv_address);
  }

  // TODO: These will need to be changed like with sprite, need to put a sampler heap from texture manager?
  ID3D12DescriptorHeap* descriptor_heaps[] = {model_common->model_diffuse_texture->texture_common.texture_manager_common->texture_manager_directx12.srv_heap, model_common->model_diffuse_texture->texture_common.texture_manager_common->texture_manager_directx12.sampler_heap};
  // ID3D12DescriptorHeap* descriptor_heaps[] = {model_common->model_diffuse_texture->texture_common.texture_manager_common->texture_manager_directx12.srv_heap, model_common->shader_handle->shader_common.shader_directx12.sampler_heap};
  gbuffer->gbuffer_common.gbuffer_directx12.command_list->lpVtbl->SetDescriptorHeaps(gbuffer->gbuffer_common.gbuffer_directx12.command_list, _countof(descriptor_heaps), descriptor_heaps);

  gbuffer->gbuffer_common.gbuffer_directx12.command_list->lpVtbl->SetGraphicsRootDescriptorTable(gbuffer->gbuffer_common.gbuffer_directx12.command_list, 2, model_common->model_diffuse_texture->texture_common.texture_directx12.srv_gpu_handle);
  gbuffer->gbuffer_common.gbuffer_directx12.command_list->lpVtbl->SetGraphicsRootDescriptorTable(gbuffer->gbuffer_common.gbuffer_directx12.command_list, 3, model_common->model_normal_texture->texture_common.texture_directx12.srv_gpu_handle);
  gbuffer->gbuffer_common.gbuffer_directx12.command_list->lpVtbl->SetGraphicsRootDescriptorTable(gbuffer->gbuffer_common.gbuffer_directx12.command_list, 4, model_common->model_metallic_texture->texture_common.texture_directx12.srv_gpu_handle);
  gbuffer->gbuffer_common.gbuffer_directx12.command_list->lpVtbl->SetGraphicsRootDescriptorTable(gbuffer->gbuffer_common.gbuffer_directx12.command_list, 5, model_common->model_roughness_texture->texture_common.texture_directx12.srv_gpu_handle);
  gbuffer->gbuffer_common.gbuffer_directx12.command_list->lpVtbl->SetGraphicsRootDescriptorTable(gbuffer->gbuffer_common.gbuffer_directx12.command_list, 6, model_common->model_ao_texture->texture_common.texture_directx12.srv_gpu_handle);
  // TODO: These will need to be changed like with sprite, need to bind the sampler from the specific texture, not the shader(it should be gone from there too)
  if (model_common->animated) {
    gbuffer->gbuffer_common.gbuffer_directx12.command_list->lpVtbl->SetGraphicsRootDescriptorTable(gbuffer->gbuffer_common.gbuffer_directx12.command_list, 8, model_common->model_diffuse_texture->texture_common.texture_directx12.sampler_gpu_handle);
    // gbuffer->gbuffer_common.gbuffer_directx12.command_list->lpVtbl->SetGraphicsRootDescriptorTable(gbuffer->gbuffer_common.gbuffer_directx12.command_list, 8, model_common->shader_handle->shader_common.shader_directx12.sampler_handle_gpu);
  } else {
    gbuffer->gbuffer_common.gbuffer_directx12.command_list->lpVtbl->SetGraphicsRootDescriptorTable(gbuffer->gbuffer_common.gbuffer_directx12.command_list, 7, model_common->model_diffuse_texture->texture_common.texture_directx12.sampler_gpu_handle);
    // gbuffer->gbuffer_common.gbuffer_directx12.command_list->lpVtbl->SetGraphicsRootDescriptorTable(gbuffer->gbuffer_common.gbuffer_directx12.command_list, 7, model_common->shader_handle->shader_common.shader_directx12.sampler_handle_gpu);
  }

  D3D12_VERTEX_BUFFER_VIEW vbv = model_common->model_directx12.vertex_buffer_view;
  gbuffer->gbuffer_common.gbuffer_directx12.command_list->lpVtbl->IASetVertexBuffers(gbuffer->gbuffer_common.gbuffer_directx12.command_list, 0, 1, &vbv);

  D3D12_INDEX_BUFFER_VIEW ibv = model_common->model_directx12.index_buffer_view;
  gbuffer->gbuffer_common.gbuffer_directx12.command_list->lpVtbl->IASetIndexBuffer(gbuffer->gbuffer_common.gbuffer_directx12.command_list, &ibv);

  gbuffer->gbuffer_common.gbuffer_directx12.command_list->lpVtbl->IASetPrimitiveTopology(gbuffer->gbuffer_common.gbuffer_directx12.command_list, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);  // Assuming triangles, adjust as necessary

  gbuffer->gbuffer_common.gbuffer_directx12.command_list->lpVtbl->DrawIndexedInstanced(gbuffer->gbuffer_common.gbuffer_directx12.command_list, (UINT)model_common->model_mesh->mesh_common.indices->size, 1, 0, 0, 0);
}

void model_directx_12_update_uniforms(struct ModelCommon* model_common, struct APICommon* api_common, struct GBuffer* gbuffer, vec3d position, vec4 light_pos, vec4 diffuse_color, vec4 ambient_color, vec4 specular_light) {
  struct LightingUniformBufferObject light_ubo = {0};
  light_ubo.direction = light_pos;
  light_ubo.ambient_color = ambient_color;
  light_ubo.diffuse_color = diffuse_color;
  light_ubo.specular_color = specular_light;

  struct ModelUniformBufferObject ubom = {0};
  ubom.proj = gbuffer->gbuffer_common.projection_matrix;
  ubom.proj = mat4_transpose(ubom.proj);

  ubom.view = gbuffer->gbuffer_common.view_matrix;
  ubom.view = mat4_transpose(ubom.view);

  ubom.model = mat4_translate(MAT4_IDENTITY, model_common->position);
  ubom.model = mat4_mul(ubom.model, quaternion_to_mat4(quaternion_normalise(model_common->rotation)));
  ubom.model = mat4_scale(ubom.model, model_common->scale);

  ubom.camera_pos = vec3d_to_vec3(position);
  ubom.model = mat4_transpose(ubom.model);

  // Map the constant buffer to update it
  void* data;
  HRESULT hr = model_common->model_directx12.constant_buffer->lpVtbl->Map(model_common->model_directx12.constant_buffer, 0, NULL, &data);
  if (SUCCEEDED(hr)) {
    memcpy(data, &ubom, sizeof(struct ModelUniformBufferObject));
    model_common->model_directx12.constant_buffer->lpVtbl->Unmap(model_common->model_directx12.constant_buffer, 0, NULL);
  } else
    log_message(LOG_SEVERITY_ERROR, "Failed to map constant buffer.\n");

  if (model_common->animated) {
    struct ModelAnimationUniformBufferObject uboa = {{{0}}};
    model_get_joint_transforms(model_common->root_joint, uboa.joint_transforms);
    for (size_t joint_num = 0; joint_num < MAX_JOINTS; joint_num++)
      uboa.joint_transforms[joint_num] = mat4_transpose(uboa.joint_transforms[joint_num]);

    void* animation_data;
    hr = model_common->model_directx12.animation_constant_buffer->lpVtbl->Map(model_common->model_directx12.animation_constant_buffer, 0, NULL, &animation_data);
    if (SUCCEEDED(hr)) {
      memcpy(animation_data, &uboa, sizeof(struct ModelAnimationUniformBufferObject));
      model_common->model_directx12.animation_constant_buffer->lpVtbl->Unmap(model_common->model_directx12.animation_constant_buffer, 0, NULL);
    } else
      log_message(LOG_SEVERITY_ERROR, "Failed to map constant buffer.\n");
  }

  void* lighting_data;
  hr = model_common->model_directx12.lighting_constant_buffer->lpVtbl->Map(model_common->model_directx12.lighting_constant_buffer, 0, NULL, &lighting_data);
  if (SUCCEEDED(hr)) {
    memcpy(lighting_data, &light_ubo, sizeof(struct LightingUniformBufferObject));
    model_common->model_directx12.lighting_constant_buffer->lpVtbl->Unmap(model_common->model_directx12.lighting_constant_buffer, 0, NULL);
  } else
    log_message(LOG_SEVERITY_ERROR, "Failed to map constant buffer.\n");
}
