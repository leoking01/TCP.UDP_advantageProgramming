// SocketManager.cpp: implementation of the CSocketManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <atlconv.h>
#include "ServerSocket.h"
#include "SocketManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/*
const UINT EVT_CONSUCCESS = 0x0000;	// Connection established
const UINT EVT_CONFAILURE = 0x0001;	// General failure - Wait Connection failed
const UINT EVT_CONDROP	  = 0x0002;	// Connection dropped
const UINT EVT_ZEROLENGTH = 0x0003;	// Zero length message
*/

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSocketManager::CSocketManager()
: m_pMsgCtrl(NULL)
{

}

CSocketManager::~CSocketManager()
{

}

//��ʾ����
void CSocketManager::DisplayData(const LPBYTE lpData, DWORD dwCount, const SockAddrIn& sfrom)
{
	CString strData;
	memcpy(strData.GetBuffer(dwCount), A2CT((LPSTR)lpData), dwCount);
	strData.ReleaseBuffer();
	//���sfrom��Ϊ��
	if (!sfrom.IsNull())
	{
		LONG  uAddr = sfrom.GetIPAddr();
		BYTE* sAddr = (BYTE*) &uAddr;
		short nPort = ntohs( sfrom.GetPort() );	// ��ʾ�˿�
		CString strAddr;
		// ��ַ��������ʽ����
		strAddr.Format(_T("%u.%u.%u.%u (%d)>"),
					(UINT)(sAddr[0]), (UINT)(sAddr[1]),
					(UINT)(sAddr[2]), (UINT)(sAddr[3]), nPort);
		//�õ���Դ������
		strData = strAddr + strData;
	}

	//д����Ϣ
	AppendMessage( strData );
}


void CSocketManager::AppendMessage(LPCTSTR strText )
{
	if (NULL == m_pMsgCtrl)
		return;

	if (::IsWindow( m_pMsgCtrl->GetSafeHwnd() ))
	{
		int nLen = m_pMsgCtrl->GetWindowTextLength();
		m_pMsgCtrl->SetSel(nLen, nLen);
		m_pMsgCtrl->ReplaceSel( strText );
	}
}


void CSocketManager::SetMessageWindow(CEdit* pMsgCtrl)
{
	m_pMsgCtrl = pMsgCtrl;
}

//���ݽ���
void CSocketManager::OnDataReceived(const LPBYTE lpBuffer, DWORD dwCount)
{
	SockAddrIn saddr_in;
	LPBYTE lpData = lpBuffer;
	if (IsSmartAddressing())
	{
		saddr_in.SetAddr((SOCKADDR_IN*) lpBuffer);
		lpData = &lpData[sizeof(SOCKADDR_IN)];
		if (IsServer())
		{
			// �����пͻ��㲥
			SockAddrIn sdest_in;
			sdest_in.sockAddrIn.sin_addr.s_addr = htonl(INADDR_BROADCAST);
			memcpy(lpBuffer, (LPSOCKADDR)sdest_in, sdest_in.Size());
			WriteComm(lpBuffer, dwCount, 0L);
		}
		dwCount -= sizeof(SOCKADDR_IN);
	}

	// ��ʾ��Ϣ
	DisplayData( lpData, dwCount, saddr_in );
	return;
}

///////////////////////////////////////////////////////////////////////////////
// OnEvent
// Send message to parent window to indicate connection status

//������Ϣ�������ѻ����Ӧ״̬
void CSocketManager::OnEvent(UINT uEvent)
{
	if (NULL == m_pMsgCtrl)
		return;

	CWnd* pParent = m_pMsgCtrl->GetParent();
	if (!::IsWindow( pParent->GetSafeHwnd()))
		return;

	switch( uEvent )
	{
		case EVT_CONSUCCESS:
			AppendMessage( _T("���ӽ���\r\n") );
			break;
		case EVT_CONFAILURE:
			AppendMessage( _T("����ʧ��\r\n") );
			break;
		case EVT_CONDROP:
			AppendMessage( _T("���ӷ���\r\n") );
			break;
		case EVT_ZEROLENGTH:
			AppendMessage( _T("�㳤����Ϣ\r\n") );
			break;
		default:
			TRACE("����socket�¼�\n");
			break;
	}

	pParent->PostMessage( WM_UPDATE_CONNECTION, uEvent, (LPARAM) this);

}
