// ��� MFC ʾ��Դ������ʾ���ʹ�� MFC Microsoft Office Fluent �û����� 
// (��Fluent UI��)����ʾ�������ο���
// ���Բ��䡶Microsoft ������ο����� 
// MFC C++ ������渽����ص����ĵ���  
// ���ơ�ʹ�û�ַ� Fluent UI ����������ǵ����ṩ�ġ�  
// ��Ҫ�˽��й� Fluent UI ��ɼƻ�����ϸ��Ϣ������� 
// http://go.microsoft.com/fwlink/?LinkId=238214��
//
// ��Ȩ����(C) Microsoft Corporation
// ��������Ȩ����

// CADCAMView.cpp : CCADCAMView ���ʵ��
//

#include "stdafx.h"
// SHARED_HANDLERS ������ʵ��Ԥ��������ͼ������ɸѡ�������
// ATL ��Ŀ�н��ж��壬�����������Ŀ�����ĵ����롣
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
	// ��׼��ӡ����
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

// CCADCAMView ����/����

CCADCAMView::CCADCAMView()
{
	// TODO: �ڴ˴���ӹ������
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
	// TODO: �ڴ˴�ͨ���޸�
	//  CREATESTRUCT cs ���޸Ĵ��������ʽ
	cs.lpszClass = AfxRegisterWndClass(CS_OWNDC | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW);
	return CView::PreCreateWindow(cs);
}

// CCADCAMView ����

void CCADCAMView::OnDraw(CDC* /*pDC*/)
{
	CCADCAMDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: �ڴ˴�Ϊ����������ӻ��ƴ���
}


// CCADCAMView ��ӡ


void CCADCAMView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CCADCAMView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// Ĭ��׼��
	return DoPreparePrinting(pInfo);
}

void CCADCAMView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: ��Ӷ���Ĵ�ӡǰ���еĳ�ʼ������
}

void CCADCAMView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: ��Ӵ�ӡ����е��������
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


// CCADCAMView ���

#ifdef _DEBUG
void CCADCAMView::AssertValid() const
{
	CView::AssertValid();
}

void CCADCAMView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CCADCAMDoc* CCADCAMView::GetDocument() const // �ǵ��԰汾��������
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCADCAMDoc)));
	return (CCADCAMDoc*)m_pDocument;
}
#endif //_DEBUG


// CCADCAMView ��Ϣ�������


void CCADCAMView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	// TODO: �ڴ����ר�ô����/����û���
	char        szTemp[256];
	long debug_flags = DEBUG_NO_WINDOWS_HOOK;

	sprintf(szTemp, "debug = %u", debug_flags);
	// TODO: �ڴ����ר�ô����/����û���

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
					   // TODO: �ڴ˴������Ϣ����������
					   // ��Ϊ��ͼ��Ϣ���� CHoopsView::OnPaint()

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
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	CHoopsView::OnLButtonDown(nFlags, point);
}


void CCADCAMView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	CHoopsView::OnLButtonUp(nFlags, point);
}


void CCADCAMView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	CHoopsView::OnMouseMove(nFlags, point);
}


void CCADCAMView::OnOrbit()
{
	// TODO: �ڴ���������������
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
	// TODO: �ڴ���������������
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
	// TODO: �ڴ���������������
	HBaseOperator * op = m_pHView->GetOperator();

	if (op)
		delete op;

	op = new HOpCameraZoom(m_pHView);
	m_pHView->SetOperator(op);
}


void CCADCAMView::OnAroundAxis()
{
	// TODO: �ڴ���������������
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
	// TODO: �ڴ���������������
	m_pHView->ZoomToExtents();
	m_pHView->Update();
}


void CCADCAMView::OnZoomtowindow()
{
	// TODO: �ڴ���������������
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