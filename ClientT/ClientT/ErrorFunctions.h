#pragma once
#include "stdafx.h"
#include "Winsock2.h"
#pragma comment(lib, "WS2_32.lib") 
#include <string>
#include <iostream>

using namespace std;

string  GetErrorMsgText(int code);

string  SetErrorMsgText(string msgText, int code);

bool GetServer(
	char* call, //[in] позывной сервера 
	SOCKADDR_IN* from, //[out] указатель на SOCKADDR_IN
	int* flen, //[out] указатель на размер from 
	SOCKET * cC, //сокет
	SOCKADDR_IN * all
);