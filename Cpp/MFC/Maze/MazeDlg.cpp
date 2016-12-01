
// MazeDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Maze.h"
#include "MazeDlg.h"
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


// CMazeDlg 对话框



CMazeDlg::CMazeDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MAZE_DIALOG, pParent)
	, m_colNumOfBoxes(0)
	, m_rowNumOfBoxes(0)
	, m_scaleOfBox(0)
	, canvasDC(NULL)
	, boxes(NULL)
	, m_StartDraw(false)
	, m_StartGame(false)
	, m_ShiftPoint(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMazeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMazeDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CMazeDlg::OnBnClickedButton1)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_BUTTON2, &CMazeDlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// CMazeDlg 消息处理程序

BOOL CMazeDlg::OnInitDialog()
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
	CWnd *pCanvasWnd = GetDlgItem(IDC_CANVAS);
	canvasDC = pCanvasWnd->GetDC();
	pCanvasWnd->Detach();
	m_scaleOfBox = 10;

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMazeDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CMazeDlg::OnPaint()
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
		CWnd *pCanvasWnd = GetDlgItem(IDC_CANVAS);
		CRect rect;

		pCanvasWnd->GetWindowRect(&rect);
		ScreenToClient(rect);
		m_ShiftPoint.x = rect.left;
		m_ShiftPoint.y = rect.top;

		pCanvasWnd->GetClientRect(&rect);
		m_colNumOfBoxes = rect.Width() / m_scaleOfBox - 1;
		m_rowNumOfBoxes = rect.Height() / m_scaleOfBox - 1;

		boxes = new Box*[m_rowNumOfBoxes];
		for (int i = 0; i < m_rowNumOfBoxes; i++) {
			boxes[i] = new Box[m_colNumOfBoxes];
			for (int j = 0; j < m_colNumOfBoxes; j++) {
				boxes[i][j].top = i*m_scaleOfBox + 1;
				boxes[i][j].bottom = (i + 1)*m_scaleOfBox + 2;
				boxes[i][j].left = j*m_scaleOfBox + 1;
				boxes[i][j].right = (j + 1)*m_scaleOfBox + 2;
			}
		}
		boxes[0][0].m_isEnd = true;
		boxes[m_rowNumOfBoxes - 1][m_colNumOfBoxes - 1].m_isEnd = true;
		InitCanvas();

		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMazeDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMazeDlg::InitCanvas()
{
	CBrush brush(RGB(255, 255, 255));
	auto oldBrush = canvasDC->SelectObject(&brush);
	oldBrush->DeleteObject();
	m_StartGame = false;
	for (int i = 0; i < m_rowNumOfBoxes; i++) {
		for (int j = 0; j < m_colNumOfBoxes; j++) {
			boxes[i][j].m_isSelected = false;
			canvasDC->Rectangle(boxes[i][j]);
		}
	}
	CBrush greenBrush(RGB(0, 255, 0));
	canvasDC->FillRect(boxes[0][0].GetContent(), &greenBrush);
	canvasDC->FillRect(boxes[m_rowNumOfBoxes - 1][m_colNumOfBoxes - 1].GetContent(), &greenBrush);
}


void CMazeDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	InitCanvas();
}


void CMazeDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_StartDraw = true;

	CDialogEx::OnLButtonDown(nFlags, point);
}


void CMazeDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_StartDraw = false;

	CDialogEx::OnLButtonUp(nFlags, point);
}


void CMazeDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_StartDraw && !m_StartGame) {
		auto rowIndex = (point.y - m_ShiftPoint.y) / m_scaleOfBox;
		auto colIndex = (point.x - m_ShiftPoint.x)/m_scaleOfBox;
		UpdateBox(rowIndex, colIndex);
	}

	CDialogEx::OnMouseMove(nFlags, point);
}

void CMazeDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	ball = new Ball();
	CBrush brush(RGB(0, 0, 0));
	auto oldBrush = canvasDC->SelectObject(&brush);
	oldBrush->DeleteObject();
	canvasDC->Ellipse(boxes[0][0].left,
		boxes[0][ball->m_colIndex].top,
		boxes[0][0].right,
		boxes[0][0].bottom);
	m_StartGame = true;
}

void CMazeDlg::UpdateBox(int rowIndex, int colIndex)
{
	CBrush rBursh(RGB(255, 0, 0));
	CBrush gBursh(RGB(0, 255, 0));
	if (rowIndex >= 0 && rowIndex < m_rowNumOfBoxes &&
		colIndex >= 0 && colIndex < m_colNumOfBoxes) {
		boxes[rowIndex][colIndex].m_isSelected = true;
		if (boxes[rowIndex][colIndex].m_isEnd)
			canvasDC->FillRect(boxes[rowIndex][colIndex].GetContent(), &gBursh);
		else
			canvasDC->FillRect(boxes[rowIndex][colIndex].GetContent(), &rBursh);
	}
}

void CMazeDlg::UpdateBall(int rowMove, int colMove)
{
	if (m_StartGame) {
		int rowIndex = ball->m_rowIndex + rowMove;
		int colIndex = ball->m_colIndex + colMove;

		if (rowIndex >= 0 && rowIndex < m_rowNumOfBoxes &&
			colIndex >= 0 && colIndex < m_colNumOfBoxes) {
			CBrush blackBrush(RGB(0, 0, 0));
			auto oldBrush = canvasDC->SelectObject(&blackBrush);
			oldBrush->DeleteObject();
			UpdateBox(ball->m_rowIndex, ball->m_colIndex);
			ball->m_rowIndex = rowIndex;
			ball->m_colIndex = colIndex;
			canvasDC->Ellipse(boxes[ball->m_rowIndex][ball->m_colIndex].left,
				boxes[ball->m_rowIndex][ball->m_colIndex].top,
				boxes[ball->m_rowIndex][ball->m_colIndex].right,
				boxes[ball->m_rowIndex][ball->m_colIndex].bottom);

			int choice = -1;
			if (rowIndex == m_rowNumOfBoxes - 1 && colIndex == m_colNumOfBoxes - 1) {
				choice = MessageBox(TEXT("成功到达目的地，游戏结束！\r\n是否重新开始？"),
									TEXT("恭喜你"), MB_OKCANCEL);
			}
			else if (!boxes[rowIndex][colIndex].m_isSelected)
				choice = MessageBox(TEXT("失败，游戏结束！\r\n是否重新开始？"),
									TEXT("好遗憾"), MB_OKCANCEL);
			if (choice == IDOK) {
				InitCanvas(); return;
			}
			else if (choice == IDCANCEL)
				exit(-1);
		}
	}
}

BOOL CMazeDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN) {
		switch (pMsg->wParam) {
		case VK_UP: UpdateBall(-1, 0); break;
		case VK_DOWN: UpdateBall(1, 0); break;
		case VK_LEFT: UpdateBall(0, -1); break;
		case VK_RIGHT: UpdateBall(0, 1); break;
		}

	}
	return CDialog::PreTranslateMessage(pMsg);
}