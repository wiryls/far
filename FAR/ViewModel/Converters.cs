using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Data;
using Microsoft.UI.Xaml.Documents;
using Microsoft.UI.Xaml.Media;
using System;
using System.Linq;
using Windows.UI;
using Windows.UI.Text;
using Fx.Diff;
using Fx.List;
using Action = Fx.Diff.Action;

namespace Far.ViewModel
{
    public class StatusConverter : IValueConverter
    {
        // We may use a regexp:
        // - pattern : ([^"]{65,65})
        // - tempalte: $1"+\n"
        //
        // to split the following strings.

        // circle-large-outline.svg
        private const string ToDo = "M9.588 2.215A5.808 5.808 0 0 0 8 2c-.554 " +
            "0-1.082.073-1.588.215l-.006.002c-.514.141-.99.342-1.432.601A6.156" +
            " 6.156 0 0 0 2.82 4.98l-.002.004A5.967 5.967 0 0 0 2.21 6.41 5.98" +
            "6 5.986 0 0 0 2 8c0 .555.07 1.085.21 1.591a6.05 6.05 0 0 0 1.548 " +
            "2.651c.37.365.774.677 1.216.94a6.1 6.1 0 0 0 1.435.609A6.02 6.02 " +
            "0 0 0 8 14c.555 0 1.085-.07 1.591-.21.515-.145.99-.348 1.426-.607" +
            "l.004-.002a6.16 6.16 0 0 0 2.161-2.155 5.85 5.85 0 0 0 .6-1.432l." +
            "003-.006A5.807 5.807 0 0 0 14 8c0-.554-.072-1.082-.215-1.588l-.00" +
            "2-.006a5.772 5.772 0 0 0-.6-1.423l-.002-.004a5.9 5.9 0 0 0-.942-1" +
            ".21l-.008-.008a5.902 5.902 0 0 0-1.21-.942l-.004-.002a5.772 5.772" +
            " 0 0 0-1.423-.6l-.006-.002zm4.455 9.32a7.157 7.157 0 0 1-2.516 2." +
            "508 6.966 6.966 0 0 1-1.668.71A6.984 6.984 0 0 1 8 15a6.984 6.984" +
            " 0 0 1-1.86-.246 7.098 7.098 0 0 1-1.674-.711 7.3 7.3 0 0 1-1.415" +
            "-1.094 7.295 7.295 0 0 1-1.094-1.415 7.098 7.098 0 0 1-.71-1.675A" +
            "6.985 6.985 0 0 1 1 8c0-.643.082-1.262.246-1.86a6.968 6.968 0 0 1" +
            " .711-1.667 7.156 7.156 0 0 1 2.509-2.516 6.895 6.895 0 0 1 1.675" +
            "-.704A6.808 6.808 0 0 1 8 1a6.8 6.8 0 0 1 1.86.253 6.899 6.899 0 " +
            "0 1 3.083 1.805 6.903 6.903 0 0 1 1.804 3.083C14.916 6.738 15 7.3" +
            "57 15 8s-.084 1.262-.253 1.86a6.9 6.9 0 0 1-.704 1.674z";
        // pass.svg
        private const string Done = "M8.6 1c1.6.1 3.1.9 4.2 2 1.3 1.4 2 3.1 2 " +
            "5.1 0 1.6-.6 3.1-1.6 4.4-1 1.2-2.4 2.1-4 2.4-1.6.3-3.2.1-4.6-.7-1" +
            ".4-.8-2.5-2-3.1-3.5C.9 9.2.8 7.5 1.3 6c.5-1.6 1.4-2.9 2.8-3.8C5.4" +
            " 1.3 7 .9 8.6 1zm.5 12.9c1.3-.3 2.5-1 3.4-2.1.8-1.1 1.3-2.4 1.2-3" +
            ".8 0-1.6-.6-3.2-1.7-4.3-1-1-2.2-1.6-3.6-1.7-1.3-.1-2.7.2-3.8 1-1." +
            "1.8-1.9 1.9-2.3 3.3-.4 1.3-.4 2.7.2 4 .6 1.3 1.5 2.3 2.7 3 1.2.7 " +
            "2.6.9 3.9.6zM6.27 10.87h.71l4.56-4.56-.71-.71-4.2 4.21-1.92-1.92L" +
            "4 8.6l2.27 2.27z";
        // error.svg
        private const string Fail = "M8.6 1c1.6.1 3.1.9 4.2 2 1.3 1.4 2 3.1 2 " +
            "5.1 0 1.6-.6 3.1-1.6 4.4-1 1.2-2.4 2.1-4 2.4-1.6.3-3.2.1-4.6-.7-1" +
            ".4-.8-2.5-2-3.1-3.5C.9 9.2.8 7.5 1.3 6c.5-1.6 1.4-2.9 2.8-3.8C5.4" +
            " 1.3 7 .9 8.6 1zm.5 12.9c1.3-.3 2.5-1 3.4-2.1.8-1.1 1.3-2.4 1.2-3" +
            ".8 0-1.6-.6-3.2-1.7-4.3-1-1-2.2-1.6-3.6-1.7-1.3-.1-2.7.2-3.8 1-1." +
            "1.8-1.9 1.9-2.3 3.3-.4 1.3-.4 2.7.2 4 .6 1.3 1.5 2.3 2.7 3 1.2.7 " +
            "2.6.9 3.9.6zM7.9 7.5L10.3 5l.7.7-2.4 2.5 2.4 2.5-.7.7-2.4-2.5-2.4" +
            " 2.5-.7-.7 2.4-2.5-2.4-2.5.7-.7 2.4 2.5z";
        // question.svg
        private const string Lost = "M7.5 1a6.5 6.5 0 1 0 0 13 6.5 6.5 0 0 0 0" +
            "-13zm0 12a5.5 5.5 0 1 1 0-11 5.5 5.5 0 0 1 0 11zm1.55-8.42a1.84 1" +
            ".84 0 0 0-.61-.42A2.25 2.25 0 0 0 7.53 4a2.16 2.16 0 0 0-.88.17c-" +
            ".239.1-.45.254-.62.45a1.89 1.89 0 0 0-.38.62 3 3 0 0 0-.15.72h1.2" +
            "3a.84.84 0 0 1 .506-.741.72.72 0 0 1 .304-.049.86.86 0 0 1 .27 0 " +
            ".64.64 0 0 1 .22.14.6.6 0 0 1 .16.22.73.73 0 0 1 .06.3c0 .173-.03" +
            "7.343-.11.5a2.4 2.4 0 0 1-.27.46l-.35.42c-.12.13-.24.27-.35.41a2." +
            "33 2.33 0 0 0-.27.45 1.18 1.18 0 0 0-.1.5v.66H8v-.49a.94.94 0 0 1" +
            " .11-.42 3.09 3.09 0 0 1 .28-.41l.36-.44a4.29 4.29 0 0 0 .36-.48 " +
            "2.59 2.59 0 0 0 .28-.55 1.91 1.91 0 0 0 .11-.64 2.18 2.18 0 0 0-." +
            "1-.67 1.52 1.52 0 0 0-.35-.55zM6.8 9.83h1.17V11H6.8V9.83z";
        // warning.svg
        private const string Gone = "M8.568 1.031A6.8 6.8 0 0 1 12.76 3.05a7.0" +
            "6 7.06 0 0 1 .46 9.39 6.85 6.85 0 0 1-8.58 1.74 7 7 0 0 1-3.12-3." +
            "5 7.12 7.12 0 0 1-.23-4.71 7 7 0 0 1 2.77-3.79 6.8 6.8 0 0 1 4.50" +
            "8-1.149zM9.04 13.88a5.89 5.89 0 0 0 3.41-2.07 6.07 6.07 0 0 0-.4-" +
            "8.06 5.82 5.82 0 0 0-7.43-.74 6.06 6.06 0 0 0 .5 10.29 5.81 5.81 " +
            "0 0 0 3.92.58zM7.375 6h1.25V5h-1.25v1zm1.25 1v4h-1.25V7h1.25z";

