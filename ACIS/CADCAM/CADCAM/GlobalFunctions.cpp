#include "stdafx.h"
#include "GlobalFunctions.h"


//************************************
// Function: 将后缀名为igs的文件转换成sat的文件
// Method:    IgsToSat
// FullName:  CWireFrameSimulationDlg::IgsToSat
// Access:    public 
// Returns:   BOOL
// Qualifier:
// Parameter: TCHAR * szIgesFilePath 传入参数 后缀为iges文件的全路径
// Parameter: TCHAR * szSatFilePath  传出参数 转后成后缀为sat的文件的全路径
// Parameter: BOOL & bSrcIsSat  传出参数 如果源文件是sat文件则为TRUE， 否则为FALSE
//************************************
BOOL IgsToSat(TCHAR *szIgesFilePath, TCHAR *szSatFilePath, BOOL &bSrcIsSat)
{
	TCHAR szFileType[32];
	TCHAR szFileName[256];
	TCHAR szDiver[12];
	TCHAR szFileParentPath[512];
	CHAR szTempIgesA[512];
	CHAR szTempSatA[512];

	memset(szFileType, 0, 32);
	memset(szFileName, 0, 256);
	memset(szFileParentPath, 0, 512);
	memset(szTempIgesA, 0, 512);
	memset(szTempSatA, 0, 512);
	memset(szDiver, 0, 12);

	if (szIgesFilePath == NULL || szSatFilePath == NULL)
	{
		return FALSE;
	}

	//获取三维模型文件的后缀
	_tsplitpath_s(szIgesFilePath, szDiver, 12, szFileParentPath, 512, szFileName, 256, szFileType, 32);

	//如果三维文件本身就是.sat文件，则直接拷贝。
	if (0 == _tcscmp(szFileType, _T(".sat")) || 0 == _tcscmp(szFileType, _T(".SAT")))
	{
		_tcscpy(szSatFilePath, szIgesFilePath);
		bSrcIsSat = TRUE;
		return  TRUE;
	}

	_tcscpy(szSatFilePath, szDiver);
	_tcscat(szSatFilePath, szFileParentPath);
	_tcscat(szSatFilePath, szFileName);
	_tcscat(szSatFilePath, _T(".sat"));

	//Unicode转换成多字节的字符串
	WideCharToMultiByte(CP_ACP, 0, szIgesFilePath, 512, szTempIgesA, 512, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, szSatFilePath, 512, szTempSatA, 512, NULL, NULL);
	SPAIDocument  src;
	SPAIDocument  dst;
	SPAIConverter converter;
	SPAIResult result;

	src.SetFilePath(szTempIgesA);
	dst.SetFilePath(szTempSatA);

	//文件类型转换， iges 或者igs后缀的三维模型文件转换成 sat后缀的ACIS文件
	result = converter.Convert(src, dst);
	if (!result.IsSuccess())
	{
		return FALSE;
	}
	bSrcIsSat = FALSE;
	return TRUE;
}