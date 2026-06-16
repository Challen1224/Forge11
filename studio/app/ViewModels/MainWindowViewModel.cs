using System.Windows;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using Forge.Studio.App.Models;
using Forge.Studio.App.ViewModels;
using Forge.Studio.Bridge;

namespace Forge.Studio.App.ViewModels;

public partial class MainWindowViewModel : ObservableObject
{
    [ObservableProperty] private string           _statusText     = "Ready";
    [ObservableProperty] private string           _title          = "Forge Studio";
    [ObservableProperty] private DesignerViewModel _activeDocument = new();

    public ToolboxViewModel Toolbox { get; } = new();

    public MainWindowViewModel()
    {
        try
        {
            var version = Forge11Native.GetVersion();
            StatusText = $"Forge11 Engine v{version} loaded";
        }
        catch (DllNotFoundException)
        {
            StatusText = "Forge11 Engine: forge11_abi.dll not found";
        }
        catch (Exception ex)
        {
            StatusText = $"Forge11 Engine: error - {ex.Message}";
        }
    }

    [RelayCommand]
    private void OpenFile()
    {
        var dialog = new Microsoft.Win32.OpenFileDialog
        {
            Filter = "Forge11 Layout (*.f11)|*.f11|All files (*.*)|*.*",
            Title  = "Open .f11 Layout"
        };

        if (dialog.ShowDialog() != true) return;

        var docVm = new DesignerViewModel();
        docVm.LoadFile(dialog.FileName);
        ActiveDocument = docVm;
        StatusText = $"Opened {docVm.FileName}";

        if (Application.Current.MainWindow is Views.MainWindow win)
            win.SyncEditor();
    }
}