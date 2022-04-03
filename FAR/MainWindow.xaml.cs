using Microsoft.UI;
using Microsoft.UI.Windowing;
using Microsoft.UI.Xaml;
using WinRT.Interop;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace Far
{
    /// <summary>
    /// An empty window that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainWindow : Window
    {
        private const string title = "FAR";
        private const int width = 800;
        private const int height = 600;

        public MainWindow()
        {
            InitializeComponent();

            {
                SetWindowSize(width, height);

                // Note for Title:
                // https://github.com/microsoft/microsoft-ui-xaml/issues/3689#issuecomment-939412961
                Title = title;
            }

            // TODO: use custom TitleBar after it is ready.
            //ExtendsContentIntoTitleBar = true;
            //SetTitleBar(AppTitleBar);
        }

        private void SetWindowSize(int width, int height)
        {
            try
            {
                var hnd = WindowNative.GetWindowHandle(this);
                var wid = Win32Interop.GetWindowIdFromWindow(hnd);
                var win = AppWindow.GetFromWindowId(wid);
                win.Resize(new Windows.Graphics.SizeInt32(width, height));
            }
            catch
            { }

            // Note for resize:
            // https://github.com/microsoft/microsoft-ui-xaml/issues/6353#issuecomment-974619032
            // https://github.com/microsoft/microsoft-ui-xaml/issues/6353#issuecomment-991837288
            // And pay attention to this proposal:
            // https://github.com/microsoft/microsoft-ui-xaml/issues/2731
        }
    }
}
