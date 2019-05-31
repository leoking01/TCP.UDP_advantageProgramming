///////////////////////////////////////////////////////////////////////////////
// FILE : SocketComm.h
// Header file for CSocketComm class
// CSocketComm
//     Generic class for Socket Communication
///////////////////////////////////////////////////////////////////////////////

#ifndef _SOCKETCOMM_H_
#define _SOCKETCOMM_H_
#include <list>

#include <stdlib.h>
#if(_WIN32_WINNT >= 0x0400)
#include <winsock2.h>
#include <mswsock.h>
#else
#include <winsock.h>
#endif /* _WIN32_WINNT >=  0x0400 */
#pragma comment(lib, "ws2_32")

// Event value
#define EVT_CONSUCCESS		0x0000	// Connection established
#define EVT_CONFAILURE		0x0001	// General failure - Wait Connection failed
#define EVT_CONDROP			0x0002	// Connection dropped
#define EVT_ZEROLENGTH		0x0003	// Zero length message


#define BUFFER_SIZE		MAX_PATH
#define HOSTNAME_SIZE	MAX_PATH
#define STRING_LENGTH	40

struct SockAddrIn {
public:
	SockAddrIn() { Clear(); }
	SockAddrIn(const SockAddrIn& sin) { Copy( sin ); }
	~SockAddrIn() { }
	SockAddrIn& Copy(const SockAddrIn& sin);
	void	Clear() { memset(&sockAddrIn, 0, sizeof(sockAddrIn)); }
	bool	IsEqual(const SockAddrIn& sin);
	bool	IsGreater(const SockAddrIn& sin);
	bool	IsLower(const SockAddrIn& pm);
	bool	IsNull() const { return ((sockAddrIn.sin_addr.s_addr==0L)&&(sockAddrIn.sin_port==0)); }
	ULONG	GetIPAddr() const { return sockAddrIn.sin_addr.s_addr; }
	short	GetPort() const { return sockAddrIn.sin_port; }
	bool	CreateFrom(LPCTSTR sAddr, LPCTSTR sService);
	SockAddrIn& operator=(const SockAddrIn& sin) { return Copy( sin ); }
	bool	operator==(const SockAddrIn& sin) { return IsEqual( sin ); }
	bool	operator!=(const SockAddrIn& sin) { return !IsEqual( sin ); }
	bool	operator<(const SockAddrIn& sin)  { return IsLower( sin ); }
	bool	operator>(const SockAddrIn& sin)  { return IsGreater( sin ); }
	bool	operator<=(const SockAddrIn& sin) { return !IsGreater( sin ); }
	bool	operator>=(const SockAddrIn& sin) { return !IsLower( sin ); }
	operator LPSOCKADDR() { return (LPSOCKADDR)(&sockAddrIn); }
	size_t	Size() const { return sizeof(sockAddrIn); }
	void	SetAddr(SOCKADDR_IN* psin) { memcpy(&sockAddrIn, psin, Size()); }
	SOCKADDR_IN sockAddrIn;
};

typedef std::list<SockAddrIn> CSockAddrList;

class CSocketComm
{
public:
	CSocketComm();
	virtual ~CSocketComm();

	bool IsOpen() const;	// �ж�socket�Ƿ���ȷ
	bool IsStart() const;	// �߳��Ƿ�����
	bool IsServer() const;	// �Ƿ��Է�������ʽ����
	bool IsBroadcast() const; // �Ƿ�����UDP�㲥
	bool IsSmartAddressing() const;	// Is Smart Addressing mode support
	SOCKET GetSocket() const;	// ����socket���
	void SetServerState(bool bServer);	// �趨����ģʽ�Ƿ�Ϊ������ģʽ
	void SetSmartAddressing(bool bSmartAddressing);	// Set Smart addressing mode
	bool GetSockName(SockAddrIn& saddr_in);	// ���socket������ַ
	bool GetPeerName(SockAddrIn& saddr_in);	// ���Peer Socket �� - ��ַ
	void AddToList(const SockAddrIn& saddr_in);	//����ַ���ӵ��б�
	void RemoveFromList(const SockAddrIn& saddr_in);	// ���б���ɾ���б�
	void CloseComm();		// �ر� Socket
	bool WatchComm();		// ���� Socket �߳�
	void StopComm();		// ֹͣ Socket �߳�

	// ������������socket
	bool CreateSocket(LPCTSTR strServiceName, int nProtocol, int nType, UINT uOptions = 0);
	// ����һ���ͻ��˵�socket
	bool ConnectTo(LPCTSTR strDestination, LPCTSTR strServiceName, int nProtocol, int nType);

	// �¼������� - ��������
	virtual void OnDataReceived(const LPBYTE lpBuffer, DWORD dwCount);
	virtual void OnEvent(UINT uEvent);
	// ���̺߳���
	virtual void Run();

	// ���ݺ���
	DWORD ReadComm(LPBYTE lpBuffer, DWORD dwSize, DWORD dwTimeout);
	DWORD WriteComm(const LPBYTE lpBuffer, DWORD dwCount, DWORD dwTimeout);

	// Utility functions
	static SOCKET WaitForConnection(SOCKET sock); // �ȴ��µ�����
	static bool ShutdownConnection(SOCKET sock);  // �ر�����
	static USHORT GetPortNumber( LPCTSTR strServiceName );	// ��÷���˿ں�
	static ULONG GetIPAddress( LPCTSTR strHostName );	// �������IP��ַ
	static bool GetLocalName(LPTSTR strName, UINT nSize);	// ��û�����
	static bool GetLocalAddress(LPTSTR strAddress, UINT nSize);	// ��ñ��ص�ַ
// SocketComm - data
protected:
	HANDLE		m_hComm;		// socket���
	HANDLE		m_hThread;		// �߳̾��
	bool		m_bServer;		// Ϊ���ʾ������ģʽ
	bool		m_bSmartAddressing;	// Smart Addressing mode (true) - many listeners
	bool		m_bBroadcast;	// Broadcast mode
	CSockAddrList m_AddrList;	// Connection address list for broadcast
	HANDLE		m_hMutex;		// Mutex object
// SocketComm - function
protected:
	// ͬ������
	void LockList();			// ��ס����
	void UnlockList();			// �⿪����

	static UINT WINAPI SocketThreadProc(LPVOID pParam);

private:
};

#endif // _SOCKETCOMM_H_
