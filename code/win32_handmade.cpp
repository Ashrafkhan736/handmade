#include <stdint.h>
#include <stdio.h>
#include <windows.h>

#define internal static
#define local_persist static
#define global_variable static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t unit8;
typedef uint16_t unit16;
typedef uint32_t unit32;
typedef uint64_t unit64;

// TODO: This is global for now
global_variable bool Running = true;

global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable int BitmapWidth;
global_variable int BitmapHeight;
global_variable int BytesPerPixel = 4;

internal void RenderWeirdGradient(int BlueOffset, int GreenOffset) {
  int Pitch = BitmapWidth * BytesPerPixel;
  unit8 *Row = (unit8 *)BitmapMemory;
  for (int Y = 0; Y < BitmapHeight; Y++) {
    unit32 *Pixel = (unit32 *)Row;
    for (int X = 0; X < BitmapWidth; X++) {
      unit8 Blue = (X + BlueOffset);
      unit8 Green = (Y + GreenOffset);
      *Pixel++ = ((Green << 8) | Blue);
    }
    Row += Pitch;
  }
}
internal void Win32ResizeDIBSection(int Width, int Height) {

  if (BitmapMemory) {
    VirtualFree(BitmapMemory, 0, MEM_RELEASE);
  }
  BitmapWidth = Width;
  BitmapHeight = Height;
  BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
  BitmapInfo.bmiHeader.biWidth = BitmapWidth;
  BitmapInfo.bmiHeader.biHeight = -BitmapHeight;
  BitmapInfo.bmiHeader.biPlanes = 1;
  BitmapInfo.bmiHeader.biBitCount = 32;
  BitmapInfo.bmiHeader.biCompression = BI_RGB;

  int BytesPerPixel = 4;
  int BitmapMemorySize = BitmapWidth * BitmapHeight * BytesPerPixel;
  BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

internal void Win32UpdateWindow(HDC DeviceContext, RECT *WindowRect, int X,
                                int Y, int Width, int Height) {
  int WindowWidth = WindowRect->right - WindowRect->left;
  int WindowHeight = WindowRect->bottom - WindowRect->top;

  // StretchDIBits(DeviceContext, X, Y, Width, Height, X, Y, Width, Height,
  //               BitmapMemory, &BitmapInfo, DIB_RGB_COLORS, SRCCOPY);
  StretchDIBits(DeviceContext, 0, 0, BitmapWidth, BitmapHeight, 0, 0,
                WindowWidth, WindowHeight, BitmapMemory, &BitmapInfo,
                DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK MainWindowCallback(HWND Window, UINT Message, WPARAM WParam,
                                    LPARAM LParam) {
  LRESULT Result = 0;
  switch (Message) {
  case WM_SIZE: {
    RECT ClientRect;
    GetClientRect(Window, &ClientRect); // Get the size of the window
    int Width = ClientRect.right - ClientRect.left;
    int Height = ClientRect.bottom - ClientRect.top;
    Win32ResizeDIBSection(Width, Height);
  } break;
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
  case WM_PAINT: {
    PAINTSTRUCT Paint;
    HDC DeviceContext = BeginPaint(Window, &Paint);
    int X = Paint.rcPaint.left;
    int Y = Paint.rcPaint.top;
    int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
    int Width = Paint.rcPaint.right - Paint.rcPaint.left;
    Win32UpdateWindow(DeviceContext, &Paint.rcPaint, X, Y, Width, Height);
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
      while (Running) {
        MSG Message;
        while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE)) {
          if (Message.message == WM_QUIT) {
            Running = false;
          }
          TranslateMessage(&Message);
          DispatchMessage(&Message);
          RenderWeirdGradient(XOffset, YOffset);
        }

        HDC DeviceContext = GetDC(Window);
        RECT ClientRect;
        GetClientRect(Window, &ClientRect); // Get the size of the window
        int WindowWidth = ClientRect.right - ClientRect.left;
        int WindowHeight = ClientRect.bottom - ClientRect.top;
        Win32UpdateWindow(DeviceContext, &ClientRect, 0, 0, WindowWidth,
                          WindowHeight);
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
