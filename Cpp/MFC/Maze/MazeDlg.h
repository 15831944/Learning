
// MazeDlg.h : 头文件
//

#pragma once
#include "atltypes.h"
#include "Box.h"
#include "Ball.h"

// CMazeDlg 对话框
class CMazeDlg : public CDialogEx
{
// 构造
public:
	CMazeDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MAZE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CDC* canvasDC;
	bool m_StartDraw;
	bool m_StartGame;
	CPoint m_ShiftPoint;

	Box** boxes;
	int m_colNumOfBoxes;
	int m_rowNumOfBoxes;
	int m_scaleOfBox;
	Ball* ball;
	
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

	void InitCanvas();
	void UpdateBox(int rowIndex, int colIndex);
	void UpdateBall(int rowIndex, int colIndex);
	
	BOOL PreTranslateMessage(MSG * pMsg);
};
