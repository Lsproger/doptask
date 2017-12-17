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

	HANDLE hAcceptServer,    // дескриптор потока AcceptServer
		hConsolePipe,     // дескриптор потока ConsolePipe 
		hGarbageCleaner,  // дескриптор потока GarbageCleaner
		hDispatchServer,  // дескриптор потока GarbageCleaner
		hResponseServer;  //дескриптор потока ResponseServer

	HANDLE hClientConnectedEvent = CreateEvent(NULL,
		FALSE, //автоматический сброс
		FALSE,
		L"ClientConnected");

	CRITICAL_SECTION scListContact;

	enum TalkersCommand {
		START, STOP, EXIT, STATISTICS, WAIT, SHUTDOWN, GETCOMAND
	};
	volatile TalkersCommand  previousCommand = GETCOMAND;      // предыдущая команда клиента 
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
	typedef list<Contact> ListContact;
	ListContact contacts;
	SOCKET sSUDP;
	HANDLE(*ts)(char*, LPVOID);
	HMODULE st;
	
	bool  GetRequestFromClient(
		char*            name, //[in] позывной сервера  
		short            port, //[in] номер просушиваемого порта 
		SOCKADDR_IN* from, //[out] указатель на SOCKADDR_IN
		int*             flen  //[out] указатель на размер from 
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

		DWORD WINAPI AcceptServer(LPVOID pPrm);  // прототип функций 

		DWORD WINAPI ConsolePipe(LPVOID pPrm);

		DWORD WINAPI GarbageCleaner(LPVOID pPrm);

		DWORD WINAPI DispatchServer(LPVOID pPrm);

		DWORD WINAPI ResponseServer(LPVOID pPrm);
	}
}