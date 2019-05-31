///////////////////////////////////////////////////////////////////////////////
//	File:		SocketComm.cpp
//	Version:	1.1
//
//	Author:		Ernest Laurentin
//	E-mail:		elaurentin@sympatico.ca
//
//	Implementation of the CSocketComm and associated classes.
//
//	This code may be used in compiled form in any way you desire. This
//	file may be redistributed unmodified by any means PROVIDING it is
//	not sold for profit without the authors written consent, and
//	providing that this notice and the authors name and all copyright
//	notices remains intact.
//
//	An email letting me know how you are using it would be nice as well.
//
//	This file is provided "as is" with no expressed or implied warranty.
//	The author accepts no liability for any damage/loss of business that
//	this c++ class may cause.
//
//	Version history
//
//	1.0	- Initial release.
//	1.1 - Add support for Smart Addressing mode
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <stdio.h>
#include <tchar.h>
#include <process.h>
#include <crtdbg.h>
#include "SocketComm.h"
// Download by http://www.srcfans.com
const DWORD DEFAULT_TIMEOUT = 100L;

///////////////////////////////////////////////////////////////////////////////
// SockAddrIn Struct

///////////////////////////////////////////////////////////////////////////////
// Copy
SockAddrIn& SockAddrIn::Copy(const SockAddrIn& sin)
{
	memcpy(&this->sockAddrIn, &sin.sockAddrIn, Size());
	return *this;
}

///////////////////////////////////////////////////////////////////////////////
// IsEqual
bool SockAddrIn::IsEqual(const SockAddrIn& sin)
{
	// Is it Equal? - ignore 'sin_zero'
	return (memcmp(&this->sockAddrIn, &sin.sockAddrIn, Size()-sizeof(sockAddrIn.sin_zero)) == 0);
}

///////////////////////////////////////////////////////////////////////////////
// IsGreater
bool SockAddrIn::IsGreater(const SockAddrIn& sin)
{
	// Is it Greater? - ignore 'sin_zero'
	return (memcmp(&this->sockAddrIn, &sin.sockAddrIn, Size()-sizeof(sockAddrIn.sin_zero)) > 0);
}

///////////////////////////////////////////////////////////////////////////////
// IsLower
bool SockAddrIn::IsLower(const SockAddrIn& sin)
{
	// Is it Lower? - ignore 'sin_zero'
	return (memcmp(&this->sockAddrIn, &sin.sockAddrIn, Size()-sizeof(sockAddrIn.sin_zero)) < 0);
}

///////////////////////////////////////////////////////////////////////////////
// CreateFrom
bool SockAddrIn::CreateFrom(LPCTSTR sAddr, LPCTSTR sService)
{
	sockAddrIn.sin_addr.s_addr = htonl( CSocketComm::GetIPAddress(sAddr) );
	sockAddrIn.sin_port = htons( CSocketComm::GetPortNumber( sService ) );
	sockAddrIn.sin_family = AF_INET;
	return true;
}


///////////////////////////////////////////////////////////////////////////////
// Construct & Destruct

//���캯��
CSocketComm::CSocketComm() :
	m_hComm(INVALID_HANDLE_VALUE), m_hMutex(NULL), m_bServer(false),
	m_bBroadcast(false), m_hThread(NULL)
{

}

CSocketComm::~CSocketComm()
{
	//ֹͣͨ��
	StopComm();
}

///////////////////////////////////////////////////////////////////////////////
// Members
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// IsOpen
bool CSocketComm::IsOpen() const
{
	return ( INVALID_HANDLE_VALUE != m_hComm );
}


///////////////////////////////////////////////////////////////////////////////
//�ж��Ƿ�����
bool CSocketComm::IsStart() const
{
	return ( NULL != m_hThread );
}


///////////////////////////////////////////////////////////////////////////////
// IsServer
bool CSocketComm::IsServer() const
{
	return m_bServer;
}


///////////////////////////////////////////////////////////////////////////////
// IsBroadcast
bool CSocketComm::IsBroadcast() const
{
	return m_bBroadcast;
}


///////////////////////////////////////////////////////////////////////////////
// IsSmartAddressing
bool CSocketComm::IsSmartAddressing() const
{
	return m_bSmartAddressing;
}


