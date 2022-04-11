using Fx.Diff;
using Fx.List;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text.RegularExpressions;
using System.Windows.Input;

namespace Far.ViewModel
{
    internal class MainViewModel : ViewModelBase, IFilesDropped
    {
        private bool enableRecursiveImport;
        private bool enableIgnoreCase;
        private bool enableRegex;
        private string pattern;
        private string template;

        private (string, bool) warning;

        public MainViewModel()
        {
            enableRecursiveImport = false;
            enableIgnoreCase = false;
            enableRegex = true;
            pattern = string.Empty;
            template = string.Empty;

            warning = (string.Empty, true);
            //differ = DifferCreator.Create(pattern, template, enableIgnoreCase, enableRegex);

            Items = new ObservableCollection<Item>();

            RenameCommand = new DelegateCommand(Rename);
            ClearSelectedCommand = new DelegateCommand(Todo);
            ClearAllCommand = new DelegateCommand(Todo, o => Items.Count != 0);
        }

        private void UpdateDiffer<T>(ref T property, T value, [CallerMemberName] string name = "")
        {
            if (SetProperty(ref property, value, name)) try
            {
                // TODO:
                // differ = DifferCreator.Create(pattern, template, enableIgnoreCase, enableRegex);
                if (!string.IsNullOrEmpty(Warning.Item1))
                    Warning = (string.Empty, Warning.Item2);
            }
            catch (RegexParseException e)
            {
                Warning = (e.Message, Warning.Item2);
            }

            // TODO:
            //foreach (var item in tree)
            //{
            //    item.View = differ(item.Source);
            //}
        }

        private void Rename(object parameter)
        {

        }

        private void Todo(object parameter)
        {
            // dummy
            Debug.WriteLine("Todo");
        }

        public void OnFilesDropped(List<string> list)
        {
            // TODO:
            //foreach (var item in list.Select(x => tree.Add(x)).Where(x => x is not null))
            //{
            //    item.View = differ(item.Source);
            //    Items.Add(item);
            //}
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

        public ObservableCollection<Item> Items { get; private set; }

        public ICommand RenameCommand { get; private set; }

        public ICommand ClearSelectedCommand { get; private set; }

        public ICommand ClearAllCommand { get; private set; }
    }

    internal class Items
    {
        private IDiffer                    differ;
        private readonly Tree              source;
        private ObservableCollection<Item> sorted;
        private ObservableCollection<Item> viewed;

        public Items()
        {
            differ = DifferCreator.Create();
            source = new Tree();
            sorted = new ObservableCollection<Item>(source);
            viewed = sorted;
        }

        public bool Add(string path)
        {
            var item = source.Add(path);
            if (item is null)
                return false;

            var view = differ.Match(item.Source);
            item.View = view;

            sorted.Add(item);
            if (view.Changed)
                viewed.Add(item);

            return true;
        }

        public bool Remove(int index)
        {


            return false;
        }

        public void Rename()
        {

        }

        public void Rediff(IDiffer differ)
        {

        }

        public void Sort()
        {

        }
    }
}
