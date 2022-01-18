using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Documents;
using System.Collections;
using System.Collections.Generic;

namespace FAR
{
    internal static class TextBlockExtensions
    {
        public static IEnumerable<Inline> GetBindableInlines(DependencyObject obj)
        {
            return (IEnumerable<Inline>)obj.GetValue(BindableInlinesProperty);
        }

        public static void SetBindableInlines(DependencyObject obj, IEnumerable<Inline> value)
        {
            obj.SetValue(BindableInlinesProperty, value);
        }

        public static readonly DependencyProperty BindableInlinesProperty = DependencyProperty.RegisterAttached
        (
            "BindableInlines",
            typeof(IEnumerable<Inline>),
            typeof(TextBlockExtensions),
            new PropertyMetadata(null, OnBindableInlinesChanged)
        );

        private static void OnBindableInlinesChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (d is TextBlock Target)
            {
                Target.Inlines.Clear();
                if (e.NewValue is IEnumerable)
                    foreach (var item in e.NewValue as IEnumerable)
                        if (item is Inline)
                            Target.Inlines.Add(item as Inline);
            }
        }
    }
}