///////////////////////////////////////////////////////////////////////////////
// GetSocket

//���socket���
SOCKET CSocketComm::GetSocket() const
{
	return (SOCKET) m_hComm;
}


///////////////////////////////////////////////////////////////////////////////
// LockList

//���̻���
void CSocketComm::LockList()
{
	if (NULL != m_hMutex)
		WaitForSingleObject(m_hMutex, INFINITE);
}


///////////////////////////////////////////////////////////////////////////////
// UnlockList

//�⿪���̻���
void CSocketComm::UnlockList()
{
	if (NULL != m_hMutex)
		ReleaseMutex(m_hMutex);
}


///////////////////////////////////////////////////////////////////////////////
// AddToList
void CSocketComm::AddToList(const SockAddrIn& saddr_in)
{
	LockList();
	m_AddrList.insert( m_AddrList.end(), saddr_in );
	UnlockList();
}

///////////////////////////////////////////////////////////////////////////////
// RemoveFromList
void CSocketComm::RemoveFromList(const SockAddrIn& saddr_in)
{
	LockList();
	m_AddrList.remove( saddr_in );
	UnlockList();
}

///////////////////////////////////////////////////////////////////////////////
// SetServerState
void CSocketComm::SetServerState(bool bServer)
{
	if (!IsStart())
		m_bServer = bServer;
}


///////////////////////////////////////////////////////////////////////////////
// SetSmartAddressing : Address is included with message

//�趨
void CSocketComm::SetSmartAddressing(bool bSmartAddressing)
{
	if (!IsStart())
		m_bSmartAddressing = bSmartAddressing;
}

///////////////////////////////////////////////////////////////////////////////
// OnDataReceived
///////////////////////////////////////////////////////////////////////////////
// DESCRIPTION:
//				This function is PURE Virtual, you MUST overwrite it.  This is
//				called every time new data is available.
// PARAMETERS:
///////////////////////////////////////////////////////////////////////////////
void CSocketComm::OnDataReceived(const LPBYTE lpBuffer, DWORD dwCount)
{
}


///////////////////////////////////////////////////////////////////////////////
// OnEvent
///////////////////////////////////////////////////////////////////////////////
// DESCRIPTION:
//				This function reports events & errors
// PARAMETERS:
//		UINT uEvent: can be one of the event value EVT_(events)
///////////////////////////////////////////////////////////////////////////////
void CSocketComm::OnEvent(UINT uEvent)
{
}

///////////////////////////////////////////////////////////////////////////////
// GetPortNumber
///////////////////////////////////////////////////////////////////////////////
// DESCRIPTION:
//				Returns a port number based on service name or port number string
// PARAMETERS:
//	LPCTSTR strServiceName: Service name or port string
///////////////////////////////////////////////////////////////////////////////

//��ö˿ں�
USHORT CSocketComm::GetPortNumber( LPCTSTR strServiceName )
{
	LPSERVENT	lpservent;
	USHORT		nPortNumber = 0;

	if ( _istdigit( strServiceName[0] ) ) {
		nPortNumber = (USHORT) _ttoi( strServiceName );
	}
	else {
#ifdef _UNICODE
		char pstrService[HOSTNAME_SIZE];
		WideCharToMultiByte(CP_ACP, 0, pstrService, -1, strServiceName, sizeof(pstrService), NULL, NULL );
#else
		LPCTSTR pstrDevice = strServiceName;
#endif
		// ת�������ֽڵ������ֽ�
		if ( (lpservent = getservbyname( pstrDevice, NULL )) != NULL )
			nPortNumber = ntohs( lpservent->s_port );
	}

	return nPortNumber;
}


///////////////////////////////////////////////////////////////////////////////
// GetIPAddress
///////////////////////////////////////////////////////////////////////////////
// DESCRIPTION:
//		Returns an IP address.
//			- It tries to convert the string directly
//			- If that fails, it tries to resolve it as a hostname
// PARAMETERS:
//	LPCTSTR strHostName: host name to get IP address
///////////////////////////////////////////////////////////////////////////////

