#pragma once
#include "ErrorFunctions.h"
#include <time.h>
#include <string>
namespace {
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