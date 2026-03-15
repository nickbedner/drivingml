#include "mana/graphics/shaders/shadervulkan.h"

static VkShaderModule shader_create_shader_module(struct APICommon* api_common, const uint32_t* code, uint_fast64_t length) {
  VkShaderModuleCreateInfo create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.codeSize = length;
  create_info.pCode = code;

  VkShaderModule shader_module;
  if (vkCreateShaderModule(api_common->vulkan_api.device, &create_info, NULL, &shader_module) != VK_SUCCESS)
    fprintf(stderr, "Failed to create shader module!\n");

  return shader_module;
}

uint_fast8_t shader_vulkan_init(struct ShaderCommon* shader_common, struct APICommon* api_common, uint32_t width, uint32_t height, uint_fast8_t supersample_scale) {
  if (shader_common->shader_settings.descriptors > 0) {
    VkDescriptorSetLayoutBinding bindings[SHADER_ATTACHMENT_LIMIT * 2];
    memset(bindings, 0, sizeof(VkDescriptorSetLayoutBinding) * SHADER_ATTACHMENT_LIMIT * 2);
    VkDescriptorPoolSize pool_sizes[SHADER_ATTACHMENT_LIMIT * 2];
    memset(pool_sizes, 0, sizeof(VkDescriptorPoolSize) * SHADER_ATTACHMENT_LIMIT * 2);
    for (uint_fast8_t uniform_num = 0; uniform_num < shader_common->shader_settings.uniforms_constants; uniform_num++) {
      VkDescriptorSetLayoutBinding ubo_layout_binding = {0};
      ubo_layout_binding.binding = shader_common->shader_settings.uniform_constant_state[uniform_num].shader_position;
      ubo_layout_binding.descriptorCount = 1;
      ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      ubo_layout_binding.pImmutableSamplers = NULL;
      if (shader_common->shader_settings.uniform_constant_state[uniform_num].shader_stage == SHADER_STAGE_VERTEX)
        ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
      else if (shader_common->shader_settings.uniform_constant_state[uniform_num].shader_stage == SHADER_STAGE_FRAGMENT)
        ubo_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
      else if (shader_common->shader_settings.uniform_constant_state[uniform_num].shader_stage == SHADER_STAGE_VERTEX_AND_FRAGMENT)
        ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

      bindings[shader_common->shader_settings.uniform_constant_state[uniform_num].shader_position] = ubo_layout_binding;
      pool_sizes[shader_common->shader_settings.uniform_constant_state[uniform_num].shader_position].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      pool_sizes[shader_common->shader_settings.uniform_constant_state[uniform_num].shader_position].descriptorCount = shader_common->shader_settings.descriptors;
    }

    for (uint_fast8_t sampler_num = 0; sampler_num < shader_common->shader_settings.texture_samples; sampler_num++) {
      VkDescriptorSetLayoutBinding sampler_layout_binding = {0};
      sampler_layout_binding.binding = shader_common->shader_settings.texture_sample_state[sampler_num].shader_position;
      sampler_layout_binding.descriptorCount = 1;
      sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      sampler_layout_binding.pImmutableSamplers = NULL;
      sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
      if (shader_common->shader_settings.texture_sample_state[sampler_num].shader_stage == SHADER_STAGE_VERTEX)
        sampler_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
      else if (shader_common->shader_settings.texture_sample_state[sampler_num].shader_stage == SHADER_STAGE_FRAGMENT)
        sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
      else if (shader_common->shader_settings.texture_sample_state[sampler_num].shader_stage == SHADER_STAGE_VERTEX_AND_FRAGMENT)
        sampler_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

      bindings[shader_common->shader_settings.texture_sample_state[sampler_num].shader_position] = sampler_layout_binding;
      pool_sizes[shader_common->shader_settings.texture_sample_state[sampler_num].shader_position].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      pool_sizes[shader_common->shader_settings.texture_sample_state[sampler_num].shader_position].descriptorCount = shader_common->shader_settings.descriptors;
    }

    VkDescriptorSetLayoutCreateInfo layout_info = {0};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = shader_common->shader_settings.uniforms_constants + shader_common->shader_settings.texture_samples;
    layout_info.pBindings = bindings;

    if (vkCreateDescriptorSetLayout(api_common->vulkan_api.device, &layout_info, NULL, &(shader_common->shader_vulkan.descriptor_set_layout)) != VK_SUCCESS)
      return 0;

    VkDescriptorPoolCreateInfo pool_info = {0};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = shader_common->shader_settings.uniforms_constants + shader_common->shader_settings.texture_samples;
    pool_info.pPoolSizes = pool_sizes;
    pool_info.maxSets = shader_common->shader_settings.descriptors;

    if (vkCreateDescriptorPool(api_common->vulkan_api.device, &pool_info, NULL, &(shader_common->shader_vulkan.descriptor_pool)) != VK_SUCCESS) {
      fprintf(stderr, "failed to create descriptor pool!\n");
      return 0;
    }
  }

  VkPipelineVertexInputStateCreateInfo vertex_input_info = {0};
  vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

  VkVertexInputBindingDescription binding_description = vulkan_graphics_utils_get_binding_description(shader_common->shader_settings.mesh_memory_size);
  VkVertexInputAttributeDescription attribute_descriptions[SHADER_ATTACHMENT_LIMIT];
  if (shader_common->shader_settings.vertex_attributes > 0) {
    if (shader_common->shader_settings.vertex_attributes > SHADER_ATTACHMENT_LIMIT) {
      log_message(LOG_SEVERITY_ERROR, "Too many vertex attributes! Max is %d\n", SHADER_ATTACHMENT_LIMIT);
      return 1;
    }
    // VkVertexInputAttributeDescription *attribute_descriptions = calloc(shader_common->shader_settings.attributes, sizeof(VkVertexInputAttributeDescription));
    memset(attribute_descriptions, 0, sizeof(VkVertexInputAttributeDescription) * SHADER_ATTACHMENT_LIMIT);
    mesh_get_attribute_descriptions(shader_common->shader_settings.mesh_type, attribute_descriptions);

    if (shader_common->shader_settings.vertex_attributes == 2 && (attribute_descriptions[1].format == VK_FORMAT_UNDEFINED || attribute_descriptions[1].location == attribute_descriptions[0].location)) {
      attribute_descriptions[0].location = 0;
      attribute_descriptions[0].binding = 0;
      attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
      attribute_descriptions[0].offset = 0;

      attribute_descriptions[1].location = 1;
      attribute_descriptions[1].binding = 0;
      attribute_descriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
      attribute_descriptions[1].offset = sizeof(float) * 3;
    }

    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.vertexAttributeDescriptionCount = shader_common->shader_settings.vertex_attributes;
    vertex_input_info.pVertexBindingDescriptions = &binding_description;
    vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions;
  } else {
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.vertexAttributeDescriptionCount = 0;  // Note: length of attributeDescriptions
    vertex_input_info.pVertexBindingDescriptions = &binding_description;
    vertex_input_info.pVertexAttributeDescriptions = NULL;
  }

  VkBool32 should_blend = (shader_common->shader_settings.blend) ? VK_TRUE : VK_FALSE;
  // VkPipelineColorBlendAttachmentState *color_blend_attachment = calloc(shader_common->shader_settings.color_attachments, sizeof(VkPipelineColorBlendAttachmentState));
  VkPipelineColorBlendAttachmentState color_blend_attachment[SHADER_ATTACHMENT_LIMIT];
  memset(color_blend_attachment, 0, sizeof(VkPipelineColorBlendAttachmentState) * SHADER_ATTACHMENT_LIMIT);
  for (uint_fast8_t pipeline_attachment_num = 0; pipeline_attachment_num < shader_common->shader_settings.color_attachments; pipeline_attachment_num++) {
    color_blend_attachment[pipeline_attachment_num].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment[pipeline_attachment_num].blendEnable = should_blend;

    color_blend_attachment[pipeline_attachment_num].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment[pipeline_attachment_num].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment[pipeline_attachment_num].colorBlendOp = VK_BLEND_OP_ADD;

    color_blend_attachment[pipeline_attachment_num].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment[pipeline_attachment_num].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment[pipeline_attachment_num].alphaBlendOp = VK_BLEND_OP_ADD;
  }

  VkPipelineColorBlendStateCreateInfo color_blending = {0};
  color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  color_blending.logicOpEnable = VK_FALSE;
  color_blending.logicOp = VK_LOGIC_OP_COPY;
  color_blending.attachmentCount = shader_common->shader_settings.color_attachments;
  color_blending.pAttachments = color_blend_attachment;
  color_blending.blendConstants[0] = 0.0f;
  color_blending.blendConstants[1] = 0.0f;
  color_blending.blendConstants[2] = 0.0f;
  color_blending.blendConstants[3] = 0.0f;
  /////////////////////////////////////////////////////////////////////////////
  char vertex_path[MAX_LENGTH_OF_PATH];
  char fragment_path[MAX_LENGTH_OF_PATH];

  snprintf(vertex_path, MAX_LENGTH_OF_PATH, "%s/shaders/spirv/%s.vert.spv", api_common->asset_directory, shader_common->shader_settings.vertex_shader);
  snprintf(fragment_path, MAX_LENGTH_OF_PATH, "%s/shaders/spirv/%s.frag.spv", api_common->asset_directory, shader_common->shader_settings.fragment_shader);

  uint_fast64_t vertex_code_length = 0;
  uint_fast64_t fragment_code_length = 0;

  uint32_t* vertex_shader_code = read_shader_file(vertex_path, &vertex_code_length);
  uint32_t* fragment_shader_code = read_shader_file(fragment_path, &fragment_code_length);

  VkShaderModule vert_shader_module = shader_create_shader_module(api_common, vertex_shader_code, vertex_code_length);
  VkShaderModule frag_shader_module = shader_create_shader_module(api_common, fragment_shader_code, fragment_code_length);

  close_shader_file(vertex_shader_code);
  close_shader_file(fragment_shader_code);
  /////////////////////////////////////////////////////////////////////////////
  VkPipelineShaderStageCreateInfo vert_shader_stage_info = {0};
  vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vert_shader_stage_info.module = vert_shader_module;
  vert_shader_stage_info.pName = "main";

  VkPipelineShaderStageCreateInfo frag_shader_stage_info = {0};
  frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  frag_shader_stage_info.module = frag_shader_module;
  frag_shader_stage_info.pName = "main";

  VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stage_info, frag_shader_stage_info};

  VkPipelineInputAssemblyStateCreateInfo input_assembly = {0};
  input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  input_assembly.primitiveRestartEnable = VK_FALSE;

  shader_vulkan_resize(shader_common, api_common, width, height, supersample_scale);

  VkPipelineViewportStateCreateInfo viewport_state = {0};
  viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_state.viewportCount = 1;
  viewport_state.pViewports = &(shader_common->shader_vulkan.viewport);
  viewport_state.scissorCount = 1;
  viewport_state.pScissors = &(shader_common->shader_vulkan.scissor);

  VkCullModeFlags cull_mode = VK_CULL_MODE_NONE;
  switch (shader_common->shader_settings.cull_mode) {
    case SHADER_CULL_MODE_NONE: {
      cull_mode = VK_CULL_MODE_NONE;
      break;
    }
    case SHADER_CULL_MODE_FRONT_BIT: {
      cull_mode = VK_CULL_MODE_FRONT_BIT;
      break;
    }
    case SHADER_CULL_MODE_BACK_BIT: {
      cull_mode = VK_CULL_MODE_BACK_BIT;
      break;
    }
    case SHADER_CULL_MODE_FRONT_AND_BACK_BIT: {
      cull_mode = VK_CULL_MODE_FRONT_AND_BACK;
      break;
    }
  }

  VkFrontFace direction = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  if (shader_common->shader_settings.front_face == SHADER_FRONT_FACE_CLOCKWISE)
    direction = VK_FRONT_FACE_CLOCKWISE;

  VkPipelineRasterizationStateCreateInfo rasterizer = {0};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = cull_mode;
  rasterizer.frontFace = direction;
  rasterizer.depthBiasEnable = VK_FALSE;

  VkPipelineMultisampleStateCreateInfo multisampling = {0};
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = (VkSampleCountFlagBits)shader_common->shader_settings.num_msaa_samples;

  VkBool32 depth_test = VK_FALSE;
  if (shader_common->shader_settings.depth_test)
    depth_test = VK_TRUE;

  VkBool32 depth_write = VK_FALSE;
  if (shader_common->shader_settings.depth_write)
    depth_write = VK_TRUE;

  VkPipelineDepthStencilStateCreateInfo depth_stencil = {0};
  depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depth_stencil.depthTestEnable = depth_test;
  depth_stencil.depthWriteEnable = depth_write;
  depth_stencil.depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;
  depth_stencil.depthBoundsTestEnable = VK_FALSE;
  depth_stencil.stencilTestEnable = VK_FALSE;

  VkPipelineLayoutCreateInfo pipeline_layout_info = {0};
  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_info.setLayoutCount = 1;
  pipeline_layout_info.pSetLayouts = &(shader_common->shader_vulkan.descriptor_set_layout);

  if (vkCreatePipelineLayout(api_common->vulkan_api.device, &pipeline_layout_info, NULL, &(shader_common->shader_vulkan.pipeline_layout)) != VK_SUCCESS)
    return 0;

  VkPipelineDynamicStateCreateInfo dynamic_state_info = {0};
  dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
  dynamic_state_info.dynamicStateCount = 2;
  dynamic_state_info.pDynamicStates = dynamicStates;

  VkGraphicsPipelineCreateInfo pipeline_info = {0};
  pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_info.stageCount = 2;
  pipeline_info.pStages = shader_stages;
  pipeline_info.pVertexInputState = &vertex_input_info;
  pipeline_info.pInputAssemblyState = &input_assembly;
  pipeline_info.pViewportState = &viewport_state;
  pipeline_info.pRasterizationState = &rasterizer;
  pipeline_info.pMultisampleState = &multisampling;
  pipeline_info.pDepthStencilState = &depth_stencil;
  pipeline_info.pColorBlendState = &color_blending;
  pipeline_info.pDynamicState = &dynamic_state_info;
  pipeline_info.layout = shader_common->shader_vulkan.pipeline_layout;
  pipeline_info.renderPass = (VkRenderPass)shader_common->shader_settings.extra_data;
  pipeline_info.subpass = 0;
  pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

  if (vkCreateGraphicsPipelines(api_common->vulkan_api.device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &(shader_common->shader_vulkan.graphics_pipeline)) != VK_SUCCESS) {
    log_message(LOG_SEVERITY_ERROR, "Failed to create pipeline layout!\n");
    return VULKAN_RENDERER_CREATE_GRAPHICS_PIPELINE_ERROR;
  }

  vkDestroyShaderModule(api_common->vulkan_api.device, frag_shader_module, NULL);
  vkDestroyShaderModule(api_common->vulkan_api.device, vert_shader_module, NULL);

  return 0;
}

