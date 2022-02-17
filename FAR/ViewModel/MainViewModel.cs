using System.Collections.Generic;
using System.Diagnostics;
using System.Windows.Input;

namespace Far.ViewModel
{
    internal class MainViewModel : ViewModelBase, IFilesDropped
    {
        public MainViewModel()
        {
            Items = new Items();
            RenameCommand = new DelegateCommand(Rename);
            DeleteCommand = new DelegateCommand(Todo);
            ClearCommand = new DelegateCommand(Todo);
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
            foreach (var file in list)
                Debug.WriteLine(file);
        }

        public bool IsImportedRecursively
        {
            get { return isImportedRecursively; }
            set { isImportedRecursively = value; OnPropertyChanged(); }
        }

        public ICommand RenameCommand { get; private set; }

        public ICommand DeleteCommand { get; private set; }

        public ICommand ClearCommand { get; private set; }

        public Items Items { get; private set; }

        private bool isImportedRecursively = false;
    }
}
