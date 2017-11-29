#pragma once
#include "stdafx.h"
#include <string>
#include "Winsock2.h"
#include <iostream>
#include <string>
#include <list>
#include <time.h>
#include "ErrorFunctions.h"
#define RPipeName L"\\\\.\\pipe\\BOX"

using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	setlocale(LC_ALL, "Rus");

	try
	{
		printf("\nCommands:\n 1 - Start  \t \n 2 - Stop  \t \n 3 - Exit  \t \n 4 - Statistics  \n 5 - Wait  \t\n 6 - Shutdown  \t \n 7 - Finish RConsole\n\n");
		char ReadBuf[200] = " ";// Буффер для примема сообшения от сервера
		char WriteBuf[2] = " ";// Буффер для отправки сообщения серверу
		DWORD nBytesRead;
		DWORD nBytesWrite;

		int Code = 0;// Код команды

		char PipeName[512];
		
		HANDLE hNamedPipe = CreateFile(/*(LPCWSTR)PipeName*/ RPipeName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
		//0x017ffc8c "\\\\.\\pipe\\BOX"
		do
		{
			printf("Command: ");
			scanf("%d", &Code);
			if (Code>0 && Code<7)
			{
				sprintf(WriteBuf, "%d", Code - 1);
				if (!WriteFile(hNamedPipe, WriteBuf, strlen(WriteBuf) + 1, &nBytesWrite, NULL))
					throw SetPipeError("WriteFile: Ошибка ", GetLastError());
				if (ReadFile(hNamedPipe, ReadBuf, sizeof(ReadBuf), &nBytesRead, NULL))
					cout << ReadBuf << endl;
				else
					throw SetPipeError("ReadFile: Ошибка ", GetLastError());
			}
			else if (Code>7) printf("Неверная команда.\n\n");
		} while (Code != 7 && Code != 3 && Code != 6);
	}
	catch (char* ErrorPipeText)
	{
		printf("%s", ErrorPipeText);
		cout << GetLastError() << endl;
	}
	system("pause");
	return 0;
}
