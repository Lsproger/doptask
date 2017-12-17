// ConcurrentServer.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "ServerProcesses.h"

using namespace std;

int main(int argc, char* argv[])
{
	int pt = 0;
	char dllN[50] = "";
	char npName[10] = "";
	setlocale(0, "Russian");
	switch (argc)
	{
	case 2: 
		serverPort = atoi(argv[1]); 
		break;
	case 3:
		serverPort = atoi(argv[1]);
		strcpy(dllName, argv[2]);
		break;
	case 4:
		serverPort = atoi(argv[1]);
		strcpy(dllName, argv[2]);
		strcpy(namedPipeName, argv[3]);
		break;
	default:
		pt = 2000;
		strcpy(dllN, "ServiceLibrary.dll");
		strcpy(npName, "BOX");
		break;
	}

	SetServerParams(pt, dllN, npName);

	cout << "server port " << serverPort << endl;
	


	







	volatile TalkersCommand  cmd = START;      // команды сервера 

	if (InitSection()) cout << "Initialized\n";
	hAcceptServer = CreateThread(NULL, NULL, AcceptServer,
		(LPVOID)&cmd, NULL, NULL);
	hDispatchServer = CreateThread(NULL, NULL, DispatchServer,
		(LPVOID)&cmd, NULL, NULL);
	hGarbageCleaner = CreateThread(NULL, NULL, GarbageCleaner,
		(LPVOID)&cmd, NULL, NULL);
	hConsolePipe = CreateThread(NULL, NULL, ConsolePipe,
		(LPVOID)&cmd, NULL, NULL);
	hResponseServer = CreateThread(NULL, NULL, ResponseServer,
		(LPVOID)&cmd, NULL, NULL);

	SetThreadPriority(hGarbageCleaner, THREAD_PRIORITY_LOWEST);
	SetThreadPriority(hGarbageCleaner, THREAD_PRIORITY_BELOW_NORMAL);
	SetThreadPriority(hConsolePipe, THREAD_PRIORITY_NORMAL);
	SetThreadPriority(hResponseServer, THREAD_PRIORITY_ABOVE_NORMAL);
	SetThreadPriority(hAcceptServer, THREAD_PRIORITY_HIGHEST);

	LoadLib();

	WaitForSingleObject(hAcceptServer, INFINITE);
	CloseHandle(hAcceptServer);
	WaitForSingleObject(hDispatchServer, INFINITE);
	CloseHandle(hDispatchServer);
	WaitForSingleObject(hGarbageCleaner, INFINITE);
	CloseHandle(hGarbageCleaner);
	WaitForSingleObject(hConsolePipe, INFINITE);
	CloseHandle(hConsolePipe);
	WaitForSingleObject(hResponseServer, INFINITE);
	CloseHandle(hResponseServer);

	FreeLibrary(st);
	return 0;
};


