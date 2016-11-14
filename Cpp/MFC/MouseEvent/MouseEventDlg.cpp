
// MouseEventDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "MouseEvent.h"
#include "MouseEventDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMouseEventDlg 对话框



CMouseEventDlg::CMouseEventDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MOUSEEVENT_DIALOG, pParent)
	, m_StartDraw(false)
	, m_PrevPoint(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMouseEventDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMouseEventDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CMouseEventDlg::OnBnClickedButton1)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()


// CMouseEventDlg 消息处理程序

BOOL CMouseEventDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMouseEventDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMouseEventDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMouseEventDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMouseEventDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	CWnd *pCanvasWnd = GetDlgItem(IDC_CANVAS);
	CDC *pDC = pCanvasWnd->GetDC();
	CRect rect; 
	pCanvasWnd->GetClientRect(&rect);
	rect.left += 1;
	rect.right -= 1;
	rect.top += 1;
	rect.bottom -= 1;
	CBrush brush;
	auto bkColor = pDC->GetBkColor();
	auto r = GetRValue(bkColor);
	auto g = GetGValue(bkColor);
	auto b = GetBValue(bkColor);
	auto color = RGB(r, g, b);
	brush.CreateSolidBrush(RGB(r, g, b));
	pDC->FillRect(&rect, &brush);
}


void CMouseEventDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CWnd *pCanvasWnd = GetDlgItem(IDC_CANVAS);
	CRect rect;
	pCanvasWnd->GetWindowRect(&rect);
	ScreenToClient(rect);
	CPoint p(rect.left, rect.top);
	m_PrevPoint = point - p;
	m_StartDraw = true;

	CDialogEx::OnLButtonDown(nFlags, point);
}


void CMouseEventDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_StartDraw = false;

	CDialogEx::OnLButtonUp(nFlags, point);
}


void CMouseEventDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_StartDraw == true)
	{
		CWnd *pCanvasWnd = GetDlgItem(IDC_CANVAS);
		CDC *pDC = pCanvasWnd->GetDC();
		CRect rect;
		pCanvasWnd->GetClientRect(&rect);
		int width = rect.right - rect.left;
		int height = rect.bottom - rect.top;
		pCanvasWnd->GetWindowRect(&rect);
		ScreenToClient(rect);
		CPoint p(rect.left, rect.top);

		CPoint prevPoint, currPoint;
		prevPoint = m_PrevPoint;
		currPoint = point - p;
		if ((prevPoint.x > 0) && (prevPoint.x < width) &&
			(prevPoint.y > 0) && (prevPoint.y < height) &&
			(currPoint.x > 0) && (currPoint.x < width) &&
			(currPoint.y > 0) && (currPoint.y < height))
		{
			pDC->MoveTo(prevPoint);
			pDC->LineTo(currPoint);
		}
		//ReleaseDC(pDC);
		pDC->Detach();
		pCanvasWnd->Detach();
		m_PrevPoint = currPoint;
	}
	CDialogEx::OnMouseMove(nFlags, point);
}
