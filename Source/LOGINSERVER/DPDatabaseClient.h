#ifndef __DPDATABASECLIENT_H__
#define __DPDATABASECLIENT_H__

#pragma once

#include "DPMng.h"

#undef	theClass
#define	theClass	CDPDatabaseClient
#undef theParameters
#define theParameters	CAr & ar, DPID, LPBYTE, u_long

class CDPDatabaseClient : public CDPClientSole
{
public:
//	Constructions
	CDPDatabaseClient();
	virtual	~CDPDatabaseClient();
//	Overrides
	virtual	void SysMessageHandler( LPDPMSG_GENERIC lpMsg, DWORD dwMsgSize, DPID idFrom );
	virtual void UserMessageHandler( LPDPMSG_GENERIC lpMsg, DWORD dwMsgSize, DPID idFrom );
//	Operations	
	void	SendToServer( DPID idFrom, LPVOID lpMsg, DWORD dwMsgSize );
	void	SendLeave( const char* lpszAccount, u_long idPlayer, DWORD dwPlayTime );
	void	SendCloseError( const char* lpszAccount );

	void	SendGetPlayerList( DPID idFrom, const char* lpszAccount, const char* lpszPassword, DWORD dwAuthKey, u_long uIdofMulti );

	USES_PFNENTRIES;
//	Handlers
	void	OnPlayerList( CAr & ar, DPID dpid, LPBYTE lpBuf, u_long uBufSize );
	void	OnCloseExistingConnection( CAr & ar, DPID dpid, LPBYTE lpBuf, u_long uBufSize );
	void	OnFail( CAr & ar, DPID dpid, LPBYTE lpBuf, u_long uBufSize );
	void	OnOneHourNotify( CAr & ar, DPID dpid, LPBYTE lpBuf, u_long uBufSize );

	void	OnLoginProtect( CAr & ar, DPID dpid, LPBYTE lpBuf, u_long uBufSize );
public:
	void	SendLoginProtect( const char* lpszAccount, const char* lpszPlayer, u_long idPlayer, int nBankPW, DPID dpid );
};

inline void CDPDatabaseClient::SendToServer( DPID idFrom, LPVOID lpMsg, DWORD dwMsgSize )
	{	*(UNALIGNED DPID*)lpMsg		= idFrom;	Send( lpMsg, dwMsgSize, DPID_SERVERPLAYER );	}

extern CDPDatabaseClient g_dpDBClient;

#endif	// __DPDATABASECLIENT_H__