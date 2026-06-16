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
    private TextEditor?  _xmlEditor;
    private ToolboxItem? _draggingItem;

    public MainWindow()
    {
        InitializeComponent();
        DataContext = new MainWindowViewModel();
        KeyDown += MainWindow_KeyDown;
    }

    private MainWindowViewModel? VM => DataContext as MainWindowViewModel;

    // ------------------------------------------------------------------ //
    //  Delete key
    // ------------------------------------------------------------------ //
    private void MainWindow_KeyDown(object sender, KeyEventArgs e)
    {
        if (e.Key != Key.Delete) return;
        if (Keyboard.FocusedElement is TextBox) return;
        if (Keyboard.FocusedElement is ICSharpCode.AvalonEdit.Editing.TextArea) return;

        if (VM?.ActiveDocument is not { } doc) return;
        if (doc.SelectedElement == null) return;

        doc.DeleteSelectedNodeCommand.Execute(null);
        e.Handled = true;
        SyncEditor();
    }

    // ------------------------------------------------------------------ //
    //  AvalonEdit — capture reference on Loaded
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
    //  Tree selection → canvas selection
    // ------------------------------------------------------------------ //
    private void TreeView_SelectedItemChanged(object sender, RoutedPropertyChangedEventArgs<object> e)
    {
        if (VM?.ActiveDocument is not { } doc) return;
        if (e.NewValue is not F11TreeNode node) return;
        var match = doc.Elements.FirstOrDefault(el => el.Node == node);
        doc.SelectElementCommand.Execute(match);
    }

    // ------------------------------------------------------------------ //
    //  Apply XML button
    // ------------------------------------------------------------------ //
    private void ApplyXml_Click(object sender, RoutedEventArgs e)
    {
        if (VM?.ActiveDocument is not { } doc) return;
        if (_xmlEditor is null) return;
        doc.ApplyXmlCommand.Execute(_xmlEditor.Text);
        _xmlEditor.Text = doc.XmlSource;
    }

    // ------------------------------------------------------------------ //
    //  Toolbox drag
    // ------------------------------------------------------------------ //
    private void Toolbox_PreviewMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
    {
        if (sender is not ListBox listBox) return;
        if (listBox.SelectedItem is not ToolboxItem item) return;
        _draggingItem = item;
        DragDrop.DoDragDrop(listBox, item, DragDropEffects.Copy);
        _draggingItem = null;
    }

    // ------------------------------------------------------------------ //
    //  Canvas drop → insert node
    // ------------------------------------------------------------------ //
    private void DesignerCanvas_Drop(object sender, DragEventArgs e)
    {
        if (VM?.ActiveDocument is not { } doc) return;
        if (_draggingItem is null) return;

        DesignerElement? dropTarget = null;
        if (e.OriginalSource is FrameworkElement { DataContext: DesignerElement el })
            dropTarget = el;

        doc.InsertNode(_draggingItem, dropTarget);
        SyncEditor();
    }

    // ------------------------------------------------------------------ //
    //  Sync AvalonEdit from XmlSource
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