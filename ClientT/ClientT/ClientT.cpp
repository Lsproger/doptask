// ServerT.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "Winsock2.h"
#pragma comment(lib, "WS2_32.lib") 
#include <string>
#include <iostream>
#include <ctime>
#include "ErrorFunctions.h"
#include <chrono>

using namespace std;
using namespace chrono;
//...................................................................


int _tmain(int argc, char* argv[])
{
	SOCKET  cS;           // дескриптор сокета 
	WSADATA wsaData;
	for (;;) {
		for (int connectionCounter = 0; connectionCounter < 1; connectionCounter++) {
			try
			{
				int port = 2000;
				char Name[20] = " ";
				char Calls[20] = "";
				if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
					throw  SetErrorMsgText("Startup:", WSAGetLastError());
				if ((cS = socket(AF_INET, SOCK_STREAM, NULL)) == INVALID_SOCKET)
					throw  SetErrorMsgText("socket:", WSAGetLastError());
				//...........................................................
				SOCKET  cC;                          // серверный сокет
				if ((cC = socket(AF_INET, SOCK_STREAM, NULL)) == INVALID_SOCKET)
					throw  SetErrorMsgText("socket:", WSAGetLastError());

				SOCKADDR_IN serv;                    // параметры  сокета сервера
				serv.sin_family = AF_INET;           // используется IP-адресация  
				serv.sin_port = htons(2000);                   // TCP-порт 2000
				serv.sin_addr.s_addr = inet_addr("127.0.0.1");  // адрес сервера
				if ((connect(cC, (sockaddr*)&serv, sizeof(serv))) == SOCKET_ERROR)
					throw  SetErrorMsgText("connect:", WSAGetLastError());
				//..............................................................

				char message[50],                     //буфер ввода 
					obuf[50]; /*= "sever: принято ";  //буфер вывода*/
				int  libuf = 0,                    //количество принятых байт
					lobuf = 0;                    //количество отправленных байь 

				cout << "Enter service type\n";
				char outMessage[5] = "Echo";
				cin >> outMessage;

				//Sleep(4000);
				if ((lobuf = send(cC, outMessage, strlen(outMessage) + 1, NULL)) == SOCKET_ERROR)
					throw  SetErrorMsgText("send:", WSAGetLastError());


				cout << "seneded: " << outMessage << endl;
				if (!strcmp(outMessage, "Echo")) {

					if ((libuf = recv(cC, message, sizeof(message), NULL)) == SOCKET_ERROR)
						throw  SetErrorMsgText("recv:", WSAGetLastError());//ожидение сообщения*

					if (strcmp(message, "TimeOUT") == 0) {
						cout << "time out" << endl;
						return -1;
					}
					for (;;) {

						cout << "Enter string:\n";
						cin >> outMessage;
						if (atoi(outMessage) == 1) strcpy(outMessage, "");
						if ((lobuf = send(cC, outMessage, strlen(outMessage) + 1, NULL)) == SOCKET_ERROR)
							throw  SetErrorMsgText("send:", WSAGetLastError());
						cout << "send:" << outMessage << endl;
						//Sleep(1000);
						if ((libuf = recv(cC, message, sizeof(message), NULL)) == SOCKET_ERROR)
							throw  SetErrorMsgText("recv:", WSAGetLastError());//ожидение сообщения*
					//	cout << SetErrorMsgText<<endl;
						if (strcmp(message, "TimeOUT") == 0) {
							cout << "time out" << endl;
							return -1;
						}

						cout << "receive:" << message << endl;


					}
				}
				else if (!strcmp(outMessage, "Time")) {


					if ((libuf = recv(cC, message, sizeof(message), NULL)) == SOCKET_ERROR)
						throw  SetErrorMsgText("recv:", WSAGetLastError());//ожидение сообщения*




					cout << "receive:" << message << endl;
					Sleep(5000);

				}

				else if (!strcmp(outMessage, "Rand")) {

					if ((libuf = recv(cC, message, sizeof(message), NULL)) == SOCKET_ERROR)
						throw  SetErrorMsgText("recv:", WSAGetLastError());//ожидение сообщения*

					if (strcmp(outMessage, "TimeOUT") == 0) {
						cout << "time out" << endl;
						return -1;
					}
					cout << "recv random message:" << message << endl;

				}
				else if (strcmp(outMessage, "Echo") != 0 && strcmp(outMessage, "Time") && strcmp(outMessage, "Rand")) {
					if ((libuf = recv(cC, message, sizeof(message), NULL)) == SOCKET_ERROR)
						throw  SetErrorMsgText("recv:", WSAGetLastError());//ожидение сообщения*

					cout << "receive:" << message << endl;
				}

				if (closesocket(cS) == SOCKET_ERROR)
					throw  SetErrorMsgText("closesocket:", WSAGetLastError());
				if (WSACleanup() == SOCKET_ERROR)
					throw  SetErrorMsgText("Cleanup:", WSAGetLastError());


				//	cout << strlen(obuf);
			}

			catch (string errorMsgText)
			{
				cout << endl << errorMsgText;
			}
		}
	}
	system("pause");
	return 0;
}
