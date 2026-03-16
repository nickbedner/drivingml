#include "mana/graphics/shaders/shaderdirectx12.h"

static HRESULT __stdcall shader_directx_12_open(ID3DInclude* this, D3D_INCLUDE_TYPE include_type, LPCSTR p_file_name, LPCVOID p_parent_data, LPCVOID* pp_data, UINT* p_bytes) {
  char full_path[MAX_PATH_LENGTH];
  FILE* file;
  errno_t err;

  // First, try to open the file using the provided name
  err = fopen_s(&file, p_file_name, "rb");

  if (err != 0 || !file) {
    // If that fails, try the common directory
    snprintf(full_path, MAX_PATH_LENGTH, "./assets/shaders/hlsl/%s", p_file_name);
    err = fopen_s(&file, full_path, "rb");

    if (err != 0 || !file) {
      log_message(LOG_SEVERITY_ERROR, "Failed to open shader include: %s\n", p_file_name);
      return E_FAIL;
    }
  }

  fseek(file, 0, SEEK_END);
  int64_t file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  char* buffer = (char*)malloc((size_t)file_size);
  fread(buffer, 1, (size_t)file_size, file);
  fclose(file);

  *pp_data = buffer;
  *p_bytes = (UINT)file_size;

  return S_OK;
}

static HRESULT __stdcall shader_directx_12_close(ID3DInclude* this, LPCVOID p_data) {
  // Note: This silences a warning about casting away const
  free((void*)(uintptr_t)p_data);
  return S_OK;
}

