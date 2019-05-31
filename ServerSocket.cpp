// ServerSocket.cpp : Defines the class behaviors for the application.
// Download by http://www.srcfans.com

#include "stdafx.h"
#include "ServerSocket.h"
#include "ServerSocketDlg.h"
#include "ClientSocketDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define WSA_VERSION  MAKEWORD(2,0)
#define MAX_HOSTNAME 256
#define MAX_HOSTADDR 40

/////////////////////////////////////////////////////////////////////////////
// CServerSocketApp

BEGIN_MESSAGE_MAP(CServerSocketApp, CWinApp)
	//{{AFX_MSG_MAP(CServerSocketApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CServerSocketApp construction

CServerSocketApp::CServerSocketApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	m_nLinkMode = 0; // server
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CServerSocketApp object

CServerSocketApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CServerSocketApp initialization

BOOL CServerSocketApp::InitInstance()
{
	WSADATA		WSAData = { 0 };
	//����winsock
	if ( 0 != WSAStartup( WSA_VERSION, &WSAData ) )
	{
		// ����д�������ʾ
		// WinSock DLL.
		if ( LOBYTE( WSAData.wVersion ) != LOBYTE(WSA_VERSION) ||
			 HIBYTE( WSAData.wVersion ) != HIBYTE(WSA_VERSION) )
			 ::MessageBox(NULL, _T("Incorrect version of Winsock.dll found"), _T("Error"), MB_OK);
		//�ر����
		WSACleanup( );
		return FALSE;
	}

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	//���������в��������Ϊ/s���ʾ����������
	//���Ϊ/c�����ʾ�ͻ��˳���
	ParseCommandLineArgs();

	//���������Ի������
	CServerSocketDlg dlg1;
	CClientSocketDlg dlg2;
	switch( m_nLinkMode )
	{
		default:
		case 0:
		m_pMainWnd = &dlg1; // ������
			break;
		case 1:
		m_pMainWnd = &dlg2; // �ͻ���
			break;
	}


	//�����Ի���
	int nResponse = ((CDialog*)m_pMainWnd)->DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		// dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

int CServerSocketApp::ExitInstance() 
{
	// Terminate use of the WS2_32.DLL
	//ж��winsock��
	WSACleanup();
	
	return CWinApp::ExitInstance();
}

void CServerSocketApp::ParseCommandLineArgs()
{
	//���������
	CString strCmdLine = (LPCTSTR) GetCommandLine();

	if (!strCmdLine.IsEmpty())
	{
		//ȫ��ת���ɴ�д
		strCmdLine.MakeUpper();
		int nPos = 0;
		do {
			//���ҿո�
			nPos = strCmdLine.Find(TCHAR(' '));
			if (nPos>0)
			{
				//ɾ����һ���ո�ǰ���ַ�
				strCmdLine.Delete( 0, nPos+1);
				CString strCurrent = strCmdLine;
				//���ҵڶ����ո�
				int nNextPos = strCmdLine.Find(TCHAR(' '));
				if (nNextPos > 0)
					strCurrent = strCmdLine.Left( nNextPos );
				//��������в�����/SERVER������/S�����趨ģʽ��0
				if (strCurrent == _T("/SERVER") || strCurrent == _T("/S"))
					m_nLinkMode = 0;
				//�����趨Ϊ1
				else if (strCurrent == _T("/CLIENT") || strCurrent == _T("/C"))
					m_nLinkMode = 1;
			}
		} while( nPos != -1);
	}
}
