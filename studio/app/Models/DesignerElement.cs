using System.Windows.Media;

namespace Forge.Studio.App.Models;

public class DesignerElement
{
    public required F11TreeNode Node { get; init; }
    public double X      { get; init; }
    public double Y      { get; init; }
    public double Width  { get; init; }
    public double Height { get; init; }
    public required Brush Fill { get; init; }
    public string Label { get; init; } = string.Empty;
}