        public object Convert(object value, Type targetType, object parameter, string language) =>
            value is Status status
            ? status switch {
                Status.Todo => ToDo,
                Status.Done => Done,
                Status.Fail => Fail,
                Status.Lost => Lost,
                _ => Gone }
            : Gone;

        public object ConvertBack(object value, Type targetType, object parameter, string language) =>
            DependencyProperty.UnsetValue;

        // Best practice when not implementing IValueConvert.ConvertBack
        // https://stackoverflow.com/a/265544
    }

    public class ChangeConverter : IValueConverter
    {
        private static readonly Brush Insert = new SolidColorBrush(Color.FromArgb(0xFF, 0x07, 0x96, 0x2F));
        private static readonly Brush Delete = new SolidColorBrush(Color.FromArgb(0xFF, 0xC4, 0x0B, 0x54));

        // SolidColorBrush Class
        // https://docs.microsoft.com/en-us/uwp/api/windows.ui.xaml.media.solidcolorbrush

        public object Convert(object value, Type targetType, object parameter, string language) =>
            value is not Patch v
            ? null
            : v.Changed
            ? v.Select(x => x.Type switch {
                Action.Retain => new Run { Text = x.Text },
                Action.Insert => new Run { Text = x.Text, Foreground = Insert, },
                Action.Delete => new Run { Text = x.Text, Foreground = Delete, TextDecorations = TextDecorations.Strikethrough },
                _ => null })
                .Where(x => x != null)
            : Enumerable.Repeat<Run>(new() { Text = v.Target }, 1)
            ?? DependencyProperty.UnsetValue;

        // References:
        // https://docs.microsoft.com/en-us/dotnet/standard/linq/
        // https://docs.microsoft.com/en-us/dotnet/csharp/language-reference/operators/null-coalescing-operator

        public object ConvertBack(object value, Type targetType, object parameter, string language) =>
            DependencyProperty.UnsetValue;
    }

    public class WarningToMessageConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, string language) =>
            value is ValueTuple<string, bool> v
            ? v.Item1
            : DependencyProperty.UnsetValue
            ;

        public object ConvertBack(object value, Type targetType, object parameter, string language) =>
            DependencyProperty.UnsetValue;
    }

    public class WarningToVisibilityConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, string language) =>
            value is ValueTuple<string, bool> v && !string.IsNullOrEmpty(v.Item1)
            ? Visibility.Visible
            : Visibility.Collapsed
            ;

        public object ConvertBack(object value, Type targetType, object parameter, string language) =>
            DependencyProperty.UnsetValue;
    }

    public class WarningToIsCheckedConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, string language) =>
            value is ValueTuple<string, bool> v
            ? v.Item2
            : DependencyProperty.UnsetValue
            ;

        public object ConvertBack(object value, Type targetType, object parameter, string language) =>
            value is bool v
            ? (null as string, v)
            : DependencyProperty.UnsetValue
            ;
    }

    public class WarningToIsOpenConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, string language) =>
            value is ValueTuple<string, bool> v
            ? v.Item2 && !string.IsNullOrEmpty(v.Item1)
            : DependencyProperty.UnsetValue
            ;

        public object ConvertBack(object value, Type targetType, object parameter, string language) =>
            value is bool v
            ? (null as string, v)
            : DependencyProperty.UnsetValue
            ;
    }

    // Converter
    // https://docs.microsoft.com/en-us/windows/winui/api/microsoft.ui.xaml.data.binding.converter
}
