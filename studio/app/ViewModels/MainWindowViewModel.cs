using System.Collections.ObjectModel;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using System.Linq;
using Forge.Studio.Bridge;

namespace Forge.Studio.App.ViewModels;

public partial class MainWindowViewModel : ObservableObject
{
    [ObservableProperty]
    private string _statusText = "Ready";

    [ObservableProperty]
    private string _title = "Forge Studio";

    // ⭐ FIXED: nullable backing field so CS8618 disappears
    [ObservableProperty]
    private DesignerViewModel? _activeDocument;

    public ObservableCollection<DesignerViewModel> OpenDocuments { get; } = new();

    public MainWindowViewModel()
    {
        try
        {
            var version = Forge11Native.GetVersion();
            StatusText = $"Forge11 Engine v{version} loaded";
        }
        catch
        {
            StatusText = "Forge11 Engine: forge11_abi.dll not found";
        }
    }

    [RelayCommand]
    private void OpenFile()
    {
        var dialog = new Microsoft.Win32.OpenFileDialog
        {
            Filter = "Forge11 Layout (*.f11)|*.f11|All files (*.*)|*.*",
            Title = "Open .f11 Layout"
        };

        if (dialog.ShowDialog() == true)
        {
            var docVm = new DesignerViewModel();
            docVm.LoadFile(dialog.FileName);

            OpenDocuments.Add(docVm);

            // ⭐ CRITICAL: this makes the designer show the file
            ActiveDocument = docVm;

            StatusText =
                $"Opened {docVm.FileName} - {docVm.RootNodes.Count} root node(s), tag={docVm.RootNodes.FirstOrDefault()?.Tag}";
        }
    }
}
