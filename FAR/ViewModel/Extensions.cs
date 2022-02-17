using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Documents;
using System;
using System.Collections.Generic;
using System.Linq;
using Windows.ApplicationModel.DataTransfer;

namespace Far.ViewModel
{
    public static class TextBlockExtension
    {
        public static readonly DependencyProperty BindableInlinesProperty = DependencyProperty.RegisterAttached
        (
            "BindableInlines",
            typeof(IEnumerable<Inline>),
            typeof(TextBlockExtension),
            new PropertyMetadata(null, OnBindableInlinesChanged)
        );

        public static IEnumerable<Inline> GetBindableInlines(DependencyObject element)
        {
            return element.GetValue(BindableInlinesProperty) as IEnumerable<Inline>;
        }

        public static void SetBindableInlines(DependencyObject element, IEnumerable<Inline> value)
        {
            element.SetValue(BindableInlinesProperty, value);
        }

        private static void OnBindableInlinesChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (d is not TextBlock target)
                throw new InvalidOperationException("target should be TextBlock");

            if (e.NewValue is not IEnumerable<Inline> items)
                throw new InvalidOperationException("value should be IEnumerable<Inline>");

            target.Inlines.Clear();
            foreach (var item in items)
                target.Inlines.Add(item);
        }
    }

    public interface IFilesDropped
    {
        void OnFilesDropped(List<string> files);
    }

    public class DropFilesBehaviorExtension
    {
        public static readonly DependencyProperty IsEnableProperty = DependencyProperty.RegisterAttached
        (
            "IsEnable",
            typeof(bool),
            typeof(DropFilesBehaviorExtension),
            new PropertyMetadata(default(bool), OnIsEnableChanged)
        );
        public static bool GetIsEnable(DependencyObject element)
        {
            return (bool)element.GetValue(IsEnableProperty);
        }

        public static void SetIsEnable(DependencyObject element, bool value)
        {
            element.SetValue(IsEnableProperty, value);
        }

        private static void OnIsEnableChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (d is not FrameworkElement target)
                throw new InvalidOperationException("target should be FrameworkElement");

            if (e.NewValue is not bool allow)
                throw new InvalidOperationException("value should be bool");

            if (target.AllowDrop = allow)
            {
                target.Drop += OnDrop;
                target.DragEnter += OnDragEnter;
            }
            else
            {
                target.Drop -= OnDrop;
                target.DragEnter -= OnDragEnter;
            }
        }

        private static void OnDragEnter(object sender, DragEventArgs e)
        {
            if (e.DataView.Contains(StandardDataFormats.StorageItems) &&
                e.AllowedOperations.HasFlag(DataPackageOperation.Link))
            {
                e.AcceptedOperation = DataPackageOperation.Link;
                e.DragUIOverride.IsCaptionVisible = false;
                e.Handled = true;
            }

            // References:
            // https://github.com/eleanorleffler/WinUI3_Problems_DragDropFile/blob/a6cbb1c74fdbd219d4f9d1fbed38623bb6ebcb78/DragDropFileWinUI/DragDropFileWinUI/MainPage.xaml.cs
            // https://stackoverflow.com/a/1863819
        }

        private static async void OnDrop(object sender, DragEventArgs e)
        {
            if (sender is not FrameworkElement target)
                throw new InvalidOperationException("sender should be FrameworkElement");

            if (target.DataContext is not IFilesDropped handler)
                throw new InvalidOperationException("DataContext (ViewModel) should implement IFilesDropped");

            if (e.DataView.Contains(StandardDataFormats.StorageItems))
            {
                var items = await e.DataView.GetStorageItemsAsync();
                var paths = items.Select(i => i.Path).ToList();
                if (paths.Count != 0)
                    handler.OnFilesDropped(paths);
            }
        }

        // Reference: Handle drag-and-drop without violating MVVM principals:
        // https://stackoverflow.com/a/65266427
    }
}
