#include "mana/core/input/controllers/gamecubecontroller.h"

#ifdef _WIN64
static BOOL gamecube_controller_is_gamecube_controller_adapter(HANDLE deviceHandle) {
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

/*static BOOL GetDeviceDescriptor(WINUSB_INTERFACE_HANDLE handle) {
  UCHAR deviceDescriptor[18]; // Typically 18 bytes for USB device descriptor
  ULONG transferred = 0;
  WINUSB_SETUP_PACKET setupPacket = {
      .RequestType = 0x80, // Standard, Device to Host, Device
      .Request = 0x06,     // GET_DESCRIPTOR
      .Value = 0x0100,     // Device Descriptor Type (0x01) and Index (0x00)
      .Index = 0,
      .Length = sizeof(deviceDescriptor)};

  BOOL result = WinUsb_ControlTransfer(handle, setupPacket, deviceDescriptor, sizeof(deviceDescriptor), &transferred, NULL);
  if (!result || transferred != sizeof(deviceDescriptor)) {
    printf("Failed to get device descriptor.\n");
    return FALSE;
  }
  return TRUE;
}

static BOOL GetConfigurationDescriptor(WINUSB_INTERFACE_HANDLE handle) {
  UCHAR configDescriptor[41]; // Size can vary, ensure to capture the correct size from the descriptor
  ULONG transferred = 0;
  WINUSB_SETUP_PACKET setupPacket = {
      .RequestType = 0x80, // Standard, Device to Host, Device
      .Request = 0x06,     // GET_DESCRIPTOR
      .Value = 0x0200,     // Configuration Descriptor Type (0x02) and Index (0x00)
      .Index = 0,
      .Length = sizeof(configDescriptor)};

  BOOL result = WinUsb_ControlTransfer(handle, setupPacket, configDescriptor, sizeof(configDescriptor), &transferred, NULL);
  if (!result || transferred != sizeof(configDescriptor)) {
    printf("Failed to get configuration descriptor.\n");
    return FALSE;
  }
  return TRUE;
}

static BOOL SetConfiguration(WINUSB_INTERFACE_HANDLE handle) {
  WINUSB_SETUP_PACKET setupPacket = {
      .RequestType = 0x00, // Standard, Host to Device, Device
      .Request = 0x09,     // SET_CONFIGURATION
      .Value = 0x0001,     // Configuration Value 1
      .Index = 0,
      .Length = 0};

  ULONG transferred = 0;
  BOOL result = WinUsb_ControlTransfer(handle, setupPacket, NULL, 0, &transferred, NULL);
  if (!result) {
    printf("Failed to set configuration.\n");
    return FALSE;
  }
  return TRUE;
}*/
// Note: Needed for certain other gamecube adapters to work or something
static bool SendHIDReport(WINUSB_INTERFACE_HANDLE winusbHandle) {
  // Correct data for the HID report based on your packet capture details
  // unsigned char hidReport[] = {0x10, 0x01, 0x06, 0x0A, 0x00, 0x00, 0x00};
  // WINUSB_SETUP_PACKET setupPacket = {
  //    .RequestType = 0x21, // Host to device, class, interface
  //    .Request = 0x09,     // SET_REPORT
  //    .Value = 0x0210,     // Report type and report ID (0x02, 0x10)
  //    .Index = 0x0002,     // Interface 2
  //    .Length = sizeof(hidReport)};
  //
  // ULONG bytesSent = 0;
  // BOOL result = WinUsb_ControlTransfer(
  //    winusbHandle,
  //    setupPacket,
  //    hidReport,         // Data buffer containing the HID report
  //    sizeof(hidReport), // Length of the HID report
  //    &bytesSent,
  //    NULL // No overlapped structure
  //);

  WINUSB_SETUP_PACKET setupPacket = {
      .RequestType = 0x21,  // Class-specific request to interface
      .Request = 0x0B,      // SET_PROTOCOL
      .Value = 0x0001,      // Protocol 1
      .Index = 0x0000,      // Interface 0
      .Length = 0           // No data phase
  };

  ULONG bytesSent = 0;
  UCHAR buffer[0];  // No data for this setup, just configuring the device
  BOOL result = WinUsb_ControlTransfer(
      winusbHandle,
      setupPacket,
      buffer,  // No data buffer since wLength is 0
      0,       // No data to transfer
      &bytesSent,
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

static uint_fast8_t gamecube_controller_initialize_winusb(WINUSB_INTERFACE_HANDLE* winusb_handle, UCHAR* read_pipe_id, UCHAR* write_pipe_id) {
  // HDEVINFO hDevInfo = SetupDiGetClassDevs(NULL, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE | DIGCF_ALLCLASSES);
  HDEVINFO h_dev_info = SetupDiGetClassDevs(NULL, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_ALLCLASSES);

  if (h_dev_info == INVALID_HANDLE_VALUE) {
    printf("Failed to get device information set.\n");
    return 1;
  }

  SP_DEVINFO_DATA dev_info_data;
  dev_info_data.cbSize = sizeof(SP_DEVINFO_DATA);
  BOOL found = FALSE;

  for (DWORD i = 0; SetupDiEnumDeviceInfo(h_dev_info, i, &dev_info_data) && !found; i++) {
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
                    for (UCHAR i = 0; i < interface_descriptor.bNumEndpoints; i++) {
                      WINUSB_PIPE_INFORMATION pipe_info;
                      if (WinUsb_QueryPipe(*winusb_handle, 0, i, &pipe_info)) {
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

uint_fast8_t gamecube_controller_init(struct ControllerCommon* controller_common) {
  // controller_common->gamecube_controller.first_read = true;
#ifdef _WIN64
  // Initialize WinUSB
  uint_fast8_t error_code_gamecube_controller = gamecube_controller_initialize_winusb(&(controller_common->gamecube_controller.winusb_handle), &(controller_common->gamecube_controller.read_pipe_id), &(controller_common->gamecube_controller.write_pipe_id));
  if (error_code_gamecube_controller)
    return error_code_gamecube_controller;

  memset(controller_common->gamecube_controller.buffer, 0, sizeof(controller_common->gamecube_controller.buffer));

  memset(&(controller_common->gamecube_controller.overlapped), 0, sizeof(OVERLAPPED));
  controller_common->gamecube_controller.overlapped.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
  if (!controller_common->gamecube_controller.overlapped.hEvent)
    return 1;

  controller_common->gamecube_controller.reading_pending = false;
#endif

  return 0;
}

void gamecube_controller_delete(struct ControllerCommon* controller_common) {
#ifdef _WIN64
  CloseHandle(controller_common->gamecube_controller.overlapped.hEvent);
  WinUsb_Free(controller_common->gamecube_controller.winusb_handle);
#endif
}

static void add_gc_action(struct ControllerCommon* controller_common, uint_fast8_t button, bool pressed, bool held, bool released, float value) {
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

static void gamecube_controller_process_controller_data(struct ControllerCommon* controller_common) {
  memset(controller_common->controller_action_list, 0, sizeof(controller_common->controller_action_list));
  controller_common->controller_action_list_size = 0;
#ifdef _WIN64
  for (uint_fast8_t i = 0; i < 37; i++) {
    if (controller_common->gamecube_controller.buffer[i] == GC_ADAPTER_START_OF_INPUT) {  // Check if it's the start of a controller's data
      for (uint_fast8_t z = 1; z < 37; z += GC_BUFFER_SIZE + 1) {
        uint_fast8_t buffer_index = (i + z) % GC_ADAPTER_TOTAL_BUFFER_SIZE;  // Handle wrapping

        if (controller_common->gamecube_controller.buffer[buffer_index] == GC_CONTROLLER_PLUGGED_IN) {
          // Read the next 8 bytes for controller data
          BYTE controllerData[GC_BUFFER_SIZE];
          for (uint_fast8_t j = 0; j < GC_BUFFER_SIZE; j++)
            controllerData[j] = controller_common->gamecube_controller.buffer[(buffer_index + j + 1) % GC_ADAPTER_TOTAL_BUFFER_SIZE];

          if (controllerData[0] & GC_CONTROLLER_A)
            add_gc_action(controller_common, GC_CONTROLLER_ACTION_A, true, true, false, 1.0f);
          if (controllerData[0] & GC_CONTROLLER_B)
            add_gc_action(controller_common, GC_CONTROLLER_ACTION_B, true, true, false, 1.0f);
          if (controllerData[0] & GC_CONTROLLER_X)
            add_gc_action(controller_common, GC_CONTROLLER_ACTION_X, true, true, false, 1.0f);
          if (controllerData[0] & GC_CONTROLLER_Y)
            add_gc_action(controller_common, GC_CONTROLLER_ACTION_Y, true, true, false, 1.0f);
          if (controllerData[0] & GC_CONTROLLER_DPAD_LEFT)
            add_gc_action(controller_common, GC_CONTROLLER_ACTION_DPAD_LEFT, true, true, false, 1.0f);
          if (controllerData[0] & GC_CONTROLLER_DPAD_RIGHT)
            add_gc_action(controller_common, GC_CONTROLLER_ACTION_DPAD_RIGHT, true, true, false, 1.0f);
          if (controllerData[0] & GC_CONTROLLER_DPAD_UP)
            add_gc_action(controller_common, GC_CONTROLLER_ACTION_DPAD_UP, true, true, false, 1.0f);
          if (controllerData[0] & GC_CONTROLLER_DPAD_DOWN)
            add_gc_action(controller_common, GC_CONTROLLER_ACTION_DPAD_DOWN, true, true, false, 1.0f);
          if (controllerData[1] & GC_CONTROLLER_START)
            add_gc_action(controller_common, GC_CONTROLLER_ACTION_START, true, true, false, 1.0f);
          if (controllerData[1] & GC_CONTROLLER_Z)
            add_gc_action(controller_common, GC_CONTROLLER_ACTION_Z, true, true, false, 1.0f);
          if (controllerData[1] & GC_CONTROLLER_R)
            add_gc_action(controller_common, GC_CONTROLLER_ACTION_R, true, true, false, 1.0f);
          if (controllerData[1] & GC_CONTROLLER_L)
            add_gc_action(controller_common, GC_CONTROLLER_ACTION_L, true, true, false, 1.0f);

          if (controllerData[2] - 128 > 10 || controllerData[2] - 128 < -10)
            add_gc_action(controller_common, GC_CONTROLLER_ACTION_JOYSTICK_X, true, true, false, (float)(controllerData[2] - 128) / 128.0f);
          if (controllerData[3] - 128 > 10 || controllerData[3] - 128 < -10)
            add_gc_action(controller_common, GC_CONTROLLER_ACTION_JOYSTICK_Y, true, true, false, (float)(controllerData[3] - 128) / 128.0f);
          if (controllerData[4] - 128 > 10 || controllerData[4] - 128 < -10)
            add_gc_action(controller_common, GC_CONTROLLER_ACTION_CSTICK_X, true, true, false, (float)(controllerData[4] - 128) / 128.0f);
          if (controllerData[5] - 128 > 10 || controllerData[5] - 128 < -10)
            add_gc_action(controller_common, GC_CONTROLLER_ACTION_CSTICK_Y, true, true, false, (float)(controllerData[5] - 128) / 128.0f);
          if (controllerData[6] > 105)
            add_gc_action(controller_common, GC_CONTROLLER_ACTION_L_TRIGGER, true, true, false, (float)controllerData[6] / 255.0f);
          if (controllerData[7] > 105)
            add_gc_action(controller_common, GC_CONTROLLER_ACTION_R_TRIGGER, true, true, false, (float)controllerData[7] / 255.0f);

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
  if (controller_common->gamecube_controller.reading_pending == false) {
    // BOOL read_result = WinUsb_ReadPipe(controller_common->gamecube_controller.winusb_handle, controller_common->gamecube_controller.pipe_id, controller_common->gamecube_controller.buffer, sizeof(controller_common->gamecube_controller.buffer), NULL, &(controller_common->gamecube_controller.overlapped));
    WinUsb_ReadPipe(controller_common->gamecube_controller.winusb_handle, controller_common->gamecube_controller.read_pipe_id, controller_common->gamecube_controller.buffer, sizeof(controller_common->gamecube_controller.buffer), NULL, &(controller_common->gamecube_controller.overlapped));
    controller_common->gamecube_controller.reading_pending = true;
    // if (!read_result && GetLastError() == ERROR_IO_PENDING)
    //   controller_common->gamecube_controller.reading_pending = true;
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
  //  controller_common->gamecube_controller.reading_pending = false;
  //}
#endif
}
