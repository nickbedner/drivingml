#include "mana/graphics/render/window.h"

static LRESULT CALLBACK window_proc(HWND hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param);

#ifdef _WIN64
static uint8_t window_pc_window(struct Window* window, uint_fast32_t width, uint_fast32_t height) {
  window->new_window = true;
  window->minimized = false;
  window->surface.hinstance = GetModuleHandle(NULL);
  MultiByteToWideChar(CP_UTF8, 0, window->title, -1, window->surface.class_name, 256);

  WNDCLASSEX windows_class;
  windows_class.cbSize = sizeof(WNDCLASSEX);
  windows_class.style = CS_HREDRAW | CS_VREDRAW;
  windows_class.lpfnWndProc = DefWindowProc;
  windows_class.cbClsExtra = 0;
  windows_class.cbWndExtra = 0;
  windows_class.hInstance = window->surface.hinstance;
  windows_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  windows_class.hCursor = LoadCursor(NULL, IDC_ARROW);
  windows_class.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
  windows_class.lpszMenuName = NULL;
  windows_class.lpszClassName = window->surface.class_name;
  windows_class.hIconSm = LoadIcon(NULL, IDI_WINLOGO);
  if (!RegisterClassExW(&windows_class)) {
    log_message(LOG_SEVERITY_ERROR, "Error registering window class!\n");
    return WINDOW_ERROR;
  }

  DWORD ex_style = WS_EX_APPWINDOW;
  // Note: WS_POPUP is needed for fullscreen mode and windowed fullscreen mode
  DWORD style = WS_OVERLAPPEDWINDOW;
  RECT window_size_rect;
  window_size_rect.left = 0;
  window_size_rect.top = 0;
  window_size_rect.right = (LONG)width;
  window_size_rect.bottom = (LONG)height;
  AdjustWindowRect(&window_size_rect, style, FALSE);
  LONG window_width = window_size_rect.right - window_size_rect.left;
  LONG window_height = window_size_rect.bottom - window_size_rect.top;
  // Center the window on the exact middle of the primary monitor
  LONG screen_width = GetSystemMetrics(SM_CXSCREEN);
  LONG screen_height = GetSystemMetrics(SM_CYSCREEN);
  LONG window_pos_x = (screen_width - window_width) / 2;
  LONG window_pos_y = (screen_height - window_height) / 2;
  window->surface.hwnd = CreateWindowExW(ex_style, window->surface.class_name, window->surface.class_name, style, window_pos_x, window_pos_y, window_width, window_height, NULL, NULL, window->surface.hinstance, NULL);
  if (!window->surface.hwnd) {
    log_message(LOG_SEVERITY_ERROR, "Error creating window!\n");
    return WINDOW_ERROR;
  }

  SetWindowLongPtr(window->surface.hwnd, GWLP_WNDPROC, (LONG_PTR)window_proc);
  SetWindowLongPtr(window->surface.hwnd, GWLP_USERDATA, (LONG_PTR)window);

  ShowWindow(window->surface.hwnd, SW_SHOW);
  UpdateWindow(window->surface.hwnd);
  window->new_window = false;

  return WINDOW_SUCCESS;
}
#endif

