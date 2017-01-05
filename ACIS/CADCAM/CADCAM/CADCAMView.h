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

// CADCAMView.h : CCADCAMView ��Ľӿ�
//

#pragma once

#include "CADCAMDoc.h"
#include "CHoopsView.h"

class CCADCAMView : public CHoopsView
{
protected: // �������л�����
	CCADCAMView();
	DECLARE_DYNCREATE(CCADCAMView)

public:
	bool m_FaceVisibility;
	bool m_EdgeVisibility;
	bool m_VerticeVisibility;
	bool m_bLightVisibility;

	COLORREF  m_rc;

// ����
public:
	CCADCAMDoc* GetDocument() const;

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
	virtual ~CCADCAMView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

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
	afx_msg void OnAroundAxis();
	afx_msg void OnZoomtoextents();
	afx_msg void OnZoomtowindow();




};

#ifndef _DEBUG  // CADCAMView.cpp �еĵ��԰汾
inline CCADCAMDoc* CCADCAMView::GetDocument() const
   { return reinterpret_cast<CCADCAMDoc*>(m_pDocument); }
#endif

