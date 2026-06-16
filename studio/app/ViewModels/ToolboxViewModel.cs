using System.Collections.ObjectModel;
using Forge.Studio.App.Models;

namespace Forge.Studio.App.ViewModels;

public class ToolboxViewModel
{
    public ObservableCollection<ToolboxItem> Items { get; } = new()
    {
        new ToolboxItem
        {
            Tag = "Panel",
            DisplayName = "Panel",
            Icon = "▣",
            DefaultAttributes = new()
            {
                { "Name", "panel1" },
                { "Orientation", "Vertical" },
                { "Width", "200" },
                { "Height", "200" }
            }
        },
        new ToolboxItem
        {
            Tag = "Label",
            DisplayName = "Label",
            Icon = "𝐀",
            DefaultAttributes = new()
            {
                { "Name", "label1" },
                { "Text", "Label" },
                { "Width", "120" },
                { "Height", "32" }
            }
        },
        new ToolboxItem
        {
            Tag = "Button",
            DisplayName = "Button",
            Icon = "⬜",
            DefaultAttributes = new()
            {
                { "Name", "button1" },
                { "Text", "Button" },
                { "Width", "120" },
                { "Height", "32" }
            }
        },
        new ToolboxItem
        {
            Tag = "TextBox",
            DisplayName = "TextBox",
            Icon = "✏",
            DefaultAttributes = new()
            {
                { "Name", "textbox1" },
                { "Placeholder", "Enter text..." },
                { "Width", "200" },
                { "Height", "32" }
            }
        },
        new ToolboxItem
        {
            Tag = "Image",
            DisplayName = "Image",
            Icon = "🖼",
            DefaultAttributes = new()
            {
                { "Name", "image1" },
                { "Source", "" },
                { "Width", "100" },
                { "Height", "100" }
            }
        }
    };
}