static LRESULT CALLBACK window_proc(HWND hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param) {
  LONG_PTR value = GetWindowLongPtrW(hwnd, GWLP_USERDATA);
  struct Window* window = (struct Window*)value;

  LRESULT result = 0;
  switch (u_msg) {
    case WM_SIZE: {
      // Note: Hack to stop recreating swap chain on window creation
      window->framebuffer_resized = (window->new_window) ? false : true;
      window->minimized = (w_param == SIZE_MINIMIZED) ? true : false;
      break;
    }
    case WM_CLOSE: {
      window->should_close = true;
      break;
    }
    case WM_DESTROY: {
      PostQuitMessage(0);
      break;
    }
    case WM_MOUSEMOVE: {
      struct KeyboardMouseController* keyboard_mouse_controller_move = input_manager_find_keyboard_mouse(&window->input_manager);
      keyboard_mouse_controller_move->mouse_x_pos_prev = keyboard_mouse_controller_move->mouse_x_pos;
      keyboard_mouse_controller_move->mouse_y_pos_prev = keyboard_mouse_controller_move->mouse_y_pos;
      keyboard_mouse_controller_move->mouse_x_pos = GET_X_LPARAM(l_param);
      keyboard_mouse_controller_move->mouse_y_pos = GET_Y_LPARAM(l_param);

      if (keyboard_mouse_controller_move->show_cursor == false)
        SetCursor(NULL);
      else
        SetCursor(LoadCursorW(NULL, IDC_ARROW));

      if (keyboard_mouse_controller_move->lock_cursor == true) {
        POINT current_cursor_pos;
        GetCursorPos(&current_cursor_pos);

        vec2 center = input_manager_move_cursor_to_center(&(window->input_manager), &window->surface);

        keyboard_mouse_controller_move->mouse_x_pos_diff = (double)(current_cursor_pos.x - (LONG)center.x);
        keyboard_mouse_controller_move->mouse_y_pos_diff = (double)(current_cursor_pos.y - (LONG)center.y);
      } else {
        keyboard_mouse_controller_move->mouse_x_pos_diff = keyboard_mouse_controller_move->mouse_x_pos - keyboard_mouse_controller_move->mouse_x_pos_prev;
        keyboard_mouse_controller_move->mouse_y_pos_diff = keyboard_mouse_controller_move->mouse_y_pos - keyboard_mouse_controller_move->mouse_y_pos_prev;
      }
      break;
    }
    case WM_MOUSEWHEEL: {
      struct KeyboardMouseController* keyboard_mouse_controller_wheel = input_manager_find_keyboard_mouse(&window->input_manager);
      keyboard_mouse_controller_wheel->mouse_wheel = -(GET_WHEEL_DELTA_WPARAM(w_param) / 120);
      break;
    }
    // case WM_DEVICECHANGE: {
    //   // Check if the device change is a joystick connection/disconnection
    //   // and perform necessary actions
    //   break;
    // }
    default: {
      result = DefWindowProcW(hwnd, u_msg, w_param, l_param);
      break;
    }
  }
  return result;
}

uint_fast8_t window_init(struct Window* window, struct APICommon* api_common, char* title, struct RendererSettings* renderer_settings) {
  window->title = title;
  window->api_common = api_common;
  window->renderer.renderer_settings.width = renderer_settings->width;
  window->renderer.renderer_settings.height = renderer_settings->height;
  window->renderer.renderer_settings.msaa_samples = renderer_settings->msaa_samples;
  window->vsync = renderer_settings->vsync;
  window->swap_chain = (struct SwapChain*)calloc(1, sizeof(struct SwapChain));
  window->gbuffer = (struct GBuffer*)calloc(1, sizeof(struct GBuffer));
  window->post_process = (struct PostProcess*)calloc(1, sizeof(struct PostProcess));
  input_manager_init(&window->input_manager, &(window->surface));
#ifdef _WIN64
  if (window_pc_window(window, renderer_settings->width, renderer_settings->height) == WINDOW_ERROR) {
    log_message(LOG_SEVERITY_ERROR, "Error creating Windows PC window!\n");
    return WINDOW_ERROR;
  }
#endif

  memcpy(&(window->renderer.renderer_settings), renderer_settings, sizeof(struct RendererSettings));

  switch (renderer_init(&(window->renderer), api_common, &(window->surface), window->swap_chain, window->gbuffer, window->post_process)) {
    case API_SUCCESS:
      return WINDOW_SUCCESS;
    case API_ERROR:
      return WINDOW_ERROR;
    default:
      return WINDOW_LAST_ERROR;
  }
}