uint_fast8_t shader_compute_vulkan_init(struct ShaderCommon* shader, struct APICommon* api_common) {
  uint_fast64_t compute_length = 0;

  uint32_t* compute_shader_code = read_shader_file(shader->shader_settings_compute.compute_shader, &compute_length);

  VkShaderModule comp_shader_module = shader_create_shader_module(api_common, compute_shader_code, compute_length);

  close_shader_file(compute_shader_code);

  VkPipelineLayoutCreateInfo pipeline_layout_create_info = {0};
  pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_create_info.setLayoutCount = 1;
  pipeline_layout_create_info.pSetLayouts = &(shader->shader_vulkan.descriptor_set_layout);

  if (vkCreatePipelineLayout(api_common->vulkan_api.device, &pipeline_layout_create_info, NULL, &(shader->shader_vulkan.pipeline_layout)))
    return 1;

  VkComputePipelineCreateInfo pipeline_info = {0};
  pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
  pipeline_info.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  pipeline_info.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
  pipeline_info.stage.module = comp_shader_module;

  pipeline_info.stage.pName = "main";
  pipeline_info.layout = shader->shader_vulkan.pipeline_layout;

  if (vkCreateComputePipelines(api_common->vulkan_api.device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &(shader->shader_vulkan.graphics_pipeline)) != VK_SUCCESS) {
    log_message(LOG_SEVERITY_ERROR, "Failed to create pipeline layout!\n");
    return VULKAN_RENDERER_CREATE_GRAPHICS_PIPELINE_ERROR;
  }

  vkDestroyShaderModule(api_common->vulkan_api.device, comp_shader_module, NULL);

  return 0;
}

