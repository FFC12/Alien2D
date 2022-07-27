/*
MIT License

Copyright (c) 2022 Furkan Fatih Cetindil

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

- Introduction:

NOTE: Library is still WIP. So many features have not finished yet! But our
goals has explained in rest of introduction text.

This library has created for helping people to create their 2D games without any
dependencies. So, you do not need integrate a lot of dependencies/3rd party
libraries when you use this Alien framework. All neccessarities has already can
be included with header files. So Alien is a header-only library and it can be
extended so easily. There are a lot of different libraries which you can use for
your game projects as well. But Alien is focusing only 2d game development
rather than being general purpose API for graphics applications.

- Modules:

We have our own mathematics module and our own Vector and Matrix types which we
are using as a module. We have our own PNG/JPG parser but it also can be
extended for the different/new type of image files. Alien library also offers a
game-oriented memory allocator which also can be replaced by custom one. Context
creation and loading graphics api functions has all done in the library. So you
don't need to use any GLEW, GLAD or etc. Also for windows and linux systems,
window creation and mouse/keyboard inputs are handling by Alien. So you do not
need to use GLFW or SDL too. And many other features are in-progress and will be
added later as well.

- Contributors:

  FFC12 (author)

*/
#ifdef _WIN32
#pragma once
#elif
#ifndef ALIEN_WINDOW_HPP
#define ALIEN_WINDOW_HPP
#endif
#endif

#ifdef _MSC_VER
#pragma comment(linker, "/ENTRY:mainCRTStartup")
#ifdef ALIEN_WINDOWS_SUBSYSTEM_WINDOWS
#pragma comment(linker, "/subsystem:windows")
#else
#pragma comment(linker, "/subsystem:console")
#endif
#endif

#ifdef _WIN32
#include <Windows.h>
#endif

#include "alien_dx11.hpp"
#include "alien_gl.hpp"
#include "base.hpp"
#include "math.hpp"

namespace Alien {
// Queue type
enum Queue { e_Init, e_Update, e_Kill };

// Basically the idea is
// for all callbacks that has an AppState
// parameter, the state will be shareable
// between each of them.
struct AppState {
  std::shared_ptr<void *> ptr;
};

class Renderer;
class App {
  friend class Renderer;

 public:
  App(const char *wn, u32 w = 800, u32 h = 600)
      : m_WindowName(wn), m_Width(w), m_Height(h) {
    auto result = init(wn, w, h);
    if (!result) {
      std::cerr << "Something went wrong!\n";
    }
  }

  void add_event_queue(const std::function<void(AppState &)> &f, Queue t) {
    switch (t) {
      case e_Init: {
        m_InitQueue.push_back(f);
        break;
      }
      case e_Update: {
        m_UpdateQueue.push_back(f);
        break;
      }
      case e_Kill: {
        m_KillQueue.push_back(f);
        break;
      }
    }
  }

  void update() { update_window(); }

  ~App() {}

#ifndef ALIEN_DX11
  Alien::GLContext m_Context;
#else
  Alien::DX11Context m_Context;
#endif
 private:
  bool init(const char *windowName, u32 w, u32 h) {
#ifdef _WIN32
    // Will be registered
    WNDCLASS winClass = {0};

    // Initialization of the Window struct
    winClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    winClass.lpfnWndProc = WinProc;

    // https://stackoverflow.com/a/11785228
    m_Instance = GetModuleHandle(0);
    winClass.hInstance = m_Instance;

    winClass.lpszClassName = TEXT(windowName);

    // Register the class to Windows to create a window
    RegisterClass(&winClass);

    // Create window
    m_WindowHandle =
        CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, TEXT(m_WindowName.c_str()),
                       TEXT(m_WindowName.c_str()), WS_OVERLAPPEDWINDOW, 0, 0,
                       m_Width, m_Height, NULL, NULL, m_Instance, NULL);

    if (!m_WindowHandle) {
      auto error = GetLastError();
      std::cerr << "Could not initialized the window\n";
      exit(error);
    }

    // Get device context
    m_DeviceContext = GetDC(m_WindowHandle);

    // Show the window
    ShowWindow(m_WindowHandle, SW_SHOW);

    // Set the context for window
    m_Context.set_context(m_WindowHandle, m_Instance, m_DeviceContext);
#endif
#ifndef ALIEN_DX11
    m_Context.create_context_gl();
#else
    m_Context.create_context_dx11();
#endif
    return true;
  }

  void update_window() {
    // Callback unit queue
    for (auto &func : m_InitQueue) {
      func(m_AppState);
    }

#ifdef _WIN32
    MSG msg;
    while (true) {
      if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) break;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }

      for (auto &func : m_UpdateQueue) {
        func(m_AppState);
      }

      SwapBuffers(m_DeviceContext);

      if (ShouldQuit) break;
    }

    ReleaseDC(m_WindowHandle, m_DeviceContext);
    DestroyWindow(m_WindowHandle);
#endif
  }

#ifdef _WIN32
  static LRESULT CALLBACK WinProc(HWND window_handle, UINT message,
                                  WPARAM param_w, LPARAM param_l) {
    switch (message) {
      case WM_DESTROY:
        if (!ShouldQuit) {
          ShouldQuit = true;
          return 0;
        } else
          PostQuitMessage(0);
        break;
    }

    return DefWindowProc(window_handle, message, param_w, param_l);
  }
#endif

#ifdef _WIN32
  HWND m_WindowHandle{nullptr};
  HINSTANCE m_Instance;
  HDC m_DeviceContext;
#endif

  u32 m_Width{800};
  u32 m_Height{600};
  std::string m_WindowName;

  AppState m_AppState;

  std::vector<std::function<void(AppState &)>> m_UpdateQueue;
  std::vector<std::function<void(AppState &)>> m_InitQueue;
  std::vector<std::function<void(AppState &)>> m_KillQueue;

  static inline bool ShouldQuit{false};
};
}  // namespace Alien

#ifndef _WIN32
#ifndef ALIEN_WINDOW_HPP
#define ALIEN_WINDOW_HPP
#endif
#endif