uint_fast8_t shader_directx_12_init(struct ShaderCommon* shader_common, struct APICommon* api_common, uint32_t width, uint32_t height, uint_fast8_t supersample_scale) {
  // Define the vtable and structure inside the function
  ID3DIncludeVtbl v_table_c = {shader_directx_12_open, shader_directx_12_close};

  struct D3DIncludeC {
    ID3DIncludeVtbl* lp_vtbl;
  };

  struct D3DIncludeC include_handler = {&v_table_c};

  wchar_t vertex_path[MAX_LENGTH_OF_PATH];
  wchar_t fragment_path[MAX_LENGTH_OF_PATH];

  // Build full shader paths directly
  swprintf_s(vertex_path, MAX_LENGTH_OF_PATH, L"%hs/shaders/hlsl/%hs.hlsl", api_common->asset_directory, shader_common->shader_settings.vertex_shader);
  swprintf_s(fragment_path, MAX_LENGTH_OF_PATH, L"%hs/shaders/hlsl/%hs.hlsl", api_common->asset_directory, shader_common->shader_settings.fragment_shader);

  wprintf(L"Vertex shader path: %ls\n", vertex_path);
  wprintf(L"Fragment shader path: %ls\n", fragment_path);

  ID3DBlob* vertex_errors = NULL;
  ID3DBlob* fragment_errors = NULL;

  HRESULT vertex_result = D3DCompileFromFile(vertex_path, NULL, (ID3DInclude*)&include_handler, "VS_main", "vs_5_0", 0, 0, &shader_common->shader_directx12.vertex_shader_blob, &vertex_errors);
  HRESULT fragment_result = D3DCompileFromFile(fragment_path, NULL, (ID3DInclude*)&include_handler, "PS_main", "ps_5_0", 0, 0, &shader_common->shader_directx12.fragment_shader_blob, &fragment_errors);

  if (FAILED(vertex_result)) {
    if (vertex_errors) {
      log_message(LOG_SEVERITY_ERROR, "Vertex Shader Compilation:\n%s\n", (char*)vertex_errors->lpVtbl->GetBufferPointer(vertex_errors));

      vertex_errors->lpVtbl->Release(vertex_errors);
    }
  }

  if (FAILED(fragment_result)) {
    if (fragment_errors) {
      log_message(LOG_SEVERITY_ERROR, "Fragment Shader Compilation:\n%s\n", (char*)fragment_errors->lpVtbl->GetBufferPointer(fragment_errors));

      fragment_errors->lpVtbl->Release(fragment_errors);
    }
  }

  if (FAILED(vertex_result) || FAILED(fragment_result)) {
    log_message(LOG_SEVERITY_ERROR, "Shader compilation failed\n");
    return 1;
  }

  shader_directx_12_resize(shader_common, api_common, width, height, supersample_scale);

  D3D12_RASTERIZER_DESC rasterizer_desc;
  ZeroMemory(&rasterizer_desc, sizeof(rasterizer_desc));  // Initialize the struct to zero

  enum D3D12_CULL_MODE cull_mode;
  if (shader_common->shader_settings.cull_mode == SHADER_CULL_MODE_BACK_BIT)
    cull_mode = D3D12_CULL_MODE_BACK;
  else if (shader_common->shader_settings.cull_mode == SHADER_CULL_MODE_FRONT_BIT)
    cull_mode = D3D12_CULL_MODE_FRONT;
  else if (shader_common->shader_settings.cull_mode == SHADER_CULL_MODE_FRONT_AND_BACK_BIT)  // Note: This is not supported in DirectX12
    cull_mode = D3D12_CULL_MODE_NONE;
  else
    cull_mode = D3D12_CULL_MODE_NONE;

  // Note: Remember Vulkan we flip y axis, so we need to flip the front face definition as well. Standard is clockwise for DirectX12 and counterclockwise for Vulkan
  BOOL engine_front_ccw = shader_common->shader_settings.front_face == SHADER_FRONT_FACE_COUNTER_CLOCKWISE ? TRUE : FALSE;
  BOOL depth_test = shader_common->shader_settings.depth_test ? TRUE : FALSE;
  BOOL multisample_enabled = shader_common->shader_settings.num_msaa_samples > 1 ? TRUE : FALSE;

  rasterizer_desc.FillMode = D3D12_FILL_MODE_SOLID;
  rasterizer_desc.CullMode = cull_mode;
  // Note: Our engine/front_face setting follows the Vulkan convention used by this renderer. Vulkan path applies a y flip, while the DirectX12 path does not
  // To keep the same triangles front-facing across both backends, we invert the engine front-face setting when mapping to DirectX12
  // D3D12: FrontCounterClockwise = TRUE  -> CCW is front
  //        FrontCounterClockwise = FALSE -> CW  is front
  // So if we have a shader using counter clockwise and it's standardized to Vulkan, then DirectX12 should be clockwise
  rasterizer_desc.FrontCounterClockwise = !engine_front_ccw;
  rasterizer_desc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
  rasterizer_desc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
  rasterizer_desc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
  rasterizer_desc.DepthClipEnable = TRUE;
  rasterizer_desc.MultisampleEnable = multisample_enabled;
  rasterizer_desc.AntialiasedLineEnable = FALSE;
  rasterizer_desc.ForcedSampleCount = 0;
  rasterizer_desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

  D3D12_ROOT_PARAMETER root_parameters[SHADER_ATTACHMENT_LIMIT * 3];
  ZeroMemory(root_parameters, sizeof(root_parameters));
  D3D12_DESCRIPTOR_RANGE descriptor_table_ranges[SHADER_ATTACHMENT_LIMIT];
  ZeroMemory(descriptor_table_ranges, sizeof(descriptor_table_ranges));

  uint32_t root_param_index = 0;

  for (uint_fast8_t constant_num = 0; constant_num < shader_common->shader_settings.uniforms_constants; constant_num++, root_param_index++) {
    root_parameters[root_param_index].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    root_parameters[root_param_index].Descriptor.ShaderRegister = shader_common->shader_settings.uniform_constant_state[constant_num].shader_position;
    root_parameters[root_param_index].Descriptor.RegisterSpace = 0;
    root_parameters[root_param_index].ShaderVisibility = (shader_common->shader_settings.uniform_constant_state[constant_num].shader_stage == SHADER_STAGE_VERTEX) ? D3D12_SHADER_VISIBILITY_VERTEX : D3D12_SHADER_VISIBILITY_PIXEL;
  }

  for (uint_fast8_t texture_num = 0; texture_num < shader_common->shader_settings.texture_samples; texture_num++, root_param_index++) {
    descriptor_table_ranges[texture_num].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptor_table_ranges[texture_num].NumDescriptors = 1;
    descriptor_table_ranges[texture_num].BaseShaderRegister = shader_common->shader_settings.texture_sample_state[texture_num].shader_position;
    descriptor_table_ranges[texture_num].RegisterSpace = 0;
    descriptor_table_ranges[texture_num].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    root_parameters[root_param_index].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    root_parameters[root_param_index].DescriptorTable.NumDescriptorRanges = 1;
    root_parameters[root_param_index].DescriptorTable.pDescriptorRanges = &descriptor_table_ranges[texture_num];
    root_parameters[root_param_index].ShaderVisibility = (shader_common->shader_settings.texture_sample_state[texture_num].shader_stage == SHADER_STAGE_VERTEX) ? D3D12_SHADER_VISIBILITY_VERTEX : D3D12_SHADER_VISIBILITY_PIXEL;
  }

  D3D12_DESCRIPTOR_RANGE sampler_table_range;
  ZeroMemory(&sampler_table_range, sizeof(sampler_table_range));
  sampler_table_range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
  sampler_table_range.NumDescriptors = 1;
  sampler_table_range.BaseShaderRegister = 0;
  sampler_table_range.RegisterSpace = 0;
  sampler_table_range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

  root_parameters[root_param_index].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
  root_parameters[root_param_index].DescriptorTable.NumDescriptorRanges = 1;
  root_parameters[root_param_index].DescriptorTable.pDescriptorRanges = &sampler_table_range;
  root_parameters[root_param_index].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
  root_param_index++;

  D3D12_ROOT_SIGNATURE_DESC root_signature_desc;
  ZeroMemory(&root_signature_desc, sizeof(root_signature_desc));
  root_signature_desc.NumParameters = root_param_index;
  root_signature_desc.pParameters = root_parameters;
  root_signature_desc.NumStaticSamplers = 0;
  root_signature_desc.pStaticSamplers = NULL;
  root_signature_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

  ID3DBlob* serialized_root_sig = NULL;
  ID3DBlob* error_blob = NULL;
  HRESULT hr = D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &serialized_root_sig, &error_blob);
  if (FAILED(hr)) {
    if (error_blob) {
      char* err_msg = (char*)error_blob->lpVtbl->GetBufferPointer(error_blob);
      log_message(LOG_SEVERITY_ERROR, "Root Signature Compilation Messages:\n%s\n", err_msg);
      error_blob->lpVtbl->Release(error_blob);
      error_blob = NULL;
    }
    log_message(LOG_SEVERITY_ERROR, "Failed to serialize root signature\n");
    return 1;
  }

  if (error_blob) {
    // Handle error - use the errorBlob's data to get a string error message
    char* err_msg = (char*)error_blob->lpVtbl->GetBufferPointer(error_blob);
    log_message(LOG_SEVERITY_ERROR, "Root Signature Compilation Messages:\n%s\n", err_msg);
    error_blob->lpVtbl->Release(error_blob);
    error_blob = NULL;
  }

  hr = api_common->directx_12_api.device->lpVtbl->CreateRootSignature(api_common->directx_12_api.device, 0, serialized_root_sig->lpVtbl->GetBufferPointer(serialized_root_sig), serialized_root_sig->lpVtbl->GetBufferSize(serialized_root_sig), &IID_ID3D12RootSignature, (void**)&shader_common->shader_directx12.root_signature);
  if (FAILED(hr)) {
    log_message(LOG_SEVERITY_ERROR, "Failed to create root signature\n");
    return 1;
  }

  if (serialized_root_sig)
    serialized_root_sig->lpVtbl->Release(serialized_root_sig);
  if (error_blob)
    error_blob->lpVtbl->Release(error_blob);

  D3D12_INPUT_ELEMENT_DESC input_element_descs[32] = {0};
  uint32_t num_attributes = mesh_get_input_layout(shader_common->shader_settings.mesh_type, input_element_descs);

  /* count color render targets (skip depth) */
  uint32_t color_targets = 0;
  for (uint_fast8_t i = 0; i < shader_common->shader_settings.render_targets; i++)
    if (shader_common->shader_settings.render_target_format[i] != SHADER_RENDER_TARGET_FORMAT_D32_FLOAT)
      color_targets++;

  D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {0};
  pso_desc.NumRenderTargets = color_targets;

  if (shader_common->shader_settings.vertex_attributes > 0) {
    pso_desc.InputLayout.pInputElementDescs = input_element_descs;
    pso_desc.InputLayout.NumElements = num_attributes;
  } else {
    pso_desc.InputLayout.pInputElementDescs = NULL;
    pso_desc.InputLayout.NumElements = 0;
  }

  D3D12_BLEND_DESC blend_desc;
  ZeroMemory(&blend_desc, sizeof(blend_desc));

  blend_desc.AlphaToCoverageEnable = FALSE;
  blend_desc.IndependentBlendEnable = FALSE;

  D3D12_RENDER_TARGET_BLEND_DESC rt_blend = {0};
  rt_blend.BlendEnable = shader_common->shader_settings.blend ? TRUE : FALSE;
  rt_blend.LogicOpEnable = FALSE;

  rt_blend.SrcBlend = D3D12_BLEND_ONE;
  rt_blend.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
  rt_blend.BlendOp = D3D12_BLEND_OP_ADD;

  rt_blend.SrcBlendAlpha = D3D12_BLEND_ONE;
  rt_blend.DestBlendAlpha = D3D12_BLEND_ZERO;
  rt_blend.BlendOpAlpha = D3D12_BLEND_OP_ADD;

  rt_blend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

  for (UINT i = 0; i < color_targets; i++)
    blend_desc.RenderTarget[i] = rt_blend;

  pso_desc.pRootSignature = shader_common->shader_directx12.root_signature;
  pso_desc.VS.pShaderBytecode = shader_common->shader_directx12.vertex_shader_blob->lpVtbl->GetBufferPointer(shader_common->shader_directx12.vertex_shader_blob);
  pso_desc.VS.BytecodeLength = shader_common->shader_directx12.vertex_shader_blob->lpVtbl->GetBufferSize(shader_common->shader_directx12.vertex_shader_blob);
  pso_desc.PS.pShaderBytecode = shader_common->shader_directx12.fragment_shader_blob->lpVtbl->GetBufferPointer(shader_common->shader_directx12.fragment_shader_blob);
  pso_desc.PS.BytecodeLength = shader_common->shader_directx12.fragment_shader_blob->lpVtbl->GetBufferSize(shader_common->shader_directx12.fragment_shader_blob);
  pso_desc.RasterizerState = rasterizer_desc;
  pso_desc.BlendState = blend_desc;
  D3D12_DEPTH_STENCIL_DESC depth_stencil_desc = {0};
  depth_stencil_desc.DepthEnable = depth_test;
  depth_stencil_desc.DepthWriteMask = shader_common->shader_settings.depth_write ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
  depth_stencil_desc.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
  depth_stencil_desc.StencilEnable = FALSE;
  pso_desc.DepthStencilState = depth_stencil_desc;
  pso_desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
  pso_desc.SampleMask = UINT_MAX;
  pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

  for (int i = 0; i < 8; i++)
    pso_desc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
  uint32_t rtv_index = 0;
  for (uint_fast8_t render_target_num = 0; render_target_num < shader_common->shader_settings.render_targets; render_target_num++) {
    if (shader_common->shader_settings.render_target_format[render_target_num] == SHADER_RENDER_TARGET_FORMAT_R8G8B8A8_UNORM)
      pso_desc.RTVFormats[rtv_index++] = DXGI_FORMAT_R8G8B8A8_UNORM;
    else if (shader_common->shader_settings.render_target_format[render_target_num] == SHADER_RENDER_TARGET_FORMAT_R11G11B10_FLOAT)
      pso_desc.RTVFormats[rtv_index++] = DXGI_FORMAT_R11G11B10_FLOAT;
    else if (shader_common->shader_settings.render_target_format[render_target_num] == SHADER_RENDER_TARGET_FORMAT_R16G16B16A16_FLOAT)
      pso_desc.RTVFormats[rtv_index++] = DXGI_FORMAT_R16G16B16A16_FLOAT;
    else if (shader_common->shader_settings.render_target_format[render_target_num] == SHADER_RENDER_TARGET_FORMAT_R32G32B32A32_FLOAT)
      pso_desc.RTVFormats[rtv_index++] = DXGI_FORMAT_R32G32B32A32_FLOAT;
  }
  pso_desc.SampleDesc.Count = shader_common->shader_settings.num_msaa_samples;
  if (shader_common->shader_settings.num_msaa_samples != 1) {
    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS ms_quality_levels;
    memset(&ms_quality_levels, 0, sizeof(ms_quality_levels));
    ms_quality_levels.Format = pso_desc.RTVFormats[0];
    ms_quality_levels.SampleCount = shader_common->shader_settings.num_msaa_samples;

    api_common->directx_12_api.device->lpVtbl->CheckFeatureSupport(api_common->directx_12_api.device, D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &ms_quality_levels, sizeof(ms_quality_levels));

    UINT max_quality_level = ms_quality_levels.NumQualityLevels;
    if (max_quality_level > 0)
      max_quality_level -= 1;  // For zero-based indexing
    else
      max_quality_level = 0;

    pso_desc.SampleDesc.Quality = max_quality_level;
  }

  HRESULT hr_pso = api_common->directx_12_api.device->lpVtbl->CreateGraphicsPipelineState(api_common->directx_12_api.device, &pso_desc, &IID_ID3D12PipelineState, (void**)&shader_common->shader_directx12.pipeline_state);
  if (FAILED(hr_pso)) {
    log_message(LOG_SEVERITY_ERROR, "Failed to create graphics pipeline state\n");
    return 1;
  }

  D3D12_DESCRIPTOR_HEAP_DESC sampler_heap_desc = {0};
  sampler_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
  sampler_heap_desc.NumDescriptors = 1;  // Adjust this number as per your requirement.
  sampler_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

  HRESULT result = api_common->directx_12_api.device->lpVtbl->CreateDescriptorHeap(api_common->directx_12_api.device, &sampler_heap_desc, &IID_ID3D12DescriptorHeap, (void**)&(shader_common->shader_directx12.sampler_heap));
  if (result != S_OK)
    return 1;

  shader_common->shader_directx12.sampler_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(shader_common->shader_directx12.sampler_heap, &(shader_common->shader_directx12.sampler_handle_cpu));
  shader_common->shader_directx12.sampler_heap->lpVtbl->GetGPUDescriptorHandleForHeapStart(shader_common->shader_directx12.sampler_heap, &(shader_common->shader_directx12.sampler_handle_gpu));

  D3D12_SAMPLER_DESC sampler_desc = {0};
  sampler_desc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;     // or other filtering modes.
  sampler_desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;  // or other address modes.
  sampler_desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;  // or other address modes.
  sampler_desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;  // or other address modes.
  sampler_desc.MipLODBias = 0;
  sampler_desc.MaxAnisotropy = 1;
  sampler_desc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
  sampler_desc.BorderColor[0] = 0.0f;
  sampler_desc.BorderColor[1] = 0.0f;
  sampler_desc.BorderColor[2] = 0.0f;
  sampler_desc.BorderColor[3] = 0.0f;
  sampler_desc.MinLOD = 0.0f;
  sampler_desc.MaxLOD = D3D12_FLOAT32_MAX;
  api_common->directx_12_api.device->lpVtbl->CreateSampler(api_common->directx_12_api.device, &sampler_desc, shader_common->shader_directx12.sampler_handle_cpu);

  return 0;
}

