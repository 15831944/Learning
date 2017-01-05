// 这段 MFC 示例源代码演示如何使用 MFC Microsoft Office Fluent 用户界面 
// (“Fluent UI”)。该示例仅供参考，
// 用以补充《Microsoft 基础类参考》和 
// MFC C++ 库软件随附的相关电子文档。
// 复制、使用或分发 Fluent UI 的许可条款是单独提供的。
// 若要了解有关 Fluent UI 许可计划的详细信息，请访问  
// http://msdn.microsoft.com/officeui。
//
// 版权所有(C) Microsoft Corporation
// 保留所有权利。

// HoopsTemplateView.cpp : CHoopsTemplateView 类的实现
//

#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "CADCAM.h"
#endif

#include "ACISHoopsDoc.h"
#include "ACISHoopsView.h"
#include "HBaseView.h"
#include "HEventInfo.h"
#include "HBaseOperator.h"
#include "HOpCameraOrbit.h"
#include "HOpCameraZoom.h"
#include "HOpCameraPan.h"
#include "HOpCameraZoomBox.h"
#include "HOpSelectArea.h"
#include "HHOOPSSelectionSet.h"
#include "HHOOPSView.h"
#include "HHOOPSModel.h"
#include "MainFrm.h"

#include "HOpCameraOrbitTurntable.h"
#include "HOpSelectAperture.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CHoopsTemplateView

IMPLEMENT_DYNCREATE(CACISHoopsView, CHoopsView)

BEGIN_MESSAGE_MAP(CACISHoopsView, CHoopsView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CACISHoopsView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()	
END_MESSAGE_MAP()

// CHoopsTemplateView 构造/析构

CACISHoopsView::CACISHoopsView()
{
	// TODO: 在此处添加构造代码
	m_pHView = NULL;	

	m_FaceVisibility = false;
	m_EdgeVisibility = false;
	m_VerticeVisibility = false;
	m_bLightVisibility = true;

	m_rc = RGB(255,255,255);
}

CACISHoopsView::~CACISHoopsView()
{
	if(m_pHView ) 
	{
		delete m_pHView;
		m_pHView = NULL;
	}
}

BOOL CACISHoopsView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式
	cs.lpszClass = AfxRegisterWndClass(CS_OWNDC|CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW);
	return CView::PreCreateWindow(cs);
}

void CACISHoopsView::OnDraw(CDC* /*pDC*/)
{
	CACISHoopsDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 在此处为本机数据添加绘制代码
}

void CACISHoopsView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CACISHoopsView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CACISHoopsView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加额外的打印前进行的初始化过程
}

void CACISHoopsView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加打印后进行的清理过程
}

void CACISHoopsView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CACISHoopsView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CHoopsTemplateView 诊断
#ifdef _DEBUG
void CACISHoopsView::AssertValid() const
{
	CView::AssertValid();
}

void CACISHoopsView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CACISHoopsDoc* CACISHoopsView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CACISHoopsDoc)));
	return (CACISHoopsDoc*)m_pDocument;
}
#endif //_DEBUG

void CACISHoopsView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
 //sun
	char        szTemp[256];
	long debug_flags = DEBUG_NO_WINDOWS_HOOK;

	sprintf (szTemp, "debug = %u", debug_flags);
	// TODO: 在此添加专用代码和/或调用基类

	m_pHView = new HHOOPSView(GetDocument()->m_pHoopsModel,0,"opengl",0, m_hWnd,NULL);
	((HHOOPSView *)m_pHView)->Init();

	HC_Open_Segment_By_Key(m_pHView->GetViewKey());
	HC_Set_Driver_Options(szTemp);
	HC_Set_Driver_Options("special events, update interrupts");
	HC_Control_Update(".", "redraw everything");
	HC_Set_Handedness("right");
	HC_Close_Segment();

	m_pHView->SetAxisMode(AxisOn);	

	HBaseOperator * op = m_pHView->GetOperator();

	if (op != NULL)
	{
		delete op;
	}

	op = new HOpCameraOrbit(m_pHView);
	m_pHView->SetOperator(op);

	((HOpCameraOrbit *)m_pHView->GetOperator())->SetLightFollowsCamera(true);
	m_pHView->RenderGouraud();  // set the render mode to gouraud

	m_pHView->FitWorld();		// fit the camera to the scene extents
	m_pHView->CameraPositionChanged();

	m_pHView->ZoomToExtents();

	HC_Set_Visibility ("lines = on, edges = off");

	COLORREF DefaultWindowBackgroundTopColor	= RGB(0.0000*255,0.501961*255, 0.501961*255);
	COLORREF DefaultWindowBackgroundBottomColor	= RGB(1.000*255, 0.984314*255, 0.941176*255);
	SetWindowColor(DefaultWindowBackgroundTopColor, DefaultWindowBackgroundBottomColor);
	SetTransparency();

	//we need to adjust the axis window outside the mvo class as the calculation of the window
	//extents is mfc specific
	AdjustAxisWindow();

	m_pHView->Update();
}

