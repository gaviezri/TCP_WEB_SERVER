#pragma once 
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
using namespace std;
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include "SocketsHandler.h"

constexpr unsigned int PORT = 27272;
constexpr unsigned int Two_Minutes = 120;

constexpr unsigned int servERR = 1;



class server
{
	WSAData wsaData;
	SOCKET listenSocket;
	sockaddr_in serverService;
	fd_set waitRecv;
	fd_set waitSend;
	SocketsHandler sock_handler;
	int nfd = 0;

	const char* src_path= "c:/tmp/";
	
public:
	server();
	int run();
private:
	bool addSocket(SOCKET id, eRecvMode r_mode);
	void makeFDset_select();
	void checkTimeout();
	void clearSocket(int idx)
	{
		SocketState& metasocket = sock_handler.sockets[idx];
		metasocket.Recv = r_EMPTY;
		metasocket.Send = s_EMPTY;
		metasocket.id = 0;
		metasocket.Request.reset();
		metasocket.Response.reset();
		metasocket.timeSinceLastByte = 0;
		--sock_handler.count;
	}
	void Service();
	void acceptConnection(int idx);
	void retrieveRequest(int idx);
	void sendResponse(int idx);
	bool extract_raw_fromSocket(int);
	char* receiveEntireMessage(int idx);
};

