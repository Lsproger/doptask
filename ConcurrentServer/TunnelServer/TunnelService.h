#include "Winsock2.h"
#include "ErrorFunctions.h"
#include <iostream>
#include <string>
#include <list>
#include <time.h>

namespace{

	HANDLE hClientConnectedEvent = CreateEvent(NULL,
		FALSE, //автоматический сброс
		FALSE,
		L"ClientConnected");

	CRITICAL_SECTION scListContact;

	struct Contact         // элемент списка подключений       
	{
		enum TE {               // состояние  сервера подключения  
			EMPTY,              // пустой элемент списка подключений 
			ACCEPT,             // подключен (accept), но не обслуживается
			CONTACT             // передан обслуживающему серверу  
		}    type;     // тип элемента списка подключений 
		enum ST {               // состояние обслуживающего сервера  
			WORK,               // идет обмен данными с клиентом
			ABORT,              // обслуживающий сервер завершился не нормально 
			TIMEOUT,            // обслуживающий сервер завершился по времени 
			FINISH              // обслуживающий сервер завершился  нормально 
		}      sthread; // состояние  обслуживающего сервера (потока)

		SOCKET      s;         // сокет для обмена данными с клиентом
		SOCKADDR_IN prms;      // параметры  сокета 
		int         lprms;     // длина prms 
		HANDLE      hthread;   // handle потока (или процесса) 
		HANDLE      htimer;    // handle таймера
		HANDLE		serverHThtead;// handle обслуживающего сервера который в последствие может зависнуть

		char msg[50];           // сообщение 
		char srvname[15];       //  наименование обслуживающего сервера 

		Contact(TE t = EMPTY, const char* namesrv = "") // конструктор 
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
		char*            name, //[in] позывной сервера  
		short            port, //[in] номер просушиваемого порта 
		SOCKADDR_IN* from, //[out] указатель на SOCKADDR_IN
		int*             flen  //[out] указатель на размер from 
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