void CACISHoopsView::SetWindowColor(COLORREF new_top_color, COLORREF new_bottom_color, bool emit_message)
{
	HPoint WindowTopColor;
	WindowTopColor.Set(
		static_cast<float>(GetRValue(new_top_color))/255.0f,
		static_cast<float>(GetGValue(new_top_color))/255.0f,
		static_cast<float>(GetBValue(new_top_color))/255.0f);

	HPoint WindowBottomColor;
	WindowBottomColor.Set(
		static_cast<float>(GetRValue(new_bottom_color))/255.0f,
		static_cast<float>(GetGValue(new_bottom_color))/255.0f,
		static_cast<float>(GetBValue(new_bottom_color))/255.0f);


	m_pHView->SetWindowColor(WindowTopColor, WindowBottomColor, emit_message);
}

void CACISHoopsView::SetTransparency()
{
	char text[4096];
	char style[4096];
	char sorting[4096];
	char layers[4096];
	bool fast_z_sort=false;

	strcpy(style, H_ASCII_TEXT("blended"));
	strcpy(sorting, H_ASCII_TEXT("Transparency Sorting"));
	strcpy(layers, H_ASCII_TEXT("Transparency Depth Peeling Layers"));

	if(strstr(sorting, "z-sort"))
	{
		if(strstr(sorting, "fast"))
			fast_z_sort=true;
		sprintf(sorting, "z-sort only");
	}

	sprintf(text,"style = %s, hsr algorithm = %s,depth peeling options=(layers= %s)",style, sorting, layers);
	m_pHView->SetTransparency(text, fast_z_sort);

}

void CACISHoopsView::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 CView::OnPaint()
	if(m_pHView)
	{	
		HC_Control_Update_By_Key (((HHOOPSView *)m_pHView)->GetViewKey(),"redraw everything");
		HC_Open_Segment_By_Key (m_pHView->GetViewKey());
		HC_Set_Visibility ("lines = on, edges = off");
		HC_Close_Segment ();
		m_pHView->Update();
	}
}

unsigned long CACISHoopsView::MapFlags(unsigned long state)
{
	unsigned long flags = 0;
	/*map the qt events state to MVO*/
	if(state & MK_LBUTTON) flags = MVO_LBUTTON;
	if(state & MK_RBUTTON) flags = MVO_RBUTTON;
	if(state & MK_MBUTTON) flags = MVO_MBUTTON;
	if(state & MK_SHIFT) flags = MVO_SHIFT;
	if(state & MK_CONTROL) flags = MVO_CONTROL;
	return flags;
}


void CACISHoopsView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	HEventInfo event(m_pHView);
	HBaseOperator *op = m_pHView->GetCurrentOperator();
	if(op)
	{
		event.SetPoint(HE_LButtonDown,point.x,point.y,MapFlags(nFlags));
		op->OnLButtonDown(event);
	}

	CHoopsView::OnLButtonDown(nFlags, point);
}


void CACISHoopsView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	// TODO: 在此添加消息处理程序代码和/或调用默认值
	HEventInfo event(m_pHView);
	HBaseOperator *op = m_pHView->GetCurrentOperator();
	if(op)
	{
		event.SetPoint(HE_LButtonUp,point.x,point.y,MapFlags(nFlags));
		op->OnLButtonUp(event);
	}

	CHoopsView::OnLButtonUp(nFlags, point);
}


void CACISHoopsView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	HEventInfo event(m_pHView);
	HBaseOperator *op = m_pHView->GetCurrentOperator();
	if(op)
	{
		//event.SetPoint(HE_MouseMove,point.x,point.y,MapFlags(nFlags));
		//op->OnMouseMove(event);
	}

	CHoopsView::OnMouseMove(nFlags, point);
}

