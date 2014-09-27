/*
 *  Based on Tyler Allen's IrcBot.h
 *	http://www.the-tech-tutorial.com/
 */


#include "Lyrica.h"
#include <iostream>
//#include <stdio.h>
//#include <stdlib.h>
//#include <sys/types.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <errno.h>
//#include <string.h>
//#include <arpa/inet.h>
//#include <sys/wait.h>
//#include <signal.h>
#include <time.h>
#include <Windows.h>

#pragma comment(lib, "ws2_32.lib") // Winsock

using std::endl;
using std::cout;
using std::cin;

// For now, edit settings here (later on config file read?)

#define DEFAULT_PORT "6667"								// Port used for connection (Rizon: 6660-6670, 7000, or SSL - 6697, 9999)
#define IRC_URL "irc.rizon.net"							// Server URL or IP
#define MAXDATASIZE 240									// Data size testing
char DEFAULT_NICKNAME[] = "LyricaPrismriver";			// Nickname
char DEFAULT_CHANNEL[] = "#testchannel";				// Channel to join onserver/do default commands at
char NICK_AUTH_PW[] = "password";						// Password to use when identifying with NickServ

//

Lyrica::Lyrica(char * _nick, char * _usr)
{
	nick = _nick;
	usr = _usr;
}

Lyrica::~Lyrica()
{
	closesocket(s);
}

int Lyrica::start()
{
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;

	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;

//	setup = true;

	ZeroMemory(&hints, sizeof(hints));

	hints.ai_family = AF_UNSPEC;			// Returns either v6 or v4 IP
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Initialize WinSock

	printf("Initializing WinSock... ");
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("Initialization failed. Error code: %d\n", WSAGetLastError());
		return 1;
	}

	printf("Initialized.\n");

	// Resolving server address and port

	iResult = getaddrinfo(IRC_URL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0)
	{
		printf("getaddrinfo failed. Error code: %d\n", iResult);
		WSACleanup(); getchar(); return 1;
	}

	// Setup & connect socket

	ptr = result;

	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

	iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		closesocket(ConnectSocket);
		ConnectSocket = INVALID_SOCKET;
	}

	// Socket Error catching

	if (ConnectSocket == INVALID_SOCKET)
	{
		printf("Error at socket() - Error code: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup(); getchar(); return 1;
	}

	freeaddrinfo(result); // clean up, add a try/catch later

	if (ConnectSocket == INVALID_SOCKET)
	{
		printf("Unable to connect to server! \n");
		WSACleanup(); getchar(); return 1;
	}

	printf("Success!");

	s = ConnectSocket;
	
	int numbytes;
	char buf[MAXDATASIZE];

	int count = 0;
	while (1)
	{
		count++;

		if (count == 3)
		{
			sendData(nick); sendData(usr);
		}
		else if (count == 4)
		{
			Sleep(3000);
			sendCmdString("PRIVMSG NickServ :IDENTIFY ", NICK_AUTH_PW);
			Sleep(5000);
			sendCmdString("JOIN ", DEFAULT_CHANNEL);
			sendCmdString("JOIN ", "#otakusdream");
		}
	}

	numbytes = recv(ConnectSocket, buf, MAXDATASIZE - 1, 0);
	buf[numbytes] = '\0';
	cout << (buf);

	msgHandle(buf);

	// Ping / Pong
	if (charSearch(buf, "PING"))
		sendPong(buf);

	if (numbytes == 0)
	{
		cout << "----------------------CONNECTION CLOSED---------------------------" << endl;
		cout << timeNow() << endl;
	}


	return 0;
}

bool Lyrica::charSearch(char *toSearch, char *searchFor)
{
	int len = strlen(toSearch);
	int forLen = strlen(searchFor);
	bool found = NULL;

	for (int i = 0; i < len; i++)
	{
		if (searchFor[0] == toSearch[i])
		{
			found = true;
			for (int x = 1; x < forLen; x++)
			{
				if (toSearch[i + x] != searchFor[x])
					found = false;
			}
		}

		if (found == true)
			return found;
	}
	return found;
}

bool Lyrica::isConnected(char *buf)
{
	if (charSearch(buf, "/MOTD") == true)
		return true;
	else
		return false;
}

char * Lyrica::timeNow()
{
	time_t rawtime;
	struct tm * timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	return asctime(timeinfo);
}

bool Lyrica::sendData(char *msg)
{
	int len = strlen(msg);
	int bytes_sent = send(s, msg, len, 0);

	if (bytes_sent == 0)
		return false;
	else
		return true;
}

void Lyrica::sendPong(char *buf)
{
	char * toSearch = "PING";
	bool found = NULL;

	for (std::size_t i = 0; i < strlen(buf); i++)
	{
		if (buf[i] == toSearch[0])
		{
			found = true;
			for (int x = 1; x < 4; x++)
			{
				if (buf[i + x] != toSearch[x])
					found = false;
			}

			if (found == true)
			{
				int count = 0;
				for (std::size_t x = (i + strlen(toSearch)); x < strlen(buf); x++)
				{
					count++;
				}

				char* returnHost = new char[count+5];
				returnHost[0] = 'P';
				returnHost[1] = 'O';
				returnHost[2] = 'N';
				returnHost[3] = 'G';
				returnHost[4] = ' ';

				count = 0;

				for (std::size_t x = (i + strlen(toSearch)); x < strlen(buf); x++)
				{
					returnHost[count + 5] = buf[x];
					count++;
				}

				if (sendData(returnHost))
				{
					cout << timeNow() << "  Ping Pong" << endl;
				}

				return;
			}
		}
	}
}

void Lyrica::msgHandle(char *buf)
{
	/*
	Write handlers here
	*/
}

void Lyrica::sendCmdString(char *cmd, char *usrinput)
{
	char * datasent = (char *)malloc(1 + strlen(cmd) + strlen(usrinput));
	strcpy(datasent, cmd);
	strcat(datasent, usrinput);
	strcat(datasent, "\r\n");
	sendData(datasent);
}

int main()
{
	Lyrica lyrica = Lyrica("NICK LyricaPrismriver\r\n", "USER Lyrica tolmoon tolsun :Lyrica\r\n");

	lyrica.start();

	return 0;
}
