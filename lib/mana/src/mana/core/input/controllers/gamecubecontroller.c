#include "mana/core/input/controllers/gamecubecontroller.h"

#ifdef _WIN64
DEFINE_GUID(GUID_DEVINTERFACE_USB_DEVICE, 0xA5DCBF10, 0x6530, 0x11D2, 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED);

internal BOOL gamecube_controller_is_gamecube_controller_adapter(HANDLE deviceHandle) {
  // Replace with the VID and PID of the GameCube controller adapter

  BOOL is_match = FALSE;
  WINUSB_INTERFACE_HANDLE winusb_handle = NULL;
  if (WinUsb_Initialize(deviceHandle, &winusb_handle)) {
    USB_DEVICE_DESCRIPTOR device_desc;
    if (WinUsb_GetDescriptor(winusb_handle, USB_DEVICE_DESCRIPTOR_TYPE, 0, 0, (PUCHAR)&device_desc, sizeof(device_desc), NULL)) {
      if (device_desc.idVendor == CONTROLLER_VID && device_desc.idProduct == CONTROLLER_PID) {
        is_match = TRUE;
      }
    }
    WinUsb_Free(winusb_handle);
  }
  return is_match;
}

// Note: Needed for certain other gamecube adapters to work or something
internal b8 SendHIDReport(WINUSB_INTERFACE_HANDLE winusbHandle) {
  WINUSB_SETUP_PACKET setup_packet = {
      .RequestType = 0x21,  // Class-specific request to interface
      .Request = 0x0B,      // SET_PROTOCOL
      .Value = 0x0001,      // Protocol 1
      .Index = 0x0000,      // Interface 0
      .Length = 0           // No data phase
  };

  ULONG bytes_sent = 0;
  BOOL result = WinUsb_ControlTransfer(
      winusbHandle,
      setup_packet,
      NULL,  // No data buffer since wLength is 0
      0,     // No data to transfer
      &bytes_sent,
      NULL  // No overlapped structure
  );

  if (!result) {
    printf("Error sending HID report. Error: %lu\n", GetLastError());
    return FALSE;
  } else {
    printf("HID report sent successfully.\n");
    return TRUE;
  }
}