void CACISHoopsView::OnButtonFaceVisiable()
{
	HC_Open_Segment_By_Key (m_pHView->GetSceneKey());
	if (m_FaceVisibility) 
	{
		m_FaceVisibility = false;
		HC_Set_Visibility ("faces = off");
		//HC_Set_Heuristics ("no hidden surfaces");
	}
	else 
	{
		m_FaceVisibility = true;
		HC_Set_Visibility ("faces = on");
		//HC_Set_Heuristics ("hidden surfaces");
	}
	HC_Close_Segment ();

	m_pHView->Update();	
}

void CACISHoopsView::OnButtonEdgeVisiable()
{
	HHOOPSModel* model = (HHOOPSModel *)(m_pHView->GetModel());

	// the modeling kernel's concept of edges are being represented in the HOOPS
	// graphics database by HOOPS polylines, etc... The visiblity of these HOOPS
	// primitives are controlled using the 'line' identifier, while the 'edge' 
	// indentifier in HOOPS is reserved for the borders of facets in HOOPS shells/meshses

	HC_Open_Segment_By_Key (m_pHView->GetSceneKey());
	if (m_EdgeVisibility) 
	{
		m_EdgeVisibility = false;
		HC_Set_Visibility ("lines = off, edges = off");
	}
	else 
	{
		m_EdgeVisibility = true;
		if (model->m_bSolidModel == true)
		{
			//HC_Set_Line_Weight(2.0f);
			HC_Set_Visibility ("lines = on, edges = off");
		}
		else
		{
			//HC_Set_Line_Weight(2.0f);
			HC_Set_Visibility ("lines = on, edges = off");
		}
	}
	HC_Close_Segment ();

	m_pHView->Update();	
}

void CACISHoopsView::OnButtonVertexVisiable()
{
	HC_Open_Segment_By_Key (m_pHView->GetSceneKey());
	if (m_VerticeVisibility) 
	{
		m_VerticeVisibility = false;
		HC_Set_Visibility ("markers = off");
	}
	else 
	{
		m_VerticeVisibility = true;
		HC_Set_Visibility ("markers = on");
	}
	HC_Close_Segment ();

	m_pHView->Update();	
}


void CACISHoopsView::OnUpdateButtonFaceUpdate(CCmdUI *pCmdUI)
{
	//pCmdUI->SetCheck (m_FaceVisibility);
}


void CACISHoopsView::OnUpdateButtonEdgeUpdate(CCmdUI *pCmdUI)
{
	//pCmdUI->SetCheck(m_EdgeVisibility);
}


void CACISHoopsView::OnUpdateButtonVertexUpdate(CCmdUI *pCmdUI)
{
	//pCmdUI->SetCheck(m_VerticeVisibility);

}

//视图旋转
void CACISHoopsView::OnOrbit()
{
	HBaseOperator * op = m_pHView->GetOperator();

	if (op != NULL)
	{
		delete op;
	}

	op = new HOpCameraOrbit(m_pHView);
	m_pHView->SetOperator(op);
}

//视图平移
void CACISHoopsView::OnPan()
{
	HBaseOperator * op = m_pHView->GetOperator();

	if (op != NULL)
	{
		delete op;
	}

	op = new HOpCameraPan(m_pHView);
	m_pHView->SetOperator(op);
}

//视图放大缩小
void CACISHoopsView::OnZoom()
{
	HBaseOperator * op = m_pHView->GetOperator();

	if (op)
		delete op;

	op = new HOpCameraZoom(m_pHView);
	m_pHView->SetOperator(op);
}

void CACISHoopsView::OnAroundaxis()
{
	 	HBaseOperator *op = m_pHView->GetOperator();
	 	if(op)
	 	{
	 		delete op;
		}
	 
	 	op = new HOpCameraOrbitTurntable(m_pHView);
	 	m_pHView->SetOperator(op);
 }

void CACISHoopsView::OnSelectByWindow()
{
	HBaseOperator *op = m_pHView->GetOperator();
	if(op)
	{
		delete op;
	}

	//op = new HOpSelectArea(m_pHView);
	op =  new HOpSelectAperture(m_pHView);
	m_pHView->SetOperator(op);
}


void CACISHoopsView::OnDeleteSelect()
{
	m_pHView->DeleteSelectionList(true);
	m_pHView->Update();	
}

//阴影
void CACISHoopsView::OnShaded()
{
	m_pHView->SetRenderMode(HRenderGouraud, true);
	m_pHView->Update();
}

