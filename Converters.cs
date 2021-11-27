using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Windows;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Media;

namespace FAR
{
    [ValueConversion(typeof(Status), typeof(string))]
    internal class StatusConverter : IValueConverter
    {
        private const string ToDo = "TODO";
        private const string Done = "DONE";
        private const string Fail = "FAIL";
        private const string Lost = "LOST";

        public object Convert(object value, Type target, object parameter, CultureInfo culture)
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

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return value as string switch
            {
                ToDo => Status.Todo,
                Done => Status.Done,
                Fail => Status.Fail,
                Lost => Status.Lost,
                _ => Status.Todo,
            };
        }
    }

    [ValueConversion(typeof(Change), typeof(IEnumerable<Inline>))]
    internal class ChangeConverter : IValueConverter
    {
        private static readonly Brush Insert = new BrushConverter().ConvertFrom("#007947") as SolidColorBrush;
        private static readonly Brush Delete = new BrushConverter().ConvertFrom("#DC143C") as SolidColorBrush;

        // References: Creating SolidColorBrush from hex color value
        // https://stackoverflow.com/a/10710201

        public object Convert(object value, Type target, object parameter, CultureInfo culture)
        {
            return (value as Change)
                .Where(x => x != null)
                .Select(x => x.Type switch
                {
                    Operation.Action.Retain => new Run(x.Text),
                    Operation.Action.Insert => new Run(x.Text) { Foreground = Insert, },
                    Operation.Action.Delete => new Run(x.Text) { Foreground = Delete, TextDecorations = TextDecorations.Strikethrough },
                    _ => null
                })
                .Where(x => x != null)
                ?? DependencyProperty.UnsetValue;

            // References:
            // https://docs.microsoft.com/en-us/dotnet/standard/linq/
            // https://docs.microsoft.com/en-us/dotnet/csharp/language-reference/operators/null-coalescing-operator
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return DependencyProperty.UnsetValue;

            // References: Best practice when not implementing IValueConvert.ConvertBack
            // https://stackoverflow.com/a/265544
        }
    }

    // References: Converter
    // https://docs.microsoft.com/en-us/dotnet/desktop/wpf/data/how-to-convert-bound-data
}
