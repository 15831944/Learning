// 这段 MFC 示例源代码演示如何使用 MFC Microsoft Office Fluent 用户界面 
// (“Fluent UI”)。该示例仅供参考，
// 用以补充《Microsoft 基础类参考》和 
// MFC C++ 库软件随附的相关电子文档。  
// 复制、使用或分发 Fluent UI 的许可条款是单独提供的。  
// 若要了解有关 Fluent UI 许可计划的详细信息，请访问 
// http://go.microsoft.com/fwlink/?LinkId=238214。
//
// 版权所有(C) Microsoft Corporation
// 保留所有权利。

// CADCAMView.cpp : CCADCAMView 类的实现
//

#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "CADCAM.h"
#endif

#include "CADCAMDoc.h"
#include "CADCAMView.h"


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


// CCADCAMView

IMPLEMENT_DYNCREATE(CCADCAMView, CView)

BEGIN_MESSAGE_MAP(CCADCAMView, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CCADCAMView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_ORBIT, &CCADCAMView::OnOrbit)
	ON_COMMAND(ID_PAN, &CCADCAMView::OnPan)
	ON_COMMAND(ID_Zoom, &CCADCAMView::OnZoom)
	ON_COMMAND(ID_AroundAxis, &CCADCAMView::OnAroundAxis)
	ON_COMMAND(ID_ZoomToExtents, &CCADCAMView::OnZoomtoextents)
	ON_COMMAND(ID_ZoomToWindow, &CCADCAMView::OnZoomtowindow)
END_MESSAGE_MAP()

// CCADCAMView 构造/析构

CCADCAMView::CCADCAMView()
{
	// TODO: 在此处添加构造代码
	m_pHView = NULL;

	m_FaceVisibility = false;
	m_EdgeVisibility = false;
	m_VerticeVisibility = false;
	m_bLightVisibility = true;

	m_rc = RGB(255, 255, 255);
}

CCADCAMView::~CCADCAMView()
{
	if (m_pHView)
	{
		delete m_pHView;
		m_pHView = NULL;
	}
}

BOOL CCADCAMView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式
	cs.lpszClass = AfxRegisterWndClass(CS_OWNDC | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW);
	return CView::PreCreateWindow(cs);
}

// CCADCAMView 绘制

void CCADCAMView::OnDraw(CDC* /*pDC*/)
{
	CCADCAMDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 在此处为本机数据添加绘制代码
}


// CCADCAMView 打印


void CCADCAMView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CCADCAMView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CCADCAMView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加额外的打印前进行的初始化过程
}

void CCADCAMView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加打印后进行的清理过程
}

void CCADCAMView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CCADCAMView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CCADCAMView 诊断

#ifdef _DEBUG
void CCADCAMView::AssertValid() const
{
	CView::AssertValid();
}

void CCADCAMView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CCADCAMDoc* CCADCAMView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCADCAMDoc)));
	return (CCADCAMDoc*)m_pDocument;
}
#endif //_DEBUG


// CCADCAMView 消息处理程序


void CCADCAMView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	// TODO: 在此添加专用代码和/或调用基类
	char        szTemp[256];
	long debug_flags = DEBUG_NO_WINDOWS_HOOK;

	sprintf(szTemp, "debug = %u", debug_flags);
	// TODO: 在此添加专用代码和/或调用基类

	m_pHView = new HHOOPSView(GetDocument()->m_pHoopsModel, 0, "opengl", 0, m_hWnd, NULL);
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

	HC_Set_Visibility("lines = on, edges = off");

	COLORREF DefaultWindowBackgroundTopColor = RGB(0.0000 * 255, 0.501961 * 255, 0.501961 * 255);
	COLORREF DefaultWindowBackgroundBottomColor = RGB(1.000 * 255, 0.984314 * 255, 0.941176 * 255);
	SetWindowColor(DefaultWindowBackgroundTopColor, DefaultWindowBackgroundBottomColor);
	SetTransparency();

	//we need to adjust the axis window outside the mvo class as the calculation of the window
	//extents is mfc specific
	AdjustAxisWindow();

	m_pHView->Update();
}


