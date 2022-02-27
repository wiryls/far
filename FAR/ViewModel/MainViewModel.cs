using Fx.Diff;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Runtime.CompilerServices;
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

        private Differ differ;

        public MainViewModel()
        {
            enableRecursiveImport = false;
            enableIgnoreCase = false;
            enableRegex = false;
            pattern = string.Empty;
            template = string.Empty;
            differ = DifferCreator.Create(pattern, template, enableIgnoreCase, enableRegex);

            RenameCommand = new DelegateCommand(Rename);
            DeleteCommand = new DelegateCommand(Todo);
            ClearCommand = new DelegateCommand(Todo);
            Items = new Items();

            OnFilesDropped(new List<string>
            {
                "/user/bin",
                "/user/local",
                "/etc/apt"
            });
        }

        private void UpdateDiffer<T>(ref T property, T value, [CallerMemberName] string name = "")
        {
            if (SetProperty(ref property, value, name)) try
            {
                differ = DifferCreator.Create(pattern, template, enableIgnoreCase, enableRegex);
            }
            catch { }
        }

        private void Rename(object parameter)
        {

        }

        private void Todo(object parameter)
        {
            // dummy
        }

        public void OnFilesDropped(List<string> list)
        {
            foreach (var path in list)
            {
                var dir = Path.GetDirectoryName(path) ?? string.Empty;
                var name = Path.GetFileName(path) ?? string.Empty;
                if (!string.IsNullOrEmpty(name) && !string.IsNullOrEmpty(dir))
                    Items.Add(dir, differ(name));
            }
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

        public ICommand RenameCommand { get; private set; }

        public ICommand DeleteCommand { get; private set; }

        public ICommand ClearCommand { get; private set; }

        public Items Items { get; private set; }
    }
}
