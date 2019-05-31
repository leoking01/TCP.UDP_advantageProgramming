// ServerSocketDlg.cpp : implementation file
// Download by http://www.srcfans.com

#include "stdafx.h"
#include <atlconv.h>
#include "ServerSocket.h"
#include "ServerSocketDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const int SOCK_TCP	= 0;
const int SOCK_UDP  = 1;

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CServerSocketDlg dialog

CServerSocketDlg::CServerSocketDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CServerSocketDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CServerSocketDlg)
	m_strPort = _T("2000");
	m_nSockType = SOCK_TCP;	// default TCP
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CServerSocketDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CServerSocketDlg)
	DDX_Control(pDX, IDC_TXT_MESSAGE, m_ctlMessage);
	DDX_Control(pDX, IDC_MESSAGE_LIST, m_ctlMsgList);
	DDX_Control(pDX, IDC_SVR_PORTINC, m_ctlPortInc);
	DDX_Text(pDX, IDC_SVR_PORT, m_strPort);
	DDX_Radio(pDX, IDC_TCP, m_nSockType);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CServerSocketDlg, CDialog)
	//{{AFX_MSG_MAP(CServerSocketDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_START, OnBtnStart)
	ON_BN_CLICKED(IDC_BTN_STOP, OnBtnStop)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_SEND, OnBtnSend)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_UPDATE_CONNECTION, OnUpdateConnection)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CServerSocketDlg message handlers


//��Ϣ����
BOOL CServerSocketDlg::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN)
	{
		int nVirtKey = (int) pMsg->wParam;
		if (nVirtKey == VK_ESCAPE)
			return TRUE;
		if (nVirtKey == VK_RETURN && (GetFocus()->m_hWnd  == m_ctlMessage.m_hWnd))
		{
			if (m_pCurServer->IsOpen())
				OnBtnSend();
			return TRUE;
		}
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}

///////////////////////////////////////////////////////////////////////////////
// PickNextAvailable : this is useful only for TCP socket�ú���ֻ��TCPʹ��

