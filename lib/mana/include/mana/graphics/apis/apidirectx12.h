#pragma once

#include "mana/graphics/apis/apicommon.h"

#ifdef DIRECTX_12_API_SUPPORTED
enum DIRECTX_12_API_STATUS {
  DIRECTX_12_API_SUCCESS = API_SUCCESS,
  DIRECTX_12_API_SETUP_DEBUG_INTERFACE_ERROR,
  DIRECTX_12_API_CREATE_FACTORY_ERROR,
  DIRECTX_12_API_CREATE_DEVICE_ERROR,
  DIRECTX_12_API_CREATE_COMMAND_QUEUE_ERROR,
  DIRECTX_12_API_LAST_ERROR
};

enum DIRECTX_12_RENDERER_STATUS {
  DIRETX_12_RENDERER_SUCCESS = 0,
  DIRETX_12_RENDERER_LAST_ERROR
};

u8 api_directx_12_init(struct APICommon* api_common);
void api_directx_12_delete(struct APICommon* api_common);

void directx_12_graphics_utils_poll_debug_messages(struct DirectX12API* directx_12_api);
u8 directx_12_graphics_utils_setup_vertex_buffer(struct DirectX12API* directx_12_api, struct Vector* vertices, ID3D12Resource** vertex_buffer);
u8 directx_12_graphics_utils_setup_index_buffer(struct DirectX12API* directx_12_api, struct Vector* indices, ID3D12Resource** index_buffer);
u8 directx_12_graphics_utils_setup_constant_buffer(struct DirectX12API* directx_12_api, UINT64 buffer_size, ID3D12Resource** constant_buffer);
#endif
