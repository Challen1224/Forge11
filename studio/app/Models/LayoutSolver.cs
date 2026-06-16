using System.Globalization;
using System.Runtime.CompilerServices;

namespace Forge.Studio.App.Models;

public class LayoutSolver
{
    public const double DefaultWidgetWidth  = 120;
    public const double DefaultWidgetHeight = 32;

    private class Box { public LayoutRect Rect; }
    private readonly ConditionalWeakTable<F11TreeNode, Box> _rects = new();

    public void Solve(F11TreeNode root, LayoutRect bounds) => SolveNode(root, bounds);

    public LayoutRect RectFor(F11TreeNode node)
        => _rects.TryGetValue(node, out var box) ? box.Rect : default;

    private void SolveNode(F11TreeNode node, LayoutRect bounds)
    {
        _rects.AddOrUpdate(node, new Box { Rect = bounds });

        if (node.Tag != "Panel" || node.Children.Count == 0)
        {
            foreach (var child in node.Children)
                SolveNode(child, bounds);
            return;
        }

        var orientation = node.Attributes.GetValueOrDefault("Orientation", "Vertical");

        if (orientation == "Vertical")
        {
            double fixedTotal = 0; int flexCount = 0;
            foreach (var child in node.Children)
            {
                double h = Parse(child, "Height");
                if (h > 0) fixedTotal += h; else flexCount++;
            }
            double flexH = flexCount > 0
                ? Math.Max((bounds.Height - fixedTotal) / flexCount, 0)
                : DefaultWidgetHeight;

            double cy = bounds.Y;
            foreach (var child in node.Children)
            {
                double h = Parse(child, "Height"); if (h <= 0) h = flexH;
                double w = Parse(child, "Width");  if (w <= 0) w = bounds.Width;
                SolveNode(child, new LayoutRect { X = bounds.X, Y = cy, Width = w, Height = h });
                cy += h;
            }
        }
        else
        {
            double fixedTotal = 0; int flexCount = 0;
            foreach (var child in node.Children)
            {
                double w = Parse(child, "Width");
                if (w > 0) fixedTotal += w; else flexCount++;
            }
            double flexW = flexCount > 0
                ? Math.Max((bounds.Width - fixedTotal) / flexCount, 0)
                : DefaultWidgetWidth;

            double cx = bounds.X;
            foreach (var child in node.Children)
            {
                double w = Parse(child, "Width");  if (w <= 0) w = flexW;
                double h = Parse(child, "Height"); if (h <= 0) h = bounds.Height;
                SolveNode(child, new LayoutRect { X = cx, Y = bounds.Y, Width = w, Height = h });
                cx += w;
            }
        }
    }

    private static double Parse(F11TreeNode node, string name)
    {
        if (node.Attributes.TryGetValue(name, out var v) &&
            double.TryParse(v, NumberStyles.Any, CultureInfo.InvariantCulture, out var r))
            return r;
        return 0;
    }
}