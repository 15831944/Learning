
// MouseEvent.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CMouseEventApp: 
// �йش����ʵ�֣������ MouseEvent.cpp
//

class CMouseEventApp : public CWinApp
{
public:
	CMouseEventApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CMouseEventApp theApp;