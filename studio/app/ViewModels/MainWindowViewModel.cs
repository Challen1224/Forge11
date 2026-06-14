using CommunityToolkit.Mvvm.ComponentModel;
using Forge.Studio.Bridge;

namespace Forge.Studio.App.ViewModels;

/// <summary>
/// Root view model for MainWindow. Will own docking layout state,
/// open documents, and the active project in later phases.
/// </summary>
public partial class MainWindowViewModel : ObservableObject
{
    [ObservableProperty]
    private string _statusText = "Ready";

    [ObservableProperty]
    private string _title = "Forge Studio";

    public MainWindowViewModel()
    {
        try
        {
            var version = Forge11Native.GetVersion();
            StatusText = $"Forge11 Engine v{version} loaded";
        }
        catch (System.DllNotFoundException)
        {
            StatusText = "Forge11 Engine: forge11_abi.dll not found";
        }
        catch (System.Exception ex)
        {
            StatusText = $"Forge11 Engine: error - {ex.Message}";
        }
    }
}