//��ȡIP��ַ�����ʧ�ܣ��򷵻�һ��������
ULONG CSocketComm::GetIPAddress( LPCTSTR strHostName )
{
	LPHOSTENT	lphostent;
	ULONG		uAddr = INADDR_NONE;
	//AfxMessageBox(strHostName);
	if ( NULL != strHostName )
	{
#ifdef _UNICODE
		char strHost[HOSTNAME_SIZE] = { 0 };
		WideCharToMultiByte(CP_ACP, 0, strHostName, -1, strHost, sizeof(strHost), NULL, NULL );
#else
		LPCTSTR strHost = strHostName;
#endif
		// ת���ɱ�׼ip��ַ
		//AfxMessageBox(strHost);
		uAddr = inet_addr( strHostName );

		if ( (INADDR_NONE == uAddr) && (strcmp( strHost, "255.255.255.255" )) )
		{
			// ��û�����
			if ( lphostent = gethostbyname( strHost ) )
				uAddr = *((ULONG *) lphostent->h_addr_list[0]);
		}
	}
	
	return ntohl( uAddr );
}


///////////////////////////////////////////////////////////////////////////////
// GetLocalName
///////////////////////////////////////////////////////////////////////////////
// DESCRIPTION:
//				Get local computer name.  Something like: "mycomputer.myserver.net"
// PARAMETERS:
//	LPTSTR strName: name of the computer is returned here
//	UINT nSize: size max of buffer "strName"
///////////////////////////////////////////////////////////////////////////////

//��ñ��ػ�����
bool CSocketComm::GetLocalName(LPTSTR strName, UINT nSize)
{
	if (strName != NULL && nSize > 0)
	{
		char strHost[HOSTNAME_SIZE] = { 0 };

		// ��û�����
		if (SOCKET_ERROR != gethostname(strHost, sizeof(strHost)))
		{
			struct hostent* hp;
			hp = gethostbyname(strHost);
			if (hp != NULL)	{
				strcpy(strHost, hp->h_name);
			}
			// ��黺������С
			if (strlen(strHost) > nSize)
			{
				SetLastError(ERROR_INSUFFICIENT_BUFFER);
				return false;
			}

			// Unicodeת��
#ifdef _UNICODE
			return (0 != MultiByteToWideChar(CP_ACP, 0, strHost, -1, strName, nSize, NULL, NULL ));
#else
			_tcscpy(strName, strHost);
			return true;
#endif
		}
	}
	else
		SetLastError(ERROR_INVALID_PARAMETER);
	return false;
}


///////////////////////////////////////////////////////////////////////////////
// GetLocalAddress
///////////////////////////////////////////////////////////////////////////////
// DESCRIPTION:
//				Get TCP address of local computer in dot format ex: "127.0.0.0"
// PARAMETERS:
//	LPTSTR strAddress: pointer to hold address string, must be long enough
//	UINT nSize: maximum size of this buffer
///////////////////////////////////////////////////////////////////////////////

//��ȡ���ؼ�����ı�׼IP��ַ����"127.0.0.0"
bool CSocketComm::GetLocalAddress(LPTSTR strAddress, UINT nSize)
{
	// ��ü�������ص�ַ
	if (strAddress != NULL && nSize > 0)
	{
		char strHost[HOSTNAME_SIZE] = { 0 };

		// ��û�����
		if (SOCKET_ERROR != gethostname(strHost, sizeof(strHost)))
		{
			struct hostent* hp;
			hp = gethostbyname(strHost);
			if (hp != NULL && hp->h_addr_list[0] != NULL)
			{
				// �鿴��ַ�Ƿ���4�ֽڴ�С
				if ( hp->h_length < 4)
					return false;

				// ת����ַ����
				strHost[0] = 0;

				// ������ַ�ַ���
				sprintf(strHost, "%u.%u.%u.%u",
					(UINT)(((PBYTE) hp->h_addr_list[0])[0]),
					(UINT)(((PBYTE) hp->h_addr_list[0])[1]),
					(UINT)(((PBYTE) hp->h_addr_list[0])[2]),
					(UINT)(((PBYTE) hp->h_addr_list[0])[3]));

				// ��黺�����Ƿ��㹻
				if (strlen(strHost) > nSize)
				{
					SetLastError(ERROR_INSUFFICIENT_BUFFER);
					return false;
				}

			// Unicodeת��

#ifdef _UNICODE
				return (0 != MultiByteToWideChar(CP_ACP, 0, strHost, -1, strAddress,
					nSize, NULL, NULL ));
#else
				_tcscpy(strAddress, strHost);
				return true;
#endif
			}
		}
	}
	else
		SetLastError(ERROR_INVALID_PARAMETER);
	return false;
}


