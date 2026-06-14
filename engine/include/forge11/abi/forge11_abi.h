#pragma once

// C ABI surface for Forge11. Stable, versioned entry points consumed by
// non-C++ hosts (e.g. Forge Studio via C# P/Invoke).

#ifdef _WIN32
  #ifdef FORGE11_BUILD_DLL
    #define FORGE11_API extern "C" __declspec(dllexport)
  #else
    #define FORGE11_API extern "C" __declspec(dllimport)
  #endif
#else
  #define FORGE11_API extern "C"
#endif

/// Opaque handle to a Forge11 Application instance.
typedef void* Forge11AppHandle;

/// Creates a new Application instance. Returns nullptr on failure.
FORGE11_API Forge11AppHandle forge11_app_create();

/// Initializes the primary window. Returns 1 on success, 0 on failure.
FORGE11_API int forge11_app_initialize(Forge11AppHandle app,
                                        const wchar_t* title,
                                        int width,
                                        int height);

/// Runs the message loop. Blocks until quit. Returns process exit code.
FORGE11_API int forge11_app_run(Forge11AppHandle app);

/// Requests application shutdown.
FORGE11_API void forge11_app_quit(Forge11AppHandle app);

/// Destroys the Application instance and frees all resources.
FORGE11_API void forge11_app_destroy(Forge11AppHandle app);

/// Returns the Forge11 engine version string (e.g. "0.1.0").
FORGE11_API const char* forge11_get_version();