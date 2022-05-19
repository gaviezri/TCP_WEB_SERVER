#include "server.h"


server::server()
{
	{
		if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
		{
			std::cout << "serv: Error at WSAStartup()\n";
			exit(1);
		}
		listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == listenSocket)
		{
			std::cout << "serv: Error at socket(): " << WSAGetLastError() << endl;
			WSACleanup();
			exit(1);
		}
		serverService.sin_family = AF_INET;
		serverService.sin_addr.s_addr = INADDR_ANY;
		serverService.sin_port = htons(PORT);
		if (SOCKET_ERROR == bind(listenSocket, (SOCKADDR*)&serverService, sizeof(serverService)))
		{
			std::cout << "serv: Error at bind(): " << WSAGetLastError() << endl;
			closesocket(listenSocket);
			WSACleanup();
			exit(1);
		}
		if (SOCKET_ERROR == listen(listenSocket, MAX_SOCKETS))
		{
			std::cout << "serv: Error at listen(): " << WSAGetLastError() << endl;
			closesocket(listenSocket);
			WSACleanup();
			exit(1);
		}
	}

	sock_handler.src_path= src_path;
}


int server::run()
{
	try
	{
		addSocket(listenSocket, LISTEN,NULL);
		cout << "Server is running at port:" << PORT << endl;
		while (true)
		{
			makeFDset_select();
			Service();
		}
		// Closing connections and Winsock.
		std::cout << "serv: Closing Connection.\n";
		closesocket(listenSocket);
		WSACleanup();
		return 0;
	}
	catch (...)
	{
		return 1;
	}
}


bool server::addSocket(SOCKET _id, eRecvMode r_mode,int* sock_idx)
{
	for (size_t i = 0; i < MAX_SOCKETS; i++)
	{
		if (sock_handler.sockets[i].Recv == r_EMPTY)
		{	
			sock_handler.init_socket(i,_id, r_mode);
			++sock_handler.count;
			if(sock_idx) *sock_idx = i;
			unsigned long flag = 1;
			if (ioctlsocket(sock_handler.sockets[i].id, FIONBIO, &flag))
			{
				std::cout << "serv: Error at ioctlsocket(): " << WSAGetLastError() << endl;
				throw servERR;
			}
			return (true);
		}
	}
	return (false);
}


void server::makeFDset_select()
{
	FD_ZERO(&waitRecv);
	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		if ((sock_handler.sockets[i].Recv == LISTEN) || (sock_handler.sockets[i].Recv == RECEIVE))
			FD_SET(sock_handler.sockets[i].id, &waitRecv);
	}


	FD_ZERO(&waitSend);
	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		if (sock_handler.sockets[i].Send == SEND)
			FD_SET(sock_handler.sockets[i].id, &waitSend);
	}
	timeval Timeout = { Two_Minutes,0 };
	nfd = select(0, &waitRecv, &waitSend, NULL, &Timeout);

	if (nfd == SOCKET_ERROR)
	{
		std::cout << "serv: Error at select(): " << WSAGetLastError() << endl;
		WSACleanup();
		throw servERR;
	}
}


void server::checkTimeout()
{
	time_t now = time(0);

	for (int i = 1; i < MAX_SOCKETS; i++)
	{
		if (now - sock_handler.sockets[i].timeSinceLastByte > Two_Minutes && sock_handler.sockets[i].timeSinceLastByte > 0)
		{
			closesocket(sock_handler.sockets[i].id);
			clearSocket(i);
		}
	}
}

void server::Service()
{
	checkTimeout();

	for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
	{	//receive requests / accept new connections
		if (FD_ISSET(sock_handler.sockets[i].id, &waitRecv))
		{
			--nfd;
			switch (sock_handler.sockets[i].Recv)
			{
			case LISTEN:
				acceptConnection(i);
				break;

			case RECEIVE:
				retrieveRequest(i);
				break;
			}
		}
	}

	for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
	{
		if (FD_ISSET(sock_handler.sockets[i].id, &waitSend))
		{
			--nfd;
			switch (sock_handler.sockets[i].Send)
			{
			case SEND:
				sendResponse(i);
				break;
			default:
				break;
			}
		}
	}
}

