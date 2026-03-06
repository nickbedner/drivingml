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

  struct D3DIncludeC include_handler_c = {&v_table_c};

  wchar_t base_path[MAX_LENGTH_OF_PATH] = {0};

  // Convert "/shaders/hlsl/" to a wide string
  const wchar_t* additional_path = L"/shaders/hlsl/";

  // Convert char* to wchar_t*
  size_t converted_chars = 0;
  // mbstowcs_s(&converted_chars, base_path, MAX_LENGTH_OF_PATH, api_common->asset_directory, _TRUNCATE);
  wcsncpy_s(base_path, MAX_LENGTH_OF_PATH, api_common->asset_directory, _TRUNCATE);

  // Append additional path
  wcscat_s(base_path, MAX_LENGTH_OF_PATH, additional_path);

  size_t vertex_path_size = wcslen(base_path) + wcslen(shader_common->shader_settings.vertex_shader) + wcslen(L".hlsl") + 1;
  size_t fragment_path_size = wcslen(base_path) + wcslen(shader_common->shader_settings.fragment_shader) + wcslen(L".hlsl") + 1;

  wchar_t* vertex_shader_path = (wchar_t*)malloc(vertex_path_size * sizeof(wchar_t));
  wchar_t* fragment_shader_path = (wchar_t*)malloc(fragment_path_size * sizeof(wchar_t));

  swprintf(vertex_shader_path, vertex_path_size, L"%ls%hs.hlsl", base_path, shader_common->shader_settings.vertex_shader);
  swprintf(fragment_shader_path, fragment_path_size, L"%ls%hs.hlsl", base_path, shader_common->shader_settings.fragment_shader);

  ID3DBlob* vertex_compilation_message_blob;
  ID3DBlob* fragment_compilation_message_blob;

  HRESULT vertex_result = D3DCompileFromFile(vertex_shader_path, NULL, (ID3DInclude*)&include_handler_c, "VS_main", "vs_5_0", 0, 0, &(shader_common->shader_directx12.vertex_shader_blob), &vertex_compilation_message_blob);
  HRESULT fragment_result = D3DCompileFromFile(fragment_shader_path, NULL, (ID3DInclude*)&include_handler_c, "PS_main", "ps_5_0", 0, 0, &(shader_common->shader_directx12.fragment_shader_blob), &fragment_compilation_message_blob);

  free(vertex_shader_path);
  free(fragment_shader_path);

  if (FAILED(vertex_result) || FAILED(fragment_result)) {
    if (vertex_compilation_message_blob) {
      char* err_msg = (char*)vertex_compilation_message_blob->lpVtbl->GetBufferPointer(vertex_compilation_message_blob);
      log_message(LOG_SEVERITY_ERROR, "Vertex Shader Compilation Messages:\n%s\n", err_msg);
      vertex_compilation_message_blob->lpVtbl->Release(vertex_compilation_message_blob);
    }

    if (fragment_compilation_message_blob) {
      char* err_msg = (char*)fragment_compilation_message_blob->lpVtbl->GetBufferPointer(fragment_compilation_message_blob);
      log_message(LOG_SEVERITY_ERROR, "Fragment Shader Compilation Messages:\n%s\n", err_msg);
      fragment_compilation_message_blob->lpVtbl->Release(fragment_compilation_message_blob);
    }

    log_message(LOG_SEVERITY_ERROR, "Failed to compile shader\n");
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

  BOOL direction = shader_common->shader_settings.front_face == SHADER_FRONT_FACE_CLOCKWISE ? TRUE : FALSE;
  BOOL depth_test = shader_common->shader_settings.depth_test ? TRUE : FALSE;
  BOOL multisample_enabled = shader_common->shader_settings.num_msaa_samples > 1 ? TRUE : FALSE;

  rasterizer_desc.FillMode = D3D12_FILL_MODE_SOLID;
  rasterizer_desc.CullMode = cull_mode;
  rasterizer_desc.FrontCounterClockwise = direction;
  rasterizer_desc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
  rasterizer_desc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
  rasterizer_desc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
  rasterizer_desc.DepthClipEnable = depth_test;
  rasterizer_desc.MultisampleEnable = multisample_enabled;
  rasterizer_desc.AntialiasedLineEnable = FALSE;
  rasterizer_desc.ForcedSampleCount = 0;
  rasterizer_desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

  D3D12_BLEND_DESC blend_desc;
  ZeroMemory(&blend_desc, sizeof(blend_desc));  // Initialize the struct to zero

  blend_desc.AlphaToCoverageEnable = FALSE;
  blend_desc.IndependentBlendEnable = FALSE;

  D3D12_RENDER_TARGET_BLEND_DESC default_render_target_blend_desc = {FALSE, FALSE, D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD, D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD, D3D12_LOGIC_OP_NOOP, D3D12_COLOR_WRITE_ENABLE_ALL};

  for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
    blend_desc.RenderTarget[i] = default_render_target_blend_desc;

  /////////////////////////////////////////////////////////////////////////////
  D3D12_ROOT_PARAMETER root_parameters[SHADER_ATTACHMENT_LIMIT * 3];
  ZeroMemory(root_parameters, sizeof(root_parameters));

  // D3D12_DESCRIPTOR_RANGE constant_buffer_range[SHADER_ATTACHMENT_LIMIT];
  // ZeroMemory(constant_buffer_range, sizeof(constant_buffer_range));
  for (uint_fast8_t constant_num = 0; constant_num < shader_common->shader_settings.uniforms_constants; constant_num++) {
    // constant_buffer_range[constant_num].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    // constant_buffer_range[constant_num].NumDescriptors = 1;
    // constant_buffer_range[constant_num].BaseShaderRegister = shader_common->shader_settings.uniform_constant_state[constant_num].shader_position;
    // constant_buffer_range[constant_num].RegisterSpace = 0;
    // constant_buffer_range[constant_num].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    root_parameters[shader_common->shader_settings.uniform_constant_state[constant_num].shader_position].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    root_parameters[shader_common->shader_settings.uniform_constant_state[constant_num].shader_position].Descriptor.ShaderRegister = shader_common->shader_settings.uniform_constant_state[constant_num].shader_position;
    root_parameters[shader_common->shader_settings.uniform_constant_state[constant_num].shader_position].Descriptor.RegisterSpace = 0;
    if (shader_common->shader_settings.uniform_constant_state[constant_num].shader_stage == SHADER_STAGE_VERTEX)
      root_parameters[shader_common->shader_settings.uniform_constant_state[constant_num].shader_position].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    else if (shader_common->shader_settings.uniform_constant_state[constant_num].shader_stage == SHADER_STAGE_FRAGMENT)
      root_parameters[shader_common->shader_settings.uniform_constant_state[constant_num].shader_position].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
  }
  D3D12_DESCRIPTOR_RANGE descriptor_table_ranges[SHADER_ATTACHMENT_LIMIT];
  ZeroMemory(descriptor_table_ranges, sizeof(descriptor_table_ranges));
  for (uint_fast8_t texture_num = 0; texture_num < shader_common->shader_settings.texture_samples; texture_num++) {
    descriptor_table_ranges[texture_num].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptor_table_ranges[texture_num].NumDescriptors = 1;
    descriptor_table_ranges[texture_num].BaseShaderRegister = shader_common->shader_settings.texture_sample_state[texture_num].shader_position;
    descriptor_table_ranges[texture_num].RegisterSpace = 0;
    descriptor_table_ranges[texture_num].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    root_parameters[shader_common->shader_settings.texture_sample_state[texture_num].shader_position].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    root_parameters[shader_common->shader_settings.texture_sample_state[texture_num].shader_position].DescriptorTable.NumDescriptorRanges = 1;
    root_parameters[shader_common->shader_settings.texture_sample_state[texture_num].shader_position].DescriptorTable.pDescriptorRanges = &descriptor_table_ranges[texture_num];
    if (shader_common->shader_settings.texture_sample_state[texture_num].shader_stage == SHADER_STAGE_VERTEX)
      root_parameters[shader_common->shader_settings.texture_sample_state[texture_num].shader_position].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    else if (shader_common->shader_settings.texture_sample_state[texture_num].shader_stage == SHADER_STAGE_FRAGMENT)
      root_parameters[shader_common->shader_settings.texture_sample_state[texture_num].shader_position].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
  }

  // TODO: This should probably be moved to whatever would need to use this shader
  D3D12_DESCRIPTOR_RANGE sampler_table_range;
  ZeroMemory(&sampler_table_range, sizeof(sampler_table_range));
  sampler_table_range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
  sampler_table_range.NumDescriptors = 1;
  sampler_table_range.BaseShaderRegister = 0;
  sampler_table_range.RegisterSpace = 0;
  sampler_table_range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
  root_parameters[shader_common->shader_settings.uniforms_constants + shader_common->shader_settings.texture_samples].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
  root_parameters[shader_common->shader_settings.uniforms_constants + shader_common->shader_settings.texture_samples].DescriptorTable.NumDescriptorRanges = 1;
  root_parameters[shader_common->shader_settings.uniforms_constants + shader_common->shader_settings.texture_samples].DescriptorTable.pDescriptorRanges = &sampler_table_range;
  root_parameters[shader_common->shader_settings.uniforms_constants + shader_common->shader_settings.texture_samples].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
  //
  // D3D12_DESCRIPTOR_RANGE sampler_table_range[SHADER_ATTACHMENT_LIMIT];
  // ZeroMemory(sampler_table_range, sizeof(sampler_table_range));
  // for (uint_fast8_t sample_num = 0; sample_num < shader_common->shader_settings.samples; sample_num++) {
  //  sampler_table_range[sample_num].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
  //  sampler_table_range[sample_num].NumDescriptors = 1;
  //  sampler_table_range[sample_num].BaseShaderRegister = sample_num;
  //  sampler_table_range[sample_num].RegisterSpace = 0;
  //  sampler_table_range[sample_num].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
  //
  //  root_parameters[sample_num + shader_common->shader_settings.constants + shader_common->shader_settings.num_textures].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
  //  root_parameters[sample_num + shader_common->shader_settings.constants + shader_common->shader_settings.num_textures].DescriptorTable.NumDescriptorRanges = 1;
  //  root_parameters[sample_num + shader_common->shader_settings.constants + shader_common->shader_settings.num_textures].DescriptorTable.pDescriptorRanges = &sampler_table_range[sample_num];
  //  if (shader_common->shader_settings.sample_state[sample_num] == SHADER_STAGE_VERTEX)
  //    root_parameters[sample_num + shader_common->shader_settings.constants + shader_common->shader_settings.num_textures].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
  //  else if (shader_common->shader_settings.sample_state[sample_num] == SHADER_STAGE_FRAGMENT)
  //    root_parameters[sample_num + shader_common->shader_settings.constants + shader_common->shader_settings.num_textures].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
  //}

  D3D12_ROOT_SIGNATURE_DESC root_signature_desc;
  ZeroMemory(&root_signature_desc, sizeof(root_signature_desc));
  root_signature_desc.NumParameters = shader_common->shader_settings.uniforms_constants + shader_common->shader_settings.texture_samples + 1;
  root_signature_desc.pParameters = root_parameters;
  root_signature_desc.NumStaticSamplers = 0;
  root_signature_desc.pStaticSamplers = NULL;
  root_signature_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

  ID3DBlob* serialized_root_sig = NULL;
  ID3DBlob* error_blob = NULL;
  D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &serialized_root_sig, &error_blob);

  if (error_blob) {
    // Handle error - use the errorBlob's data to get a string error message
    char* err_msg = (char*)error_blob->lpVtbl->GetBufferPointer(vertex_compilation_message_blob);
    log_message(LOG_SEVERITY_ERROR, "Root Signature Compilation Messages:\n%s\n", err_msg);
    error_blob->lpVtbl->Release(vertex_compilation_message_blob);
  }

  api_common->directx_12_api.device->lpVtbl->CreateRootSignature(api_common->directx_12_api.device, 0, serialized_root_sig->lpVtbl->GetBufferPointer(serialized_root_sig), serialized_root_sig->lpVtbl->GetBufferSize(serialized_root_sig), &IID_ID3D12RootSignature, (void**)&(shader_common->shader_directx12.root_signature));

  if (serialized_root_sig)
    serialized_root_sig->lpVtbl->Release(serialized_root_sig);
  if (error_blob)
    error_blob->lpVtbl->Release(error_blob);
  /////////////////////////////////////////////////////////////////////////////
  // Assume you have a valid ID3D12Device pointer named 'device' and ID3D12RootSignature pointer named 'rootSignature'
  D3D12_INPUT_ELEMENT_DESC input_element_descs[32] = {0};
  uint32_t num_attributes = mesh_get_input_layout(shader_common->shader_settings.mesh_type, input_element_descs);

  D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {0};
  if (shader_common->shader_settings.vertex_attributes > 0) {
    pso_desc.InputLayout.pInputElementDescs = input_element_descs;
    pso_desc.InputLayout.NumElements = num_attributes;
  } else {
    pso_desc.InputLayout.pInputElementDescs = NULL;
    pso_desc.InputLayout.NumElements = 0;
  }
  pso_desc.pRootSignature = shader_common->shader_directx12.root_signature;
  pso_desc.VS.pShaderBytecode = shader_common->shader_directx12.vertex_shader_blob->lpVtbl->GetBufferPointer(shader_common->shader_directx12.vertex_shader_blob);
  pso_desc.VS.BytecodeLength = shader_common->shader_directx12.vertex_shader_blob->lpVtbl->GetBufferSize(shader_common->shader_directx12.vertex_shader_blob);
  pso_desc.PS.pShaderBytecode = shader_common->shader_directx12.fragment_shader_blob->lpVtbl->GetBufferPointer(shader_common->shader_directx12.fragment_shader_blob);
  pso_desc.PS.BytecodeLength = shader_common->shader_directx12.fragment_shader_blob->lpVtbl->GetBufferSize(shader_common->shader_directx12.fragment_shader_blob);
  pso_desc.RasterizerState = rasterizer_desc;
  pso_desc.BlendState = blend_desc;
  pso_desc.DepthStencilState.DepthEnable = depth_test;
  if (depth_test) {
    // pso_desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    // pso_desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;
    D3D12_DEPTH_STENCIL_DESC depth_stencil_desc = {0};

    // Enable depth testing.
    depth_stencil_desc.DepthEnable = TRUE;
    depth_stencil_desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;

    // For reverse depth, objects closer to the camera will have a greater depth value.
    depth_stencil_desc.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;

    // Stencil settings, for now, we'll disable stencil testing.
    depth_stencil_desc.StencilEnable = FALSE;
    depth_stencil_desc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    depth_stencil_desc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;

    // Default stencil operations in case stencil test is enabled later.
    depth_stencil_desc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    depth_stencil_desc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    depth_stencil_desc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    depth_stencil_desc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

    depth_stencil_desc.BackFace = depth_stencil_desc.FrontFace;  // Just copy the front face settings to the back face for simplicity.
    pso_desc.DepthStencilState = depth_stencil_desc;
    pso_desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;  // Or whatever your depth format is
  }
  pso_desc.DepthStencilState.StencilEnable = FALSE;
  pso_desc.SampleMask = UINT_MAX;
  pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
  pso_desc.NumRenderTargets = shader_common->shader_settings.render_targets;
  for (uint_fast8_t render_target_num = 0; render_target_num < shader_common->shader_settings.render_targets; render_target_num++) {
    if (shader_common->shader_settings.render_target_format[render_target_num] == SHADER_RENDER_TARGET_FORMAT_R8G8B8A8_UNORM)
      pso_desc.RTVFormats[render_target_num] = DXGI_FORMAT_R8G8B8A8_UNORM;
    else if (shader_common->shader_settings.render_target_format[render_target_num] == SHADER_RENDER_TARGET_FORMAT_R11G11B10_FLOAT)
      pso_desc.RTVFormats[render_target_num] = DXGI_FORMAT_R11G11B10_FLOAT;
    else if (shader_common->shader_settings.render_target_format[render_target_num] == SHADER_RENDER_TARGET_FORMAT_R16G16B16A16_FLOAT)
      pso_desc.RTVFormats[render_target_num] = DXGI_FORMAT_R16G16B16A16_FLOAT;
    else if (shader_common->shader_settings.render_target_format[render_target_num] == SHADER_RENDER_TARGET_FORMAT_R32G32B32A32_FLOAT)
      pso_desc.RTVFormats[render_target_num] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    else if (shader_common->shader_settings.render_target_format[render_target_num] == SHADER_RENDER_TARGET_FORMAT_D32_FLOAT)
      pso_desc.RTVFormats[render_target_num] = DXGI_FORMAT_D32_FLOAT;
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

  api_common->directx_12_api.device->lpVtbl->CreateGraphicsPipelineState(api_common->directx_12_api.device, &pso_desc, &IID_ID3D12PipelineState, (void**)&(shader_common->shader_directx12.pipeline_state));
  /////////////////////////////////////////////////////////////////////////////
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
  /////////////////////////////////////////////////////////////////////////////
  // D3D12_DESCRIPTOR_HEAP_DESC srv_heap_desc = {0};
  // srv_heap_desc.NumDescriptors = shader_common->shader_settings.render_pass->descriptors * shader_common->shader_settings.num_textures; // Or however many you need
  // srv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  // srv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  // HRESULT result = api_common->directx_12_api.device->lpVtbl->CreateDescriptorHeap(api_common->directx_12_api.device, &srv_heap_desc, &IID_ID3D12DescriptorHeap, (void **)&(shader_common->shader_directx12.srv_heap));
  //
  // if (result != S_OK)
  //  return 1;
  //
  // D3D12_RESOURCE_DESC texture_desc = {0};
  // texture_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  // texture_desc.Width = swap_chain_common->swap_chain_extent.width;
  // texture_desc.Height = swap_chain_common->swap_chain_extent.height;
  // texture_desc.DepthOrArraySize = 1;
  // texture_desc.MipLevels = 1;
  // texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Adjust as needed
  // texture_desc.SampleDesc.Count = 1;
  // texture_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
  // texture_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
  //
  // D3D12_HEAP_PROPERTIES heap_properties;
  // heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;
  // heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
  // heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
  // heap_properties.CreationNodeMask = 1;
  // heap_properties.VisibleNodeMask = 1;
  //
  // for (int desc = 0; desc < shader_common->shader_settings.render_pass->descriptors; desc++) {
  //  HRESULT hr = api_common->directx_12_api.device->lpVtbl->CreateCommittedResource(api_common->directx_12_api.device, &heap_properties, D3D12_HEAP_FLAG_NONE, &texture_desc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, &IID_ID3D12Resource, (void **)&(shader_common->shader_settings.render_pass->directx_12_render_pass.intermediate_buffers[desc]));
  //
  //  if (FAILED(hr))
  //    log_message(LOG_SEVERITY_ERROR, "Failed to create intermediate buffer for shader\n");
  //}
  //
  // UINT descriptor_size = api_common->directx_12_api.device->lpVtbl->GetDescriptorHandleIncrementSize(api_common->directx_12_api.device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  //
  // shader_common->shader_directx12.srv_heap->lpVtbl->GetCPUDescriptorHandleForHeapStart(shader_common->shader_directx12.srv_heap, &(shader_common->shader_directx12.srv_cpu_handle));
  //
  // shader_common->shader_directx12.srv_heap->lpVtbl->GetGPUDescriptorHandleForHeapStart(shader_common->shader_directx12.srv_heap, &(shader_common->shader_directx12.srv_gpu_handle));
  //
  // UINT total_heap_size = srv_heap_desc.NumDescriptors * descriptor_size;
  // UINT current_offset = 0;

  /// Create SRVs for each of your resources
  // or (UINT desc = 0; desc < shader_common->shader_settings.render_pass->descriptors; desc++) {
  //  D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {0};
  //  srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Adjust this based on your resource's format
  //  srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
  //  srv_desc.Texture2D.MipLevels = 1;
  //  srv_desc.Texture2D.MostDetailedMip = 0;
  //  srv_desc.Texture2D.ResourceMinLODClamp = 0.0f;
  //  srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

  // api_common->directx_12_api.device->lpVtbl->CreateShaderResourceView(api_common->directx_12_api.device, shader_common->shader_settings.render_pass->directx_12_render_pass.intermediate_buffers[desc], &srv_desc, shader_common->shader_directx12.srv_cpu_handle);

  // current_offset += descriptor_size;

  // if (current_offset > total_heap_size) {
  //   // Print an error message or halt execution in some way
  //   printf("Error: Exceeded bounds of the descriptor heap!\n");
  //   break;
  // }

  // // Move to the next descriptor slot
  // shader_common->shader_directx12.srv_cpu_handle.ptr += descriptor_size;
  //}

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
