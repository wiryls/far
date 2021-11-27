using System.Windows.Input;

namespace FAR
{
    internal class MainViewModel : ViewModelBase
    {
        public MainViewModel()
        {
            List = new List();
            RenameCommand = new DelegateCommand(Rename);
            ImportCommand = new DelegateCommand(ImportFiles);
            DeleteCommand = new DelegateCommand(Dummy);
            ClearCommand  = new DelegateCommand(Dummy);
        }

        private void Rename(object parameter)
        {

        }

        private void ImportFiles(object parameter)
        {
            //if (dialog.ShowDialog() == true)
            //{
            //    //foreach (var file in dialog.FileNames)
            //    //    lbFiles.Items.Add(Path.GetFileName(filename));
            //}
        }

        private void Dummy(object parameter)
        {

        }

        public List List { get; private set; }

        public ICommand RenameCommand { get; private set; }

        public ICommand ImportCommand { get; private set; }

        public ICommand DeleteCommand { get; private set; }

        public ICommand ClearCommand { get; private set; }
    }
}
