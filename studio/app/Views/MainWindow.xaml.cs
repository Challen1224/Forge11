using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using Forge.Studio.App.Models;
using Forge.Studio.App.ViewModels;
using ICSharpCode.AvalonEdit;

namespace Forge.Studio.App.Views;

public partial class MainWindow : Window
{
    private TextEditor? _xmlEditor;

    public MainWindow()
    {
        InitializeComponent();
        DataContext = new MainWindowViewModel();
    }

    private MainWindowViewModel? VM => DataContext as MainWindowViewModel;

    // ------------------------------------------------------------------ //
    //  AvalonEdit loads inside AvalonDock — capture reference here
    // ------------------------------------------------------------------ //
    private void XmlEditor_Loaded(object sender, RoutedEventArgs e)
    {
        if (sender is not TextEditor editor) return;
        _xmlEditor = editor;
        SyncEditor();
    }

    // ------------------------------------------------------------------ //
    //  Canvas click → select element
    // ------------------------------------------------------------------ //
    private void DesignerElement_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
    {
        if (VM?.ActiveDocument is not { } doc) return;
        if (sender is not Border { Tag: DesignerElement element }) return;
        doc.SelectElementCommand.Execute(element);
        e.Handled = true;
    }

    // ------------------------------------------------------------------ //
    //  Tree selection → select matching canvas element
    // ------------------------------------------------------------------ //
    private void TreeView_SelectedItemChanged(object sender, RoutedPropertyChangedEventArgs<object> e)
    {
        if (VM?.ActiveDocument is not { } doc) return;
        if (e.NewValue is not F11TreeNode node) return;
        var match = doc.Elements.FirstOrDefault(el => el.Node == node);
        doc.SelectElementCommand.Execute(match);
    }

    // ------------------------------------------------------------------ //
    //  Apply button — take editor text, reload canvas
    // ------------------------------------------------------------------ //
    private void ApplyXml_Click(object sender, RoutedEventArgs e)
    {
        if (VM?.ActiveDocument is not { } doc) return;
        if (_xmlEditor is null) return;

        doc.ApplyXmlCommand.Execute(_xmlEditor.Text);

        // Sync back in case serializer reformatted.
        _xmlEditor.Text = doc.XmlSource;
    }

    // ------------------------------------------------------------------ //
    //  Sync editor text from view model XmlSource
    // ------------------------------------------------------------------ //
    internal void SyncEditor()
    {
        if (_xmlEditor is null) return;
        if (VM?.ActiveDocument is { } doc)
            _xmlEditor.Text = doc.XmlSource;
    }

    protected override void OnContentRendered(EventArgs e)
    {
        base.OnContentRendered(e);
        SyncEditor();
    }
}