uint_fast8_t shader_compute_directx_12_init(struct ShaderCommon* shader, struct APICommon* api_common) {
  return 0;
}

void shader_directx_12_delete(struct ShaderCommon* shader_common, struct APICommon* api_common) {
  // Release the pipeline state
  if (shader_common->shader_directx12.pipeline_state) {
    shader_common->shader_directx12.pipeline_state->lpVtbl->Release(shader_common->shader_directx12.pipeline_state);
    shader_common->shader_directx12.pipeline_state = NULL;
  }

  // Release the root signature
  if (shader_common->shader_directx12.root_signature) {
    shader_common->shader_directx12.root_signature->lpVtbl->Release(shader_common->shader_directx12.root_signature);
    shader_common->shader_directx12.root_signature = NULL;
  }

  //// Release the descriptor heap
  // if (shader_common->shader_directx12.srv_heap) {
  //   shader_common->shader_directx12.srv_heap->lpVtbl->Release(shader_common->shader_directx12.srv_heap);
  //   shader_common->shader_directx12.srv_heap = NULL;
  // }

  // Release the shader blobs
  if (shader_common->shader_directx12.vertex_shader_blob) {
    shader_common->shader_directx12.vertex_shader_blob->lpVtbl->Release(shader_common->shader_directx12.vertex_shader_blob);
    shader_common->shader_directx12.vertex_shader_blob = NULL;
  }

  if (shader_common->shader_directx12.fragment_shader_blob) {
    shader_common->shader_directx12.fragment_shader_blob->lpVtbl->Release(shader_common->shader_directx12.fragment_shader_blob);
    shader_common->shader_directx12.fragment_shader_blob = NULL;
  }

  // Reset descriptor handles - note that you don't "release" these handles,
  // they're just a way to reference into the descriptor heap.
  // shader_common->shader_directx12.srv_cpu_handle = (D3D12_CPU_DESCRIPTOR_HANDLE){0};
  // shader_common->shader_directx12.srv_gpu_handle = (D3D12_GPU_DESCRIPTOR_HANDLE){0};
}

