#include "stdafx.h"
#include "TunnelService.h"

namespace {

	bool AcceptCycle()
	{
		bool rc = false;
		Contact c(Contact::ACCEPT, "EchoServer");

		while (rc == false)
		{
			if ((c.s = accept(sS,
				(sockaddr*)&c.prms, &c.lprms)) == INVALID_SOCKET)
			{
				if (WSAGetLastError() != WSAEWOULDBLOCK)
					throw  SetErrorMsgText("accept:", WSAGetLastError());
			}
			else
			{
				rc = true;               // подключился
				EnterCriticalSection(&scListContact);
				contacts.push_front(c);
				LeaveCriticalSection(&scListContact);
				cout << "contact connected" << endl;
				SetEvent(hClientConnectedEvent);
			}
			SleepEx(0, TRUE);
		}
		SleepEx(0, TRUE);
		return rc;
	};

	bool  GetRequestFromClient(char* name, short port, SOCKADDR_IN* from, int* flen)
	{
		SOCKADDR_IN clnt;               // параметры  сокета клиента
		memset(&clnt, 0, sizeof(clnt));   // обнулить память

		int lc = sizeof(clnt);
		char ibuf[500];                  //буфер ввода 
		int  lb = 0;                    //количество принятых байт
		int optval = 1;
		int TimeOut = 1000;
		setsockopt(sSUDP, SOL_SOCKET, SO_BROADCAST, (char*)&optval, sizeof(int));
		setsockopt(sSUDP, SOL_SOCKET, SO_RCVTIMEO, (char*)&TimeOut, sizeof(TimeOut));
		while (true) {
			if ((lb = recvfrom(sSUDP, ibuf, sizeof(ibuf), NULL, (sockaddr*)&clnt, &lc)) == SOCKET_ERROR)
				//throw  SetErrorMsgText("recfrom:", WSAGetLastError());
			{
				return false;
			}

			//	if (GetLastError() == WSAETIMEDOUT) return false;

			if (!strcmp(name, ibuf)) {
				*from = clnt;
				*flen = lc;
				return true;
			}
			cout << "\nbad name\n";
		}
		return false;
	}

	bool PutAnswerToClient(char * name, struct sockaddr* to, int * lto) {

		if ((sendto(sSUDP, name, sizeof(name) + 1, NULL, to, *lto)) == SOCKET_ERROR)

			throw  SetErrorMsgText("sendto:", WSAGetLastError());
		return false;
	}

	void openSocket() {
		SOCKADDR_IN serv;                     // параметры  сокета sS
		sockaddr_in clnt;
		u_long nonblk;
		int lclnt;

		if ((sS = socket(AF_INET, SOCK_STREAM, NULL)) == INVALID_SOCKET)
			throw  SetErrorMsgText("socket:", WSAGetLastError());

		lclnt = sizeof(clnt);
		serv.sin_family = AF_INET;           // используется IP-адресация  
		serv.sin_port = htons(serverPort);          // порт 2000
		serv.sin_addr.s_addr = INADDR_ANY; //inet_addr("192.168.0.111"); ;   // любой собственный IP-адрес 

		if (bind(sS, (LPSOCKADDR)&serv, sizeof(serv)) == SOCKET_ERROR)
			throw  SetErrorMsgText("bind:", WSAGetLastError());

		if (listen(sS, SOMAXCONN) == SOCKET_ERROR)
			throw  SetErrorMsgText("listen:", WSAGetLastError());

		//	if (ioctlsocket(sS, FIONBIO, &(nonblk = 1)) == SOCKET_ERROR)
			//	throw SetErrorMsgText("ioctlsocket:", WSAGetLastError());
	}

	void closeSocket() {
		if (closesocket(sS) == SOCKET_ERROR)
			throw  SetErrorMsgText("closesocket:", WSAGetLastError());
	}

