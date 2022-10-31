/************************************************************************
 *   IRC - Internet Relay Chat, windows/service.c
 *   Copyright (C) 2002-2004 Dominick Meglio (codemastr)
 *   
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 1, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "unrealircd.h"

SERVICE_STATUS IRCDStatus; 
SERVICE_STATUS_HANDLE IRCDStatusHandle;

/* Signal to rehash */
#define IRCD_SERVICE_CONTROL_REHASH 128

MODVAR BOOL IsService = FALSE;

#define WIN32_VERSION BASE_VERSION "-" PATCH1 PATCH2 PATCH3 PATCH4 PATCH5

/* Places the service in the STOPPED state
 * Parameters:
 *  code - The error code (or 0)
 */
void SetServiceStop(int code)
{
	IRCDStatus.dwCurrentState = SERVICE_STOPPED;
	IRCDStatus.dwCheckPoint = 0;
	IRCDStatus.dwWaitHint = 0;
	IRCDStatus.dwWin32ExitCode = code;
	IRCDStatus.dwServiceSpecificExitCode = code;
	SetServiceStatus(IRCDStatusHandle, &IRCDStatus);
}	

/* Handles the service messages
 * Parameters:
 *  opcode - The message to process
 */
VOID WINAPI IRCDCtrlHandler(DWORD opcode) 
{
	DWORD status;
	int i;
	Client *acptr;

	/* Stopping */
	if (opcode == SERVICE_CONTROL_STOP) 
	{
		IRCDStatus.dwCurrentState = SERVICE_STOP_PENDING;
		SetServiceStatus(IRCDStatusHandle, &IRCDStatus);

/*		for (i = 0; i <= LastSlot; i++) 
		{
			if (!(acptr = local[i]))
				continue;
			if (IsUser(acptr))
				sendnotice(acptr, "Server Terminating.");
			else if (IsServer(acptr))
				sendto_one(acptr, NULL, ":%s ERROR :Terminated", me.name);
		} */
		unload_all_modules();
/*		for (i = LastSlot; i >= 0; i--)
			if ((acptr = local[i]) && DBufLength(&acptr->local->sendQ) > 0)
				(void)send_queued(acptr); */
		SetServiceStop(0);
	}
	/* Rehash */
	else if (opcode == IRCD_SERVICE_CONTROL_REHASH) 
	{
		request_rehash(NULL);
	}

	SetServiceStatus(IRCDStatusHandle, &IRCDStatus);
} 

/* Entry point function
 * Parameters:
 *  dwArgc   - Argument count
 *  lpszArgv - Arguments
 */
VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv) 
{
	DWORD error = 0;
	char path[MAX_PATH], *folder;

	IsService = TRUE;

	/* Initialize the service structure */
	IRCDStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	IRCDStatus.dwCurrentState = SERVICE_START_PENDING;
	IRCDStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP|SERVICE_ACCEPT_SHUTDOWN;
	IRCDStatus.dwWin32ExitCode = NO_ERROR;
	IRCDStatus.dwServiceSpecificExitCode = 0;
	IRCDStatus.dwCheckPoint = 0;
	IRCDStatus.dwWaitHint = 0;
 
	GetModuleFileName(NULL,path,MAX_PATH);
	folder = strrchr(path, '\\');
	*folder = 0;
	chdir(path);

	/* Go one level up, since we are currently in the bin\ subdir
	 * and we want to be in (f.e.) "C:\Program Files\UnrealIRCd 6"
	 */
	chdir("..");

	/* Register the service controller */
	IRCDStatusHandle = RegisterServiceCtrlHandler("UnrealIRCd", IRCDCtrlHandler); 
 
	GetOSName(OSName);

	InitDebug();
	init_winsock();

	/* Initialize the IRCd */
	if ((error = InitUnrealIRCd(dwArgc, lpszArgv)) != 1) 
	{
		SetServiceStop(error);
		return;
	}
	
	/* Go into the running state */
	IRCDStatus.dwCurrentState = SERVICE_RUNNING;
	IRCDStatus.dwCheckPoint = 0;
	IRCDStatus.dwWaitHint = 0;  
	SetServiceStatus(IRCDStatusHandle, &IRCDStatus);

	SocketLoop(0);
	return;
}
