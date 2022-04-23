using CommunityToolkit.WinUI.UI.Controls;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;
using Microsoft.UI.Xaml.Documents;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Input;
using Windows.ApplicationModel.DataTransfer;
using Windows.Storage;

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

                command.Execute(items
                    .Where(i => i.IsOfType(StorageItemTypes.File) || i.IsOfType(StorageItemTypes.Folder))
                    .Select(i => (i.Path, i.IsOfType(StorageItemTypes.Folder))));
            }
        }

        // Reference: Handle drag-and-drop without violating MVVM principals:
        // https://stackoverflow.com/a/65266427
        // https://www.damirscorner.com/blog/posts/20130624-BindingEventsToViewModelMethodsInWindowsStoreApps.html
    }

    public static class DataGridBehaviorExtension
    {
        #region command select

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

            if (e.OldValue is ICommand o)
                target.SelectionChanged -= o.OnSelectionChanged;

            if (e.NewValue is ICommand n)
                target.SelectionChanged += n.OnSelectionChanged;
        }

        private static void OnSelectionChanged(this ICommand command, object sender, SelectionChangedEventArgs e)
        {
            // (IEnumerable<Item>, IEnumerable<Item>)
            command.Execute((e.AddedItems.Cast<Item>(), e.RemovedItems.Cast<Item>()));
        }

        #endregion

        #region command sort

        public static readonly DependencyProperty SortCommandProperty = DependencyProperty.RegisterAttached
        (
            "SortCommand",
            typeof(ICommand),
            typeof(DataGridBehaviorExtension),
            new PropertyMetadata(default(ICommand), OnSortCommandChanged)
        );

        public static ICommand GetSortCommand(DependencyObject element)
        {
            return element.GetValue(SortCommandProperty) as ICommand;
        }

        public static void SetSortCommand(DependencyObject element, ICommand value)
        {
            element.SetValue(SortCommandProperty, value);
        }

        private static void OnSortCommandChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (d is not DataGrid target)
                throw new InvalidOperationException($"target should be {nameof(DataGrid)}");

            if (e.OldValue == e.NewValue)
                return;

            if (e.OldValue is ICommand o)
                target.Sorting -= o.OnSorted;

            if (e.NewValue is ICommand n)
                target.Sorting += n.OnSorted;
        }

        private static void OnSorted(this ICommand command, object sender, DataGridColumnEventArgs e)
        {
            if (sender is not DataGrid target)
                throw new InvalidOperationException($"target should be {nameof(DataGrid)}");

            var dir = e.Column.SortDirection; // current
            foreach (var column in target.Columns)
                column.SortDirection = null;

            e.Column.SortDirection // next
                = dir is null || dir is DataGridSortDirection.Descending
                ? DataGridSortDirection.Ascending
                : DataGridSortDirection.Descending
                ;

            // (string, bool)
            command.Execute((e.Column.Tag.ToString(), e.Column.SortDirection is DataGridSortDirection.Ascending));
        }

        #endregion
    }
}
