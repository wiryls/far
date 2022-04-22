using CommunityToolkit.WinUI.UI.Controls;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Documents;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Input;
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

    public static class DropFilesBehaviorExtension
    {
        public static readonly DependencyProperty CommandProperty = DependencyProperty.RegisterAttached
        (
            "Command",
            typeof(ICommand),
            typeof(DropFilesBehaviorExtension),
            new PropertyMetadata(default(ICommand), OnCommandChanged)
        );

        public static ICommand GetCommand(DependencyObject element)
        {
            return element.GetValue(CommandProperty) as ICommand;
        }

        public static void SetCommand(DependencyObject element, ICommand value)
        {
            element.SetValue(CommandProperty, value);
        }

        private static void OnCommandChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (d is not FrameworkElement target)
                throw new InvalidOperationException("target should be FrameworkElement");

            if (e.OldValue == e.NewValue)
                return;

            if (e.OldValue is ICommand prev)
            {
                target.AllowDrop = false;
                target.Drop -= prev.OnDrop;
                target.DragEnter -= prev.OnDragEnter;
            }

            if (e.NewValue is ICommand next)
            {
                target.Drop += next.OnDrop;
                target.DragEnter += next.OnDragEnter;
                target.AllowDrop = true;
            }
        }

        private static void OnDragEnter(this ICommand command, object sender, DragEventArgs e)
        {
            if (e.DataView.Contains(StandardDataFormats.StorageItems) &&
                e.AllowedOperations.HasFlag(DataPackageOperation.Link) &&
                command.CanExecute(sender))
            {
                e.AcceptedOperation = DataPackageOperation.Link;
                e.DragUIOverride.IsCaptionVisible = false;
                e.Handled = true;
            }

            // References:
            // https://github.com/eleanorleffler/WinUI3_Problems_DragDropFile/blob/a6cbb1c74fdbd219d4f9d1fbed38623bb6ebcb78/DragDropFileWinUI/DragDropFileWinUI/MainPage.xaml.cs
            // https://stackoverflow.com/a/1863819
        }

        private static async void OnDrop(this ICommand command, object sender, DragEventArgs e)
        {
            if (sender is not FrameworkElement target)
                throw new InvalidOperationException($"target should be {nameof(FrameworkElement)}");

            if (e.DataView.Contains(StandardDataFormats.StorageItems) &&
                command.CanExecute(sender))
            {
                var items = await e.DataView.GetStorageItemsAsync();
                command.Execute(items.Select(i => i.Path));
            }
        }

        // Reference: Handle drag-and-drop without violating MVVM principals:
        // https://stackoverflow.com/a/65266427
        // https://www.damirscorner.com/blog/posts/20130624-BindingEventsToViewModelMethodsInWindowsStoreApps.html
    }

    public static class DataGridBehaviorExtension
    {
        public static readonly DependencyProperty SelectCommandProperty = DependencyProperty.RegisterAttached
        (
            "SelectCommand",
            typeof(ICommand),
            typeof(DataGridBehaviorExtension),
            new PropertyMetadata(default(ICommand), OnSelectCommandChanged)
        );

        public static ICommand GetSelectCommand(DependencyObject element)
        {
            return element.GetValue(SelectCommandProperty) as ICommand;
        }

        public static void SetSelectCommand(DependencyObject element, ICommand value)
        {
            element.SetValue(SelectCommandProperty, value);
        }

        private static void OnSelectCommandChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (d is not DataGrid target)
                throw new InvalidOperationException($"target should be {nameof(DataGrid)}");

            if (e.OldValue == e.NewValue)
                return;

            if (e.OldValue is ICommand prev)
                target.SelectionChanged -= prev.OnSelectionChanged;

            if (e.NewValue is ICommand next)
                target.SelectionChanged += next.OnSelectionChanged;
        }

        private static void OnSelectionChanged(this ICommand command, object sender, SelectionChangedEventArgs e)
        {
            command.Execute((e.AddedItems.Cast<Item>(), e.RemovedItems.Cast<Item>()));
        }
    }
}
