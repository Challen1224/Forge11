#pragma once

#ifdef _WIN32
  #ifdef FORGE11_BUILD_DLL
    #define FORGE11_API extern "C" __declspec(dllexport)
  #else
    #define FORGE11_API extern "C" __declspec(dllimport)
  #endif
#else
  #define FORGE11_API extern "C"
#endif

typedef void* Forge11AppHandle;
typedef void* Forge11ViewHandle;

// ------------------------------------------------------------------ //
//  Application
// ------------------------------------------------------------------ //
FORGE11_API Forge11AppHandle forge11_app_create();
FORGE11_API int              forge11_app_initialize(Forge11AppHandle app,
                                                     const wchar_t*   title,
                                                     int width, int height);
FORGE11_API int              forge11_app_run(Forge11AppHandle app);
FORGE11_API void             forge11_app_quit(Forge11AppHandle app);
FORGE11_API void             forge11_app_destroy(Forge11AppHandle app);
FORGE11_API const char*      forge11_get_version();

// ------------------------------------------------------------------ //
//  Embedded view (child HWND + D3D12 renderer, driven by host)
// ------------------------------------------------------------------ //

/// Creates a Win32 child window + D3D12 renderer inside parentHwnd.
/// Returns nullptr on failure.
FORGE11_API Forge11ViewHandle forge11_view_create(void* parentHwnd,
                                                   int width, int height);

/// Renders one frame (clear color for now). Call each frame from host.
FORGE11_API void forge11_view_tick(Forge11ViewHandle view,
                                    float r, float g, float b);

/// Resizes the swap chain when the host window resizes.
FORGE11_API void forge11_view_resize(Forge11ViewHandle view,
                                      int width, int height);

/// Returns the child HWND (as void*) so WPF HwndHost can return it.
FORGE11_API void* forge11_view_hwnd(Forge11ViewHandle view);

/// Destroys the view and releases all D3D12 resources.
FORGE11_API void forge11_view_destroy(Forge11ViewHandle view);