void server::acceptConnection(int idx)
{
	SOCKET id = sock_handler.sockets[idx].id;
	sockaddr_in from;	  // Address of sending partner
	int fromLen = sizeof(from);
	
	SOCKET msgSocket = accept(id, (struct sockaddr*)&from, &fromLen);
	if (INVALID_SOCKET == msgSocket)
	{
		std::cout << "serv:Error at accept(): " << WSAGetLastError() << endl;
		throw servERR;
	}
	//successful connection 
	std::cout << "serv: Client " << inet_ntoa(from.sin_addr) << ":" << ntohs(from.sin_port) << " is connected." << endl;
	int new_idx;
	if (!addSocket(msgSocket,RECEIVE,&new_idx))
	{//cant add socket
		std::cout << "\t\tToo many connections; dropped!\n";
		closesocket(id);
	}
	else
		sock_handler.sockets[new_idx].Request.Host = string(inet_ntoa(from.sin_addr)) + string(":") + to_string(ntohs(from.sin_port));

}


void server::retrieveRequest(int idx)
{
	//extract the entire message from socket in NIC.
	//if its a valid message, lets process
	if (extract_raw_fromSocket(idx))
	{
		sock_handler.breakdownRequest(idx);
	}
	//server will not respond back to corrupted/blank requests...
	else cout << "socket:" << sock_handler.sockets[idx].id << " contained corrupted/blank message, therefore closed."<<endl;
}


void server::sendResponse(int idx)
{
	SocketState& metaSocket = sock_handler.sockets[idx];
	
	sock_handler.generateResponse(idx);
 	int total_size_to_send = metaSocket.Response.responseMSG.size();
	int bytesSent = send(metaSocket.id, metaSocket.Response.responseMSG.data(),total_size_to_send, 0);

	if (SOCKET_ERROR == bytesSent)
	{
		cout << "Server: Error at send(): " << WSAGetLastError() << endl;
		return;
	}
	metaSocket.reset();
	cout << "Server: Sent: " << bytesSent << "/" << total_size_to_send
		<< " bytes of TCP Payload"<<endl;
	if (metaSocket.Request.Connection == "close")
	{
		metaSocket.clear();
	}
}

char* server::receiveEntireMessage(int idx)
{
	SocketState& metaSocket = sock_handler.sockets[idx];

	char* cbuffer = new char[INIT_BUFFER_SIZE] {0};
	int cbuffer_capacity = INIT_BUFFER_SIZE;
	bool socketEmpty = false;
	int bytesReceived = 0, total_bytesReceived = 0;
	for (int iter = 0; !socketEmpty && iter<MAX_SOCKETS; ++iter)
	{
		bytesReceived = recv(metaSocket.id, cbuffer + total_bytesReceived, cbuffer_capacity, NULL);
		//blank message or socket error, close socket:
		if (!iter && (!bytesReceived || bytesReceived == SOCKET_ERROR))
		{
			std::cout << "serv: Error at recv(): " << WSAGetLastError() << endl;
			closesocket(metaSocket.id);
			clearSocket(idx);
			delete[] cbuffer;
			return NULL;
		}
		metaSocket.timeSinceLastByte = time(0);
		//finished reading:
		if (iter && bytesReceived == SOCKET_ERROR) socketEmpty = true;
		else total_bytesReceived += bytesReceived;

		if (total_bytesReceived == cbuffer_capacity)
		{
			cbuffer_capacity *= 2;
			char* tmp = new char[cbuffer_capacity] {0};
			memcpy(tmp, cbuffer, total_bytesReceived);
			delete[] cbuffer;
			cbuffer = tmp;
		}
	}
	cbuffer[total_bytesReceived] = '\0';
	metaSocket.Request.raw_message.resize(total_bytesReceived);
	metaSocket.Request.raw_message.shrink_to_fit();

	return cbuffer;
}

bool server::extract_raw_fromSocket(int idx)
{
	SocketState& metaSocket = sock_handler.sockets[idx];
	if (metaSocket.Recv == r_EMPTY) return false;
	char* retval = receiveEntireMessage(idx);
	if (retval != NULL)
	{
		metaSocket.Request.raw_message = retval;
		return true;
	}
	delete[] retval;
	return false;
}


