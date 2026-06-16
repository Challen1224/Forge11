using System.Collections.ObjectModel;
using System.Globalization;
using System.IO;
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
    [ObservableProperty] private string _filePath    = string.Empty;
    [ObservableProperty] private string _fileName    = "Untitled.f11";
    [ObservableProperty] private double _canvasWidth  = 800;
    [ObservableProperty] private double _canvasHeight = 600;
    [ObservableProperty] private DesignerElement? _selectedElement;
    [ObservableProperty] private string _xmlSource   = string.Empty;
    [ObservableProperty] private string _xmlError    = string.Empty;

    public ObservableCollection<F11TreeNode>     RootNodes  { get; } = new();
    public ObservableCollection<DesignerElement> Elements   { get; } = new();
    public ObservableCollection<AttributeItem>   Properties { get; } = new();

    // ------------------------------------------------------------------ //
    //  Load
    // ------------------------------------------------------------------ //
    public void LoadFile(string path)
    {
        FilePath = path;
        FileName = Path.GetFileName(path);
        XmlSource = File.ReadAllText(path, Encoding.UTF8);
        ReloadFromXml(XmlSource);
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
            };
            Properties.Add(item);
        }
    }

    // ------------------------------------------------------------------ //
    //  Code editor → canvas sync
    // ------------------------------------------------------------------ //
    [RelayCommand]
    public void ApplyXml(string xml)
    {
        ReloadFromXml(xml);
    }

    // ------------------------------------------------------------------ //
    //  Internal helpers
    // ------------------------------------------------------------------ //
    private void ReloadFromXml(string xml)
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
        CanvasWidth  = width;
        CanvasHeight = height;

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
            Label  = node.DisplayName
        });
        foreach (var child in node.Children)
            AddElements(child, solver);
    }

    // ------------------------------------------------------------------ //
    //  XML serializer (F11TreeNode → pretty-printed XML string)
    // ------------------------------------------------------------------ //
    private static string SerializeToXml(F11TreeNode root)
    {
        var sb = new StringBuilder();
        var settings = new XmlWriterSettings
        {
            Indent = true,
            IndentChars = "    ",
            Encoding = Encoding.UTF8,
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

    // ------------------------------------------------------------------ //
    //  Misc helpers
    // ------------------------------------------------------------------ //
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
}