internal u8 gamecube_controller_initialize_winusb(WINUSB_INTERFACE_HANDLE* winusb_handle, UCHAR* read_pipe_id, UCHAR* write_pipe_id) {
  // HDEVINFO hDevInfo = SetupDiGetClassDevs(NULL, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE | DIGCF_ALLCLASSES);
  HDEVINFO h_dev_info = SetupDiGetClassDevs(NULL, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_ALLCLASSES);

  if (h_dev_info == INVALID_HANDLE_VALUE) {
    printf("Failed to get device information set.\n");
    return 1;
  }

  SP_DEVINFO_DATA dev_info_data;
  dev_info_data.cbSize = sizeof(SP_DEVINFO_DATA);
  BOOL found = FALSE;

  for (DWORD device_index = 0; SetupDiEnumDeviceInfo(h_dev_info, device_index, &dev_info_data) && !found; device_index++) {
    TCHAR device_iD[MAX_DEVICE_ID_LEN];
    if (CM_Get_Device_ID(dev_info_data.DevInst, device_iD, MAX_DEVICE_ID_LEN, 0) == CR_SUCCESS) {
      // Check if this is the device we are looking for
      if (wcsstr(device_iD, L"VID_057E&PID_0337")) {
        found = TRUE;
        SP_DEVICE_INTERFACE_DATA interfaceData = {0};
        interfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

        if (SetupDiEnumDeviceInterfaces(h_dev_info, &dev_info_data, &GUID_DEVINTERFACE_USB_DEVICE, 0, &interfaceData)) {
          DWORD needed;
          SetupDiGetDeviceInterfaceDetail(h_dev_info, &interfaceData, NULL, 0, &needed, NULL);

          PSP_DEVICE_INTERFACE_DETAIL_DATA detail_data = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(needed);
          if (detail_data) {
            detail_data->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
            if (SetupDiGetDeviceInterfaceDetail(h_dev_info, &interfaceData, detail_data, needed, NULL, &dev_info_data)) {
              // Use detailData->DevicePath to open the device
              HANDLE hDevice = CreateFile(detail_data->DevicePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
              if (hDevice != INVALID_HANDLE_VALUE) {
                if (WinUsb_Initialize(hDevice, winusb_handle)) {
                  // Device handle opened and WinUSB initialized successfully
                  SendHIDReport(*winusb_handle);

                  // Now enumerate the pipes
                  USB_INTERFACE_DESCRIPTOR interface_descriptor;
                  if (WinUsb_QueryInterfaceSettings(*winusb_handle, 0, &interface_descriptor)) {
                    for (UCHAR interface_index = 0; interface_index < interface_descriptor.bNumEndpoints; interface_index++) {
                      WINUSB_PIPE_INFORMATION pipe_info;
                      if (WinUsb_QueryPipe(*winusb_handle, 0, interface_index, &pipe_info)) {
                        // Check the type and direction of the pipe
                        if (pipe_info.PipeType == UsbdPipeTypeInterrupt && USB_ENDPOINT_DIRECTION_IN(pipe_info.PipeId)) {
                          // This is an interrupt IN pipe, which is typically used for device input
                          *read_pipe_id = pipe_info.PipeId;
                          // break; // Break if you only need the first matching pipe
                        }
                        if (pipe_info.PipeType == UsbdPipeTypeInterrupt && USB_ENDPOINT_DIRECTION_OUT(pipe_info.PipeId)) {
                          // This is an interrupt OUT pipe, which is typically used for device output
                          *write_pipe_id = pipe_info.PipeId;
                          // break;                   // We found the OUT endpoint
                        }
                      }
                    }
                  }

                  // WinUsb_Free(winusbHandle);
                } else
                  printf("Failed to initialize WinUSB. Error: %lu\n", GetLastError());
              } else
                printf("Failed to open device handle. Error: %lu\n", GetLastError());
            }
            free(detail_data);
          }
        } else
          printf("Failed to get device interface detail. Error: %lu\n", GetLastError());
      }
    }
  }

  if (!found) {
    printf("Device not found.\n");
    return 2;
  }

  SetupDiDestroyDeviceInfoList(h_dev_info);

  // Send the single-byte initialization command to tell adapter to start sending controller data
  UCHAR initCommand[] = {0x13};  // The initialization command observed
  ULONG bytesWritten;
  BOOL writeResult = WinUsb_WritePipe(
      *winusb_handle,
      *write_pipe_id,
      initCommand,
      sizeof(initCommand),
      &bytesWritten,
      NULL);

  if (!writeResult)
    printf("Failed to send initialization command. Error: %lu\n", GetLastError());
  else
    printf("Initialization command sent successfully, %lu bytes written.\n", bytesWritten);

  // Send the rumble off command as part of initialization to ensure rumble is off because it can be left on from previous use
  UCHAR rumbleOffCommand[] = {0x11, 0x00, 0x00, 0x00, 0x00};
  writeResult = WinUsb_WritePipe(
      *winusb_handle,
      *write_pipe_id,
      rumbleOffCommand,
      sizeof(rumbleOffCommand),
      &bytesWritten,
      NULL);

  if (!writeResult)
    printf("Failed to send rumble off command. Error: %lu\n", GetLastError());
  else
    printf("Rumble off command sent successfully, %lu bytes written.\n", bytesWritten);

  return 0;
}
#endif

u8 gamecube_controller_init(struct ControllerCommon* controller_common) {
  // controller_common->gamecube_controller.first_read = TRUE;
#ifdef _WIN64
  // Initialize WinUSB
  u8 error_code_gamecube_controller = gamecube_controller_initialize_winusb(&(controller_common->gamecube_controller.winusb_handle), &(controller_common->gamecube_controller.read_pipe_id), &(controller_common->gamecube_controller.write_pipe_id));
  if (error_code_gamecube_controller)
    return error_code_gamecube_controller;

  memset(controller_common->gamecube_controller.buffer, 0, sizeof(controller_common->gamecube_controller.buffer));

  memset(&(controller_common->gamecube_controller.overlapped), 0, sizeof(OVERLAPPED));
  controller_common->gamecube_controller.overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
  if (!controller_common->gamecube_controller.overlapped.hEvent)
    return 1;

  controller_common->gamecube_controller.reading_pending = FALSE;
#endif

  return 0;
}

void gamecube_controller_delete(struct ControllerCommon* controller_common) {
#ifdef _WIN64
  CloseHandle(controller_common->gamecube_controller.overlapped.hEvent);
  WinUsb_Free(controller_common->gamecube_controller.winusb_handle);
#endif
}

internal void add_gc_action(struct ControllerCommon* controller_common, u8 button, b8 pressed, b8 held, b8 released, r32 value) {
  if (controller_common->controller_action_list_size < 16) {
    struct ControllerAction* controller_action = &(controller_common->controller_action_list[controller_common->controller_action_list_size]);
    controller_action->button = button;
    controller_action->pressed = pressed;
    controller_action->held = held;
    controller_action->released = released;
    controller_action->value = value;
    controller_common->controller_action_list_size++;
  }
}

internal void gamecube_controller_process_controller_data(struct ControllerCommon* controller_common) {
  memset(controller_common->controller_action_list, 0, sizeof(controller_common->controller_action_list));
  controller_common->controller_action_list_size = 0;
#ifdef _WIN64
  for (u8 i = 0; i < 37; i++) {
    if (controller_common->gamecube_controller.buffer[i] == GC_ADAPTER_START_OF_INPUT) {  // Check if it's the start of a controller's data
      for (u8 z = 1; z < 37; z += GC_BUFFER_SIZE + 1) {
        u8 buffer_index = (i + z) % GC_ADAPTER_TOTAL_BUFFER_SIZE;  // Handle wrapping

        if (controller_common->gamecube_controller.buffer[buffer_index] == GC_CONTROLLER_PLUGGED_IN) {
          // Read the next 8 bytes for controller data
          BYTE controllerData[GC_BUFFER_SIZE];
          for (u8 j = 0; j < GC_BUFFER_SIZE; j++)
            controllerData[j] = controller_common->gamecube_controller.buffer[(buffer_index + j + 1) % GC_ADAPTER_TOTAL_BUFFER_SIZE];

          if (controllerData[0] & GC_CONTROLLER_A)
            add_gc_action(controller_common, GC_CONTROLLER_ACTION_A, TRUE, TRUE, FALSE, 1.0f);
          if (controllerData[0] & GC_CONTROLLER_B)
            add_gc_action(controller_common, GC_CONTROLLER_ACTION_B, TRUE, TRUE, FALSE, 1.0f);
          if (controllerData[0] & GC_CONTROLLER_X)
            add_gc_action(controller_common, GC_CONTROLLER_ACTION_X, TRUE, TRUE, FALSE, 1.0f);
          if (controllerData[0] & GC_CONTROLLER_Y)
            add_gc_action(controller_common, GC_CONTROLLER_ACTION_Y, TRUE, TRUE, FALSE, 1.0f);
          if (controllerData[0] & GC_CONTROLLER_DPAD_LEFT)
            add_gc_action(controller_common, GC_CONTROLLER_ACTION_DPAD_LEFT, TRUE, TRUE, FALSE, 1.0f);
          if (controllerData[0] & GC_CONTROLLER_DPAD_RIGHT)
            add_gc_action(controller_common, GC_CONTROLLER_ACTION_DPAD_RIGHT, TRUE, TRUE, FALSE, 1.0f);
          if (controllerData[0] & GC_CONTROLLER_DPAD_UP)
            add_gc_action(controller_common, GC_CONTROLLER_ACTION_DPAD_UP, TRUE, TRUE, FALSE, 1.0f);
          if (controllerData[0] & GC_CONTROLLER_DPAD_DOWN)
            add_gc_action(controller_common, GC_CONTROLLER_ACTION_DPAD_DOWN, TRUE, TRUE, FALSE, 1.0f);
          if (controllerData[1] & GC_CONTROLLER_START)
            add_gc_action(controller_common, GC_CONTROLLER_ACTION_START, TRUE, TRUE, FALSE, 1.0f);
          if (controllerData[1] & GC_CONTROLLER_Z)
            add_gc_action(controller_common, GC_CONTROLLER_ACTION_Z, TRUE, TRUE, FALSE, 1.0f);
          if (controllerData[1] & GC_CONTROLLER_R)
            add_gc_action(controller_common, GC_CONTROLLER_ACTION_R, TRUE, TRUE, FALSE, 1.0f);
          if (controllerData[1] & GC_CONTROLLER_L)
            add_gc_action(controller_common, GC_CONTROLLER_ACTION_L, TRUE, TRUE, FALSE, 1.0f);

          if (controllerData[2] - 128 > 10 || controllerData[2] - 128 < -10)
            add_gc_action(controller_common, GC_CONTROLLER_ACTION_JOYSTICK_X, TRUE, TRUE, FALSE, (r32)(controllerData[2] - 128) / 128.0f);
          if (controllerData[3] - 128 > 10 || controllerData[3] - 128 < -10)
            add_gc_action(controller_common, GC_CONTROLLER_ACTION_JOYSTICK_Y, TRUE, TRUE, FALSE, (r32)(controllerData[3] - 128) / 128.0f);
          if (controllerData[4] - 128 > 10 || controllerData[4] - 128 < -10)
            add_gc_action(controller_common, GC_CONTROLLER_ACTION_CSTICK_X, TRUE, TRUE, FALSE, (r32)(controllerData[4] - 128) / 128.0f);
          if (controllerData[5] - 128 > 10 || controllerData[5] - 128 < -10)
            add_gc_action(controller_common, GC_CONTROLLER_ACTION_CSTICK_Y, TRUE, TRUE, FALSE, (r32)(controllerData[5] - 128) / 128.0f);
          if (controllerData[6] > 105)
            add_gc_action(controller_common, GC_CONTROLLER_ACTION_L_TRIGGER, TRUE, TRUE, FALSE, (r32)controllerData[6] / 255.0f);
          if (controllerData[7] > 105)
            add_gc_action(controller_common, GC_CONTROLLER_ACTION_R_TRIGGER, TRUE, TRUE, FALSE, (r32)controllerData[7] / 255.0f);

        } else if (controller_common->gamecube_controller.buffer[buffer_index] == GC_CONTROLLER_NOT_PLUGGED_IN) {
          // Controller is not plugged in
          // printf("Controller not plugged in\n");
        }
      }
      break;
    }
  }
#endif
}

void gamecube_controller_process_input(struct ControllerCommon* controller_common) {
#if defined(_WIN64)
  //  (enable ? 0x01 : 0x00)
  // UCHAR rumbleCommand[] = {0x11, 0x01, 0x00, 0x00, 0x00}; // Example command to enable rumble
  // ULONG bytesWritten;
  // BOOL writeResult = WinUsb_WritePipe(controller_common->gamecube_controller.winusb_handle, controller_common->gamecube_controller.write_pipe_id, rumbleCommand, sizeof(rumbleCommand), &bytesWritten, NULL);
  //
  // if (!writeResult)
  //  printf("Failed to send rumble command. Error: %lu\n", GetLastError());
  ////////////////////
  if (controller_common->gamecube_controller.reading_pending == FALSE) {
    // BOOL read_result = WinUsb_ReadPipe(controller_common->gamecube_controller.winusb_handle, controller_common->gamecube_controller.pipe_id, controller_common->gamecube_controller.buffer, sizeof(controller_common->gamecube_controller.buffer), NULL, &(controller_common->gamecube_controller.overlapped));
    WinUsb_ReadPipe(controller_common->gamecube_controller.winusb_handle, controller_common->gamecube_controller.read_pipe_id, controller_common->gamecube_controller.buffer, sizeof(controller_common->gamecube_controller.buffer), NULL, &(controller_common->gamecube_controller.overlapped));
    controller_common->gamecube_controller.reading_pending = TRUE;
    // if (!read_result && GetLastError() == ERROR_IO_PENDING)
    //   controller_common->gamecube_controller.reading_pending = TRUE;
    // else if (read_result) // The operation completed immediately
    //   gamecube_controller_process_controller_data(controller_common);
  }
  // Non-blocking check if the read operation has completed
  // TODO: CURRENT gamecube controller area I'm working on
  // if (HasOverlappedIoCompleted(&(controller_common->gamecube_controller.overlapped))) {
  //  DWORD bytes_read;
  //  if (!WinUsb_GetOverlappedResult(controller_common->gamecube_controller.winusb_handle, &(controller_common->gamecube_controller.overlapped), &bytes_read, FALSE))
  //    printf("Failed to read data from the controller. Error: %lu\n", GetLastError());
  //  else {
  //    // Data was read successfully, process it
  //    if (bytes_read > 1)
  //      gamecube_controller_process_controller_data(controller_common);
  //    else
  //      printf("Insufficient data received\n");
  //  }
  //  controller_common->gamecube_controller.reading_pending = FALSE;
  //}
#endif
}
