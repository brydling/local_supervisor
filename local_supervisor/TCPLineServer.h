#ifndef TCPLINESERVER_H
#define TCPLINESERVER_H

#include <WinSock2.h>
#include <queue>
#include <string>

class TCPLineServer {
public:
	TCPLineServer(unsigned int port) { this->port = port; state = SOCKET_NOT_CREATED; }
	int Update();
	bool HasData();
	std::string Get();
	void AddToSendQueue(std::string message);
	bool ClientConnected();

private:
	int CreateSocket();
	int Accept();
	void SendAndReceive();
	int Send();
	int Receive();

	std::queue<std::string> sendQueue;
	std::queue<std::string> recvQueue;
	
	static const unsigned int RECVBUF_SIZE = 4096;
	char recvBuf[RECVBUF_SIZE];
	int recvBufStart, recvBufEnd;

	unsigned int port;
	SOCKET serverSocket;
	SOCKET clientSocket;
	enum StateType { CLIENT_CONNECTED, SOCKET_NOT_CREATED, WAITING_FOR_CLIENT };
	StateType state;
};

#endif