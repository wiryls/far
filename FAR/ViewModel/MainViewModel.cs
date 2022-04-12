using Fx.Diff;
using Fx.List;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
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

    internal struct Items
    {
        private IDiffer                    differ;
        private readonly Tree              source;
        private List<Item>                 sorted;
        private List<Item>                 wanted;
        private ObservableCollection<Item> viewed;

        public Items()
        {
            differ = DifferCreator.Create();
            source = new ();
            sorted = new ();
            wanted = new ();
            viewed = new ();
        }

        public bool Add(string path)
        {
            var item = source.Add(path);
            if (item is null)
                return false;

            var view = differ.Match(item.Source);
            item.View = view;
            sorted.Add(item);

            if (item.View.Matched)
                wanted.Add(item);

            if (item.View.Changed)
                viewed.Add(item);

            return true;
        }

        public bool Remove(int index)
        {
            if (viewed.Count <= index || index < 0)
                return false;

            var item = viewed[index];
            item.Stat = Status.Gone;

            viewed.RemoveAt(index);
            source.Remove(item);
            return true;
        }

        public void Clear()
        {
            source.Clear();
            sorted.Clear();
            wanted.Clear();
            viewed.Clear();
        }

        public ObservableCollection<Item> Rename()
        {
            if (differ.Empty is false)
                return viewed;

            foreach (var item in viewed)
            {
                var info = new FileInfo(Path.Combine(item.Path, item.Source));
                if (info.Exists is false)
                {
                    item.Stat = Status.Lost;
                    continue;
                }

                try
                {
                    // TODO: test it
                    // info.MoveTo(Path.Combine(item.Path, item.Source));
                    item.Stat = Status.Done;
                }
                catch (Exception)
                {
                    item.Stat = Status.Fail;
                    continue;
                }

                if (source.Rename(item) is false)
                {
                    item.Stat = Status.Fail;
                }
            }

            return viewed;
        }

        public void Differ(IDiffer target)
        {
            var listed = Equals(differ.GetType(), target.GetType()) && differ.Pattern == target.Pattern;
            var list = listed ? wanted : sorted;

            differ = target;

            foreach (var item in list.Where(x => x.Stat is not Status.Gone))
            {
                item.View = differ.Match(item.Source);
                // TODO:
            }

            // TODO:
        }

        public void Sort(/* by */)
        {

        }
    }
}