//�����һ���ͻ�����
void CServerSocketDlg::PickNextAvailable()
{
	m_pCurServer = NULL;
	for(int i=0; i<MAX_CONNECTION; i++)
	{
		if (!m_SocketManager[i].IsOpen())
		{
			m_pCurServer = &m_SocketManager[i];
			break;
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
// ����������
bool CServerSocketDlg::StartServer()
{
	bool bSuccess = false;
	//�жϷ������Ƿ�Ϊ��
	if (m_pCurServer != NULL)
	{
		//�����TCP����
		if (m_nSockType == SOCK_TCP)
		{
			// no smart addressing - we use connection oriented
			//���趨smart��ַ
			m_pCurServer->SetSmartAddressing( false );
			//����TCP socket
			bSuccess = m_pCurServer->CreateSocket( m_strPort, AF_INET, SOCK_STREAM, SO_REUSEADDR); // TCP
		}
		//�����UDP����
		else
		{
			m_pCurServer->SetSmartAddressing( true );
			//����UDP socket
			bSuccess = m_pCurServer->CreateSocket( m_strPort, AF_INET, SOCK_DGRAM, SO_BROADCAST); // UDP
		}
		//�ж��Ƿ񴴽��ɹ�
		if (bSuccess && m_pCurServer->WatchComm())
		{
			//�趨��ؿؼ�����
			GetDlgItem(IDC_BTN_SEND)->EnableWindow( TRUE );
			GetDlgItem(IDC_BTN_STOP)->EnableWindow( TRUE );
			NextDlgCtrl();
			GetDlgItem(IDC_BTN_START)->EnableWindow( FALSE );
			GetDlgItem(IDC_TCP)->EnableWindow( FALSE );
			GetDlgItem(IDC_UDP)->EnableWindow( FALSE );
			CString strServer, strAddr;
			//��ñ��ػ�����
			m_pCurServer->GetLocalName( strServer.GetBuffer(256), 256);
			strServer.ReleaseBuffer();
			//��ñ���IP��ַ
			m_pCurServer->GetLocalAddress( strAddr.GetBuffer(256), 256);
			strAddr.ReleaseBuffer();
			CString strMsg = _T("������: ") + strServer;
					strMsg += _T(", @IP��ַ: ") + strAddr;
					strMsg += _T(" �����ڶ˿� ") + m_strPort + CString("\r\n");
			m_pCurServer->AppendMessage( strMsg );
		}
	}

	return bSuccess;
}

//�Ի����ʼ��
BOOL CServerSocketDlg::OnInitDialog()
{
	ASSERT( GetDlgItem(IDC_BTN_SEND) != NULL );
	ASSERT( GetDlgItem(IDC_BTN_START) != NULL );
	ASSERT( GetDlgItem(IDC_BTN_STOP) != NULL );

	CDialog::OnInitDialog();

	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	SetIcon(m_hIcon, TRUE);			// �趨��ͼ��
	SetIcon(m_hIcon, FALSE);		// �趨Сͼ��
	
	//���������ĳ�ʼ������
	m_ctlPortInc.SetRange32( 2000, 4500);
	GetDlgItem(IDC_BTN_SEND)->EnableWindow( FALSE );
	GetDlgItem(IDC_BTN_STOP)->EnableWindow( FALSE );

	//�趨������ӵ�����
	for(int i=0; i<MAX_CONNECTION; i++)
	{
		m_SocketManager[i].SetMessageWindow( &m_ctlMsgList );
		m_SocketManager[i].SetServerState( true );	// ��Ϊ����������
	}

	PickNextAvailable();

	return TRUE;  
}

void CServerSocketDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CServerSocketDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

///////////////////////////////////////////////////////////////////////////////
// OnUpdateConnection
// This message is sent by server manager to indicate connection status

//�¼�����
LRESULT CServerSocketDlg::OnUpdateConnection(WPARAM wParam, LPARAM lParam)
{
	UINT uEvent = (UINT) wParam;
	CSocketManager* pManager = reinterpret_cast<CSocketManager*>( lParam );

	// ���ʽ����TCP���򷵻�
	if (m_nSockType != SOCK_TCP)
		return 0L;

	if ( pManager != NULL)
	{
		// ������socket�Ѿ�������
		if (uEvent == EVT_CONSUCCESS)
		{
			PickNextAvailable();
			StartServer();
		}
		else if (uEvent == EVT_CONFAILURE || uEvent == EVT_CONDROP)
		{
			pManager->StopComm();
			if (m_pCurServer == NULL)
			{
				PickNextAvailable();
				StartServer();
			}
		}
	}

	return 1L;
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CServerSocketDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

//��������ť�¼�
void CServerSocketDlg::OnBtnStart() 
{
	//���ݸ���
	UpdateData();
	StartServer();
}

void CServerSocketDlg::OnBtnStop() 
{
	// �Ͽ�������
	m_pCurServer->StopComm();
	if (!m_pCurServer->IsOpen())
	{
		GetDlgItem(IDC_BTN_START)->EnableWindow( TRUE );
		PrevDlgCtrl();
		GetDlgItem(IDC_BTN_STOP)->EnableWindow( FALSE );
		GetDlgItem(IDC_TCP)->EnableWindow( TRUE );
		GetDlgItem(IDC_UDP)->EnableWindow( TRUE );
	}
}

//���Ͱ�ť�����¼���������������
void CServerSocketDlg::OnBtnSend() 
{
	//���建����
	BYTE byBuffer[256] = { 0 };
	CString strText;
	//���Ҫ���͵���Ϣ
	m_ctlMessage.GetWindowText( strText );
	int nLen = strText.GetLength();

	if (nLen > 0)
	{
		//��Ҫ������Ϣ�ĺ�����ϻس�
		strText += _T("\r\n");
		nLen = strText.GetLength();
		//�����UDPЭ��
		if (m_nSockType == SOCK_UDP)
		{
			USES_CONVERSION;
			SockAddrIn sin;
			// ���͹㲥��Ϣ
			sin.CreateFrom(_T("255.255.255.255"), m_strPort);

			memcpy(byBuffer, (LPSOCKADDR) sin, sin.Size());
			//��Ҫ���͵���Ϣ���Ƶ�������
			strcpy((LPSTR)&byBuffer[sin.Size()], T2CA(strText));
			nLen += sin.Size();
		}
		else
		{
			USES_CONVERSION;
			strcpy((LPSTR)byBuffer, T2CA(strText));
		}

		// ��������
		if (m_nSockType == SOCK_UDP)
			m_pCurServer->WriteComm( byBuffer, nLen+1, INFINITE);
		else
		{
			// ���͵����пͻ���

			for(int i=0; i<MAX_CONNECTION; i++)
			{
				if (m_SocketManager[i].IsOpen() && (m_pCurServer != &m_SocketManager[i]))
					m_SocketManager[i].WriteComm( byBuffer, nLen+1, INFINITE);
			}
		}
	}	
}

void CServerSocketDlg::OnDestroy() 
{
	for(int i=0; i<MAX_CONNECTION; i++)
	m_SocketManager[i].StopComm();

	CDialog::OnDestroy();	
}

