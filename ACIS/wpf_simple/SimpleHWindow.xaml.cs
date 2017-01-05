using System;
using System.Collections.Generic;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

using hoops_base;
using System.Windows.Interop;
using Microsoft.Win32;
using System.Windows.Forms.Integration;
using System.Windows.Xps.Packaging; //For printing from ReachFramework
using System.Windows.Threading;


namespace wpf_simple
{
    public partial class WPFSimpleWindow : HWindow
    {        
        OpenFileDialog openFileDialog;        
        PrintDialog printDialog;
        int is_host_added;
        DispatcherTimer timer;

        public WPFSimpleWindow()
        {
            m_pWinFormsHost = new WindowsFormsHost();

			HCS.Define_System_Options("license = `4Bn9BCQKDyQ2Ega40Ci_xQMV8Tez0y26vUIUEeM89gRs5S6Nj7DFjANaAzaZ8TRrrCRy9iUjwvTFjDNp8Fn8CSmT0hUZ1gQT3jqU9EeU0SuYDQQ70CuPAUfkCCZ9Cjjx1gzt8vjtCTjq9CUI5BnovE7lCRVr7TaL2ji72hns6i7$xVr9ABIH2EbvDU31zUZ44AEJ8eUU1he6wSV42gY5DwqZxgIS4gUXAFbcxDezxhVi8eM02UjdwldNj7Fw2hmNurHFjCMXBE1Gj7DFBhMV9VPIj7EM2C28GrPFjEJaCwJpvQU0vTRmwUbk9fe28uM0wxY18AVk8AFl7QQzj7F43C2_CEVkCCEV3iB02Re8CS2HvRa_0QJw8EuU6gA18ii32BIS3hQ48xU0wxU7xBnnvANm9fnlxQQ28fnpxBnp8QY8xBbm7Re78TNpxDQ0xeM8xRjowuQ7xfe7xuU18ia6xvjpxAY1wuQ68AZpwhQ59eY6xUbowfe6xvm6vDI4wTQ4xxI8vAFk8vnpwARkwDI4weJl8DY6weRlwhY08TRkwQNnvQI7xhQzxeU3wRa89ibmwvfo8uNpvTY28Ea6xia7wRe08fmzwvfowQVl8eZn8uU58uJlwRe7wxY2whI6whM0wDZl8hM7wuI7whJowuI2vARnvQI7xAFk8va57RnovBbmxvbpvTVpxhQ2j7FqCxa23vUZ0Tbq3UnnxAQNCCEZ8vnwDgnp8fQ11hMVvCu1AuI3CUq1vQUWxSIYCBYH0Dm5xyb87SnxAfUV4Rfz8hVp0fNzCjny1iUSAfmIChMZ3gUJ2uZd9QFrARJc1SJ$CxVq0EiI8AQW8inc5UQKzRI09gJs7UeI3Cnn9Svv1eZo4Tn98wq6xg6H8SEIAivnAQJr8hVq0in10UM4Cva58BZu2jiY0ffowxYY0Fa73SUzARNy1CZtDTnr4RiJ4za3Eiy7EjnbvUEOCCqRwTn1xvn23iq_5jm59w3pBSZaBw6O5Ce58fj7ARj$0AVx2iVqCyI01CRs5fiS7Ti55C65BFmL1fJ2wBf6BER24CfmDgR42g2KwyyI9DjpwFf$Djny1gI6EgEVDi71AUJ4wiEQ5EuI5fiSAyrc4xr6BEfd1Ub5BhQ6AwE7AENc2haO2C2N5Fbl3Sj9xfj91uFo1Dbm3gE_8i71ww67xAQTCSZ3BDjb2eYz4EeKCvJw5UByxSe82hbpBfb3Dg7q8zfbCvRr6i3tAuZyvEnq6jrpAwZbBgjn4Frm2uITwCJl4RI74eQzwVrl9DMLxU39zTa26hmRxCm1vRJoEgaGAhRn4yny3VjoEeFlDQYREifbBuIOwffq1gr03hjvEhf13RiRxTjs7Tbqj5`");

            m_pHDB = new HDB();
            m_pHDB.Init();

            string fontDir = System.IO.Path.Combine(Environment.GetEnvironmentVariable("SystemRoot"), "Fonts");
            HCS.Define_System_Options(string.Format("font directory = {0}, errors = on, warnings = on, multi-threading = full", fontDir));

            // This timer should really be at application scope, however since we're reinitializing
            // the HDB each time we create a window we might as well create a new timer too.
            timer = new DispatcherTimer();
            timer.Tick += new EventHandler(timer_Tick);
            timer.Interval = TimeSpan.FromMilliseconds(10);
            timer.Start();

            InitializeComponent();
            
            ////////////////////////////////////////////////////////////////////
            ActionMenu.Visibility = Visibility.Hidden;
            Print_Item.Visibility = Visibility.Collapsed;
            PrintSeperator.Visibility = Visibility.Collapsed;
            Load_Item.Visibility = Visibility.Collapsed;

            OpenButton.Visibility = Visibility.Collapsed;

            FileSeperator.Visibility = Visibility.Collapsed;

            OrbitButton.Visibility = Visibility.Collapsed;
            SphereButton.Visibility = Visibility.Collapsed;
            ConeButton.Visibility = Visibility.Collapsed;
            CylinderButton.Visibility = Visibility.Collapsed;
            AnnotateButton.Visibility = Visibility.Collapsed;
            AnnotateButton.Visibility = Visibility.Collapsed;
            ////////////////////////////////////////////////////////////////////

        }

