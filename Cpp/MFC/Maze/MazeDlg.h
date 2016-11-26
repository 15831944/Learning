
// MazeDlg.h : 头文件
//

#pragma once
#include "Box.h"
#include "atltypes.h"
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
	int m_colNumOfBoxes;
	int m_rowNumOfBoxes;
	int m_scaleOfBox;
	CDC* canvasDC;
	void InitCanvas();
	Box** boxes;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	bool m_StartDraw;
	bool m_StartGame;
	CPoint m_ShiftPoint;
	afx_msg void OnBnClickedButton2();
	Ball* ball;
	void UpdateBox(int rowIndex, int colIndex);
	void UpdateBall(int rowIndex, int colIndex);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	BOOL PreTranslateMessage(MSG * pMsg);
};
