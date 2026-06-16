using System.Windows;
using Forge.Studio.App.ViewModels;

namespace Forge.Studio.App.Views;

public partial class MainWindow : Window
{
    public MainWindow()
    {
        InitializeComponent();

        var viewModel = new MainWindowViewModel();
        DataContext = viewModel;
    }
}