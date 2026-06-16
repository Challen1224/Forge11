using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Interop;
using Forge.Studio.Bridge;

namespace Forge.Studio.App.Views;

/// <summary>
/// WPF HwndHost that embeds a Forge11 D3D12 child window.
/// Rendering runs on a dedicated background thread so the WPF UI
/// thread is never blocked by GPU work or Present calls.
/// </summary>
public class ForgeHwndHost : HwndHost
{
    private IntPtr _view   = IntPtr.Zero;
    private int    _width  = 1;
    private int    _height = 1;

    private Thread?           _renderThread;
    private volatile bool     _running = false;
    private volatile int      _pendingWidth  = 0;
    private volatile int      _pendingHeight = 0;

    public ForgeHwndHost(int width, int height)
    {
        _width  = Math.Max(width,  1);
        _height = Math.Max(height, 1);
    }

    // ------------------------------------------------------------------ //
    //  HwndHost overrides
    // ------------------------------------------------------------------ //
    protected override HandleRef BuildWindowCore(HandleRef hwndParent)
    {
        _view = Forge11Native.forge11_view_create(
            hwndParent.Handle, _width, _height);

        if (_view == IntPtr.Zero)
            throw new InvalidOperationException(
                "forge11_view_create failed — check D3D12 device creation.");

        IntPtr childHwnd = Forge11Native.forge11_view_hwnd(_view);

        // Start dedicated render thread.
        _running = true;
        _renderThread = new Thread(RenderLoop)
        {
            IsBackground = true,
            Name         = "Forge11RenderThread"
        };
        _renderThread.Start();

        return new HandleRef(this, childHwnd);
    }

    protected override void DestroyWindowCore(HandleRef hwnd)
    {
        // Signal render thread to stop and wait for it.
        _running = false;
        _renderThread?.Join(2000);
        _renderThread = null;

        if (_view != IntPtr.Zero)
        {
            Forge11Native.forge11_view_destroy(_view);
            _view = IntPtr.Zero;
        }
    }

    protected override void OnRenderSizeChanged(SizeChangedInfo info)
    {
        base.OnRenderSizeChanged(info);

        // Signal resize to render thread — it will apply it next frame.
        _pendingWidth  = Math.Max((int)info.NewSize.Width,  1);
        _pendingHeight = Math.Max((int)info.NewSize.Height, 1);
    }

    // ------------------------------------------------------------------ //
    //  Render loop (background thread)
    // ------------------------------------------------------------------ //
    private void RenderLoop()
    {
        while (_running)
        {
            // Apply pending resize if any.
            int pw = _pendingWidth;
            int ph = _pendingHeight;
            if (pw > 0 && ph > 0)
            {
                _pendingWidth  = 0;
                _pendingHeight = 0;
                Forge11Native.forge11_view_resize(_view, pw, ph);
            }

            // Tick renderer.
            if (_view != IntPtr.Zero)
                Forge11Native.forge11_view_tick(_view, 0.08f, 0.08f, 0.12f);

            // ~60fps cap — sleep between frames so we don't spin at 100% CPU.
            Thread.Sleep(16);
        }
    }

    // ------------------------------------------------------------------ //
    //  Win32 message filter (required by HwndHost)
    // ------------------------------------------------------------------ //
    protected override IntPtr WndProc(IntPtr hwnd, int msg,
                                       IntPtr wParam, IntPtr lParam,
                                       ref bool handled)
    {
        handled = false;
        return IntPtr.Zero;
    }
}