void window_delete(struct Window* window) {
  renderer_delete(&(window->renderer), window->api_common, &(window->surface), window->swap_chain, window->gbuffer, window->post_process);
  free(window->post_process);
  free(window->gbuffer);
  free(window->swap_chain);

  input_manager_delete(&window->input_manager);

  DestroyWindow(window->surface.hwnd);
  UnregisterClass(window->surface.class_name, window->surface.hinstance);
}

void window_prepare_frame(struct Window* window) {
  // TODO: Put this in window platform stuff function
  MSG msg = {0};
  while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
    // TODO: If this is coming from windows should this trigger a shutdown? Or is it already shutting down by this point?
    if (msg.message == WM_QUIT)
      break;
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }

  if (input_manager_is_window_in_focus(&window->surface))
    input_manager_process_input(&window->input_manager);

  // Note: Wait for swap chain presentation to finish and check if window has been resized
  if (!window->minimized && window->swap_chain->swap_chain_func.swap_chain_wait_for_fences(&(window->swap_chain->swap_chain_common), window->api_common, window->swap_chain->swap_chain_common.current_frame))
    window_recreate_swap_chain(window);
}

void window_end_frame(struct Window* window) {
  if (!window->minimized) {
    uint_fast8_t result = window->swap_chain->swap_chain_func.swap_chain_end_frame(&(window->swap_chain->swap_chain_common), &(window->post_process->post_process_common), window->api_common);
    if (result == SWAP_CHAIN_UPDATE_FRAMERBUFFER || window->framebuffer_resized) {
      window->framebuffer_resized = false;
      RECT rect;
      if (GetClientRect(window->surface.hwnd, &rect)) {
        window->renderer.renderer_settings.width = (uint_fast32_t)(rect.right - rect.left);
        window->renderer.renderer_settings.height = (uint_fast32_t)(rect.bottom - rect.top);
      }
      if (!IsIconic(window->surface.hwnd)) {
        window_recreate_swap_chain(window);
        window->should_resize = true;
      }
    } else if (result != VK_SUCCESS)
      fprintf(stderr, "failed to present swap chain image!\n");

    window->swap_chain->swap_chain_common.current_frame = (window->swap_chain->swap_chain_common.current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
  }
}

void window_recreate_swap_chain(struct Window* window) {
  // NOTE: When window is minimized will pause thread and gpu
  // renderer_wait_for_device(&(window->renderer), window->api_common);

  swap_chain_resize(window->swap_chain, window->api_common, window->renderer.renderer_settings.width, window->renderer.renderer_settings.height, window->renderer.renderer_settings.supersample_scale);
  post_process_resize(window->post_process, window->api_common, &(window->swap_chain->swap_chain_common));
  gbuffer_resize(window->gbuffer, window->api_common, &(window->swap_chain->swap_chain_common), window->renderer.renderer_settings.msaa_samples);

  swap_chain_blit_update(window->swap_chain, window->api_common, &(window->post_process->post_process_common));
  post_process_resolve_update(window->post_process, window->api_common, window->gbuffer, &(window->swap_chain->swap_chain_common));

  // post_process_delete(window->post_process, window->api_common);
  // gbuffer_delete(window->gbuffer, window->api_common, window->renderer.renderer_settings.msaa_samples);
  // swap_chain_delete(window->swap_chain, window->api_common);

  // renderer_wait_for_device(&(window->renderer), window->api_common);

  // swap_chain_init(window->swap_chain, window->api_common, window->renderer.renderer_settings.width, window->renderer.renderer_settings.height, window->surface.hwnd);
  //  post_process_init(window->post_process, window->api_common, &(window->swap_chain->swap_chain_common));
  //  gbuffer_init(window->gbuffer, window->api_common, &(window->swap_chain->swap_chain_common), window->renderer.renderer_settings.msaa_samples);
  //
  // swap_chain_blit_init(window->swap_chain, window->api_common, &(window->post_process->post_process_common));
  //  post_process_blit_init(window->post_process, window->api_common, window->gbuffer, &(window->swap_chain->swap_chain_common));
}
