using System.Collections.ObjectModel;
using System.Globalization;
using System.IO;
using System.Windows.Media;
using CommunityToolkit.Mvvm.ComponentModel;
using Forge.Studio.App.Models;
using Forge.Studio.App.Services;

namespace Forge.Studio.App.ViewModels;

public partial class DesignerViewModel : ObservableObject
{
    [ObservableProperty]
    private string _filePath = string.Empty;

    [ObservableProperty]
    private string _fileName = "Untitled.f11";

    [ObservableProperty]
    private double _canvasWidth = 800;

    [ObservableProperty]
    private double _canvasHeight = 600;

    public ObservableCollection<F11TreeNode> RootNodes { get; } = new();
    public ObservableCollection<DesignerElement> Elements { get; } = new();

    public void LoadFile(string path)
    {
        FilePath = path;
        FileName = Path.GetFileName(path);

        RootNodes.Clear();
        Elements.Clear();

        var root = F11XmlLoader.LoadFromFile(path);
        RootNodes.Add(root);
        BuildElements(root);
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

    private static double ParseAttr(F11TreeNode node, string name, double fallback)
    {
        if (node.Attributes.TryGetValue(name, out var value) &&
            double.TryParse(value, NumberStyles.Any, CultureInfo.InvariantCulture, out var result))
            return result;
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