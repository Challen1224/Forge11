using System.Runtime.InteropServices;

namespace Forge.Studio.Bridge;

public static class Forge11Native
{
    private const string DllName = "forge11_abi.dll";

    // ------------------------------------------------------------------ //
    //  Application
    // ------------------------------------------------------------------ //
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

    // ------------------------------------------------------------------ //
    //  Embedded view
    // ------------------------------------------------------------------ //
    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr forge11_view_create(IntPtr parentHwnd, int width, int height);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void forge11_view_tick(IntPtr view, float r, float g, float b);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void forge11_view_resize(IntPtr view, int width, int height);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr forge11_view_hwnd(IntPtr view);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern void forge11_view_destroy(IntPtr view);
}