void CCADCAMView::SetWindowColor(COLORREF new_top_color, COLORREF new_bottom_color, bool emit_message)
{
	HPoint WindowTopColor;
	WindowTopColor.Set(
		static_cast<float>(GetRValue(new_top_color)) / 255.0f,
		static_cast<float>(GetGValue(new_top_color)) / 255.0f,
		static_cast<float>(GetBValue(new_top_color)) / 255.0f);

	HPoint WindowBottomColor;
	WindowBottomColor.Set(
		static_cast<float>(GetRValue(new_bottom_color)) / 255.0f,
		static_cast<float>(GetGValue(new_bottom_color)) / 255.0f,
		static_cast<float>(GetBValue(new_bottom_color)) / 255.0f);


	m_pHView->SetWindowColor(WindowTopColor, WindowBottomColor, emit_message);
}

void CCADCAMView::SetTransparency()
{
	char text[4096];
	char style[4096];
	char sorting[4096];
	char layers[4096];
	bool fast_z_sort = false;

	strcpy(style, H_ASCII_TEXT("blended"));
	strcpy(sorting, H_ASCII_TEXT("Transparency Sorting"));
	strcpy(layers, H_ASCII_TEXT("Transparency Depth Peeling Layers"));

	if (strstr(sorting, "z-sort"))
	{
		if (strstr(sorting, "fast"))
			fast_z_sort = true;
		sprintf(sorting, "z-sort only");
	}

	sprintf(text, "style = %s, hsr algorithm = %s,depth peeling options=(layers= %s)", style, sorting, layers);
	m_pHView->SetTransparency(text, fast_z_sort);

}

void CCADCAMView::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: 在此处添加消息处理程序代码
					   // 不为绘图消息调用 CHoopsView::OnPaint()

	if (m_pHView)
	{
		HC_Control_Update_By_Key(((HHOOPSView *)m_pHView)->GetViewKey(), "redraw everything");
		HC_Open_Segment_By_Key(m_pHView->GetViewKey());
		HC_Set_Visibility("lines = on, edges = off");
		HC_Close_Segment();
		m_pHView->Update();
	}
}

unsigned long CCADCAMView::MapFlags(unsigned long state)
{
	unsigned long flags = 0;
	/*map the qt events state to MVO*/
	if (state & MK_LBUTTON) flags = MVO_LBUTTON;
	if (state & MK_RBUTTON) flags = MVO_RBUTTON;
	if (state & MK_MBUTTON) flags = MVO_MBUTTON;
	if (state & MK_SHIFT) flags = MVO_SHIFT;
	if (state & MK_CONTROL) flags = MVO_CONTROL;
	return flags;
}

void CCADCAMView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CHoopsView::OnLButtonDown(nFlags, point);
}


void CCADCAMView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CHoopsView::OnLButtonUp(nFlags, point);
}


void CCADCAMView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CHoopsView::OnMouseMove(nFlags, point);
}


void CCADCAMView::OnOrbit()
{
	// TODO: 在此添加命令处理程序代码
	HBaseOperator * op = m_pHView->GetOperator();

	if (op != NULL)
	{
		delete op;
	}

	op = new HOpCameraOrbit(m_pHView);
	m_pHView->SetOperator(op);
}


void CCADCAMView::OnPan()
{
	// TODO: 在此添加命令处理程序代码
	HBaseOperator * op = m_pHView->GetOperator();

	if (op != NULL)
	{
		delete op;
	}

	op = new HOpCameraPan(m_pHView);
	m_pHView->SetOperator(op);
}


void CCADCAMView::OnZoom()
{
	// TODO: 在此添加命令处理程序代码
	HBaseOperator * op = m_pHView->GetOperator();

	if (op)
		delete op;

	op = new HOpCameraZoom(m_pHView);
	m_pHView->SetOperator(op);
}


void CCADCAMView::OnAroundAxis()
{
	// TODO: 在此添加命令处理程序代码
	HBaseOperator *op = m_pHView->GetOperator();
	if (op)
	{
		delete op;
	}

	op = new HOpCameraOrbitTurntable(m_pHView);
	m_pHView->SetOperator(op);
}


void CCADCAMView::OnZoomtoextents()
{
	// TODO: 在此添加命令处理程序代码
	m_pHView->ZoomToExtents();
	m_pHView->Update();
}


void CCADCAMView::OnZoomtowindow()
{
	// TODO: 在此添加命令处理程序代码
	LocalSetOperator(new HOpCameraZoomBox(m_pHView));
	((HOpCameraZoomBox *)m_pHView->GetOperator())->SetLightFollowsCamera(true);
}



// set current operator
void CCADCAMView::LocalSetOperator(HBaseOperator *NewOperator)
{
	HBaseOperator * op = m_pHView->GetOperator();

	if (op)
		delete op;

	m_pHView->SetOperator(NewOperator);
}