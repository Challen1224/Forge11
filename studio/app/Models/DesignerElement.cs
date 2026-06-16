using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Windows.Media;

namespace Forge.Studio.App.Models;

public class DesignerElement : INotifyPropertyChanged
{
    public required F11TreeNode Node { get; init; }
    public double X      { get; init; }
    public double Y      { get; init; }
    public double Width  { get; init; }
    public double Height { get; init; }
    public required Brush Fill { get; init; }
    public string Label  { get; init; } = string.Empty;

    private bool _isSelected;
    public bool IsSelected
    {
        get => _isSelected;
        set
        {
            if (_isSelected == value) return;
            _isSelected = value;
            OnPropertyChanged();
            OnPropertyChanged(nameof(SelectionBorder));
        }
    }

    public Brush SelectionBorder => IsSelected
        ? new SolidColorBrush(Color.FromRgb(255, 165, 0))
        : new SolidColorBrush(Color.FromRgb(68,  68,  68));

    public event PropertyChangedEventHandler? PropertyChanged;
    private void OnPropertyChanged([CallerMemberName] string? name = null)
        => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
}