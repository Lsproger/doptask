#pragma once
#include "ErrorFunctions.h"
#include <time.h>
#include <string>
namespace {
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
			strcpy(msg, "a");
		};

		void SetST(ST sth, const char* m = "")
		{
			sthread = sth;
			strcpy(msg, m);
		}
	};

	string runServer;

	const std::string currentDateTime();

	void CALLBACK ASStartMessage(DWORD p);

	void CALLBACK ASFinishMessage(DWORD p);

	void QueueUserAPCWrapper(PAPCFUNC functionName, Contact *contact);
	extern"C" {
		DWORD WINAPI EchoServer(LPVOID pPrm);

		DWORD WINAPI TimeServer(LPVOID pPrm);

		DWORD WINAPI RandServer(LPVOID pPrm);
	}
}