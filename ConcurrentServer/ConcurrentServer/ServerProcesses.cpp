#pragma once
#include "stdafx.h"
#include "Winsock2.h"
#include "ErrorFunctions.h"
#include <iostream>
#include <string>
#include <list>
#include <time.h>
#include "ServerProcesses.h"

namespace {
	
	
	void CALLBACK ASWTimer(LPVOID Prm, DWORD, DWORD) {
		Contact *contact = (Contact*)(Prm);
		cout << "ASWTimer is calling " << contact->hthread << endl;

		Beep(500, 100); // звоним!

		TerminateThread(contact->serverHThtead, NULL);
		send(contact->s, "TimeOUT", strlen("TimeOUT") + 1, NULL);
		EnterCriticalSection(&scListContact);
		CancelWaitableTimer(contact->htimer);
		contact->type = contact->EMPTY;
		contact->sthread = contact->TIMEOUT;
		LeaveCriticalSection(&scListContact);
		//InterlockedIncrement(&sayNoCount);
	}

	void CALLBACK ASFinishMessage(LPVOID Prm) {
		Contact *contact = (Contact*)(Prm);

		CancelWaitableTimer(contact->htimer);
		contact->sthread = contact->TIMEOUT;
		contact->type = contact->EMPTY;
	}

	VOID CALLBACK TimerAPCProc(LPVOID, DWORD, DWORD)
	{
		Beep(500, 100); // звоним!
		cout << "TimerAPCPRoc is running\n" << endl;
	};