	extern "C" {
		bool InitSection()
		{
			InitializeCriticalSection(&scListContact);
			return true;
		}

		DWORD WINAPI AcceptServer(LPVOID pPrm)    // прототип 
		{
			DWORD rc = 0;    // код возврата 
			WSADATA wsaData;
			try
			{
				if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
					throw  SetErrorMsgText("Startup:", WSAGetLastError());
				openSocket();
				while (true)
					AcceptCycle();
				closeSocket();
			}
			catch (string errorMsgText)
			{
				cout << endl << errorMsgText;
			}
			cout << "shutdown acceptServer" << endl;
			ExitThread(rc);  // завершение работы потока
		}

		DWORD WINAPI DispatchServer(LPVOID pPrm)
		{
			DWORD rc = 0;    // код возврата 	
							 //cout << "marker 1" << endl;
			while (true)     // цикл обработки команд консоли и подключений		
			{
				WaitForSingleObject(hClientConnectedEvent, INFINITE);// ждем пока подключиться клилент
																  // т.е. в acceptCycle событие перейдет в сигнальное состояние
				ResetEvent(hClientConnectedEvent);
				//	cout << "marker 2" << endl;
				EnterCriticalSection(&scListContact);
				for (auto i = contacts.begin(); i != contacts.end(); i++) {
					if (i->type == i->ACCEPT) {

						u_long nonblk;
						if (ioctlsocket(i->s, FIONBIO, &(nonblk = 0)) == SOCKET_ERROR)
							throw SetErrorMsgText("ioctlsocket:", WSAGetLastError());
						recv(i->s, i->msg, sizeof(i->msg), NULL);
						//strcpy(i->msg, serviceType);
						i->type = i->CONTACT;
						i->hthread = hAcceptServer;
						i->serverHThtead = CreateThread(NULL, NULL, TunnelServer, (LPVOID)&(*i), NULL, NULL);
					}
					SleepEx(0, TRUE);
					LeaveCriticalSection(&scListContact);
				}
			}
			cout << "shutdown dispatchServer" << endl;
			ExitThread(rc);  // завершение работы потока
		}

		DWORD WINAPI TunnelServer(LPVOID pPrm)
		{
			Contact *contact = (Contact*)(pPrm);
			cout << "Tunnel Server started!";
			cout << "recieved: " << contact->msg << endl;
			DWORD rc = 0;
			WSADATA wsaData;
			try
			{
				if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
					throw  SetErrorMsgText("Startup:", WSAGetLastError());

				SOCKET  cC;                          // серверный сокет
				if ((cC = socket(AF_INET, SOCK_STREAM, NULL)) == INVALID_SOCKET)
					throw  SetErrorMsgText("socket:", WSAGetLastError());

				SOCKADDR_IN serv;                    // параметры  сокета сервера
				serv.sin_family = AF_INET;           // используется IP-адресация  
				serv.sin_port = htons(2001);                   // TCP-порт 2000
				serv.sin_addr.s_addr = inet_addr("127.0.0.1");  // адрес сервера
				if ((connect(cC, (sockaddr*)&serv, sizeof(serv))) == SOCKET_ERROR)
					throw  SetErrorMsgText("connect:", WSAGetLastError());

				char rclnt[50] = "";
				char sclnt[50] = "";
				char rsrv[50] = "";
				char ssrv[50] = "";
				int rl;
				int sl;
				strcpy(rclnt, contact->msg);

				for (;;)
				{
					try
					{
						
						if ((sl = send(cC, rclnt, strlen(rclnt) + 1, NULL)) == SOCKET_ERROR)
							throw SetErrorMsgText("Client send:", GetLastError());
						cout << "sended to server: " << rclnt << endl;

						if ((rl = recv(cC, rclnt, sizeof(rsrv), NULL)) == SOCKET_ERROR)
							throw SetErrorMsgText("Server recv:", GetLastError());
						cout << "recv from server:" << rclnt << endl;

						if ((sl = send(contact->s, rclnt, strlen(rsrv) + 1, NULL)) == SOCKET_ERROR)
							throw SetErrorMsgText("Client send:", GetLastError());
						cout << "sended to client: " << rclnt  << endl;

						if ((rl = recv(contact->s, rclnt, sizeof(rclnt), NULL)) == SOCKET_ERROR)
							throw SetErrorMsgText("Client recv:", GetLastError());
						cout << "recv from client: " << rclnt << endl;
					}
					catch (string errMsg) {
						cout << endl << errMsg << endl;
					}
				}

			}
			catch (string errMsg)
			{
				cout << endl << errMsg << endl;
			}
			ExitThread(rc);
		}

		DWORD WINAPI ResponseServer(LPVOID pPrm)
		{
			DWORD rc = 0;    // код возврата 	

			WSADATA wsaData;
			SOCKADDR_IN serv;                     // параметры  сокета sS


			int lclnt;
			char message[5],                     //буфер ввода 
				obuf[50] = "sever: принято ";  //буфер вывода
			int  libuf = 0,                    //количество принятых байт
				lobuf = 0;                    //количество отправленных байт

			if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
				throw  SetErrorMsgText("Startup:", WSAGetLastError());

			if ((sSUDP = socket(AF_INET, SOCK_DGRAM, NULL)) == INVALID_SOCKET)
				throw  SetErrorMsgText("socket:", WSAGetLastError());

			serv.sin_family = AF_INET;           // используется IP-адресация  
			serv.sin_port = htons(serverPort);          // порт 2000
			serv.sin_addr.s_addr = INADDR_ANY;//inet_addr("192.168.43.78");   // любой собственный IP-адрес 

			if (bind(sSUDP, (LPSOCKADDR)&serv, sizeof(serv)) == SOCKET_ERROR)
				throw  SetErrorMsgText("bind:", WSAGetLastError());


			SOCKADDR_IN someServer;
			int serverSize = sizeof(someServer);

			SOCKADDR_IN from;               // параметры  сокета клиента
			memset(&from, 0, sizeof(from));   // обнулить память

			int lc = sizeof(from);
			char ibuf[500];                  //буфер ввода 
			int  lb = 0;                    //количество принятых байт

			char name[50];
			int numberOfClients = 0;

			while (true)     // цикл обработки команд консоли и подключений		
			{
				try
				{
					if (GetRequestFromClient("Hello", serverPort, &from, &lc) == true) {
						cout << "\nconnected Client: " << numberOfClients++;
						cout << " port: " << htons(from.sin_port);
						cout << " adress: " << inet_ntoa(from.sin_addr);
						PutAnswerToClient("Hello", (sockaddr*)&from, &lc);
					}
				}
				catch (string errorMsgText)
				{
					cout << endl << errorMsgText;
				}
			}
			if (closesocket(sSUDP) == SOCKET_ERROR)
				throw  SetErrorMsgText("closesocket:", WSAGetLastError());
			if (WSACleanup() == SOCKET_ERROR)
				throw  SetErrorMsgText("Cleanup:", WSAGetLastError());

			ExitThread(rc);  // завершение работы потока
		}
	}
}