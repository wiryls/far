using Fx.Diff;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Text.RegularExpressions;

namespace Far.ViewModel
{
    using DelegateCommand = DelegateCommand<object, object>;
    
    public class MainViewModel : ViewModelBase
    {
        private bool enableRecursiveImport;
        private bool enableIgnoreCase;
        private bool enableRegex;
        private string pattern;
        private string template;

        private (string, bool) warning;
        private readonly Items items;
        private int count;

        public MainViewModel()
        {
            enableRecursiveImport = false;
            enableIgnoreCase = false;
            enableRegex = true;
            pattern = string.Empty;
            template = string.Empty;

            warning = (string.Empty, true);
            items = new();
            count = 0;

            AddItemsCommand = new (AddItem);
            SelectCommand = new (x => UpdateSelection(x.Item1 , x.Item2));
            RenameCommand = new (_ => Rename());
            ClearSelectedCommand = new (_ => ClearSelected(), _ => count is not 0);
            ClearAllCommand = new (_ => ClearAll(), _ => items.IsEmpty is false);
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
                items.Differ(differ);
        }

        private void Rename()
        {
            Debug.WriteLine("Todo");
        }

        private void ClearAll()
        {
            items.Clear();
            ClearAllCommand.RaiseCanExecuteChanged();
        }

        private void ClearSelected()
        {
            items.RemoveSelected();
            ClearSelectedCommand.RaiseCanExecuteChanged();
            ClearAllCommand.RaiseCanExecuteChanged();
        }

        private void AddItem(IEnumerable<string> list)
        {
            var empty = items.IsEmpty;
            foreach (var item in list)
                items.Add(item);

            if (empty)
                ClearAllCommand.RaiseCanExecuteChanged();
        }

        private void UpdateSelection(IEnumerable<Item> added, IEnumerable<Item> removed)
        {
            var number = count;
            foreach (var item in removed)
            {
                count--;
                item.Selected = false;
            }
            foreach (var item in added)
            {
                count++;
                item.Selected = true;
            }
            if (number is 0 != count is 0)
                ClearSelectedCommand.RaiseCanExecuteChanged();
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

        public ObservableCollection<Item> Items => items.View;

        public DelegateCommand<IEnumerable<string>, object> AddItemsCommand { get; private set; }

        public DelegateCommand<ValueTuple<IEnumerable<Item>, IEnumerable<Item>>, object> SelectCommand { get; private set; }

        public DelegateCommand RenameCommand { get; private set; }

        public DelegateCommand ClearSelectedCommand { get; private set; }

        public DelegateCommand ClearAllCommand { get; private set; }

    }
}
