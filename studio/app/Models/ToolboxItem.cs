namespace Forge.Studio.App.Models;

/// <summary>
/// A draggable widget type in the Toolbox pane.
/// </summary>
public class ToolboxItem
{
    public required string Tag { get; init; }
    public required string DisplayName { get; init; }
    public required string Icon { get; init; }

    /// <summary>
    /// Default attributes applied when this widget is dropped onto the canvas.
    /// </summary>
    public required Dictionary<string, string> DefaultAttributes { get; init; }
}