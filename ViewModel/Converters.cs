using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Data;
using Microsoft.UI.Xaml.Documents;
using Microsoft.UI.Xaml.Media;
using System;
using System.Linq;
using Windows.UI;
using Windows.UI.Text;

namespace FAR.ViewModel
{
    internal class StatusConverter : IValueConverter
    {
        private const string ToDo = "TODO";
        private const string Done = "DONE";
        private const string Fail = "FAIL";
        private const string Lost = "LOST";

        public object Convert(object value, Type targetType, object parameter, string language)
        {
            return (Status)value switch
            {
                Status.Todo => ToDo,
                Status.Done => Done,
                Status.Fail => Fail,
                Status.Lost => Lost,
                _ => ToDo,
            };
        }

        public object ConvertBack(object value, Type targetType, object parameter, string language)
        {
            return (value as string) switch
            {
                ToDo => Status.Todo,
                Done => Status.Done,
                Fail => Status.Fail,
                Lost => Status.Lost,
                _ => Status.Todo,
            };
        }
    }

    internal class ChangeConverter : IValueConverter
    {
        private static readonly Brush Insert = new SolidColorBrush(Color.FromArgb(0xFF, 0x00, 0x79, 0x47));
        private static readonly Brush Delete = new SolidColorBrush(Color.FromArgb(0xFF, 0xDC, 0x14, 0x3C));

        // SolidColorBrush Class
        // https://docs.microsoft.com/en-us/uwp/api/windows.ui.xaml.media.solidcolorbrush

        public object Convert(object value, Type targetType, object parameter, string language)
        {
            return (value as Change)
                .Where(x => x != null)
                .Select(x => x.Type switch
                {
                    Operation.Action.Retain => new Run { Text = x.Text },
                    Operation.Action.Insert => new Run { Text = x.Text, Foreground = Insert, },
                    Operation.Action.Delete => new Run { Text = x.Text, Foreground = Delete, TextDecorations = TextDecorations.Strikethrough },
                    _ => null
                })
                .Where(x => x != null)
                ?? DependencyProperty.UnsetValue;

            // References:
            // https://docs.microsoft.com/en-us/dotnet/standard/linq/
            // https://docs.microsoft.com/en-us/dotnet/csharp/language-reference/operators/null-coalescing-operator
        }

        public object ConvertBack(object value, Type targetType, object parameter, string language)
        {
            return DependencyProperty.UnsetValue;

            // Best practice when not implementing IValueConvert.ConvertBack
            // https://stackoverflow.com/a/265544
        }
    }

    // Converter
    // https://docs.microsoft.com/en-us/windows/winui/api/microsoft.ui.xaml.data.binding.converter
}