///////////////////////////////////////////////////////////////////////////////
// WaitForConnection
///////////////////////////////////////////////////////////////////////////////
// DESCRIPTION:
//				Wait for a network connection.  Only for connection type of socket
//				This function may fail, in this case it returns "INVALID_SOCKET"
// PARAMETERS:
//	SOCKET sock: a socket capable of receiving new connection (TCP: SOCK_STREAM)
///////////////////////////////////////////////////////////////////////////////

//�ȴ�һ���������ӣ��������INVALID_SOCKET����ʾʧ��
SOCKET CSocketComm::WaitForConnection(SOCKET sock)
{
	// ����һ������
	return accept(sock, 0, 0);
}


///////////////////////////////////////////////////////////////////////////////
// ShutdownConnection
///////////////////////////////////////////////////////////////////////////////
// DESCRIPTION:
//				Shutdown a connection and close socket.  This will force all
//				transmission/reception to fail.
// PARAMETERS:
//	SOCKET sock: Socket to close
///////////////////////////////////////////////////////////////////////////////

//�ر�һ�����Ӳ��ر�һ��socket���⽫ǿ�����еĴ���ͽ���ʧ��
bool CSocketComm::ShutdownConnection(SOCKET sock)
{
	//
	shutdown(sock, SD_BOTH);
	return ( 0 == closesocket( sock ));
}


///////////////////////////////////////////////////////////////////////////////
// GetSockName
///////////////////////////////////////////////////////////////////////////////
// DESCRIPTION:
//				retrieves the local name for a socket
// PARAMETERS:
//	SockAddrIn& saddr_in: object to store address
///////////////////////////////////////////////////////////////////////////////

//���socket����
bool CSocketComm::GetSockName(SockAddrIn& saddr_in)
{
	if (IsOpen())
	{
		int namelen = saddr_in.Size();
		return (SOCKET_ERROR != getsockname(GetSocket(), (LPSOCKADDR)saddr_in, &namelen));
	}

	return false;
}


///////////////////////////////////////////////////////////////////////////////
// GetPeerName
///////////////////////////////////////////////////////////////////////////////
// DESCRIPTION:
//				retrieves the name of the peer to which a socket is connected
// PARAMETERS:
//	SockAddrIn& saddr_in: object to store address
///////////////////////////////////////////////////////////////////////////////

//���socketҪ���ӵĵ�ַ
bool CSocketComm::GetPeerName(SockAddrIn& saddr_in)
{
	if (IsOpen())
	{
		int namelen = saddr_in.Size();
		return (SOCKET_ERROR != getpeername(GetSocket(), (LPSOCKADDR)saddr_in, &namelen));	
	}

	return false;
}


///////////////////////////////////////////////////////////////////////////////
// CreateSocket
///////////////////////////////////////////////////////////////////////////////
// DESCRIPTION:
//				This function creates a new socket for connection (SOCK_STREAM)
//				or an connectionless socket (SOCK_DGRAM).  A connectionless
//				socket should not call "accept()" since it cannot receive new
//				connection.  This is used as SERVER socket
// PARAMETERS:
//	LPCTSTR strServiceName: Service name or port number
//	int nProtocol: protocol to use (set to AF_INET)
//	int nType: type of socket to create (SOCK_STREAM, SOCK_DGRAM)
//	UINT uOptions: other options to use
///////////////////////////////////////////////////////////////////////////////

