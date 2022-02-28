using Fx.Diff;
using System;
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

        private string patternError;

        private Differ differ;

        public MainViewModel()
        {
            enableRecursiveImport = false;
            enableIgnoreCase = false;
            enableRegex = false;
            pattern = string.Empty;
            template = string.Empty;
            patternError = string.Empty;
            differ = DifferCreator.Create(pattern, template, enableIgnoreCase, enableRegex);

            Items = new Items();
            RenameCommand = new DelegateCommand(Rename);
            ClearSelectedCommand = new DelegateCommand(Todo);
            ClearAllCommand = new DelegateCommand(Todo, o => Items.Count != 0);
        }

        private void UpdateDiffer<T>(ref T property, T value, [CallerMemberName] string name = "")
        {
            if (SetProperty(ref property, value, name)) try
            {
                differ = DifferCreator.Create(pattern, template, enableIgnoreCase, enableRegex);
                if (!string.IsNullOrEmpty(PatternError))
                    PatternError = string.Empty;
            }
            catch (Exception e)
            {
                PatternError = e.Message;
            }
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
            foreach (var path in list)
            {
                var name = Path.GetFileName(path);
                var dir = Path.GetDirectoryName(path);
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

        public string PatternError
        {
            get => patternError;
            set => SetProperty(ref patternError, value);
        }

        public Items Items { get; private set; }

        public ICommand RenameCommand { get; private set; }

        public ICommand ClearSelectedCommand { get; private set; }

        public ICommand ClearAllCommand { get; private set; }
    }
}
