using Fx.Diff;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text.RegularExpressions;

namespace Far.ViewModel
{
    using DelegateCommand = DelegateCommand<object, object>;
    
    public class MainViewModel : ViewModelBase
    {
        // options
        private bool enableRecursiveImport;
        private bool enableIgnoreCase;
        private bool enableRegex;

        // items
        private string pattern;
        private string template;
        private readonly ItemList items;

        // tips
        private (string, bool) warning;
        private int selection;

        public MainViewModel()
        {
            enableRecursiveImport = false;
            enableIgnoreCase = false;
            enableRegex = true;

            pattern = string.Empty;
            template = string.Empty;
            items = new();

            warning = (string.Empty, true);
            selection = 0;

            AddCommand = new (AddItems);
            SelectCommand = new (x => SelectItems(x.Item1 , x.Item2));
            SortCommand = new (x => SortItems(x.Item1, x.Item2));
            ClearSelectedCommand = new (_ => ClearSelectedItems(), _ => selection is not 0);
            ClearAllCommand = new (_ => ClearAllItems(), _ => items.IsEmpty is false);
            RenameCommand = new (_ => RenameItems(), _ => items.IsRenamable);
        }

        public bool EnableRecursiveImport
        {
            get => enableRecursiveImport;
            set => SetProperty(ref enableRecursiveImport, value);
        }

        public bool EnableRegex
        {
            get => enableRegex;
            set => UpdateDiffer(ref enableRegex, value);
        }

        public bool EnableCaseSensitive
        {
            get => !enableIgnoreCase;
            set => UpdateDiffer(ref enableIgnoreCase, !value);
        }

        public string Pattern
        {
            get => pattern;
            set => UpdateDiffer(ref pattern, value);
        }

        public string Template
        {
            get => template;
            set => UpdateDiffer(ref template, value);
        }

        public (string, bool) Warning
        {
            get => warning;
            set => SetProperty(ref warning, value.Item1 is null ? (warning.Item1, value.Item2) : value);
        }

        public ObservableCollection<Item> ItemList => items.View;

        public DelegateCommand<IEnumerable<ValueTuple<string, bool>>, object> AddCommand { get; private set; }

        public DelegateCommand<ValueTuple<IEnumerable<Item>, IEnumerable<Item>>, object> SelectCommand { get; private set; }

        public DelegateCommand<ValueTuple<string, bool>, object> SortCommand { get; private set; }

        public DelegateCommand ClearSelectedCommand { get; private set; }

        public DelegateCommand ClearAllCommand { get; private set; }

        public DelegateCommand RenameCommand { get; private set; }

        private void AddItems(IEnumerable<(string Name, bool IsFolder)> list)
        {
            var notEmptyAnyMore = items.IsEmpty;

            if (EnableRecursiveImport)
            {
                foreach (var item in WalkThrough(list))
                    items.Add(item);
            }
            else
            {
                foreach (var item in list.Select(x => x.Name))
                    items.Add(item);
            }

            if (notEmptyAnyMore)
            {
                ClearAllCommand.RaiseCanExecuteChanged();
                RenameCommand.RaiseCanExecuteChanged();
            }

            static IEnumerable<string> WalkThrough(IEnumerable<(string Name, bool IsFolder)> list)
            {
                foreach (var (path, isFolder) in list)
                {
                    yield return path;
                    if (isFolder is false)
                        continue;

                    var walk = Directory.EnumerateFileSystemEntries(path, "*", SearchOption.AllDirectories).GetEnumerator();
                    var next = true;
                    while (next)
                    {
                        try
                        {
                            next = walk.MoveNext();
                        }
                        catch (Exception e)
                        {
                            // TODO: we'd better display some error messages.
                            Debug.Print(e.Message);
                        }
                        if (next)
                        {
                            yield return walk.Current;
                        }
                    }
                }
            }
        }

        private void SelectItems(IEnumerable<Item> added, IEnumerable<Item> removed)
        {
            var number = selection;
            foreach (var item in removed)
            {
                selection--;
                item.Selected = false;
            }
            foreach (var item in added)
            {
                selection++;
                item.Selected = true;
            }
            if (number is 0 != selection is 0)
                ClearSelectedCommand.RaiseCanExecuteChanged();
        }

        private void SortItems(string tag, bool ascending)
        {
            var order = tag switch
            {
                "Stat" => ViewModel.ItemList.OrderBy.Stat,
                "View" => ViewModel.ItemList.OrderBy.View,
                "Path" => ViewModel.ItemList.OrderBy.Path,
                _ => throw new ArgumentException($"unknown tag '{tag}'"),
            };
            items.Sort(order, ascending);
        }

        private void RenameItems()
        {
            if (items.Rename())
                RenameCommand.RaiseCanExecuteChanged();
        }

        private void ClearSelectedItems()
        {
            items.RemoveSelected();
            ClearSelectedCommand.RaiseCanExecuteChanged();
            ClearAllCommand.RaiseCanExecuteChanged();
            RenameCommand.RaiseCanExecuteChanged();
        }

        private void ClearAllItems()
        {
            items.Clear();
            ClearAllCommand.RaiseCanExecuteChanged();
            RenameCommand.RaiseCanExecuteChanged();
        }

        private void UpdateDiffer<T>(ref T property, T value, [CallerMemberName] string name = "")
        {
            var differ = null as IDiffer;
            if (SetProperty(ref property, value, name)) try
            {
                differ = DifferCreator.Create(pattern, template, enableIgnoreCase, enableRegex);
                if (!string.IsNullOrEmpty(Warning.Item1))
                    Warning = (string.Empty, Warning.Item2);
            }
            catch (RegexParseException e)
            {
                Warning = (e.Message, Warning.Item2);
            }

            if (differ is not null)
            {
                items.Differ(differ);
                RenameCommand.RaiseCanExecuteChanged();
            }
        }
    }
}
