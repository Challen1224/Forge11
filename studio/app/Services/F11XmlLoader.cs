using System.IO;
using System.Xml;
using Forge.Studio.App.Models;

namespace Forge.Studio.App.Services;

public static class F11XmlLoader
{
    public static F11TreeNode LoadFromFile(string path)
    {
        var doc = new XmlDocument();
        doc.Load(path);

        if (doc.DocumentElement == null)
            throw new InvalidDataException($"'{path}' has no root element.");

        return ConvertElement(doc.DocumentElement);
    }

    public static F11TreeNode LoadFromString(string xml)
    {
        var doc = new XmlDocument();
        doc.LoadXml(xml);

        if (doc.DocumentElement == null)
            throw new InvalidDataException("XML has no root element.");

        return ConvertElement(doc.DocumentElement);
    }

    private static F11TreeNode ConvertElement(XmlElement element)
    {
        var node = new F11TreeNode { Tag = element.Name };

        foreach (XmlAttribute attr in element.Attributes)
            node.Attributes[attr.Name] = attr.Value;

        foreach (XmlNode child in element.ChildNodes)
            if (child is XmlElement childElement)
                node.Children.Add(ConvertElement(childElement));

        return node;
    }
}