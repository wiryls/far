﻿using System.Collections.Generic;
using System.Diagnostics;
using System.Windows.Input;

namespace FAR.ViewModel
{
    internal class MainViewModel : ViewModelBase
    {
        public MainViewModel()
        {
            List = new List();
            RenameCommand = new DelegateCommand(Rename);
            ImportCommand = new DelegateCommand(ImportFiles);
            DeleteCommand = new DelegateCommand(Todo);
            ClearCommand = new DelegateCommand(Todo);
        }

        private void Rename(object parameter)
        {

        }

        private void ImportFiles(object parameter)
        {
            if (parameter is List<string> list)
            {
                foreach (var file in list)
                    Debug.WriteLine(file);
            }
            else
            {
                Debug.WriteLine($"Command ImportFiles: unsupported parameter {parameter}");
            }
        }

        private void Todo(object parameter)
        {
            // dummy
        }

        public List List { get; private set; }

        public ICommand RenameCommand { get; private set; }

        public ICommand ImportCommand { get; private set; }

        public ICommand DeleteCommand { get; private set; }

        public ICommand ClearCommand { get; private set; }
    }
}
