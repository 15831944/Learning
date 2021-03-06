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

// CADCAMDoc.cpp : CCADCAMDoc 类的实现
//

#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
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


// CCADCAMDoc 构造/析构

CCADCAMDoc::CCADCAMDoc()
{
	// TODO: 在此添加一次性构造代码
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

	// TODO: 在此添加重新初始化代码
	// (SDI 文档将重用该文档)	
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




// CCADCAMDoc 序列化

void CCADCAMDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: 在此添加存储代码
	}
	else
	{
		// TODO: 在此添加加载代码
	}
}

#ifdef SHARED_HANDLERS

// 缩略图的支持
void CCADCAMDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// 修改此代码以绘制文档数据
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

// 搜索处理程序的支持
void CCADCAMDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// 从文档数据设置搜索内容。
	// 内容部分应由“;”分隔

	// 例如:     strSearchContent = _T("point;rectangle;circle;ole object;")；
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

// CCADCAMDoc 诊断

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


// CCADCAMDoc 命令


BOOL CCADCAMDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CHoopsDoc::OnOpenDocument(lpszPathName))
		return FALSE;

	// TODO:  在此添加您专用的创建代码
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