//隐线
void CACISHoopsView::OnHiddenline()
{
	m_pHView->SetRenderMode(HRenderHiddenLine, true);
	m_pHView->Update();
}

//隐面
void CACISHoopsView::OnRendermodeBrepwireframe()
{
	m_pHView->SetRenderMode(HRenderWireframe, true);
	m_pHView->Update();
}

//前视
void CACISHoopsView::OnFont()
{
	m_pHView->SetViewMode(HViewFront);
}


void CACISHoopsView::OnUpdateFont(CCmdUI *pCmdUI)
{
	if (m_pHView->GetViewActive() && m_pHView->GetModel()->GetFileLoadComplete() &&
		m_pHView->GetViewMode() == HViewXY)
		pCmdUI->SetCheck (1);
	else
		pCmdUI->SetCheck (0);
}

//后视
void CACISHoopsView::OnBack()
{
	m_pHView->SetViewMode(HViewBack);
}


void CACISHoopsView::OnUpdateBack(CCmdUI *pCmdUI)
{

	if (m_pHView->GetViewActive() && m_pHView->GetModel()->GetFileLoadComplete() &&
		m_pHView->GetViewMode() == HViewYX)
		pCmdUI->SetCheck (1);
	else
		pCmdUI->SetCheck (0);
}

//上视
void CACISHoopsView::OnTop()
{
	m_pHView->SetViewMode(HViewTop);
}


void CACISHoopsView::OnUpdateTop(CCmdUI *pCmdUI)
{
	if (m_pHView->GetViewActive() && m_pHView->GetModel()->GetFileLoadComplete() &&
		m_pHView->GetViewMode() == HViewXZ)
		pCmdUI->SetCheck (1);
	else
		pCmdUI->SetCheck (0);
}

//下视
void CACISHoopsView::OnButtom()
{
	m_pHView->SetViewMode(HViewBottom);
}


void CACISHoopsView::OnUpdateButtom(CCmdUI *pCmdUI)
{
	if (m_pHView->GetViewActive() && m_pHView->GetModel()->GetFileLoadComplete() &&
		m_pHView->GetViewMode() == HViewZX)
		pCmdUI->SetCheck (1);
	else
		pCmdUI->SetCheck (0);
}

//左视
void CACISHoopsView::OnLeft()
{
	m_pHView->SetViewMode(HViewLeft);
}


void CACISHoopsView::OnUpdateLeft(CCmdUI *pCmdUI)
{
	if (m_pHView->GetViewActive() && m_pHView->GetModel()->GetFileLoadComplete() &&
		m_pHView->GetViewMode() == HViewYZ)
		pCmdUI->SetCheck (1);
	else
		pCmdUI->SetCheck (0);
}

//右视
void CACISHoopsView::OnRight()
{
	m_pHView->SetViewMode(HViewRight);
}


void CACISHoopsView::OnUpdateRight(CCmdUI *pCmdUI)
{
	if (m_pHView->GetViewActive() && m_pHView->GetModel()->GetFileLoadComplete() &&
		m_pHView->GetViewMode() == HViewZY)
		pCmdUI->SetCheck (1);
	else
		pCmdUI->SetCheck (0);
}


void CACISHoopsView::OnIsometric()
{
	m_pHView->SetViewMode(HViewIsoFrontRightTop);
	m_pHView->Update();
}

void CACISHoopsView::OnUpdateIsometric(CCmdUI *pCmdUI)
{
	if (m_pHView->GetViewActive() && m_pHView->GetModel()->GetFileLoadComplete() &&
		m_pHView->GetViewMode() == HViewIso)
		pCmdUI->SetCheck (1);
	else
		pCmdUI->SetCheck (0);
}


void CACISHoopsView::OnZoomtoextents()
{
	m_pHView->ZoomToExtents();
	m_pHView->Update();
}

void CACISHoopsView::OnZoomToWindow()
{
	LocalSetOperator(new HOpCameraZoomBox(m_pHView));
	((HOpCameraZoomBox *)m_pHView->GetOperator())->SetLightFollowsCamera(true);
}

// set current operator
void CACISHoopsView::LocalSetOperator(HBaseOperator *NewOperator)
{
	HBaseOperator * op =m_pHView->GetOperator();

	if (op)
		delete op;	

	m_pHView->SetOperator(NewOperator);
}

