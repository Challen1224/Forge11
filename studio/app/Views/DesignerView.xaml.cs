using System.Windows.Controls;
using Forge.Studio.App.ViewModels;

namespace Forge.Studio.App.Views;

public partial class DesignerView : UserControl
{
    public DesignerView()
    {
        InitializeComponent();
    }

    public DesignerView(DesignerViewModel viewModel) : this()
    {
        DataContext = viewModel;
    }
}