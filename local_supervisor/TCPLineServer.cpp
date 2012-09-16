#include "stdafx.h"
#include "TCPLineServer.h"

TCPLineServer::TCPLineServer(unsigned int port) {
	this->port = port;
	state = SOCKET_NOT_CREATED;
}

int TCPLineServer::Update() {
	switch(state) {
	case SOCKET_NOT_CREATED:
		return CreateSocket();
		break;

	case WAITING_FOR_CLIENT:
		return Accept();
		break;

	case CLIENT_CONNECTED:
		SendAndReceive();
		return 0;
		break;
	};
}

int TCPLineServer::CreateSocket() {
	serverSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(serverSocket==INVALID_SOCKET) {
		return -1;
	}

	// If iMode!=0, non-blocking mode is enabled.
	u_long iMode=1;
	ioctlsocket(serverSocket,FIONBIO,&iMode);

	SOCKADDR_IN serverInf;
	serverInf.sin_family=AF_INET;
	serverInf.sin_addr.s_addr=INADDR_ANY;
	serverInf.sin_port=htons(port);

	if(bind(serverSocket,(SOCKADDR*)(&serverInf),sizeof(serverInf))==SOCKET_ERROR) {
		return -1;
	}

	if(listen(serverSocket,1)==SOCKET_ERROR) {
		return -1;
	}

	state = WAITING_FOR_CLIENT;
	return 0;
}

int TCPLineServer::Accept() {
	SOCKET tempSock;

	tempSock=accept(serverSocket,NULL,NULL);
	if(tempSock == INVALID_SOCKET) {
		int error = WSAGetLastError();

		if(error != WSAEWOULDBLOCK) {
			return -1;		// some error occured
		} else {
			return 0;		// no connection pending
		}
	} else {
		clientSocket = tempSock;

		// If iMode!=0, non-blocking mode is enabled.
		u_long iMode=1;
		ioctlsocket(clientSocket,FIONBIO,&iMode);

		recvBufStart = 0;
		recvBufEnd = 0;

		closesocket(serverSocket);

		state = CLIENT_CONNECTED;
		return 1; // connection made
	}
}

void TCPLineServer::SendAndReceive() {
	Send();
	Receive();
}

int TCPLineServer::Send() {
	while(sendQueue.empty() == false) {
		std::string message = sendQueue.front();
		sendQueue.pop();

		if(message[message.size() - 1] != '\n') {
			message = message + '\n';
		}

		if(send(clientSocket, message.c_str(), message.length(), 0) == SOCKET_ERROR) {
			int error = WSAGetLastError();

			if(error != WSAEWOULDBLOCK) {
				closesocket(clientSocket);
				state = SOCKET_NOT_CREATED;
				return 0;
			} else {
				return -3;
			}
		}
		return 0;
	}
	return 0;
}

int TCPLineServer::Receive() {
	// receive
	int limit, bytesReceived, newRecvBufEnd;

	if(recvBufEnd < recvBufStart) {
		limit = recvBufStart;
	} else {
		limit = RECVBUF_SIZE;
	}

	if((bytesReceived = recv(clientSocket, &(recvBuf[recvBufEnd]), limit - recvBufEnd, 0)) == SOCKET_ERROR) {	// receive as many bytes as is free in
		// the recv buffer
		int error = WSAGetLastError();

		if(error != WSAEWOULDBLOCK) {
			closesocket(clientSocket);
			state = SOCKET_NOT_CREATED;
			return 0;
		}
		newRecvBufEnd = recvBufEnd;
		return 0;
	} else if(bytesReceived == 0) { // connection gracefully closed
		closesocket(clientSocket);
		state = SOCKET_NOT_CREATED;
		return 0;
	} else {
		newRecvBufEnd = recvBufEnd + bytesReceived;

		if(newRecvBufEnd == RECVBUF_SIZE) {
			newRecvBufEnd = 0;
		}

		if(newRecvBufEnd == 0 && recvBufStart > 0) {	// we have reached the end of the allocated memory but
			// there may still be free space at the beginning

			if((bytesReceived = recv(clientSocket, &(recvBuf[0]), recvBufStart, 0)) == SOCKET_ERROR) {	// receive more if possible
				int error = WSAGetLastError();

				if(error != WSAEWOULDBLOCK) {
					closesocket(clientSocket);
					state = SOCKET_NOT_CREATED;
					return 0;
				}
				newRecvBufEnd = 0;
			} else if(bytesReceived == 0) { // connection gracefully closed
				closesocket(clientSocket);
				state = SOCKET_NOT_CREATED;
				return 0;
			} else {
				newRecvBufEnd = 0 + bytesReceived;
			}
		}

		bool messageReceived = false;

		while(recvBufEnd != newRecvBufEnd) {
			while(recvBuf[recvBufEnd] != '\n' && recvBufEnd != newRecvBufEnd) {
				recvBufEnd++;

				if(recvBufEnd == RECVBUF_SIZE) {
					recvBufEnd = 0;
				}
			}

			if(recvBufEnd != newRecvBufEnd) {
				messageReceived = true;

				if(recvBufEnd < RECVBUF_SIZE - 1 && recvBufEnd != newRecvBufEnd - 1 && recvBuf[recvBufEnd+1] == '\n') {
					recvBufEnd++;
				} else if(recvBufEnd == RECVBUF_SIZE - 1 && newRecvBufEnd > 0 && recvBuf[0] == '\n') {
					recvBufEnd = 0;
				}

				if(recvBufEnd >= recvBufStart) {
					char *command = new char[recvBufEnd - recvBufStart + 2];
					strncpy(command, &(recvBuf[recvBufStart]), recvBufEnd - recvBufStart + 1);
					command[recvBufEnd - recvBufStart + 1] = 0;

					std::string cmd = command;
					recvQueue.push(cmd);

					if(recvBufEnd != RECVBUF_SIZE - 1) {
						recvBufEnd++;
					} else {
						recvBufEnd = 0;
					}

					recvBufStart = recvBufEnd;
				} else if(recvBufEnd < recvBufStart) {
					char *command = new char[RECVBUF_SIZE - recvBufStart + recvBufEnd + 2];
					strncpy(command, &(recvBuf[recvBufStart]), RECVBUF_SIZE - recvBufStart);
					strncpy((command + RECVBUF_SIZE - recvBufStart), recvBuf, recvBufEnd + 1);
					command[RECVBUF_SIZE - recvBufStart + recvBufEnd + 1] = 0;

					std::string cmd = command;
					recvQueue.push(cmd);

					recvBufEnd++;
					recvBufStart = recvBufEnd;
				}
			}
		}

		if(newRecvBufEnd == recvBufStart && !messageReceived) {
			return -2;
		}

		return 0;
	}
}

bool TCPLineServer::HasData() {
	if(recvQueue.size() > 0) {
		return true;
	} else {
		return false;
	}
}

std::string TCPLineServer::Get() {
	std::string message = recvQueue.front();
	recvQueue.pop();
	return message;
}

void TCPLineServer::AddToSendQueue(std::string message) {
	sendQueue.push(message);
}

bool TCPLineServer::ClientConnected() {
	if(state == CLIENT_CONNECTED)
		return true;
	else
		return false;
}