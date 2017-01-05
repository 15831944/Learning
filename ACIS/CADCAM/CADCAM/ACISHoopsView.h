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

// HoopsTemplateView.h : CHoopsTemplateView 类的接口
//

#pragma once
#include "ACISHoopsDoc.h"
#include "CHoopsView.h"

class CACISHoopsView : public CHoopsView
{
protected: // 仅从序列化创建
	CACISHoopsView();
	DECLARE_DYNCREATE(CACISHoopsView)

public:
	
	bool m_FaceVisibility;
	bool m_EdgeVisibility;
	bool m_VerticeVisibility;
	bool m_bLightVisibility;

// 特性
public:
	CACISHoopsDoc* GetDocument() const;

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
//	HBaseView* m_pHBaseView;
	virtual ~CACISHoopsView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:


	COLORREF  m_rc;
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
	afx_msg void OnButtonFaceVisiable();
	afx_msg void OnButtonEdgeVisiable();
	afx_msg void OnButtonVertexVisiable();
	afx_msg void OnUpdateButtonFaceUpdate(CCmdUI *pCmdUI);
	afx_msg void OnUpdateButtonEdgeUpdate(CCmdUI *pCmdUI);
	afx_msg void OnUpdateButtonVertexUpdate(CCmdUI *pCmdUI);
	afx_msg void OnAroundaxis();
	afx_msg void OnShaded();
	afx_msg void OnHiddenline();
	afx_msg void OnRendermodeBrepwireframe();
	afx_msg void OnFont();
	afx_msg void OnUpdateFont(CCmdUI *pCmdUI);
	afx_msg void OnBack();
	afx_msg void OnUpdateBack(CCmdUI *pCmdUI);
	afx_msg void OnTop();
	afx_msg void OnUpdateTop(CCmdUI *pCmdUI);
	afx_msg void OnButtom();
	afx_msg void OnUpdateButtom(CCmdUI *pCmdUI);
	afx_msg void OnLeft();
	afx_msg void OnUpdateLeft(CCmdUI *pCmdUI);
	afx_msg void OnRight();
	afx_msg void OnUpdateRight(CCmdUI *pCmdUI);
	afx_msg void OnIsometric();
	afx_msg void OnUpdateIsometric(CCmdUI *pCmdUI);
	afx_msg void OnSelectByWindow();
	afx_msg void OnDeleteSelect();
	afx_msg void OnZoomtoextents();
	afx_msg void OnZoomToWindow();
	afx_msg void OnModifyBackgroundColor();
	afx_msg void OnModifyEntitycolor();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnVisibileVertex();
	afx_msg void OnVisibileEdge();
	afx_msg void OnVisibileFace();


	afx_msg void OnSellevelVertex();
	afx_msg void OnSellevelEdge();
	afx_msg void OnSellevelFace();

	afx_msg void OnVisibilityLights();
	afx_msg void OnSellevelBody();
	afx_msg void OnSellevelAssembly();
	afx_msg void OnButtonClear();
};

#ifndef _DEBUG  // HoopsTemplateView.cpp 中的调试版本
inline CACISHoopsDoc* CACISHoopsView::GetDocument() const
   { return reinterpret_cast<CACISHoopsDoc*>(m_pDocument); }
#endif

