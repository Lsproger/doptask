#include "Winsock2.h"
#include "ErrorFunctions.h"
#include <iostream>
#include <string>
#include <list>
#include <time.h>


namespace {
#define AS_SQUIRT 10

	SOCKET sS;
	int serverPort = 2000;
	char dllName[50] = " ";
	char namedPipeName[10] = " ";



	//server info
	volatile long connectionCount = 0;
	volatile long sayNoCount = 0;
	volatile long successConnections = 0;
	volatile long currentActiveConnections = 0;

	HANDLE hAcceptServer,    // ���������� ������ AcceptServer
		hConsolePipe,     // ���������� ������ ConsolePipe 
		hGarbageCleaner,  // ���������� ������ GarbageCleaner
		hDispatchServer,  // ���������� ������ GarbageCleaner
		hResponseServer;  //���������� ������ ResponseServer

	HANDLE hClientConnectedEvent = CreateEvent(NULL,
		FALSE, //�������������� �����
		FALSE,
		L"ClientConnected");

	CRITICAL_SECTION scListContact;

	enum TalkersCommand {
		START, STOP, EXIT, STATISTICS, WAIT, SHUTDOWN, GETCOMAND
	};
	volatile TalkersCommand  previousCommand = GETCOMAND;      // ���������� ������� ������� 
	struct Contact         // ������� ������ �����������       
	{
		enum TE {               // ���������  ������� �����������  
			EMPTY,              // ������ ������� ������ ����������� 
			ACCEPT,             // ��������� (accept), �� �� �������������
			CONTACT             // ������� �������������� �������  
		}    type;     // ��� �������� ������ ����������� 
		enum ST {               // ��������� �������������� �������  
			WORK,               // ���� ����� ������� � ��������
			ABORT,              // ������������� ������ ���������� �� ��������� 
			TIMEOUT,            // ������������� ������ ���������� �� ������� 
			FINISH              // ������������� ������ ����������  ��������� 
		}      sthread; // ���������  �������������� ������� (������)

		SOCKET      s;         // ����� ��� ������ ������� � ��������
		SOCKADDR_IN prms;      // ���������  ������ 
		int         lprms;     // ����� prms 
		HANDLE      hthread;   // handle ������ (��� ��������) 
		HANDLE      htimer;    // handle �������
		HANDLE		serverHThtead;// handle �������������� ������� ������� � ����������� ����� ���������

		char msg[50];           // ��������� 
		char srvname[15];       //  ������������ �������������� ������� 

		Contact(TE t = EMPTY, const char* namesrv = "") // ����������� 
		{
			memset(&prms, 0, sizeof(SOCKADDR_IN));
			lprms = sizeof(SOCKADDR_IN);
			type = t;
			strcpy(srvname, namesrv);
			msg[0] = 0;
		};

		void SetST(ST sth, const char* m = "")
		{
			sthread = sth;
			strcpy(msg, m);
		}
	};
	typedef list<Contact> ListContact;
	ListContact contacts;
	SOCKET sSUDP;
	HANDLE(*ts)(char*, LPVOID);
	HMODULE st;
	
	bool  GetRequestFromClient(
		char*            name, //[in] �������� �������  
		short            port, //[in] ����� �������������� ����� 
		SOCKADDR_IN* from, //[out] ��������� �� SOCKADDR_IN
		int*             flen  //[out] ��������� �� ������ from 
	);
	bool PutAnswerToClient(char * name, struct sockaddr* to, int * lto);

	VOID CALLBACK TimerAPCProc(LPVOID, DWORD, DWORD);
	void CALLBACK ASWTimer(LPVOID Prm, DWORD, DWORD);
	void CALLBACK ASFinishMessage(LPVOID Prm);
	void CommandsCycle(TalkersCommand& cmd);
	void closeSocket();
	void openSocket();
	bool AcceptCycle(int squirt);
	extern "C" {
		bool InitSection();

		void SetServerParams(int port, char* lib, char* npName);

		void LoadLib();

		DWORD WINAPI AcceptServer(LPVOID pPrm);  // �������� ������� 

		DWORD WINAPI ConsolePipe(LPVOID pPrm);

		DWORD WINAPI GarbageCleaner(LPVOID pPrm);

		DWORD WINAPI DispatchServer(LPVOID pPrm);

		DWORD WINAPI ResponseServer(LPVOID pPrm);
	}
}