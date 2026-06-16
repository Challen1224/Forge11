using System.IO;
using System.Windows.Controls;
using System.Xml;
using Forge.Studio.App.Models;

namespace Forge.Studio.App.Services;

/// <summary>
/// Loads .f11 layout files (a constrained XML dialect) into F11TreeNode
/// trees for display in the designer. Uses System.Xml since .f11 is
/// well-formed XML.
/// </summary>
public static class F11XmlLoader
{
    public static F11TreeNode LoadFromFile(string path)
    {
        var doc = new XmlDocument();
        doc.Load(path);

        if (doc.DocumentElement == null)
        {
            throw new InvalidDataException($"'{path}' has no root element.");
        }

        return ConvertElement(doc.DocumentElement);
    }

    private static F11TreeNode ConvertElement(XmlElement element)
    {
        var node = new F11TreeNode { Tag = element.Name };

        foreach (XmlAttribute attr in element.Attributes)
        {
            node.Attributes[attr.Name] = attr.Value;
        }

        foreach (XmlNode child in element.ChildNodes)
        {
            if (child is XmlElement childElement)
            {
                node.Children.Add(ConvertElement(childElement));
            }
        }

        return node;
    }
}