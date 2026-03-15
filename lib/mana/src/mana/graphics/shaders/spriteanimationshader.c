#include "mana/graphics/shaders/spriteanimationshader.h"

int sprite_animation_shader_init(struct SpriteAnimationShader* sprite_animation_shader, struct APICommon* api_common, uint32_t width, uint32_t height, uint_fast8_t supersample_scale, struct GBufferCommon* gbuffer_common, bool depth_test, const uint_fast32_t msaa_samples, uint_fast32_t descriptors) {
  struct ShaderSettings* shader_settings = &(sprite_animation_shader->shader.shader_common.shader_settings);
  shader_settings->vertex_shader = "spriteanimation";
  shader_settings->fragment_shader = "spriteanimation";
  shader_settings->front_face = SHADER_FRONT_FACE_COUNTER_CLOCKWISE;
  shader_settings->cull_mode = SHADER_CULL_MODE_BACK_BIT;  // SHADER_CULL_MODE_NONE;
  shader_settings->depth_test = depth_test;
  shader_settings->supersampled = true;
  shader_settings->num_msaa_samples = msaa_samples;
  shader_settings->color_attachments = SPRITE_ANIMATION_SHADER_COLOR_ATTACHEMENTS;
  shader_settings->vertex_attributes = SPRITE_ANIMATION_SHADER_VERTEX_ATTRIBUTES;
  shader_settings->blend = false;
  shader_settings->mesh_type = MESH_TYPE_SPRITE;
  shader_settings->mesh_memory_size = mesh_get_memory_size(shader_settings->mesh_type);
  shader_settings->uniforms_constants = 1;
  shader_settings->uniform_constant_state[0] = (struct ShaderAttributeSettings){.shader_stage = SHADER_STAGE_VERTEX_AND_FRAGMENT, .shader_position = 0};
  shader_settings->texture_samples = 1;
  shader_settings->texture_sample_state[0] = (struct ShaderAttributeSettings){.shader_stage = SHADER_STAGE_FRAGMENT, .shader_position = 1};
  shader_settings->render_targets = 3;
  shader_settings->render_target_format[0] = SHADER_RENDER_TARGET_FORMAT_R8G8B8A8_UNORM;
  shader_settings->render_target_format[1] = SHADER_RENDER_TARGET_FORMAT_R8G8B8A8_UNORM;
  shader_settings->render_target_format[2] = SHADER_RENDER_TARGET_FORMAT_D32_FLOAT;
  shader_settings->descriptors = descriptors;
// Note: This is a hack to get the render pass to the shader for vulkan
#ifdef VULKAN_API_SUPPORTED
  if (api_common->api_type == API_VULKAN)
    shader_settings->extra_data = gbuffer_common->gbuffer_vulkan.render_pass;
#endif

  shader_init(&(sprite_animation_shader->shader), api_common, width, height, supersample_scale);

  // VkDescriptorSetLayoutBinding ubo_layout_binding = {0};
  // ubo_layout_binding.binding = 0;
  // ubo_layout_binding.descriptorCount = 1;
  // ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  // ubo_layout_binding.pImmutableSamplers = NULL;
  //// ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  // ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  //
  // VkDescriptorSetLayoutBinding sampler_layout_binding = {0};
  // sampler_layout_binding.binding = 1;
  // sampler_layout_binding.descriptorCount = 1;
  // sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  // sampler_layout_binding.pImmutableSamplers = NULL;
  // sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  //
  // VkDescriptorSetLayoutBinding bindings[2] = {ubo_layout_binding, sampler_layout_binding};
  // VkDescriptorSetLayoutCreateInfo layout_info = {0};
  // layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  // layout_info.bindingCount = 2;
  // layout_info.pBindings = bindings;
  //
  // if (vkCreateDescriptorSetLayout(api_common->vulkan_api.device, &layout_info, NULL, &sprite_animation_shader->shader.descriptor_set_layout) != VK_SUCCESS)
  //  return 0;
  //
  // int sprite_descriptors = 64;
  // VkDescriptorPoolSize pool_sizes[2] = {{0}};
  // pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  // pool_sizes[0].descriptorCount = sprite_descriptors; // Max number of uniform descriptors
  // pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  // pool_sizes[1].descriptorCount = sprite_descriptors; // Max number of image sampler descriptors
  //
  // VkDescriptorPoolCreateInfo pool_info = {0};
  // pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  // pool_info.poolSizeCount = 2; // Number of things being passed to GPU
  // pool_info.pPoolSizes = pool_sizes;
  // pool_info.maxSets = sprite_descriptors; // Max number of sets made from this pool
  //
  // if (vkCreateDescriptorPool(api_common->vulkan_api.device, &pool_info, NULL, &sprite_animation_shader->shader.descriptor_pool) != VK_SUCCESS) {
  //  fprintf(stderr, "failed to create descriptor pool!\n");
  //  return 0;
  //}
  //
  // VkPipelineVertexInputStateCreateInfo vertex_input_info = {0};
  // vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  //
  // VkVertexInputBindingDescription binding_description = vulkan_graphics_utils_get_binding_description(sprite_animation_shader->shader.shader_common.shader_settings.mesh_memory_size);
  // VkVertexInputAttributeDescription attribute_descriptions[SPRITE_ANIMATION_SHADER_VERTEX_ATTRIBUTES];
  // memset(attribute_descriptions, 0, sizeof(attribute_descriptions));
  // mesh_get_attribute_descriptions(sprite_animation_shader->shader.shader_common.shader_settings.mesh_type, attribute_descriptions);
  //
  // vertex_input_info.vertexBindingDescriptionCount = 1;
  // vertex_input_info.vertexAttributeDescriptionCount = SPRITE_ANIMATION_SHADER_VERTEX_ATTRIBUTES;
  // vertex_input_info.pVertexBindingDescriptions = &binding_description;
  // vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions;
  //
  //// Note: Independent blending is for certain devices only, all attachments must blend the same otherwise
  // VkPipelineColorBlendAttachmentState color_blend_attachments[SPRITE_ANIMATION_SHADER_COLOR_ATTACHEMENTS];
  // memset(color_blend_attachments, 0, sizeof(VkPipelineColorBlendAttachmentState) * SPRITE_ANIMATION_SHADER_COLOR_ATTACHEMENTS);
  // for (int pipeline_attachment_num = 0; pipeline_attachment_num < SPRITE_ANIMATION_SHADER_COLOR_ATTACHEMENTS; pipeline_attachment_num++) {
  //   color_blend_attachments[pipeline_attachment_num].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  //   color_blend_attachments[pipeline_attachment_num].blendEnable = VK_TRUE;
  //   color_blend_attachments[pipeline_attachment_num].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
  //   color_blend_attachments[pipeline_attachment_num].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  //   color_blend_attachments[pipeline_attachment_num].colorBlendOp = VK_BLEND_OP_ADD;
  //   color_blend_attachments[pipeline_attachment_num].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  //   color_blend_attachments[pipeline_attachment_num].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  //   color_blend_attachments[pipeline_attachment_num].alphaBlendOp = VK_BLEND_OP_ADD;
  // }
  //
  // VkPipelineColorBlendStateCreateInfo color_blending = {0};
  // color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  // color_blending.logicOpEnable = VK_FALSE;
  // color_blending.logicOp = VK_LOGIC_OP_COPY;
  // color_blending.attachmentCount = SPRITE_ANIMATION_SHADER_COLOR_ATTACHEMENTS;
  // color_blending.pAttachments = color_blend_attachments;
  // color_blending.blendConstants[0] = 0.0f;
  // color_blending.blendConstants[1] = 0.0f;
  // color_blending.blendConstants[2] = 0.0f;
  // color_blending.blendConstants[3] = 0.0f;
  //
  // shader_init(&sprite_animation_shader->shader, &api_common->vulkan_api, "./assets/shaders/spirv/spriteanimation.vert.spv", "./assets/shaders/spirv/spriteanimation.frag.spv", NULL, vertex_input_info, api_common->vulkan_api.gbuffer->render_pass, color_blending, VK_FRONT_FACE_COUNTER_CLOCKWISE, depth_test, api_common->vulkan_api.msaa_samples, 0, VK_CULL_MODE_BACK_BIT);

  return 0;
}

void sprite_animation_shader_delete(struct SpriteAnimationShader* sprite_animation_shader, struct APICommon* api_common) {
  shader_delete(&sprite_animation_shader->shader, api_common);

  // vkDestroyDescriptorPool(api_common->vulkan_api.device, sprite_animation_shader->shader.shader_common.shader_vulkan.descriptor_pool, NULL);
}