bool CSocketComm::CreateSocket(LPCTSTR strServiceName, int nProtocol, int nType, UINT uOptions /* = 0 */)
{
	// ����Ѿ��򿪣��򷵻�false
	if ( IsOpen() )
		return false;

	SOCKADDR_IN sockAddr = { 0 };

	//��ַ�趨
	SOCKET sock = socket(nProtocol, nType, 0);
	if (INVALID_SOCKET != sock)
	{
		sockAddr.sin_port = htons( GetPortNumber( strServiceName ) );
		if ( 0 != sockAddr.sin_port)
		{
			sockAddr.sin_addr.s_addr = htonl( INADDR_ANY );
			sockAddr.sin_family = nProtocol;

			if (uOptions & SO_REUSEADDR)
			{
				//�趨���ѡ��
				BOOL optval = TRUE;
				if ( SOCKET_ERROR == setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, (char *) &optval, sizeof( BOOL ) ) )
				{
					closesocket( sock );
					return false;
				}
			}
			//�����UDPЭ��
			if (nType == SOCK_DGRAM)
			{
				//�������㲥
				if (uOptions & SO_BROADCAST)
				{
					// ����㲥
					BOOL optval = TRUE;
					if ( SOCKET_ERROR == setsockopt( sock, SOL_SOCKET, SO_BROADCAST, (char *) &optval, sizeof( BOOL ) ) )
					{
						closesocket( sock );
						return false;
					}

					// �趨����
					m_bBroadcast = true;
				}

				// �����Ҫ�㲥������Ҫ�趨���̻���
				m_hMutex = CreateMutex(NULL, FALSE, NULL);
				if (NULL == m_hMutex)
				{
					closesocket( sock );
					return false;
				}

			}

			// ��һ�����ص�ַ
			if ( SOCKET_ERROR == bind(sock, (LPSOCKADDR)&sockAddr, sizeof(SOCKADDR_IN)))
			{
				closesocket( sock );
				m_bBroadcast = false;
				if (NULL != m_hMutex)
					CloseHandle( m_hMutex );
				m_hMutex = NULL;
				return false;
			}

			// �����TCP����
			if (SOCK_STREAM == nType)
			{
				if ( SOCKET_ERROR == listen(sock, SOMAXCONN))
				{
					closesocket( sock );
					return false;
				}
			}

			// ����socket
			m_hComm = (HANDLE) sock;
			return true;
		}
		else
			SetLastError(ERROR_INVALID_PARAMETER); //�����ҵ��˿�

		// ɾ��socket
		closesocket( sock );
	}

	return false;
}


///////////////////////////////////////////////////////////////////////////////
// ConnectTo
///////////////////////////////////////////////////////////////////////////////
// DESCRIPTION:
//				Establish connection with a server service or port
// PARAMETERS:
//	LPCTSTR strDestination: hostname or address to connect (in .dot format)
//	LPCTSTR strServiceName: Service name or port number
//	int nProtocol: protocol to use (set to AF_INET)
//	int nType: type of socket to create (SOCK_STREAM, SOCK_DGRAM)
///////////////////////////////////////////////////////////////////////////////

//��������
bool CSocketComm::ConnectTo(LPCTSTR strDestination, LPCTSTR strServiceName, int nProtocol, int nType)
{
	// ���socket�Ѿ���
	if ( IsOpen() )
		return false;

	SOCKADDR_IN sockAddr = { 0 };

	//����һ��socket
	SOCKET sock = socket(nProtocol, nType, 0);
	if (INVALID_SOCKET != sock)
	{
		// ��socket��һ����ַ
		TCHAR strHost[HOSTNAME_SIZE] = { 0 };
		if (false == CSocketComm::GetLocalName( strHost, sizeof(strHost)/sizeof(TCHAR)))
		{
			closesocket( sock );
			return false;
		}
		//AfxMessageBox(strHost);
		sockAddr.sin_addr.s_addr = htonl( CSocketComm::GetIPAddress( strHost ) );
		sockAddr.sin_family = nProtocol;

		if ( SOCKET_ERROR == bind(sock, (LPSOCKADDR)&sockAddr, sizeof(SOCKADDR_IN)))
		{
			closesocket( sock );
			return false;
		}

		// ���Ŀ���ַ
		if ( strDestination[0]) {
			sockAddr.sin_addr.s_addr = htonl(CSocketComm::GetIPAddress( strDestination ) );
		}

		// ��ö˿�
		sockAddr.sin_port = htons( GetPortNumber( strServiceName ) );
		if ( 0 != sockAddr.sin_port )
		{
			// ���ӷ�����
			if (SOCKET_ERROR == connect( sock, (LPSOCKADDR)&sockAddr, sizeof(SOCKADDR_IN)))
			{
				closesocket( sock );
				return false;
			}

			// ����socket
			m_hComm = (HANDLE) sock;
			return true;
		}
	}
	return false;
}