        public void Init()
        {
            //Critical portion to connect between HOOPS and WPF
            if(is_host_added == 0)
            {
                m_pHPanel = new SimpleWPFPanel();

                m_pWinFormsHost.Child = m_pHPanel;

                CanvasGrid.Children.Add(m_pWinFormsHost);
                is_host_added = 1;
            }
            //Critical portion

            m_pHPanel.m_pHModel.Flush();
            m_pHPanel.m_pHView.Update();
            m_pHPanel.m_pHView.SetAxisMode(AxisMode.AxisOn);
            m_pHPanel.m_pHView.AdjustAxisWindow();

            ////////////////////////////////////////////////////////////////////
            ActionMenu.Visibility = Visibility.Visible;
            Print_Item.Visibility = Visibility.Visible;
            PrintSeperator.Visibility = Visibility.Visible;
            Load_Item.Visibility = Visibility.Visible;

            OpenButton.Visibility = Visibility.Visible;

            FileSeperator.Visibility = Visibility.Visible;

            OrbitButton.Visibility = Visibility.Visible;
            SphereButton.Visibility = Visibility.Visible;
            ConeButton.Visibility = Visibility.Visible;
            CylinderButton.Visibility = Visibility.Visible;
            AnnotateButton.Visibility = Visibility.Visible;
            ////////////////////////////////////////////////////////////////////

        }

        void timer_Tick(object sender, EventArgs e)
        {
            float time;
            HCS.Show_Time(out time);
            HDB.GetHTManager().Tick(time);
        }

        private void Load_File(object sender, RoutedEventArgs e)
        {
            openFileDialog = new OpenFileDialog();
            openFileDialog.Title = "Load";
            openFileDialog.Filter = "HMF/HSF files (*.hmf, *.hsf)|*.hmf;*.hsf" + "|All files (*.*)|*.*";

            bool? result = openFileDialog.ShowDialog();

            if (result == true)
            {
                m_pHPanel.m_pHModel.Flush();
				m_pHPanel.m_pHModel.Read(openFileDialog.FileName, m_pHPanel.m_pHView);

                this.Title = "WPF Simple Application : " + openFileDialog.FileName;

                m_pHPanel.m_pHView.FitWorld(); // This resets the camera so that it views the extents of the                                                                                                        scene
                m_pHPanel.m_pHView.Update();
            }
        }

        /////////////////////////// Menu Item Events ////////////////
        private void New_Click(object sender, RoutedEventArgs e)
        {
            WPFSimpleWindow newWindow = new WPFSimpleWindow();
            newWindow.Init();
            newWindow.Show();
            //((System.Windows.Interop.IKeyboardInputSink)sender).TabInto(new System.Windows.Input.TraversalRequest(FocusNavigationDirection.First));
        }

