using Fx.Diff;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Text.RegularExpressions;

namespace Far.ViewModel
{
    public class MainViewModel : ViewModelBase, IFilesDropped
    {
        private bool enableRecursiveImport;
        private bool enableIgnoreCase;
        private bool enableRegex;
        private string pattern;
        private string template;

        private (string, bool) warning;
        private readonly Items items;

        public MainViewModel()
        {
            enableRecursiveImport = false;
            enableIgnoreCase = false;
            enableRegex = true;
            pattern = string.Empty;
            template = string.Empty;

            warning = (string.Empty, true);
            items = new();

            RenameCommand = new DelegateCommand(Rename);
            ClearSelectedCommand = new DelegateCommand(Todo);
            ClearAllCommand = new DelegateCommand(ClearAll, o => items.IsEmpty is false);
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

        private void Rename(object parameter)
        {

        }

        private void ClearAll(object parameter)
        {
            items.Clear();
            ClearAllCommand.RaiseCanExecuteChanged();
        }

        private void Todo(object parameter)
        {
            // dummy
            Debug.WriteLine("Todo");
        }

        public void OnFilesDropped(List<string> list)
        {
            list.ForEach(item => items.Add(item));
            ClearAllCommand.RaiseCanExecuteChanged();

            //if (list.Aggregate(false, (value, path) => items.Add(path) || value))
            //{
            //    Items = null;
            //    Items = items.View;
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

        public object Selected { get; set; }

        public ObservableCollection<Item> Items => items.View;

        public DelegateCommand RenameCommand { get; private set; }

        public DelegateCommand ClearSelectedCommand { get; private set; }

        public DelegateCommand ClearAllCommand { get; private set; }
    }
}