void shader_directx_12_resize(struct ShaderCommon* shader_common, struct APICommon* api_common, uint32_t width, uint32_t height, uint_fast8_t supersample_scale) {
  shader_common->shader_directx12.viewport.TopLeftX = 0.0f;
  shader_common->shader_directx12.viewport.TopLeftY = 0.0f;
  if (shader_common->shader_settings.supersampled) {
    shader_common->shader_directx12.viewport.Width = (float)(width * supersample_scale);
    shader_common->shader_directx12.viewport.Height = (float)(height * supersample_scale);
  } else {
    shader_common->shader_directx12.viewport.Width = (float)(width);
    shader_common->shader_directx12.viewport.Height = (float)(height);
  }
  shader_common->shader_directx12.viewport.MinDepth = 0.0f;
  shader_common->shader_directx12.viewport.MaxDepth = 1.0f;

  shader_common->shader_directx12.scissor_rect.left = 0;
  shader_common->shader_directx12.scissor_rect.top = 0;
  if (shader_common->shader_settings.supersampled) {
    shader_common->shader_directx12.scissor_rect.right = (LONG)(width * supersample_scale);
    shader_common->shader_directx12.scissor_rect.bottom = (LONG)(height * supersample_scale);
  } else {
    shader_common->shader_directx12.scissor_rect.right = (LONG)(width);
    shader_common->shader_directx12.scissor_rect.bottom = (LONG)(height);
  }
}
