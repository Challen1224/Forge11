using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using Forge.Studio.App.Models;
using Forge.Studio.App.ViewModels;

namespace Forge.Studio.App.Views;

public partial class MainWindow : Window
{
    public MainWindow()
    {
        InitializeComponent();
        DataContext = new MainWindowViewModel();
    }

    private MainWindowViewModel? VM => DataContext as MainWindowViewModel;

    private void DesignerElement_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
    {
        if (VM?.ActiveDocument is not { } doc) return;
        if (sender is not Border { Tag: DesignerElement element }) return;
        doc.SelectElementCommand.Execute(element);
        e.Handled = true;
    }

    private void TreeView_SelectedItemChanged(object sender, RoutedPropertyChangedEventArgs<object> e)
    {
        if (VM?.ActiveDocument is not { } doc) return;
        if (e.NewValue is not F11TreeNode node) return;
        var match = doc.Elements.FirstOrDefault(el => el.Node == node);
        doc.SelectElementCommand.Execute(match);
    }
}