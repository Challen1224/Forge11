using System.Runtime.InteropServices;

namespace Forge.Studio.Bridge;

/// <summary>
/// Raw P/Invoke bindings to forge11_abi.dll.
/// Mirrors engine/include/forge11/abi/forge11_abi.h.
/// </summary>
public static class Forge11Native
{
    private const string DllName = "forge11_abi.dll";

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr forge11_app_create();

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    public static extern int forge11_app_initialize(IntPtr app, string title, int width, int height);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern int forge11_app_run(IntPtr app);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void forge11_app_quit(IntPtr app);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void forge11_app_destroy(IntPtr app);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr forge11_get_version();

    public static string GetVersion()
    {
        IntPtr ptr = forge11_get_version();
        return Marshal.PtrToStringAnsi(ptr) ?? "unknown";
    }
}