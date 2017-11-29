// TunnelServer.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "TunnelService.h"


int main()
{
	setlocale(0, "Russian");
		
	InitSection();

	hAcceptServer = CreateThread(NULL, NULL, AcceptServer, NULL, NULL, NULL);
	hDispatchServer = CreateThread(NULL, NULL, DispatchServer, NULL, NULL, NULL);
	hResponseServer = CreateThread(NULL, NULL, ResponseServer, NULL, NULL, NULL);
    
	WaitForSingleObject(hAcceptServer, INFINITE);
	CloseHandle(hAcceptServer);
	WaitForSingleObject(hResponseServer, INFINITE);
	CloseHandle(hResponseServer);

	return 0;
}

