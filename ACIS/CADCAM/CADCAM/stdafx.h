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

// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // �� Windows ͷ���ų�����ʹ�õ�����
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // ĳЩ CString ���캯��������ʽ��

// �ر� MFC ��ĳЩ�����������ɷ��ĺ��Եľ�����Ϣ������
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC ��������ͱ�׼���
#include <afxext.h>         // MFC ��չ


#include <afxdisp.h>        // MFC �Զ�����



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC �� Internet Explorer 4 �����ؼ���֧��
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC �� Windows �����ؼ���֧��
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // �������Ϳؼ����� MFC ֧��









#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif



// standard includes
#include <assert.h>

// HOOPS includes
#include "hc.h"

// Acis kernel includes
#include "base.hxx"  
#include "logical.h"  
#include "box.hxx"
#include "position.hxx"
#include "transf.hxx"
#include "unitvec.hxx"
#include "body.hxx"
#include "boolapi.hxx"

#include "raytest.hxx"
#include "intrapi.hxx" 
#include <algorithm>
#include "kernapi.hxx"
#include "kernopts.hxx"
#include "acis.hxx"  
#include "acistype.hxx"  
#include "attrib.hxx"
#include "entity.hxx"
#include "cstrapi.hxx"
#include "curve.hxx"
#include "surface.hxx"
#include "point.hxx"
#include "transfrm.hxx"
#include "body.hxx"
#include "shell.hxx"
#include "face.hxx"
#include "edge.hxx"
#include "vertex.hxx"
#include "surdef.hxx"
#include "bulletin.hxx"
#include "ckoutcom.hxx"
#include "rgbcolor.hxx"
// HOOPS-Acis bridge includes
#include "ha_bridge.h"
#include <afxdlgs.h>
#include "ga_api.hxx"

#include "skinapi.hxx"
#include "spline.hxx"
#include "skin_opts.hxx"
/*#include "extend_entity.h"*/
#include "curextnd.hxx"
#include "curve.hxx"
#include "curdef.hxx"

#include <iostream>
#include <fstream>
#include <ostream>
#include <stdlib.h>
#include <vector>


//ACIS include
#include "ctapi.hxx"
#include "base.hxx"  
#include "logical.h"  
#include "box.hxx"
#include "position.hxx"
#include "transf.hxx"
#include "unitvec.hxx"

#include "boolapi.hxx"

#include "raytest.hxx"
#include "intrapi.hxx" 

#include "kernapi.hxx"
#include "kernopts.hxx"
#include "acis.hxx"  
#include "acistype.hxx"  
#include "attrib.hxx"
#include "entity.hxx"

#include "curve.hxx"
#include "surface.hxx"
#include "point.hxx"
#include "transfrm.hxx"
#include "body.hxx"
#include "shell.hxx"
#include "face.hxx"
#include "edge.hxx"
#include "vertex.hxx"
#include "surdef.hxx"
#include "bulletin.hxx"
#include "ckoutcom.hxx"

#include "cstrapi.hxx"
#include "part_api.hxx"
#include "ga_api.hxx"
#include "rnd_api.hxx"
#include "ctapi.hxx"

#include "coedge.hxx"
#include "skinapi.hxx"
#include "skin.hxx"
#include "skin_opts.hxx"
#include "law_base.hxx"
#include "main_law.hxx"

#include "sweepapi.hxx"
#include "coverapi.hxx"
#include "surdef.hxx"
#include "cover_options.hxx"

#include "sp3crtn.hxx"
#include "sps3crtn.hxx"

#include "loop.hxx"
#include "lump.hxx"
#include "wire.hxx"
#include "coedge.hxx"

#include "rgbcolor.hxx"
#include "ofstapi.hxx"

#include "blendapi.hxx"
#include "cone.hxx"
#include "intcurve.hxx"
#include "ellipse.hxx"

//sun-define
//const	int	RESULT_OK = 0;
//const	int	RESULT_ERR = 1;
//const	int	RESULT_WARNING = 2;



#include "stchapi.hxx"
//���ڷ�ϲ���

#include "SPAIConverter.h"
#include "SPAIDocument.h"
#include "SPAIResult.h"
#include "SPAIAcisDocument.h"
#include "SPAIDocument.h"
#include "SPAIAcisDocument.h"
#include "SPAIConverter.h"
#include "SPAIFile.h"
#include "SPAIOptions.h"
#include "SPAIOptionName.h"
#include "SPAIResult.h"
#include "SPAIValue.h"
#include "SPAIUnit.h"

#include "stchapi.hxx"

//#include "StringConver.h"



/////////
#include "GlobalFunctions.h"