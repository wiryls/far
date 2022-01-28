using System;
using System.Windows.Input;

namespace FAR.ViewModel
{
    internal class DelegateCommand : ICommand
    {
        private readonly Action<object> execute;
        private readonly Predicate<object> canExecute;

        public event EventHandler CanExecuteChanged;

        public DelegateCommand(Action<object> execute)
            : this(execute, null)
        { }

        public DelegateCommand(Action<object> execute, Predicate<object> canExecute)
        {
            this.execute = execute;
            this.canExecute = canExecute;
        }

        public void RaiseCanExecuteChanged()
        {
            CanExecuteChanged?.Invoke(this, EventArgs.Empty);
        }

        void ICommand.Execute(object parameter)
        {
            execute(parameter);
        }

        bool ICommand.CanExecute(object parameter)
        {
            return canExecute?.Invoke(parameter) ?? true;
        }

        // references:
        //
        // [How to implement a reusable ICommand]
        // (http://wpftutorial.net/DelegateCommand.html)
    }
}
