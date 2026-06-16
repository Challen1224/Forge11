using System.Collections.ObjectModel;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows.Media;
using System.Xml;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using Forge.Studio.App.Models;
using Forge.Studio.App.Services;

namespace Forge.Studio.App.ViewModels;

public partial class DesignerViewModel : ObservableObject
{
    [ObservableProperty] private string           _filePath     = string.Empty;
    [ObservableProperty] private string           _fileName     = "Untitled.f11";
    [ObservableProperty] private double           _canvasWidth  = 800;
    [ObservableProperty] private double           _canvasHeight = 600;
    [ObservableProperty] private DesignerElement? _selectedElement;
    [ObservableProperty] private string           _xmlSource    = string.Empty;
    [ObservableProperty] private string           _xmlError     = string.Empty;
    [ObservableProperty] private string           _statusText   = string.Empty;
    [ObservableProperty] private bool             _isDirty      = false;

    private bool _isLoading = false;

    public string TabTitle => IsDirty ? $"{FileName} *" : FileName;

    public ObservableCollection<F11TreeNode>     RootNodes  { get; } = new();
    public ObservableCollection<DesignerElement> Elements   { get; } = new();
    public ObservableCollection<AttributeItem>   Properties { get; } = new();

    // ------------------------------------------------------------------ //
    //  Load
    // ------------------------------------------------------------------ //
    public void LoadFile(string path)
    {
        _isLoading = true;
        FilePath   = path;
        FileName   = Path.GetFileName(path);
        XmlSource  = File.ReadAllText(path, Encoding.UTF8);
        ReloadFromXml(XmlSource, markDirty: false);
        IsDirty    = false;
        _isLoading = false;
    }

    // ------------------------------------------------------------------ //
    //  Save
    // ------------------------------------------------------------------ //
    [RelayCommand]
    public void Save()
    {
        if (string.IsNullOrEmpty(FilePath))
        {
            SaveAs();
            return;
        }

        File.WriteAllText(FilePath, XmlSource, Encoding.UTF8);
        IsDirty    = false;
        StatusText = $"Saved {FileName}";
    }

    [RelayCommand]
    public void SaveAs()
    {
        var dialog = new Microsoft.Win32.SaveFileDialog
        {
            Filter   = "Forge11 Layout (*.f11)|*.f11|All files (*.*)|*.*",
            Title    = "Save .f11 Layout",
            FileName = FileName
        };

        if (dialog.ShowDialog() != true) return;

        FilePath   = dialog.FileName;
        FileName   = Path.GetFileName(dialog.FileName);
        File.WriteAllText(FilePath, XmlSource, Encoding.UTF8);
        IsDirty    = false;
        StatusText = $"Saved {FileName}";
    }

    // ------------------------------------------------------------------ //
    //  Selection
    // ------------------------------------------------------------------ //
    [RelayCommand]
    public void SelectElement(DesignerElement? element)
    {
        if (SelectedElement != null)
            SelectedElement.IsSelected = false;

        SelectedElement = element;
        Properties.Clear();

        if (element == null) return;

        element.IsSelected = true;

        foreach (var kvp in element.Node.Attributes)
        {
            var item = new AttributeItem { Key = kvp.Key, Value = kvp.Value };
            item.ValueChanged += (_, _) =>
            {
                element.Node.Attributes[item.Key] = item.Value;
                XmlSource = SerializeToXml(RootNodes[0]);
                IsDirty   = true;
            };
            Properties.Add(item);
        }
    }

    // ------------------------------------------------------------------ //
    //  Delete selected node
    // ------------------------------------------------------------------ //
    [RelayCommand]
    public void DeleteSelectedNode()
    {
        if (SelectedElement == null) return;
        if (RootNodes.Count == 0) return;

        var target = SelectedElement.Node;

        if (RootNodes[0] == target) return;

        if (RemoveNode(RootNodes[0], target))
        {
            XmlSource = SerializeToXml(RootNodes[0]);
            ReloadFromXml(XmlSource);
        }
    }

    private static bool RemoveNode(F11TreeNode parent, F11TreeNode target)
    {
        for (int i = 0; i < parent.Children.Count; i++)
        {
            if (parent.Children[i] == target)
            {
                parent.Children.RemoveAt(i);
                return true;
            }
            if (RemoveNode(parent.Children[i], target))
                return true;
        }
        return false;
    }

    // ------------------------------------------------------------------ //
    //  Toolbox drop → insert new node
    // ------------------------------------------------------------------ //
    public void InsertNode(ToolboxItem item, DesignerElement? dropTarget)
    {
        if (RootNodes.Count == 0) return;

        var newNode = new F11TreeNode { Tag = item.Tag };
        foreach (var kvp in item.DefaultAttributes)
            newNode.Attributes[kvp.Key] = kvp.Value;

        F11TreeNode? parent = null;

        if (dropTarget != null)
            parent = FindParentPanel(RootNodes[0], dropTarget.Node);

        parent ??= FindFirstPanel(RootNodes[0]);
        parent ??= RootNodes[0];

        parent.Children.Add(newNode);

        XmlSource = SerializeToXml(RootNodes[0]);
        ReloadFromXml(XmlSource);

        var newElement = Elements.FirstOrDefault(e => e.Node.Tag == item.Tag &&
                                                       e.Node == FindNodeByRef(RootNodes[0], newNode));
        SelectElementCommand.Execute(newElement);

        StatusText = $"Inserted <{item.Tag}>";
    }

