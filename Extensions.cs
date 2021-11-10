using System.Collections;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;

namespace FAR
{
    public static class TextBlockExtensions
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
                Target.Inlines.AddRange(e.NewValue as IEnumerable);
            }
        }
    }
}
