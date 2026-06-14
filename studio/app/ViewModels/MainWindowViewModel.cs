using CommunityToolkit.Mvvm.ComponentModel;

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
}