void CACISHoopsView::OnModifyBackgroundColor()
{
	CColorDialog  dlg;
	if (dlg.DoModal() == IDOK)
	{
		m_rc = dlg.GetColor();
	
		int R = GetRValue(m_rc);
		int G = GetGValue(m_rc);
		int B = GetBValue(m_rc);

		COLORREF DefaultWindowBackgroundTopColor	= RGB(0.0000*R,0.501961*G, 0.501961*B);
		COLORREF DefaultWindowBackgroundBottomColor	= RGB(1.000*255, 0.984314*255, 0.941176*255);
		SetWindowColor(DefaultWindowBackgroundTopColor, DefaultWindowBackgroundBottomColor);
		SetTransparency();

		//we need to adjust the axis window outside the mvo class as the calculation of the window
		//extents is mfc specific
		AdjustAxisWindow();
		
		m_pHView->Update();
	}
}

void CACISHoopsView::OnModifyEntitycolor()
{
	CColorDialog  dlg;
	if (dlg.DoModal() != IDOK)
	{
		return;
	}

	COLORREF rc = dlg.GetColor();

	int R = GetRValue(rc);
	int G = GetGValue(rc);
	int B = GetBValue(rc);


	HHOOPSView* my_pHBaseView = (HHOOPSView *)(m_pHView);
	HHOOPSModel* my_pHBaseModel = (HHOOPSModel *)(GetDocument()->m_pHoopsModel);

	//HC_Open_Segment_By_Key(my_pHBaseModel->GetModelKey());
	HC_Open_Segment_By_Key(my_pHBaseView->GetSceneKey());
		HC_Set_Color_By_Value("everything", "RGB", double(R)/255.0, double(G)/255.0, double(B)/255.0);
	HC_Open_Segment("ACIS");	
		//((HHOOPS10Model *)my_pHBaseModel)->AddAcisEntity(theApp.m_Centrifugal_Impeller_Model);
	HC_Close_Segment();
	HC_Close_Segment();

	my_pHBaseModel->Update();
	my_pHBaseView->Update();
}


void CACISHoopsView::OnRButtonDown(UINT nFlags, CPoint point)
{
	CMenu menu;
	//menu.LoadMenu(IDR_RIGHTBUTTON_MENU);
	CMenu *pPopupMenu = menu.GetSubMenu(0);

	ClientToScreen(&point);
	pPopupMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);

	CHoopsView::OnRButtonDown(nFlags, point);
}


void CACISHoopsView::OnVisibileVertex()
{
	if(!m_VerticeVisibility)
	{
		HC_Open_Segment_By_Key (m_pHView->GetSceneKey());
		HC_Set_Visibility ("markers = on, vertices = off");
		HC_Close_Segment ();

		m_VerticeVisibility = true;
	}
	else
	{
		HC_Open_Segment_By_Key (m_pHView->GetSceneKey());
		HC_Set_Visibility ("markers = off, vertices = off");
		HC_Close_Segment ();

		m_VerticeVisibility = false;
	}

	m_pHView->Update();	
}


void CACISHoopsView::OnVisibileEdge()
{
	HHOOPSModel * model = (HHOOPSModel *)(m_pHView->GetModel());

	HC_Open_Segment_By_Key (m_pHView->GetSceneKey());
	if (m_EdgeVisibility) 
	{
		m_EdgeVisibility = false;
		if (model->IsBRepGeometry())
			HC_Set_Visibility ("lines = off");
		else
			HC_Set_Visibility ("lines = off, edges = off");
	}
	else 
	{
		m_EdgeVisibility = true;
		if (model->IsBRepGeometry())
			HC_Set_Visibility ("lines = on");
		else
			HC_Set_Visibility ("lines = on, edges = on");  
	}
	HC_Close_Segment ();

	HSelectionSet * selection=m_pHView->GetSelection();
	if(selection)
		selection->UpdateHighlighting();

	m_pHView->Update();	
}


void CACISHoopsView::OnVisibileFace()
{

	((HHOOPSView* )m_pHView)->SetVisibilityFaces(!((HHOOPSView*)m_pHView)->GetVisibilityFaces());

	HHOOPS10SelectionSet * selection=(HHOOPS10SelectionSet*)(m_pHView->GetSelection());
	if(selection)
		selection->UpdateHighlighting();

	m_pHView->Update();
}


void CACISHoopsView::OnVisibilityLights()
{
	HC_Open_Segment_By_Key (m_pHView->GetSceneKey());
	if (m_bLightVisibility) 
	{
		m_bLightVisibility = false;
		HC_Set_Visibility ("lights = off");
		m_bFastPrint = false;
	}
	else 
	{
		m_bLightVisibility = true;
		HC_Set_Visibility ("lights = (faces = on, edges = off, markers = off)");
		m_bFastPrint = true;
	}
	HC_Close_Segment ();

	m_pHView->Update();	
}