void shader_vulkan_delete(struct ShaderCommon* shader_common, struct APICommon* api_common) {
  vkDestroyDescriptorPool(api_common->vulkan_api.device, shader_common->shader_vulkan.descriptor_pool, NULL);
  vkDestroyPipeline(api_common->vulkan_api.device, shader_common->shader_vulkan.graphics_pipeline, NULL);
  vkDestroyPipelineLayout(api_common->vulkan_api.device, shader_common->shader_vulkan.pipeline_layout, NULL);
  vkDestroyDescriptorSetLayout(api_common->vulkan_api.device, shader_common->shader_vulkan.descriptor_set_layout, NULL);
}

void shader_vulkan_resize(struct ShaderCommon* shader_common, struct APICommon* api_common, uint32_t width, uint32_t height, uint_fast8_t supersample_scale) {
  shader_common->shader_vulkan.viewport.x = 0.0f;
  shader_common->shader_vulkan.viewport.y = (float)height;
  shader_common->shader_vulkan.viewport.minDepth = 0.0f;
  shader_common->shader_vulkan.viewport.maxDepth = 1.0f;
  if (shader_common->shader_settings.supersampled) {
    shader_common->shader_vulkan.viewport.width = (float)width * supersample_scale;
    shader_common->shader_vulkan.viewport.height = -(float)height * supersample_scale;
  } else {
    shader_common->shader_vulkan.viewport.width = (float)width;
    shader_common->shader_vulkan.viewport.height = -(float)height;
  }

  shader_common->shader_vulkan.scissor.offset.x = 0;
  shader_common->shader_vulkan.scissor.offset.y = 0;
  if (shader_common->shader_settings.supersampled) {
    shader_common->shader_vulkan.scissor.extent.width = (uint32_t)((float)width * supersample_scale);
    shader_common->shader_vulkan.scissor.extent.height = (uint32_t)((float)height * supersample_scale);
  } else {
    shader_common->shader_vulkan.scissor.extent.width = width;
    shader_common->shader_vulkan.scissor.extent.height = height;
  }
}
