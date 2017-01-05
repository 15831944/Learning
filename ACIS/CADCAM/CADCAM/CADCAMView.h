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

// CADCAMView.h : CCADCAMView 类的接口
//

#pragma once

#include "CADCAMDoc.h"
#include "CHoopsView.h"

class CCADCAMView : public CHoopsView
{
protected: // 仅从序列化创建
	CCADCAMView();
	DECLARE_DYNCREATE(CCADCAMView)

public:
	bool m_FaceVisibility;
	bool m_EdgeVisibility;
	bool m_VerticeVisibility;
	bool m_bLightVisibility;

	COLORREF  m_rc;

// 特性
public:
	CCADCAMDoc* GetDocument() const;

// 操作
public:
	unsigned long MapFlags(unsigned long state);

	void SetWindowColor(COLORREF new_top_color, COLORREF new_bottom_color, bool emit_message = true);
	void SetTransparency();

// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

	void LocalSetOperator(HBaseOperator *NewOperator);

// 实现
public:
	virtual ~CCADCAMView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnOrbit();
	afx_msg void OnPan();
	afx_msg void OnZoom();
	afx_msg void OnAroundAxis();
	afx_msg void OnZoomtoextents();
	afx_msg void OnZoomtowindow();




};

#ifndef _DEBUG  // CADCAMView.cpp 中的调试版本
inline CCADCAMDoc* CCADCAMView::GetDocument() const
   { return reinterpret_cast<CCADCAMDoc*>(m_pDocument); }
#endif

