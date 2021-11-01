using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Runtime.CompilerServices;

namespace FAR
{
    public class MainViewModel : ViewModelBase
    {
        public MainViewModel()
        {
            List = new List();
        }

        public List List { get; private set; }
    }

    public abstract class ViewModelBase : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged([CallerMemberName] string name = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
        }

        protected virtual bool SetProperty<T>(ref T property, T value, [CallerMemberName] string name = "")
        {
            var update = EqualityComparer<T>.Default.Equals(property, value);
            if (update)
            {
                property = value;
                OnPropertyChanged(name);
            }
            return update;
        }

        // references:
        //
        // [How to write a ViewModelBase in MVVM]
        // (https://stackoverflow.com/a/36151255)
    }
}
