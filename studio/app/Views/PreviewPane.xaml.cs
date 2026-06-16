using System.Windows;
using System.Windows.Controls;

namespace Forge.Studio.App.Views;

public partial class PreviewPane : UserControl
{
    private ForgeHwndHost? _host;
    private bool _hostCreated = false;

    public PreviewPane()
    {
        InitializeComponent();
        Loaded += OnLoaded;
    }

    private void OnLoaded(object sender, RoutedEventArgs e)
    {
        CreateHost();
    }

    private void CreateHost()
    {
        if (_hostCreated) return;
        _hostCreated = true;

        int w = Math.Max((int)HostBorder.ActualWidth,  1);
        int h = Math.Max((int)HostBorder.ActualHeight, 1);

        try
        {
            _host = new ForgeHwndHost(w, h);
            HostBorder.Child = _host;
        }
        catch (Exception ex)
        {
            HostBorder.Child = new TextBlock
            {
                Text       = $"Preview unavailable: {ex.Message}",
                Foreground = System.Windows.Media.Brushes.OrangeRed,
                Margin     = new Thickness(8),
                TextWrapping = TextWrapping.Wrap
            };
        }
    }

    private void HostBorder_SizeChanged(object sender, SizeChangedEventArgs e)
    {
        // Resize is handled by ForgeHwndHost.OnRenderSizeChanged automatically.
    }
}