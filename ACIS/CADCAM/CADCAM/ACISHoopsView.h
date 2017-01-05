// ��� MFC ʾ��Դ������ʾ���ʹ�� MFC Microsoft Office Fluent �û����� 
// (��Fluent UI��)����ʾ�������ο���
// ���Բ��䡶Microsoft ������ο����� 
// MFC C++ ������渽����ص����ĵ���
// ���ơ�ʹ�û�ַ� Fluent UI ����������ǵ����ṩ�ġ�
// ��Ҫ�˽��й� Fluent UI ��ɼƻ�����ϸ��Ϣ�������  
// http://msdn.microsoft.com/officeui��
//
// ��Ȩ����(C) Microsoft Corporation
// ��������Ȩ����

// HoopsTemplateView.h : CHoopsTemplateView ��Ľӿ�
//

#pragma once
#include "ACISHoopsDoc.h"
#include "CHoopsView.h"

class CACISHoopsView : public CHoopsView
{
protected: // �������л�����
	CACISHoopsView();
	DECLARE_DYNCREATE(CACISHoopsView)

public:
	
	bool m_FaceVisibility;
	bool m_EdgeVisibility;
	bool m_VerticeVisibility;
	bool m_bLightVisibility;

// ����
public:
	CACISHoopsDoc* GetDocument() const;

// ����
public:
	unsigned long MapFlags(unsigned long state);

	void SetWindowColor(COLORREF new_top_color, COLORREF new_bottom_color, bool emit_message = true);
	void SetTransparency();

// ��д
public:
	virtual void OnDraw(CDC* pDC);  // ��д�Ի��Ƹ���ͼ
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	void LocalSetOperator(HBaseOperator *NewOperator);
// ʵ��
public:
//	HBaseView* m_pHBaseView;
	virtual ~CACISHoopsView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:


	COLORREF  m_rc;
// ���ɵ���Ϣӳ�亯��
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

#ifndef _DEBUG  // HoopsTemplateView.cpp �еĵ��԰汾
inline CACISHoopsDoc* CACISHoopsView::GetDocument() const
   { return reinterpret_cast<CACISHoopsDoc*>(m_pDocument); }
#endif

