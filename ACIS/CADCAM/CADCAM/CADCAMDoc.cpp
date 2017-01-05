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

// CADCAMDoc.cpp : CCADCAMDoc ���ʵ��
//

#include "stdafx.h"
// SHARED_HANDLERS ������ʵ��Ԥ��������ͼ������ɸѡ�������
// ATL ��Ŀ�н��ж��壬�����������Ŀ�����ĵ����롣
#ifndef SHARED_HANDLERS
#include "CADCAM.h"
#endif

#include "CADCAMDoc.h"

////   add by lu
#include "HHOOPSModel.h"
#include "HBaseModel.h"
#include "HUtility.h"
#include "hc.h"
//////////////////////////////////

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CCADCAMDoc

IMPLEMENT_DYNCREATE(CCADCAMDoc, CDocument)

BEGIN_MESSAGE_MAP(CCADCAMDoc, CDocument)
END_MESSAGE_MAP()


// CCADCAMDoc ����/����

CCADCAMDoc::CCADCAMDoc()
{
	// TODO: �ڴ����һ���Թ������
	m_pHoopsModel = NULL;

}

CCADCAMDoc::~CCADCAMDoc()
{
	if (m_pHoopsModel)
	{
		delete m_pHoopsModel;
		m_pHoopsModel = NULL;
	}
}

BOOL CCADCAMDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: �ڴ�������³�ʼ������
	// (SDI �ĵ������ø��ĵ�)	
	if (m_pHoopsModel)
	{
		delete m_pHoopsModel;
		m_pHoopsModel = NULL;
	}

	m_pHoopsModel = new HHOOPSModel();
	((HHOOPSModel *)m_pHoopsModel)->Init();

	if (!m_pHoopsModel)
		return false;

	return TRUE;
}




// CCADCAMDoc ���л�

void CCADCAMDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: �ڴ���Ӵ洢����
	}
	else
	{
		// TODO: �ڴ���Ӽ��ش���
	}
}

#ifdef SHARED_HANDLERS

// ����ͼ��֧��
void CCADCAMDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// �޸Ĵ˴����Ի����ĵ�����
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// ������������֧��
void CCADCAMDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// ���ĵ����������������ݡ�
	// ���ݲ���Ӧ�ɡ�;���ָ�

	// ����:     strSearchContent = _T("point;rectangle;circle;ole object;")��
	SetSearchContent(strSearchContent);
}

void CCADCAMDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CCADCAMDoc ���

#ifdef _DEBUG
void CCADCAMDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CCADCAMDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CCADCAMDoc ����


BOOL CCADCAMDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CHoopsDoc::OnOpenDocument(lpszPathName))
		return FALSE;

	// TODO:  �ڴ������ר�õĴ�������
	if (m_pHoopsModel)
	{
		delete m_pHoopsModel;
		m_pHoopsModel = NULL;
	}

	// create a new HstartModel object for this Document
	m_pHoopsModel = new HHOOPSModel();
	((HHOOPSModel *)m_pHoopsModel)->Init();

	//if (((HHOOPS10Model *)m_pHBaseModel)->Read(H_ASCII_TEXT(lpszPathName),m_pHBaseModel->GetModelKey()) != InputOK)
	if (((HHOOPSModel *)m_pHoopsModel)->Read(H_ASCII_TEXT(lpszPathName)) != InputOK)
		return FALSE;

	m_pHoopsModel->Update();



	return TRUE;
}
