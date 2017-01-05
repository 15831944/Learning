using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Windows;

namespace wpf_simple
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        void AppStartup(object sender, StartupEventArgs args)
        {
            WPFSimpleWindow mainForm = new WPFSimpleWindow();            
            mainForm.Init();
            mainForm.Show();     
        }
    }
}
