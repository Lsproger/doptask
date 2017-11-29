#include "stdafx.h"
#include "Service.h"
namespace {
	const std::string currentDateTime() {
		time_t     now = time(0);
		struct tm  tstruct;
		char       buf[80];
		tstruct = *localtime(&now);


		strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

		return buf;
	}

	void CALLBACK ASStartMessage(DWORD p)
	{
		char *hostName = new char(4);
		gethostname(hostName, sizeof(hostName));

		cout << "RunServer: " << runServer << " start time: " << currentDateTime() << endl;

	}

	void CALLBACK ASFinishMessage(DWORD p)
	{
		cout << " finish time: " << currentDateTime() << endl;
	}

	void QueueUserAPCWrapper(PAPCFUNC functionName, Contact *contact) {
		QueueUserAPC(functionName, contact->hthread, 0);
	}
	extern "C" {
		DWORD WINAPI EchoServer(LPVOID pPrm)
		{
			DWORD rc = 0;
			Contact *contact = (Contact*)(pPrm);
			try
			{
				runServer = "EchoServer\n";
				QueueUserAPCWrapper(ASStartMessage, contact);

				cout << "Entry to Echo server{\n";
				int lobuf, libuf;
				contact->sthread = contact->WORK;
				contact->type = contact->CONTACT;

				if ((lobuf = send(contact->s, "Echo", strlen("Echo") + 1, NULL)) == SOCKET_ERROR)
					throw  SetErrorMsgText("send:", WSAGetLastError());

				int	whenINeedToStop = 1;

				while (true) {

					//	itoa(1, contact->msg, 10);

					if ((libuf = recv(contact->s, contact->msg, sizeof(contact->msg), NULL)) == SOCKET_ERROR)
						throw  SetErrorMsgText("recv:", WSAGetLastError());//ожидение сообщения*
					cout << "recv:" << contact->msg << endl;
					if ((lobuf = send(contact->s, contact->msg, strlen(contact->msg) + 1, NULL)) == SOCKET_ERROR)
						throw  SetErrorMsgText("send:", WSAGetLastError());

					cout << "send:" << contact->msg << endl;
					if (strlen(contact->msg) == 0)break;
				}
				cout << "end Echo}\n";
			}
			catch (...)
			{
				cout << "EchoServer: abnormal termination" << endl;
				contact->sthread = contact->ABORT;
				contact->type = contact->EMPTY;
				rc = contact->sthread;

				QueueUserAPCWrapper(ASFinishMessage, contact);
				CancelWaitableTimer(contact->htimer);
				ExitThread(rc);
			}
			contact->sthread = contact->FINISH;
			contact->type = contact->EMPTY;
			rc = contact->sthread;

			QueueUserAPCWrapper(ASFinishMessage, contact);
			CancelWaitableTimer(contact->htimer);
			ExitThread(rc);


		}

		DWORD WINAPI TimeServer(LPVOID pPrm)
		{
			DWORD rc = 0;
			Contact *contact = (Contact*)(pPrm);

			runServer = "TimeServer";
			int lobuf, libuf;

			QueueUserAPCWrapper(ASStartMessage, contact);

			strcpy(contact->msg, currentDateTime().c_str());
			if ((lobuf = send(contact->s, contact->msg, strlen(contact->msg) + 1, NULL)) == SOCKET_ERROR)
				throw  SetErrorMsgText("send:", WSAGetLastError());

			contact->sthread = contact->FINISH;
			contact->type = contact->EMPTY;
			rc = contact->sthread;
			QueueUserAPCWrapper(ASFinishMessage, contact);
			CancelWaitableTimer(contact->htimer);
			ExitThread(rc);
		}

		DWORD WINAPI RandServer(LPVOID pPrm)
		{

			DWORD rc = 0;
			Contact *contact = (Contact*)(pPrm);
			runServer = "RandServer";
			QueueUserAPCWrapper(ASStartMessage, contact);

			srand(time(NULL));
			int lobuf, libuf, randNumber;
			randNumber = rand() % 100 + 1;
			itoa(randNumber, contact->msg, 10);

			if ((lobuf = send(contact->s, contact->msg, strlen(contact->msg) + 1, NULL)) == SOCKET_ERROR)
				throw  SetErrorMsgText("send:", WSAGetLastError());

			contact->sthread = contact->FINISH;
			contact->type = contact->EMPTY;
			rc = contact->sthread;

			QueueUserAPCWrapper(ASFinishMessage, contact);
			CancelWaitableTimer(contact->htimer);
			ExitThread(rc);
		}
	}
}