///////////////////////////////////////////////////////////////////////////////
// CloseComm
///////////////////////////////////////////////////////////////////////////////
// DESCRIPTION:
//		Close Socket Communication
// PARAMETERS:
//		None
///////////////////////////////////////////////////////////////////////////////

//�ر�socket
void CSocketComm::CloseComm()
{
	if (IsOpen())
	{
		//����ShutdownConnection�ر�
		ShutdownConnection((SOCKET)m_hComm);
		m_hComm = INVALID_HANDLE_VALUE;
		m_bBroadcast = false;
	}
}


///////////////////////////////////////////////////////////////////////////////
// WatchComm
///////////////////////////////////////////////////////////////////////////////
// DESCRIPTION:
//		Starts Socket Communication Working thread
// PARAMETERS:
//		None
///////////////////////////////////////////////////////////////////////////////

//����socketͨ���߳�
bool CSocketComm::WatchComm()
{
	//�����ж��Ƿ�����
	if (!IsStart())
	{
		//�ж��Ƿ��ͨ�ţ���socket�Ƿ�ɹ�����
		if (IsOpen())
		{
			HANDLE hThread;
			UINT uiThreadId = 0;
			//�����̣߳�ʹ��_beginthreadex
			hThread = (HANDLE)_beginthreadex(NULL,	// ��ȫ����
									  0,	// ��ջ
						SocketThreadProc,	// �̳߳���
									this,	// �̲߳���
						CREATE_SUSPENDED,	//����ģʽ
							&uiThreadId);	// �߳�ID
			//����̲߳�Ϊ��
			if ( NULL != hThread)
			{
				//�����߳�
				ResumeThread( hThread );
				m_hThread = hThread;
				return true;
			}
		}
	}
	return false;
}


///////////////////////////////////////////////////////////////////////////////
// StopComm
///////////////////////////////////////////////////////////////////////////////
// DESCRIPTION:
//		Close Socket and Stop Communication thread
// PARAMETERS:
//		None
///////////////////////////////////////////////////////////////////////////////
void CSocketComm::StopComm()
{
	// Close Socket
	if (IsOpen())
	{
		CloseComm();
		Sleep(50);
	}

	// Kill Thread
	if (IsStart())
	{
		if (WaitForSingleObject(m_hThread, 5000L) == WAIT_TIMEOUT)
			TerminateThread(m_hThread, 1L);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}

	// Clear Address list
	if (!m_AddrList.empty())
		m_AddrList.clear();

	// Destroy Synchronization objects
	if (NULL != m_hMutex)
	{
		CloseHandle( m_hMutex );
		m_hMutex = NULL;
	}

}


///////////////////////////////////////////////////////////////////////////////
// ReadComm
///////////////////////////////////////////////////////////////////////////////
// DESCRIPTION:
//		Reads the Socket Communication
// PARAMETERS:
//		LPBYTE lpBuffer: buffer to place new data
//		DWORD dwSize: maximum size of buffer
//		DWORD dwTimeout: timeout to use in millisecond
///////////////////////////////////////////////////////////////////////////////