    private static F11TreeNode? FindParentPanel(F11TreeNode root, F11TreeNode target)
    {
        foreach (var child in root.Children)
        {
            if (child == target && root.Tag == "Panel") return root;
            if (child == target) return null;
            var found = FindParentPanel(child, target);
            if (found != null) return found;
        }
        return null;
    }

    private static F11TreeNode? FindFirstPanel(F11TreeNode root)
    {
        if (root.Tag == "Panel") return root;
        foreach (var child in root.Children)
        {
            var found = FindFirstPanel(child);
            if (found != null) return found;
        }
        return null;
    }

    private static F11TreeNode? FindNodeByRef(F11TreeNode root, F11TreeNode target)
    {
        if (root == target) return root;
        foreach (var child in root.Children)
        {
            var found = FindNodeByRef(child, target);
            if (found != null) return found;
        }
        return null;
    }

    // ------------------------------------------------------------------ //
    //  Code editor → canvas sync
    // ------------------------------------------------------------------ //
    [RelayCommand]
    public void ApplyXml(string xml)
    {
        XmlSource = xml;
        ReloadFromXml(xml, markDirty: true);
    }

    // ------------------------------------------------------------------ //
    //  Internal helpers
    // ------------------------------------------------------------------ //
    private void ReloadFromXml(string xml, bool markDirty = true)
    {
        XmlError = string.Empty;

        try
        {
            var root = F11XmlLoader.LoadFromString(xml);
            RootNodes.Clear();
            Elements.Clear();
            Properties.Clear();
            SelectedElement = null;

            RootNodes.Add(root);
            BuildElements(root);

            if (markDirty && !_isLoading)
                IsDirty = true;
        }
        catch (Exception ex)
        {
            XmlError = ex.Message;
        }
    }

    private void BuildElements(F11TreeNode root)
    {
        double width  = ParseAttr(root, "Width",  800);
        double height = ParseAttr(root, "Height", 600);
        CanvasWidth   = width;
        CanvasHeight  = height;

        var solver = new LayoutSolver();
        solver.Solve(root, new LayoutRect { X = 0, Y = 0, Width = width, Height = height });

        foreach (var child in root.Children)
            AddElements(child, solver);
    }

    private void AddElements(F11TreeNode node, LayoutSolver solver)
    {
        var rect = solver.RectFor(node);
        Elements.Add(new DesignerElement
        {
            Node   = node,
            X      = rect.X,
            Y      = rect.Y,
            Width  = rect.Width,
            Height = rect.Height,
            Fill   = BrushForTag(node.Tag),
            Label  = GetCanvasLabel(node)
        });
        foreach (var child in node.Children)
            AddElements(child, solver);
    }

    private static string GetCanvasLabel(F11TreeNode node)
    {
        if (node.Attributes.TryGetValue("Text",  out var text)  && !string.IsNullOrEmpty(text))
            return text;
        if (node.Attributes.TryGetValue("Title", out var title) && !string.IsNullOrEmpty(title))
            return title;
        if (node.Attributes.TryGetValue("Name",  out var name)  && !string.IsNullOrEmpty(name))
            return name;
        return node.Tag;
    }

    private static string SerializeToXml(F11TreeNode root)
    {
        var sb       = new StringBuilder();
        var settings = new XmlWriterSettings
        {
            Indent             = true,
            IndentChars        = "    ",
            Encoding           = Encoding.UTF8,
            OmitXmlDeclaration = true
        };

        using var writer = XmlWriter.Create(sb, settings);
        WriteNode(writer, root);
        writer.Flush();
        return sb.ToString();
    }

    private static void WriteNode(XmlWriter writer, F11TreeNode node)
    {
        writer.WriteStartElement(node.Tag);
        foreach (var attr in node.Attributes)
            writer.WriteAttributeString(attr.Key, attr.Value);
        foreach (var child in node.Children)
            WriteNode(writer, child);
        writer.WriteEndElement();
    }

    private static double ParseAttr(F11TreeNode node, string name, double fallback)
    {
        if (node.Attributes.TryGetValue(name, out var v) &&
            double.TryParse(v, NumberStyles.Any, CultureInfo.InvariantCulture, out var r))
            return r;
        return fallback;
    }

    private static Brush BrushForTag(string tag) => tag switch
    {
        "Panel"  => new SolidColorBrush(Color.FromRgb(38,  38,  46)),
        "Label"  => new SolidColorBrush(Color.FromRgb(51,  102, 179)),
        "Button" => new SolidColorBrush(Color.FromRgb(77,  179, 77)),
        _        => new SolidColorBrush(Color.FromRgb(102, 102, 102))
    };

    partial void OnIsDirtyChanged(bool value)    => OnPropertyChanged(nameof(TabTitle));
    partial void OnFileNameChanged(string value)  => OnPropertyChanged(nameof(TabTitle));
}