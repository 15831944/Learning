using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Controls;
using System.Windows;
using System.Windows.Input;
using System.Runtime.InteropServices;

using System.Windows.Interop;
using hoops_base;

#if _M_X64
using HCS_KEY = System.Int64;
#else
using HCS_KEY = System.Int32;
#endif

namespace wpf_simple
{
    public partial class SimpleWPFPanel : HPanel
    {

        public SimpleWPFPanel()
            : base()
        {
            InitializeComponent();
        }

        //public void InitializeComponent(IntPtr panelHandle)
        public void InitializeComponent()
        {
            winid = Handle;
            m_pHModel = new HSimpleModel();
            m_pHModel.Init();

            m_pHView = new HSimpleView(m_pHModel, "?picture" + winid, "dx9", "", winid);
            m_pHView.Init();

            m_pHSelection = new HSimpleSelectionSet(m_pHView);
            m_pHView.SetSelection(m_pHSelection);
            m_pHView.GetSelection().Init();

            Init();

            // Set the default operator
            m_pHOperator = new HOpCameraManipulate(m_pHView);
            m_pHView.SetCurrentOperator((HBaseOperator)m_pHOperator);

            //Setup the view
            m_pHView.CameraPositionChanged(true);
            m_pHView.RenderGouraud();
            m_pHView.SetHandedness(HandednessMode.HandednessRight);

            // set the gradient window background
            HPoint window_top_color = new HPoint(0.0000f, 0.501961f, 0.501961f);
            HPoint window_bottom_color = new HPoint(1.000f, 0.984314f, 0.941176f);
            m_pHView.SetWindowColor(window_top_color, window_bottom_color);

            HCS.Open_Segment_By_Key(m_pHView.GetSceneKey());
            HCS.Set_Color_By_Index("faces", 2);
            HCS.Set_Color_By_Index("text, lights", 1);
            HCS.Set_Color_By_Index("edges, lines", 1);
            HCS.Set_Color_By_Index("markers", 1);
            HCS.Set_Rendering_Options("color interpolation, color index interpolation");
            HCS.Set_Visibility("lights = (faces = on, edges = off, markers = off), markers = off, faces=on, edges=off, lines=off, text = on");
            HCS.Set_Selectability("everything = off, geometry = on");
            HCS.Set_Heuristics("polygon handedness = left");
            HCS.Close_Segment();

            // new segment for temporary construction geometry
            HCS.Open_Segment_By_Key(m_pHView.GetConstructionKey());
            HCS.Set_Heuristics("quick moves");
            HCS.Set_Visibility("faces = off, edges = on, lines = on");
            HCS.Close_Segment();

            // for quickmoves
            HCS.Open_Segment_By_Key(m_pHView.GetWindowspaceKey());
            HCS.Set_Color_By_Index("geometry", 3);
            HCS.Set_Color_By_Index("window contrast", 1);
            HCS.Set_Color_By_Index("windows", 1);

            HCS.Set_Visibility("markers=on");
            HCS.Set_Color("markers = green, lines = green");
            HCS.Set_Marker_Symbol("+");
            HCS.Set_Selectability("off");
            HCS.Close_Segment();

            HCS.Open_Segment_By_Key(m_pHView.GetSceneKey());
            HCS.Set_Text_Font("transforms = off");
            HCS.Close_Segment();


            HCS.Open_Segment("clock");
                HCS.Set_Color("dark blue");
                HCS.Set_Text_Font("transforms, size = 0.1 oru");
                const float rimr = 0.98f;         // radius of rim
                const float numr = 0.80f;         // radius of numbers
                HCS.Set_Color("faces=light silver");
                HCS.Set_Edge_Weight(3.0);
                HCS.Insert_Circle_By_Radius(new float[] { 0, 0, 0 }, rimr, null);
                HCS.Open_Segment("hands");
                    HCS.Set_Color("red");
                    HCS.Define_Line_Style("thick_solid_line", "3 pixel weight, dash");
                    HCS.Set_Line_Pattern("(-->");

                    HCS.Open_Segment("minute hand");
                    HCS.Insert_Line(0, 0, 0, 1, 1, 1);
                    HCS.Close_Segment();

                    HCS.Open_Segment("minute hand");
                    HCS.Insert_Line(1, 1, 1, 2, 2, 2);
                    HCS.Close_Segment();
                HCS.Close_Segment();
            HCS.Close_Segment();
        }


        public override void DisposePanel()
        {
            base.DisposePanel();         
        }

        public void ShowObject()
        {
            
        }

    }


    //! This class is derived from Hoops-specific HBaseView class
    /*!
     * It contains two parametrized constructors which are used to create HSimpleView object and Init()            
     * function which overrides base class's Init() method
    */
    public class HSimpleView : HBaseView
    {
        public HSimpleView(HBaseModel model, String alias, String driver_type, String instance_name,
                            IntPtr window_handle, IntPtr colormap, IntPtr clip_override,
                             IntPtr window_handle_2)
            : base(model, alias, driver_type, instance_name, window_handle, colormap, clip_override, window_handle_2)
        {
        }
        public HSimpleView(HBaseModel model, String alias, String driver_type, String instance_name,
                IntPtr window_handle)
            : base(model, alias, driver_type, instance_name, window_handle)
        {
        }
        // app-specific init function
        override public void Init()
        {
            // call base's init function first to get the default HOOPS hierarchy for the view
            base.Init();
        }

    }


    //! This class is derived from Hoops-specific HBaseModel class
    /*!
     * It contains  constructor which is used to create HBaseModel object and Init() and Flush()                  
     * functions which overrides base class's corresponding methods
    */
    public class HSimpleModel : HBaseModel
    {
        public HSimpleModel()
            : base()
        {
        }
        override public void Init()
        {
            base.Init();
            // TODO: your init here
        }
        override public void Flush()
        {
            // TODO: cleanup here

            base.Flush();
        }

    }


    //! This class is derived from Hoops-specific HSelectionSet class
    /*!
     * It contains  constructor which is used to create HSelectionSet object and Select(), DeSelect()             
     * and DeSelectAll() functions which overrides base class's corresponding methods
    */
    public class HSimpleSelectionSet : HSelectionSet
    {
        public HSimpleSelectionSet(HBaseView view)
            : base(view)
        {
        }
        override public void Select(HCS_KEY key, int num_include_keys, HCS_KEY[] include_keys, bool emit_message)
        {            
            //// TODO: Process your selection
            base.Select(key, num_include_keys, include_keys, emit_message);
        }
        override public void DeSelect(HCS_KEY key, int num_include_keys, HCS_KEY[] include_keys, bool emit_message)
        {
            //// TODO: Process your De-selection
            base.DeSelect(key, num_include_keys, include_keys, emit_message);
        }
        override public void DeSelectAll()
        {
            // TODO: Process your de-selectall
            base.DeSelectAll();
        }
    }

}
