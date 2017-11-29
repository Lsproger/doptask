#pragma once
#include "stdafx.h"
#include "Winsock2.h"
#pragma comment(lib, "WS2_32.lib") 
#include <string>
#include <iostream>

using namespace std;

string  GetErrorMsgText(int code);
string  SetErrorMsgText(string msgText, int code);