void CACISHoopsView::OnSellevelVertex()
{
	HC_Open_Segment_By_Key (m_pHView->GetSceneKey());
	HC_Set_Selectability("faces = off, lines = off, edges = off, markers = on");
	HC_Close_Segment();
	((HHOOPS10SelectionSet*)m_pHView->GetSelection())->SetSelectLevel(VERTEX_TYPE);

	int leve = -2;
	leve= ((HHOOPS10SelectionSet*)m_pHView->GetSelection())->GetSelectLevel();
}

void CACISHoopsView::OnSellevelEdge()
{
	HC_Open_Segment_By_Key (m_pHView->GetSceneKey());
	HC_Set_Selectability("faces = off, lines = on, edges = off, markers = off");
	HC_Close_Segment();
	((HHOOPS10SelectionSet*)m_pHView->GetSelection())->SetSelectLevel(EDGE_TYPE);

	int leve = -2;
	leve= ((HHOOPS10SelectionSet*)m_pHView->GetSelection())->GetSelectLevel();
}


void CACISHoopsView::OnSellevelFace()
{
	HC_Open_Segment_By_Key (m_pHView->GetSceneKey());
	HC_Set_Selectability("faces = on, lines = off, edges = off, markers = off");
	HC_Close_Segment();
	((HHOOPS10SelectionSet*)m_pHView->GetSelection())->SetSelectLevel(FACE_TYPE);

	int leve = -2;
	leve= ((HHOOPS10SelectionSet*)m_pHView->GetSelection())->GetSelectLevel();
}

void CACISHoopsView::OnSellevelBody()
{
	HC_Open_Segment_By_Key (m_pHView->GetSceneKey());
	HC_Set_Selectability("faces = on, lines = on, edges = off, markers = on");
	HC_Close_Segment();
	((HHOOPS10SelectionSet*)m_pHView->GetSelection())->SetSelectLevel(BODY_TYPE);

	int leve = -2;
	leve= ((HHOOPS10SelectionSet*)m_pHView->GetSelection())->GetSelectLevel();
}


void CACISHoopsView::OnSellevelAssembly()
{
	HC_Open_Segment_By_Key (m_pHView->GetSceneKey());
	HC_Set_Selectability("faces = on, lines = on, edges = off");
	HC_Close_Segment();
	((HHOOPS10SelectionSet*)m_pHView->GetSelection())->SetSelectLevel(ENTITY_TYPE);

	int leve = -2;
	leve= ((HHOOPS10SelectionSet*)m_pHView->GetSelection())->GetSelectLevel();
}


// 清除屏幕上显示的HOOPS图元
void CACISHoopsView::OnButtonClear()
{
	CMainFrame* pMainFrame = (CMainFrame *)(AfxGetApp()->GetMainWnd());  //AfxGetMainWnd();
	if(pMainFrame == NULL)
	{
		AfxMessageBox(_T("error!"));
		return;
	}

	CACISHoopsView* pView = (CACISHoopsView *)(pMainFrame->GetActiveView());
	if(pView == NULL)
		return;

	HHOOPSView* my_pHBaseView = (HHOOPSView *)(pView->m_pHView);
	if(my_pHBaseView == NULL)
		return;
	HHOOPSModel* my_pHBaseModel = (HHOOPSModel *)(pView->GetDocument()->m_pHoopsModel);
	if(my_pHBaseModel == NULL)
		return;

	//long key = my_pHBaseView->GetSceneKey();
	//HC_Open_Segment_By_Key(my_pHBaseView->GetSceneKey());
	HC_Open_Segment_By_Key(my_pHBaseModel->GetModelKey());
	HC_Flush_Geometry("HOOPS");
	HC_Close_Segment();
	HC_Update_Display();
	my_pHBaseModel->Update();
	my_pHBaseView->Update();

	HC_Open_Segment_By_Key(my_pHBaseModel->GetModelKey());
	HC_Open_Segment("Color");
	//HC_Flush_Geometry("Color");
	HC_Flush_Contents("...", "geometry");
	HC_Close_Segment();
	HC_Close_Segment();
	HC_Update_Display();
	my_pHBaseModel->Update();
	my_pHBaseView->Update();

}
