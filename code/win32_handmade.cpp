#include <stdint.h>
#include <stdio.h>
#include <windows.h>
#include <xinput.h>

#define internal static
#define local_persist static
#define global_variable static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t unit8;
typedef uint16_t unit16;
typedef uint32_t unit32;
typedef uint64_t unit64;

// TODO: This is global for now
global_variable bool Running;

#define InitializeXInputGetState(name)                                         \
  DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef InitializeXInputGetState(x_input_get_state);
InitializeXInputGetState(XInputGetStateStub) { return 0; }
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

#define InitializeXInputSetState(name)                                         \
  DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef InitializeXInputSetState(x_input_set_state);
InitializeXInputSetState(XInputSetStateStub) { return 0; }
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

internal void Win32LoadXInput(void) {
  HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
  if (!XInputLibrary) {
    // Load xinput 1_3
    XInputLibrary = LoadLibraryA("xinput1_3.dll");
  }
  if (XInputLibrary) {
    XInputGetState =
        (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
    XInputSetState =
        (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
  }
}
struct win32_offscreen_buffer {
  BITMAPINFO Info;
  void *Memory;
  int Width;
  int Height;
  int Pitch;
  int BytesPerPixel = 4;
};

struct win32_window_dimension {
  int Width;
  int Height;
};
global_variable win32_offscreen_buffer GlobalBackBuffer;

internal void RenderWeirdGradient(win32_offscreen_buffer *Buffer,
                                  int BlueOffset, int GreenOffset) {
  unit8 *Row = (unit8 *)Buffer->Memory;
  for (int Y = 0; Y < Buffer->Height; Y++) {
    unit32 *Pixel = (unit32 *)Row;
    for (int X = 0; X < Buffer->Width; X++) {
      unit8 Blue = (X + BlueOffset);
      unit8 Green = (Y + GreenOffset);
      *Pixel++ = ((Green << 8) | Blue);
    }
    Row += Buffer->Pitch;
  }
}

internal void Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width,
                                    int Height) {

  if (Buffer->Memory) {
    VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
  }
  Buffer->Width = Width;
  Buffer->Height = Height;
  Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
  Buffer->Info.bmiHeader.biWidth = Buffer->Width;
  // Negative height means that the bitmap is top-down and the origin is the top
  Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
  Buffer->Info.bmiHeader.biPlanes = 1;
  Buffer->Info.bmiHeader.biBitCount = 32;
  Buffer->Info.bmiHeader.biCompression = BI_RGB;

  int BitmapMemorySize = Buffer->Width * Buffer->Height * Buffer->BytesPerPixel;
  Buffer->Memory =
      VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
  Buffer->Pitch = Width * Buffer->BytesPerPixel;
}

internal void Win32DisplayBufferInWindow(HDC DeviceContext, int WindowWidth,
                                         int WindowHeight,
                                         win32_offscreen_buffer Buffer) {

  // StretchDIBits(DeviceContext, X, Y, Width, Height, X, Y, Width, Height,
  //               BitmapMemory, &BitmapInfo, DIB_RGB_COLORS, SRCCOPY);
  StretchDIBits(DeviceContext, 0, 0, WindowWidth, WindowHeight, 0, 0,
                Buffer.Width, Buffer.Height, Buffer.Memory, &Buffer.Info,
                DIB_RGB_COLORS, SRCCOPY);
}

internal win32_window_dimension Win32GetWindowDimension(HWND Window) {
  win32_window_dimension Result;
  RECT ClientRect;
  GetClientRect(Window, &ClientRect);
  Result.Width = ClientRect.right - ClientRect.left;
  Result.Height = ClientRect.bottom - ClientRect.top;
  return Result;
}

LRESULT CALLBACK MainWindowCallback(HWND Window, UINT Message, WPARAM WParam,
                                    LPARAM LParam) {
  LRESULT Result = 0;

  switch (Message) {

  case WM_DESTROY: {
    Running = false;
    OutputDebugStringA("WM_DESTROY\n");
  } break;

  case WM_CLOSE: {
    Running = false;
    OutputDebugStringA("WM_CLOSE\n");
  } break;

  case WM_ACTIVATEAPP: {
    OutputDebugStringA("WM_ACTIVATEAPP\n");
  } break;

  case WM_SYSKEYDOWN:
  case WM_SYSKEYUP:
  case WM_KEYDOWN:
  case WM_KEYUP: {
    unit32 VKCode = WParam;
    bool WasDown = ((LParam & (1 << 30)) != 0);
    bool IsDown = ((LParam & (1 << 31)) == 0);
    if (WasDown != IsDown) {
      if (VKCode == 'W') {
      } else if (VKCode == 'A') {
      } else if (VKCode == 'S') {
      } else if (VKCode == 'D') {
      } else if (VKCode == 'Q') {
      } else if (VKCode == 'E') {
      } else if (VKCode == VK_UP) {
      } else if (VKCode == VK_DOWN) {
      } else if (VKCode == VK_LEFT) {
      } else if (VKCode == VK_RIGHT) {
      } else if (VKCode == VK_ESCAPE) {
      } else if (VKCode == VK_SPACE) {
      } else if (VKCode == VK_F4) {
        bool32 wasAltKeyDown = (LParam & (1 << 29));
        if (IsDown && wasAltKeyDown) {
          Running = false;
        }
      }
    }
  } break;

  case WM_PAINT: {
    PAINTSTRUCT Paint;
    HDC DeviceContext = BeginPaint(Window, &Paint);
    win32_window_dimension Dimension = Win32GetWindowDimension(Window);
    Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height,
                               GlobalBackBuffer);
    EndPaint(Window, &Paint);
  } break;

  default: {
    OutputDebugStringA("default\n");
    Result = DefWindowProc(Window, Message, WParam, LParam);
  } break;
  }
  return Result;
}

