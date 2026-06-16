using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace Forge.Studio.App.Models;

/// <summary>
/// Editable key/value pair for the Properties pane.
/// Wraps a KeyValuePair from F11TreeNode.Attributes with a settable Value.
/// </summary>
public class AttributeItem : INotifyPropertyChanged
{
    public string Key { get; init; } = string.Empty;

    private string _value = string.Empty;
    public string Value
    {
        get => _value;
        set
        {
            if (_value == value) return;
            _value = value;
            OnPropertyChanged();
            ValueChanged?.Invoke(this, EventArgs.Empty);
        }
    }

    public event EventHandler? ValueChanged;
    public event PropertyChangedEventHandler? PropertyChanged;

    private void OnPropertyChanged([CallerMemberName] string? name = null)
        => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
}