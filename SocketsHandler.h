#pragma once

#include <time.h>
#include <winsock2.h>
#include <iostream>
#include "request.h"
#include "response.h"

enum eSendMode { s_EMPTY, IDLE, SEND };
enum eRecvMode { r_EMPTY, RECEIVE, LISTEN };


constexpr unsigned int MAX_SOCKETS = 60;
constexpr unsigned int INIT_BUFFER_SIZE = 4096;

class SocketState
{

public:
	SOCKET id;	// Socket handle
	eRecvMode Recv;
	eSendMode Send;
	response Response;
	request Request;
	time_t timeSinceLastByte; // Time since last byte read
	SocketState()
	{
		Request.raw_message.resize(INIT_BUFFER_SIZE);
		id = 0;
		Recv = r_EMPTY;
		Send = s_EMPTY;
		timeSinceLastByte = 0;
	}
	void init(const SOCKET& _id, eRecvMode r_mode)
	{
		reset();
		Recv = r_mode;
		id = _id;
	}
	void clear()
	{
		Recv = r_EMPTY;
		Send = s_EMPTY;
	}
	void reset()
	{
		Recv = RECEIVE;
		Send = IDLE;
	}

};

class SocketsHandler
{
public:
	const char* CRLF = "\r\n";
	const char* src_path;
	SocketState sockets[MAX_SOCKETS];
	size_t count = 0;
	void init_socket(size_t idx,const SOCKET& _id, eRecvMode r_mode)
	{
		if (idx < MAX_SOCKETS)
			sockets[idx].init(_id, r_mode);
	}
	void breakdownRequest(int);
	void generateResponse(int);
	void printPOSTmessage(int);
	void generateValidResponse(int);
	void generateINValidResponse(int);

	bool canCreateFile(int);
	bool isValidRequest(int);

	
};
