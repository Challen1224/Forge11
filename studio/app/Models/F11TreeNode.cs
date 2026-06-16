using System.Collections.ObjectModel;

namespace Forge.Studio.App.Models;

/// <summary>
/// Represents a single element in a parsed .f11 layout tree, for display
/// in the designer's TreeView. Mirrors the structure of F11Node on the
/// C++ side (tag name + attributes + children).
/// </summary>
public class F11TreeNode
{
    public string Tag { get; set; } = string.Empty;

    public Dictionary<string, string> Attributes { get; set; } = new();

    public ObservableCollection<F11TreeNode> Children { get; } = new();

    /// <summary>
    /// Display string shown in the TreeView, e.g., "Button (Name=btnSubmit)".
    /// </summary>
    public string DisplayName
    {
        get
        {
            if (Attributes.Count == 0)
            {
                return Tag;
            }

            var keyAttr = Attributes.ContainsKey("Name") ? "Name"
                        : Attributes.ContainsKey("Title") ? "Title"
                        : Attributes.ContainsKey("Text") ? "Text"
                        : null;

            return keyAttr != null
                ? $"{Tag} ({keyAttr}={Attributes[keyAttr]})"
                : Tag;
        }
    }
}