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

// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // 从 Windows 头中排除极少使用的资料
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 某些 CString 构造函数将是显式的

// 关闭 MFC 对某些常见但经常可放心忽略的警告消息的隐藏
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC 核心组件和标准组件
#include <afxext.h>         // MFC 扩展


#include <afxdisp.h>        // MFC 自动化类



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC 对 Internet Explorer 4 公共控件的支持
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC 对 Windows 公共控件的支持
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // 功能区和控件条的 MFC 支持









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
//用于缝合操作

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