//��������
DWORD CSocketComm::ReadComm(LPBYTE lpBuffer, DWORD dwSize, DWORD dwTimeout)
{
	_ASSERTE( IsOpen() );
	_ASSERTE( lpBuffer != NULL );

	if (lpBuffer == NULL || dwSize < 1L)
		return 0L;

	fd_set	fdRead  = { 0 };
	TIMEVAL	stTime;
	TIMEVAL	*pstTime = NULL;

	if ( INFINITE != dwTimeout ) {
		stTime.tv_sec = 0;
		stTime.tv_usec = dwTimeout*1000;
		pstTime = &stTime;
	}

	SOCKET s = (SOCKET) m_hComm;
	// �趨������
	if ( !FD_ISSET( s, &fdRead ) )
		FD_SET( s, &fdRead );

	// ѡ�������趨��ʱʱ��
	DWORD dwBytesRead = 0L;
	int res = select( s+1, &fdRead, NULL, NULL, pstTime );
	if ( res > 0)
	{
		if (IsBroadcast() || IsSmartAddressing())
		{
			SOCKADDR_IN sockAddr = { 0 }; // ��õ�ַ
			int nOffset = IsSmartAddressing() ? sizeof(sockAddr) : 0; 
			int nLen = sizeof(sockAddr);
			if ( dwSize < (DWORD) nLen)	// ������̫С
			{
				SetLastError( ERROR_INVALID_USER_BUFFER );
				return -1L;
			}
			//�������
			res = recvfrom( s, (LPSTR)&lpBuffer[nOffset], dwSize, 0, (LPSOCKADDR)&sockAddr, &nLen);

			// clear 'sin_zero', we will ignore them with 'SockAddrIn' anyway!
			memset(&sockAddr.sin_zero, 0, sizeof(sockAddr.sin_zero));
			
			if ( res >= 0)
			{
				LockList(); //������ַ�б� 

				// ɾ�����ظ���ַ
				SockAddrIn sockin;
				sockin.SetAddr( &sockAddr );
				m_AddrList.remove( sockin );
				m_AddrList.insert(m_AddrList.end(), sockin);
				
				if (IsSmartAddressing())
				{
					memcpy(lpBuffer, &sockAddr, sizeof(sockAddr));
					res += sizeof(sockAddr);
				}

				UnlockList(); // �⿪��ַ�б�
			}
		}
		else
		{
			res = recv( s, (LPSTR)lpBuffer, dwSize, 0);
		}

		dwBytesRead = (DWORD)((res > 0)?(res) : (-1));
	}

	return dwBytesRead;
}


///////////////////////////////////////////////////////////////////////////////
// WriteComm
///////////////////////////////////////////////////////////////////////////////
// DESCRIPTION:
//		Writes data to the Socket Communication
// PARAMETERS:
//		const LPBYTE lpBuffer: data to write
//		DWORD dwCount: maximum characters to write
//		DWORD dwTimeout: timeout to use in millisecond
///////////////////////////////////////////////////////////////////////////////

//��������
DWORD CSocketComm::WriteComm(const LPBYTE lpBuffer, DWORD dwCount, DWORD dwTimeout)
{
	_ASSERTE( IsOpen() );
	_ASSERTE( NULL != lpBuffer );

	// ���û�н������ӻ��߻�����Ϊ�գ��򷵻�
	if (!IsOpen() || NULL == lpBuffer)
		return 0L;
	//fd_set ��һ���ṹ�壬���Ա��ܶ��windows socket����ʹ�ã���select����socket2.0��ʹ��
	//typedef struct fd_set {
	//u_int    fd_count;                 // ����
	//SOCKET   fd_array[FD_SETSIZE];     //socket ����
	//} fd_set;

	fd_set	fdWrite  = { 0 };
	TIMEVAL	stTime;
	TIMEVAL	*pstTime = NULL;

	if ( INFINITE != dwTimeout ) {
		stTime.tv_sec = 0;
		stTime.tv_usec = dwTimeout*1000;
		pstTime = &stTime;
	}

	SOCKET s = (SOCKET) m_hComm;
	// �趨������
	if ( !FD_ISSET( s, &fdWrite ) )
		FD_SET( s, &fdWrite );

	// ѡ�����趨��ʱʱ��
	DWORD dwBytesWritten = 0L;
	int res = select( s+1, NULL, &fdWrite, NULL, pstTime );
	if ( res > 0)
	{
		// ������Ϣ�㲥���ߵ�Ե㷢��
		if (IsBroadcast() || IsSmartAddressing())
		{
			// use offset for Smart addressing
			int nOffset = IsSmartAddressing() ? sizeof(SOCKADDR_IN) : 0;
			if (IsSmartAddressing())
			{
				if ( dwCount < sizeof(SOCKADDR_IN))	// error - buffer to small
				{
					SetLastError( ERROR_INVALID_USER_BUFFER );
					return -1L;
				}

				// �ӻ������л�õ�ַ
				SockAddrIn sockAddr;
				sockAddr.SetAddr((SOCKADDR_IN*) lpBuffer);

				// ��õ�ַȻ����
				if (sockAddr.sockAddrIn.sin_addr.s_addr != htonl(INADDR_BROADCAST))
				{
					res = sendto( s, (LPCSTR)&lpBuffer[nOffset], dwCount-nOffset, 0,
						(LPSOCKADDR)sockAddr, sockAddr.Size());
					dwBytesWritten = (DWORD)((res >= 0)?(res) : (-1));
					return dwBytesWritten;
				}
			}

			// �������û��㲥
			LockList(); // ��ס��ַ�б�
			
			CSockAddrList::iterator iter = m_AddrList.begin();
			for( ; iter != m_AddrList.end(); )
			{
				//ѭ��������Ϣ
				res = sendto( s, (LPCSTR)&lpBuffer[nOffset], dwCount-nOffset, 0, (LPSOCKADDR)(*iter), iter->Size());
				if (res < 0)
				{
					CSockAddrList::iterator deladdr = iter;
					++iter;	// ��һ��
					m_AddrList.erase( deladdr );
				}
				else
					++iter;	// ��һ��
			}

			UnlockList(); // ����

			// UDP���Ƿ���true
			res = (int) dwCount - nOffset;
		}
		else // ���͵������ͻ���
			res = send( s, (LPCSTR)lpBuffer, dwCount, 0);

		dwBytesWritten = (DWORD)((res >= 0)?(res) : (-1));
	}

	return dwBytesWritten;
}


