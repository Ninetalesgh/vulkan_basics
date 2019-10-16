
#include "brew.h"

#include <renderer_vulkan.h>

#include <core/core.h>

//#include <stdio.h>
#include <iostream>

#include "Windowsx.h"
#include "Windows.h"


LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
  LRESULT result = 0;

  switch (message)
  {
  case WM_DESTROY:
    //destroy();
    break;
  case WM_MOUSEMOVE:
   // engine_instance->update_mouse_position(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
    break;
  case WM_LBUTTONDOWN:
   // engine_instance->set_input(0, true);
    break;
  case WM_RBUTTONDOWN:
  //  engine_instance->set_input(1, true);
    break;
  case WM_MBUTTONDOWN:
  //  engine_instance->set_input(2, true);
    break;
  case WM_XBUTTONDOWN:
  case WM_KEYDOWN:
  //  engine_instance->set_input((int32_t)wparam, true);
    //TODO remove esc key kill
  //  if (GET_WPARAM(wparam, lparam) == 27) running = false;
    break;
  case WM_LBUTTONUP:
   // engine_instance->set_input(0, false);
    break;
  case WM_RBUTTONUP:
   // engine_instance->set_input(1, false);
    break;
  case WM_MBUTTONUP:
   // engine_instance->set_input(2, false);
    break;
  case WM_XBUTTONUP:
  case WM_KEYUP:
   // engine_instance->set_input((int32_t)wparam, false);
    break;
  case WM_MOUSEWHEEL:

    break;

  default:
    result = DefWindowProcA(window, message, wparam, lparam);
    break;
  }

  return result;
}

int WINAPI main(HINSTANCE instance, HINSTANCE previous_instance, PWSTR command_line, int show_command)
{
  AllocConsole();

  FILE* consoleFile = freopen("conin$", "r", stdin);
        consoleFile = freopen("conout$", "w", stdout);
        consoleFile = freopen("conout$", "w", stderr);

  brs::renderer::vulkan::RendererVulkan test;
  test.main();
  return 0;

  HRESULT hresult;

  WNDCLASSA window_class = {};
  window_class.lpfnWndProc = WindowProc;
  window_class.lpszClassName = "dx12";
  window_class.hInstance = instance;
  window_class.style = CS_HREDRAW | CS_VREDRAW;

  hresult = RegisterClassA(&window_class);
  HWND window = CreateWindowA("dx12", 0, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, instance, 0);

  SetWindowText(window, "Brewing Station");

  ShowWindow(window, SW_SHOW);
  printf("window initialized.\n");


  //bs::renderer::RendererDx12 renderer;
  //auto brewing_station = bs::core::Core::Create();
  bs::brew::Brew brew;
  bs::core::Core brewing_station_instance;

  brewing_station_instance.Init();
  brewing_station_instance.RegisterApp(&brew);
  brewing_station_instance.Run();

 // while (running)
  {
  //  float dt = float(get_frame_delta());
  //  if (dt == 0.0) continue;

    MSG message;
    while (PeekMessage(&message, window, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&message);
      DispatchMessage(&message);
    }
  }

  return 0;
}