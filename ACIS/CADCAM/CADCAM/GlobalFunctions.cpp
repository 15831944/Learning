#include "stdafx.h"
#include "GlobalFunctions.h"


//************************************
// Function: ����׺��Ϊigs���ļ�ת����sat���ļ�
// Method:    IgsToSat
// FullName:  CWireFrameSimulationDlg::IgsToSat
// Access:    public 
// Returns:   BOOL
// Qualifier:
// Parameter: TCHAR * szIgesFilePath ������� ��׺Ϊiges�ļ���ȫ·��
// Parameter: TCHAR * szSatFilePath  �������� ת��ɺ�׺Ϊsat���ļ���ȫ·��
// Parameter: BOOL & bSrcIsSat  �������� ���Դ�ļ���sat�ļ���ΪTRUE�� ����ΪFALSE
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

	//��ȡ��άģ���ļ��ĺ�׺
	_tsplitpath_s(szIgesFilePath, szDiver, 12, szFileParentPath, 512, szFileName, 256, szFileType, 32);

	//�����ά�ļ��������.sat�ļ�����ֱ�ӿ�����
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

	//Unicodeת���ɶ��ֽڵ��ַ���
	WideCharToMultiByte(CP_ACP, 0, szIgesFilePath, 512, szTempIgesA, 512, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, szSatFilePath, 512, szTempSatA, 512, NULL, NULL);
	SPAIDocument  src;
	SPAIDocument  dst;
	SPAIConverter converter;
	SPAIResult result;

	src.SetFilePath(szTempIgesA);
	dst.SetFilePath(szTempSatA);

	//�ļ�����ת���� iges ����igs��׺����άģ���ļ�ת���� sat��׺��ACIS�ļ�
	result = converter.Convert(src, dst);
	if (!result.IsSuccess())
	{
		return FALSE;
	}
	bSrcIsSat = FALSE;
	return TRUE;
}