///////////////////////////////////////////////////////////////////////////////
// Run
///////////////////////////////////////////////////////////////////////////////
// DESCRIPTION:
//		This function runs the main thread loop
//		this implementation can be overloaded.
//      This function calls CSocketComm::OnDataReceived() (Virtual Function)
// PARAMETERS:
// NOTES:
//		You should not wait on the thread to end in this function or overloads
///////////////////////////////////////////////////////////////////////////////

//�ú��������߳���ѭ������
void CSocketComm::Run()
{
	BYTE	buffer[BUFFER_SIZE];
	DWORD	dwBytes  = 0L;

	HANDLE	hThread = GetCurrentThread();
	DWORD	dwTimeout = DEFAULT_TIMEOUT;

	// �Ƿ����з�����ģʽ
	if (IsServer())
	{
		//�Ƿ�㲥ģʽ
		if (!IsBroadcast())
		{
			SOCKET sock = (SOCKET) m_hComm;
			sock = WaitForConnection( sock );

			// �ȴ��µ�����
			if (sock != INVALID_SOCKET)
			{
				//�ر�����
				ShutdownConnection( (SOCKET) m_hComm);
				m_hComm = (HANDLE) sock;
				OnEvent( EVT_CONSUCCESS ); // connect
			}
			else
			{
				// ����Ѿ��ر��򲻷����¼���������
				if (IsOpen())
					OnEvent( EVT_CONFAILURE ); // �ȴ�ʧ��
				return;
			}
		}
	}

	//���socket�Ѿ�����
	while( IsOpen() )
	{
		// ��������ʽsocket���ȴ��¼�֪ͨ
		dwBytes = ReadComm(buffer, sizeof(buffer), dwTimeout);

		// ����д�����
		if (dwBytes == (DWORD)-1)
		{
			// ���Ҫ�رգ��򲻷����¼�
			if (IsOpen())
				OnEvent( EVT_CONDROP ); // ʧȥ����
			break;
		}

		// �Ƿ��������յ�
		if (IsSmartAddressing() && dwBytes == sizeof(SOCKADDR_IN))
			OnEvent( EVT_ZEROLENGTH );
		else if (dwBytes > 0L)
		{
			OnDataReceived( buffer, dwBytes);
		}

		Sleep(0);
	}
}


///////////////////////////////////////////////////////////////////////////////
// SocketThreadProc
///////////////////////////////////////////////////////////////////////////////
// DESCRIPTION:
//     Socket Thread function.  This function is the main thread for socket
//     communication - Asynchronous mode.
// PARAMETERS:
//     LPVOID pParam : Thread parameter - a CSocketComm pointer
// NOTES:
///////////////////////////////////////////////////////////////////////////////

//socket�߳�
UINT WINAPI CSocketComm::SocketThreadProc(LPVOID pParam)
{
	//reinterpret_cast���ڸ���ָ���ת��
	CSocketComm* pThis = reinterpret_cast<CSocketComm*>( pParam );
	_ASSERTE( pThis != NULL );

	pThis->Run();

	return 1L;
} 
