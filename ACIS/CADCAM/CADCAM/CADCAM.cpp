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

// CADCAM.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "CADCAM.h"
#include "MainFrm.h"

#include "CADCAMDoc.h"
#include "CADCAMView.h"


/////   add by lu
#include "HDB.h"
#include "CTDriver.h"
#include "license.hxx"
#include "spa_unlock_result.hxx"

#include "ha_bridge.h"
#include "part_api.hxx"

#include "visualize_license.h"
//////////////////////////////



#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCADCAMApp

BEGIN_MESSAGE_MAP(CCADCAMApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CCADCAMApp::OnAppAbout)
	// �����ļ��ı�׼�ĵ�����
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
	// ��׼��ӡ��������
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)
END_MESSAGE_MAP()


// CCADCAMApp ����

CCADCAMApp::CCADCAMApp()
{
	//////   add by lu
	HDB::EnableErrorManager();
	m_pHDB = NULL;
	m_pCTDriver = NULL;
	////////////////////////////////////////


	m_bHiColorIcons = TRUE;

	// ֧����������������
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
#ifdef _MANAGED
	// ���Ӧ�ó��������ù�����������ʱ֧��(/clr)�����ģ���: 
	//     1) �����д˸������ã�������������������֧�ֲ�������������
	//     2) ��������Ŀ�У������밴������˳���� System.Windows.Forms ������á�
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// TODO: ������Ӧ�ó��� ID �ַ����滻ΪΨһ�� ID �ַ�����������ַ�����ʽ
	//Ϊ CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("CADCAM.AppID.NoVersion"));

	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}

CCADCAMApp::~CCADCAMApp()
{
	if (m_pHDB)
	{
		delete m_pHDB;
		m_pHDB = NULL;
	}
}

// Ψһ��һ�� CCADCAMApp ����

CCADCAMApp theApp;

/////////    add  by lu
void unlock_spatial_products_8036();


// CCADCAMApp ��ʼ��

BOOL CCADCAMApp::InitInstance()
{
	///////  add by lu
	HC_Define_System_Options("license = `" VISUALIZE_LICENSE "`");

	m_pHDB = new HDB();
	m_pHDB->Init();

	m_pCTDriver = new CTDriver(10);
	m_pCTDriver->StartTimer();

	char fontDirectory[MAX_PATH + 32];
	::GetWindowsDirectoryA(fontDirectory, MAX_PATH);
	strcat(fontDirectory, "\\Fonts");
	char buf[MAX_PATH + 64] = { "" };
	sprintf(buf, "font directory = %s", fontDirectory);
	HC_Define_System_Options(buf);

	base_configuration base_config;
	logical ok = initialize_base(&base_config);


	unlock_spatial_products_8036();
	check_outcome(api_start_modeller(0));
	check_outcome(api_initialize_hoops_acis_bridge());
	check_outcome(api_initialize_part_manager());
	//////////////////////////////


	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControlsEx()��  ���򣬽��޷��������ڡ�
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õ�
	// �����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();


	// ��ʼ�� OLE ��
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction(FALSE);

	// ʹ�� RichEdit �ؼ���Ҫ AfxInitRichEdit2()	
	// AfxInitRichEdit2();

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));
	LoadStdProfileSettings(4);  // ���ر�׼ INI �ļ�ѡ��(���� MRU)


	InitContextMenuManager();
	InitShellManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// ע��Ӧ�ó�����ĵ�ģ�塣  �ĵ�ģ��
	// �������ĵ�����ܴ��ں���ͼ֮�������
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CCADCAMDoc),
		RUNTIME_CLASS(CMainFrame),       // �� SDI ��ܴ���
		RUNTIME_CLASS(CCADCAMView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);


	// ������׼ shell ���DDE�����ļ�������������
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);



	// ��������������ָ�������  ���
	// �� /RegServer��/Register��/Unregserver �� /Unregister ����Ӧ�ó����򷵻� FALSE��
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// Ψһ��һ�������ѳ�ʼ���������ʾ����������и���
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	return TRUE;
}

int CCADCAMApp::ExitInstance()
{
	//TODO: �����������ӵĸ�����Դ
	////////  add by lu
	if (m_pCTDriver)
	{
		delete m_pCTDriver;
		m_pCTDriver = NULL;
	}

	if (m_pHDB)
	{
		delete m_pHDB;
		m_pHDB = NULL;

		api_terminate_part_manager();
		api_terminate_hoops_acis_bridge();
		api_stop_modeller();
		terminate_base();
	}
	///////////////////////

	AfxOleTerm(FALSE);

	return CWinAppEx::ExitInstance();
}

// CCADCAMApp ��Ϣ�������


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// �������жԻ����Ӧ�ó�������
void CCADCAMApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CCADCAMApp �Զ������/���淽��

void CCADCAMApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
}

void CCADCAMApp::LoadCustomState()
{
}

void CCADCAMApp::SaveCustomState()
{
}

// CCADCAMApp ��Ϣ�������



