using System;
using System.Windows.Input;

namespace Far.ViewModel
{
    public class DelegateCommand<A, P> : ICommand
    {
        private readonly Action<A> execute;
        private readonly Predicate<P> canExecute;

        public event EventHandler CanExecuteChanged;

        public DelegateCommand(Action<A> execute, Predicate<P> canExecute = null)
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
            if (parameter is null)
                execute.Invoke(default);
            else if (parameter is A value)
                execute.Invoke(value);
            else
                throw new ArgumentException($"parameter should be {nameof(A)}");
        }

        bool ICommand.CanExecute(object parameter)
        {
            return canExecute is null
                ? true
                : parameter is null
                ? canExecute.Invoke(default)
                : parameter is P value
                ? canExecute.Invoke(value)
                : throw new ArgumentException($"parameter should be {nameof(P)}");
            ;
        }

        // References:
        //
        // [How to implement a reusable ICommand]
        // (http://wpftutorial.net/DelegateCommand.html)
        // [Command experiences using the ICommand interface]
        // https://docs.microsoft.com/en-us/windows/apps/design/controls/commanding
    }
}