	bool AcceptCycle(int squirt)
	{

		bool rc = false;
		Contact c(Contact::ACCEPT, "EchoServer");

		while (squirt-- > 0 && rc == false)
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
				InterlockedIncrement(&connectionCount);

			}
		}
		return rc;
	};

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

		if (ioctlsocket(sS, FIONBIO, &(nonblk = 1)) == SOCKET_ERROR)
			throw SetErrorMsgText("ioctlsocket:", WSAGetLastError());
	}

	void closeSocket() {
		if (closesocket(sS) == SOCKET_ERROR)
			throw  SetErrorMsgText("closesocket:", WSAGetLastError());
	}

	void CommandsCycle(TalkersCommand& cmd)      // цикл обработки команд
	{
		int  squirt = 0;
		while (cmd != EXIT)     // цикл обработки команд консоли и подключений
		{
			switch (cmd)
			{
			case START: cmd = GETCOMAND; // возобновить подключение клиентов
				if (previousCommand != START) {
					squirt = AS_SQUIRT;
					cout << "Start command" << endl;
					openSocket();
					previousCommand = START;
				}
				else cout << "start already in use" << endl;
				break;
			case STOP:  cmd = GETCOMAND; // остановить подключение клиентов   
				if (previousCommand != STOP) {
					squirt = 0;
					cout << "Stop command" << endl;
					closeSocket();
					previousCommand = STOP;
				}
				else cout << "stop already in use" << endl;

				break;
			case WAIT:  cmd = GETCOMAND; // Команда приостанавливает подключение клиентов до тех пор, пока не обслужится последний клиент, подключенный к серверу.   
				squirt = 0;
				cout << "Wait command" << endl;

				cout << "сокет закрыт для ожидиния заветрешия обслуживания клиентов" << endl;
				closeSocket();
				while (contacts.size() != 0); // ожидание завершения 
				cout << "size of contacts" << contacts.size() << endl;
				cmd = START;
				previousCommand = WAIT;
				cout << "сокет открыт" << endl;

				break;
			case SHUTDOWN:  // Команда равносильна последовательности команд: wait, exit.    
				squirt = 0;

				cout << "SHUTDOWN command" << endl;

				cout << "........выключение..........." << endl;
				closeSocket();
				while (contacts.size() != 0); // ожидание завершения 
				cout << "size of contacts" << contacts.size() << endl;
				cmd = EXIT;

				break;
			case GETCOMAND:  cmd = GETCOMAND; // Служебная команда, которая не предназначена для ввода с консоли управления, а устанавливается сервером для указания, что сервер готов принять и обработать, очередную команду управления.

				break;
			};
			if (cmd != STOP) {
				if (AcceptCycle(squirt))   //цикл  запрос/подключение (accept)
				{
					cmd = GETCOMAND;
					//.... запуск потока EchoServer.......................
					SetEvent(hClientConnectedEvent); // сигнальное состояние события
				}
				else SleepEx(0, TRUE);    // выполнить асинхронные процедуры 
			}
		};
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

	extern "C" {
		bool InitSection()
		{
			InitializeCriticalSection(&scListContact);
			return true;
		}

		void SetServerParams(int port, char* lib, char* npName)
		{
			serverPort = port;
			strcpy(dllName, lib);
			strcpy(namedPipeName, npName);
		}
		
		void LoadLib()
		{
			wchar_t wtext[20];
			mbstowcs(wtext, dllName, strlen(dllName) + 1);//Plus null
			LPWSTR lib = wtext;
			st = LoadLibrary(lib);
			ts = (HANDLE(*)(char*, LPVOID))GetProcAddress(st, "SSS");
		}

		DWORD WINAPI AcceptServer(LPVOID pPrm)    // прототип 
		{
			DWORD rc = 0;    // код возврата 
			WSADATA wsaData;
			try
			{
				if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
					throw  SetErrorMsgText("Startup:", WSAGetLastError());

				CommandsCycle(*((TalkersCommand*)pPrm));

			}
			catch (string errorMsgText)
			{
				cout << endl << errorMsgText;
			}
			cout << "shutdown acceptServer" << endl;
			ExitThread(*(DWORD*)pPrm);  // завершение работы потока
		}

		DWORD WINAPI ConsolePipe(LPVOID pPrm)    // прототип 
		{
			//cout << "\nStarting Console...\n";
			DWORD rc = 0;    // код возврата

			char rbuf[100]; //буфер для чтения

			DWORD dwRead; // Количество принятых байт
			DWORD dwWrite; // Количество записанных байт
			HANDLE hPipe; // дескриптор канала
			DWORD myTime = 5;

			try
			{
				char namedPipeConnectionString[50] = "\\\\.\\pipe\\";
				strcat(namedPipeConnectionString, namedPipeName);

				if ((hPipe = CreateNamedPipeA(namedPipeConnectionString,
					PIPE_ACCESS_DUPLEX,           //дуплексный канал 
					PIPE_TYPE_MESSAGE | PIPE_WAIT,  // сообщения|синхронный
					1, NULL, NULL,               // максимум 1 экземпляр
					INFINITE, NULL)) == INVALID_HANDLE_VALUE)throw SetPipeError("create:", GetLastError());
				if (!ConnectNamedPipe(hPipe, NULL))   throw SetPipeError("connect:", GetLastError());

			}
			catch (string ErrorPipeText)
			{
				cout << endl << ErrorPipeText;
			}
			while (*((TalkersCommand*)pPrm) != EXIT) {
				
				if(ConnectNamedPipe(hPipe, NULL))
					cout << "enter to named pipe" << endl;
				for (;;) {
					bool check = ReadFile
					(
						hPipe,   // [in] дескриптор  канала
						rbuf,   // [out] указатель на буфер  ввода
						sizeof(rbuf),   // [in] количество читаемых байт
						&dwRead,   // [out] количество прочитанных байт  
						NULL    // [in,out] для асинхронной обработки 
					);
					if (check == FALSE) break;
					cout << "Сообщение от клиента:  " << rbuf << endl;
					int cmd = atoi(rbuf);
					if (cmd >= 0 && cmd < 6)
						*((TalkersCommand*)pPrm) = (TalkersCommand)cmd;
					if (cmd == 3)
					{
						char sendStastistics[200] = " ";
						strcpy(sendStastistics, "\nSTATS\nобщее количество подключений:    ");
						char numBuf1[10] = " ";
						itoa(connectionCount, numBuf1, 10);
						strcat(sendStastistics, numBuf1);

						strcat(sendStastistics, "\nобщее количество отказов:        ");

						itoa(sayNoCount, numBuf1, 10);
						strcat(sendStastistics, numBuf1);

						strcat(sendStastistics, "\nзавершилось успешно:             ");

						itoa(successConnections, numBuf1, 10);
						strcat(sendStastistics, numBuf1);

						strcat(sendStastistics, "\nколичество активных подключений: ");

						itoa(contacts.size(), numBuf1, 10);
						strcat(sendStastistics, numBuf1);
						strcat(sendStastistics, "\n-----------------\n");

						WriteFile
						(
							hPipe,   // [in] дескриптор  канала
							sendStastistics,   // [in] указатель на буфер  вывода
							sizeof(sendStastistics),   // [in] количество записываемых байт
							&dwWrite,   // [out] количество записанных байт  
							NULL    // [in,out] для асинхронной обработки 
						);
					}

					else if (!strcmp(rbuf, "getcomand")) {
						strcpy(rbuf, "getcomand");

					}
					else {
						strcpy(rbuf, "nocmd");
					}

					if (!(strcmp(rbuf, "3") == 0))
						WriteFile
						(
							hPipe,   // [in] дескриптор  канала
							rbuf,   // [in] указатель на буфер  вывода
							sizeof(rbuf),   // [in] количество записываемых байт
							&dwWrite,   // [out] количество записанных байт  
							NULL    // [in,out] для асинхронной обработки 
						);
				}

				cout << "--------------Канал закрыт-----------------" << endl;
				DisconnectNamedPipe(hPipe);
			}
			CloseHandle(hPipe);

			ExitThread(rc);  // завершение работы потока
		}

		DWORD WINAPI GarbageCleaner(LPVOID pPrm)    // прототип 
		{
			DWORD rc = 0;    // код возврата 	

			while (*((TalkersCommand*)pPrm) != EXIT) {
				//	Sleep(2000);
				int listSize = 0;
				int howMuchClean = 0;

				if (contacts.size() != 0) {
					for (auto i = contacts.begin(); i != contacts.end();) {
						if (i->type == i->EMPTY) {

							EnterCriticalSection(&scListContact);


							if (i->sthread == i->FINISH)
								InterlockedIncrement(&successConnections);
							if (i->sthread == i->ABORT || i->sthread == i->TIMEOUT)
								InterlockedIncrement(&sayNoCount);

							i = contacts.erase(i);
							howMuchClean++;
							listSize = contacts.size();
							LeaveCriticalSection(&scListContact);
						}
						else
							++i;
					}

					//cout << "GarbageCleaner size of clients @" << howMuchClean << "@" << endl;	

				}
			}
			cout << "shutdown garbageCleaner" << endl;
			ExitThread(rc);  // завершение работы потока
		}

		DWORD WINAPI DispatchServer(LPVOID pPrm)
		{
			DWORD rc = 0;    // код возврата 	
							 //cout << "marker 1" << endl;
			while (*(TalkersCommand*)pPrm != EXIT)     // цикл обработки команд консоли и подключений		
			{
				if (*(TalkersCommand*)pPrm != STOP) {

					WaitForSingleObject(hClientConnectedEvent, 10000);// ждем пока подключиться клилент
																	  // т.е. в acceptCycle событие перейдет в сигнальное состояние

					ResetEvent(hClientConnectedEvent);
					//	cout << "marker 2" << endl;

					EnterCriticalSection(&scListContact);
					for (auto i = contacts.begin(); i != contacts.end(); i++) {
						if (i->type == i->ACCEPT) {

							u_long nonblk;
							if (ioctlsocket(i->s, FIONBIO, &(nonblk = 0)) == SOCKET_ERROR)
								throw SetErrorMsgText("ioctlsocket:", WSAGetLastError());

							char serviceType[5] = "m";// = "Echo", "Time", "0001"
							clock_t start = clock();
							recv(i->s, serviceType, sizeof(serviceType), NULL);
							strcpy(i->msg, serviceType);

							clock_t delta = clock() - start;
							if (delta > 10000) {
								cout << "so long" << endl;
								i->sthread = i->TIMEOUT;
								if ((send(i->s, "TimeOUT", strlen("TimeOUT") + 1, NULL)) == SOCKET_ERROR)
									throw  SetErrorMsgText("send:", WSAGetLastError());

								if (closesocket(i->s) == SOCKET_ERROR)
									throw  SetErrorMsgText("closesocket:", WSAGetLastError());

								i->type = i->EMPTY;
							}
							else if (delta <= 10000) {

								if (strcmp(i->msg, "Echo") != 0 && strcmp(i->msg, "Time") != 0 && strcmp(i->msg, "Rand") != 0) {
									if ((send(i->s, "ErrorInquiry", strlen("ErrorInquiry") + 1, NULL)) == SOCKET_ERROR)
										throw  SetErrorMsgText("send:", WSAGetLastError());

									i->sthread = i->ABORT;
									i->type = i->EMPTY;
									if (closesocket(i->s) == SOCKET_ERROR)
										throw  SetErrorMsgText("closesocket:", WSAGetLastError());
								}
								else {
									i->type = i->CONTACT;
									i->hthread = hAcceptServer;
									i->serverHThtead = ts(serviceType, (LPVOID)&(*i));
									i->htimer = CreateWaitableTimer(0, FALSE, 0);
									LARGE_INTEGER Li;
									int seconds = 100;
									Li.QuadPart = -(10000000 * seconds);
									SetWaitableTimer(i->htimer, &Li, 0, ASWTimer, (LPVOID)&(*i), FALSE);
									//WaitForSingleObjectEx(i->htimer,  INFINITE,TRUE);

									//SleepEx(0, TRUE);
									//Sleep(200);
								}
							}
						}
					}
					SleepEx(0, TRUE);
					LeaveCriticalSection(&scListContact);

					//	cout << "marker 3" << endl;
					//Sleep(2000);
				}
			}
			cout << "shutdown dispatchServer" << endl;
			ExitThread(rc);  // завершение работы потока
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

			while (*(TalkersCommand*)pPrm != EXIT)     // цикл обработки команд консоли и подключений		
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