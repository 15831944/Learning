#pragma once


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
BOOL IgsToSat(TCHAR *szIgesFilePath, TCHAR *szSatFilePath, BOOL &bSrcIsSat);