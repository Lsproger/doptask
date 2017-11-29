#include "Winsock2.h"
#include "ErrorFunctions.h"
#include <iostream>
#include <string>
#include <list>
#include <time.h>

namespace{

	HANDLE hClientConnectedEvent = CreateEvent(NULL,
		FALSE, //�������������� �����
		FALSE,
		L"ClientConnected");

	CRITICAL_SECTION scListContact;

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
	
	SOCKET sS;
	SOCKET sSUDP;
	
	int serverPort = 2000;
	char dllName[50] = " ";
	char namedPipeName[10] = " ";

	HANDLE hTunnelServer;
	HANDLE hResponseServer;
	HANDLE hAcceptServer;
	HANDLE hDispatchServer;

	typedef list<Contact> ListContact;
	ListContact contacts;

	bool  GetRequestFromClient(
		char*            name, //[in] �������� �������  
		short            port, //[in] ����� �������������� ����� 
		SOCKADDR_IN* from, //[out] ��������� �� SOCKADDR_IN
		int*             flen  //[out] ��������� �� ������ from 
	);
	bool PutAnswerToClient(char * name, struct sockaddr* to, int * lto);

	void closeSocket();
	void openSocket();


	bool AcceptCycle();

	extern "C" {
		bool InitSection();
		DWORD WINAPI AcceptServer(LPVOID pPrm);
		DWORD WINAPI DispatchServer(LPVOID pPrm);
		DWORD WINAPI TunnelServer(LPVOID pPrm);
		DWORD WINAPI ResponseServer(LPVOID pPrm);
	}

}