        private void Load_Click(object sender, RoutedEventArgs e)
        {
            //Load_File(sender, e);
            (m_pHPanel as SimpleWPFPanel).ShowObject();
        }

        private void Open_Click(object sender, RoutedEventArgs e)
        {
            WPFSimpleWindow newWindow = new WPFSimpleWindow();            
            newWindow.Init();
            newWindow.Load_File(sender, e);
            newWindow.Show();
        }

        private void Print_Click(object sender, RoutedEventArgs e)
        {
            // Create the print dialog object and set options
            printDialog = new PrintDialog();
            printDialog.PageRangeSelection = PageRangeSelection.AllPages;
            printDialog.UserPageRangeEnabled = true;

            bool? print = printDialog.ShowDialog();
            if (print == true)
            {
                //XpsDocument xpsDocument = new XpsDocument("FixedDocumentSequence.xps", System.IO.FileAccess.ReadWrite);
                //FixedDocumentSequence fixedDocSeq = xpsDocument.GetFixedDocumentSequence();
                //printDialog.PrintDocument(fixedDocSeq.DocumentPaginator, "Printing Hoops View");
                printDialog.PrintVisual(m_pWinFormsHost, "Printing Hoops View");
            }
        }

        public void DisposeAll()
        {
            if (m_pHPanel != null)
            {
                m_pHPanel.DisposePanel();
            }
        }

        private void Exit_Click(object sender, RoutedEventArgs e)
        {     
            this.Close();
        }

        protected override void OnClosed(EventArgs e)
        {
            this.DisposeAll();
            m_pHDB.Dispose();

            base.OnClosed(e);
        }

        private void CameraManip_Click(object sender, RoutedEventArgs e)
        {
            m_pHPanel.SetCurrentOperator(new HOpCameraManipulate(m_pHPanel.m_pHView));
        }

        private void CreateSphere_Click(object sender, RoutedEventArgs e)
        {
            m_pHPanel.SetCurrentOperator(new HOpCreateSphere(m_pHPanel.m_pHView));
        }

        private void CreateCone_Click(object sender, RoutedEventArgs e)
        {
            m_pHPanel.SetCurrentOperator(new HOpCreateCone(m_pHPanel.m_pHView));
        }

        private void CreateCylinder_Click(object sender, RoutedEventArgs e)
        {
            m_pHPanel.SetCurrentOperator(new HOpCreateCylinder(m_pHPanel.m_pHView));
        }

        private void CreateAnnotate_Click(object sender, RoutedEventArgs e)
        {
            m_pHPanel.SetCurrentOperator(new HOpMarkupAnnotate(m_pHPanel.m_pHView));
        }

        /////////////////////////// Menu Item Events ////////////////

        /////////////////////////// ToolBar Events ////////////////
        private void New_PreviewMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            Init();
        }

        private void Open_PreviewMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            Load_File(sender, e);
        }

        private void Orbit_PreviewMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            m_pHPanel.SetCurrentOperator(new HOpCameraManipulate(m_pHPanel.m_pHView));
        }

        private void Sphere_PreviewMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            m_pHPanel.SetCurrentOperator(new HOpCreateSphere(m_pHPanel.m_pHView));
        }

        private void Cone_PreviewMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            m_pHPanel.SetCurrentOperator(new HOpCreateCone(m_pHPanel.m_pHView));
        }

        private void Cylinder_PreviewMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            m_pHPanel.SetCurrentOperator(new HOpCreateCylinder(m_pHPanel.m_pHView));
        }

        private void Annotate_PreviewMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            m_pHPanel.SetCurrentOperator(new HOpMarkupAnnotate(m_pHPanel.m_pHView));
        }

        private void WPFWindow_KeyDown(object sender, KeyEventArgs e)
        {
            //if (m_pHPanel != null)
            //    m_pHPanel.OnKeyDown(e);
        }

        private void WPFWindow_KeyUp(object sender, KeyEventArgs e)
        {
            //if (m_pHPanel != null)
            //    m_pHPanel.OnKeyUp(e);
        }

        /////////////////////////// ToolBar Events ////////////////

    }
}