int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance,
                     LPSTR CommandLine, int ShowCode)

{
  Win32ResizeDIBSection(&GlobalBackBuffer, 1280, 720);
  WNDCLASSA WindowClass = {};
  // The following styles tell window to redraw the whole window if the size
  // changes and the last flag is to make sure that every window has its own
  // device context
  WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  WindowClass.lpfnWndProc = MainWindowCallback;
  WindowClass.hInstance = Instance;
  // WindowClass.hIcon=;
  WindowClass.hCursor = 0;
  WindowClass.lpszClassName = "HandmadeHeroWindowClass";
  // MessageBoxA(0, "This is a handmade hero", "Handmade hero", MB_OK |
  // MB_ICONINFORMATION);
  if (RegisterClassA(&WindowClass)) {
    HWND Window = CreateWindowExA(0, WindowClass.lpszClassName, "Handmade Hero",
                                  WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                  CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                  CW_USEDEFAULT, 0, 0, Instance, 0);
    if (Window) {
      int XOffset = 0;
      int YOffset = 0;
      Running = true;
      while (Running) {
        MSG Message;
        while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {
          if (Message.message == WM_QUIT) {
            Running = false;
          }
          TranslateMessage(&Message);
          DispatchMessage(&Message);
        }
        Win32LoadXInput();
        for (DWORD ControllerIndex = 0; ControllerIndex < XUSER_MAX_COUNT;
             ControllerIndex++) {
          XINPUT_STATE ControllerState;
          if (XInputGetState(ControllerIndex, &ControllerState) ==
              ERROR_SUCCESS) {
            // Controller is plugged in
            // See if ControllerState.dwPacketNumber increments too fast
            XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;
            bool Up = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
            bool Down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
            bool Left = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
            bool Right = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
            bool Start = (Pad->wButtons & XINPUT_GAMEPAD_START);
            bool Back = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
            bool LeftShoulder = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
            bool RightShoulder =
                (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
            bool AButton = (Pad->wButtons & XINPUT_GAMEPAD_A);
            bool BButton = (Pad->wButtons & XINPUT_GAMEPAD_B);
            bool XButton = (Pad->wButtons & XINPUT_GAMEPAD_X);
            bool YButton = (Pad->wButtons & XINPUT_GAMEPAD_Y);

            int16 StickX = Pad->sThumbLX;
            int16 StickY = Pad->sThumbLY;

            XOffset += StickX >> 12;
            YOffset += StickY >> 12;
            XINPUT_VIBRATION Vibration;
            Vibration.wLeftMotorSpeed = 60000;
            Vibration.wRightMotorSpeed = 60000;
            XInputSetState(0, &Vibration);
          } else {
            // Controller is not available
            printf("Controller not available\n");
          }
        }

        RenderWeirdGradient(&GlobalBackBuffer, XOffset, YOffset);

        HDC DeviceContext = GetDC(Window);
        win32_window_dimension Dimension = Win32GetWindowDimension(Window);
        Win32DisplayBufferInWindow(DeviceContext, Dimension.Width,
                                   Dimension.Height, GlobalBackBuffer);
        ReleaseDC(Window, DeviceContext); // Release the device context
        ++XOffset;
        ++YOffset;
      }
    } else {
      // TODO: log error
    }
  } else {
    // TODO: log error
  }

  return 0;
};
