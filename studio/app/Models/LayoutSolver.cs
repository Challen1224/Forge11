using System.Globalization;
using System.Runtime.CompilerServices;

namespace Forge.Studio.App.Models;

public class LayoutSolver
{
    public const double DefaultWidgetWidth  = 120;
    public const double DefaultWidgetHeight = 32;

    private class LayoutRectBox { public LayoutRect Rect; }

    private readonly ConditionalWeakTable<F11TreeNode, LayoutRectBox> _rects = new();

    public void Solve(F11TreeNode root, LayoutRect bounds)
    {
        SolveNode(root, bounds);
    }

    public LayoutRect RectFor(F11TreeNode node)
    {
        return _rects.TryGetValue(node, out var box) ? box.Rect : default;
    }

    private void SolveNode(F11TreeNode node, LayoutRect bounds)
    {
        _rects.AddOrUpdate(node, new LayoutRectBox { Rect = bounds });

        if (node.Tag != "Panel" || node.Children.Count == 0)
        {
            foreach (var child in node.Children)
                SolveNode(child, bounds);
            return;
        }

        var orientation = node.Attributes.GetValueOrDefault("Orientation", "Vertical");

        if (orientation == "Vertical")
        {
            double fixedTotal = 0;
            int flexCount = 0;

            foreach (var child in node.Children)
            {
                double h = ParseDouble(child, "Height");
                if (h > 0) fixedTotal += h;
                else flexCount++;
            }

            double flexHeight = flexCount > 0
                ? Math.Max((bounds.Height - fixedTotal) / flexCount, 0)
                : DefaultWidgetHeight;

            double cursorY = bounds.Y;
            foreach (var child in node.Children)
            {
                double h = ParseDouble(child, "Height");
                if (h <= 0) h = flexHeight;
                double w = ParseDouble(child, "Width");
                if (w <= 0) w = bounds.Width;

                SolveNode(child, new LayoutRect { X = bounds.X, Y = cursorY, Width = w, Height = h });
                cursorY += h;
            }
        }
        else
        {
            double fixedTotal = 0;
            int flexCount = 0;

            foreach (var child in node.Children)
            {
                double w = ParseDouble(child, "Width");
                if (w > 0) fixedTotal += w;
                else flexCount++;
            }

            double flexWidth = flexCount > 0
                ? Math.Max((bounds.Width - fixedTotal) / flexCount, 0)
                : DefaultWidgetWidth;

            double cursorX = bounds.X;
            foreach (var child in node.Children)
            {
                double w = ParseDouble(child, "Width");
                if (w <= 0) w = flexWidth;
                double h = ParseDouble(child, "Height");
                if (h <= 0) h = bounds.Height;

                SolveNode(child, new LayoutRect { X = cursorX, Y = bounds.Y, Width = w, Height = h });
                cursorX += w;
            }
        }
    }

    private static double ParseDouble(F11TreeNode node, string name)
    {
        if (node.Attributes.TryGetValue(name, out var value) &&
            double.TryParse(value, NumberStyles.Any, CultureInfo.InvariantCulture, out var result))
            return result;
        return 0;
    }
}