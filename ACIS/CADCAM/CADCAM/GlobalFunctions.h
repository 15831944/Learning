#pragma once


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
BOOL IgsToSat(TCHAR *szIgesFilePath, TCHAR *szSatFilePath, BOOL &bSrcIsSat);