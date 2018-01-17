#include "stdafx.h"
#include "TCPLineServer.h"

TCPLineServer::TCPLineServer(unsigned int port, unsigned int recvbuf_size)
: port(port), recvBufSize(recvbuf_size) {
   recvBuf = new char[recvBufSize];
   state = State::SOCKET_NOT_CREATED;
}

TCPLineServer::Result TCPLineServer::Update() {
   switch(state) {
   case State::SOCKET_NOT_CREATED:
      return CreateSocket();
      break;

   case State::WAITING_FOR_CLIENT:
      return Accept();
      break;

   case State::CLIENT_CONNECTED:
      return SendAndReceive();
      break;

   default:                   // To get rid of annoying compiler warning. If this was Ada I'm
      return Result::SUCCESS; // sure this wouldn't be needed. Ada is good. Be like Ada.
   };
}

TCPLineServer::Result TCPLineServer::CreateSocket() {
   serverSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

   if(serverSocket==INVALID_SOCKET)
      return Result::ERROR_SOCKET_NOT_CREATED;

   // If iMode!=0, non-blocking mode is enabled.
   u_long iMode=1;
   if(ioctlsocket(serverSocket, FIONBIO, &iMode) == SOCKET_ERROR)
      return Result::ERROR_SOCKET_NOT_CREATED;

   SOCKADDR_IN serverInf;
   serverInf.sin_family=AF_INET;
   serverInf.sin_addr.s_addr=INADDR_ANY;
   serverInf.sin_port=htons(port);

   if(bind(serverSocket,(SOCKADDR*)(&serverInf),sizeof(serverInf))==SOCKET_ERROR)
      return Result::ERROR_SOCKET_NOT_CREATED;

   if(listen(serverSocket,1)==SOCKET_ERROR)
      return Result::ERROR_SOCKET_NOT_CREATED;

   state = State::WAITING_FOR_CLIENT;
   return Result::SUCCESS;
}

TCPLineServer::Result TCPLineServer::Accept() {
   SOCKET tempSock;

   tempSock=accept(serverSocket,NULL,NULL);
   if(tempSock == INVALID_SOCKET) {
      int error = WSAGetLastError();

      if(error != WSAEWOULDBLOCK) {
         closesocket(serverSocket);
         state = State::SOCKET_NOT_CREATED;
         return Result::ERROR_ACCEPT;		// some error occured
      } else {
         return Result::SUCCESS;				// no connection pending
      }
   } else {
      clientSocket = tempSock;

      // If iMode!=0, non-blocking mode is enabled.
      u_long iMode=1;
      if (ioctlsocket(clientSocket, FIONBIO, &iMode) == SOCKET_ERROR) {
         closesocket(clientSocket);
         return Result::ERROR_ACCEPT;
      }

      // Clear the circular receive buffer
      recvBufStart = 0;
      recvBufEnd = 0;

      // Clear the send and receive queues (who came up with the idea of _not_ including
      // a clear()-function in a standard queue implementation?)
      std::queue<std::string> emptyStringQueue;
      std::swap(sendQueue, emptyStringQueue);

      std::queue<std::string> anotherEmptyStringQueue;
      std::swap(recvQueue, anotherEmptyStringQueue);

      closesocket(serverSocket);       // Close the listening server socket

      state = State::CLIENT_CONNECTED; // Connection made
      return Result::SUCCESS;
   }
}

TCPLineServer::Result TCPLineServer::SendAndReceive() {
   Result res;
   if ((res = Send()) != Result::SUCCESS)
      return res;
   
   if ((res = Receive()) != Result::SUCCESS)
      return res;

   return Result::SUCCESS;
}

TCPLineServer::Result TCPLineServer::Send() {
   while (sendQueue.empty() == false) {
      std::string message = sendQueue.front();

      if (send(clientSocket, message.c_str(), message.length(), 0) == SOCKET_ERROR) {
         int error = WSAGetLastError();

         if (error != WSAEWOULDBLOCK) {	// Connection error
            closesocket(clientSocket);
            state = State::SOCKET_NOT_CREATED;
            return Result::ERROR_CONNECTION_CLOSED;
         } else {						// Try again later
            return Result::SUCCESS;
         }
      } else {	// Sending succeeded!
         sendQueue.pop(); // Remove the sent message from the queue
      }
   }
   return Result::SUCCESS;
}

TCPLineServer::Result TCPLineServer::Receive() {
   // receive
   int bytesToReceive, bytesReceived, newRecvBufEnd;

   // Circular receive buffer. recvBufEnd points to the next position
   // that should be written (end-of-data + 1), recvBufStart points
   // to the first position that contains data.

   // How many bytes shall we receive at most (in this single recv call)?
   if(recvBufEnd < recvBufStart) {
      bytesToReceive = recvBufStart - recvBufEnd;		// Up until the start of data
   } else {
      bytesToReceive = recvBufSize - recvBufEnd;		// Up until the end of the memory buffer
   }

   if((bytesReceived = recv(clientSocket, &(recvBuf[recvBufEnd]), bytesToReceive, 0)) == SOCKET_ERROR) {	// receive as many bytes as there is consecutive free memory
      int error = WSAGetLastError();

      if(error != WSAEWOULDBLOCK) {	// Undefined error
         closesocket(clientSocket);
         state = State::SOCKET_NOT_CREATED;
         return Result::ERROR_CONNECTION_CLOSED;
      } else {
         return Result::SUCCESS;		// Nothing received, try again next time
      }
   } else if(bytesReceived == 0) { // Connection gracefully closed
      closesocket(clientSocket);
      state = State::SOCKET_NOT_CREATED;
      return Result::SUCCESS;
   } else {	// Data received
      newRecvBufEnd = recvBufEnd + bytesReceived;		// "Pointer" to the new end of data

      if(newRecvBufEnd == recvBufSize) {	// If we have reached the end of the memory buffer,
         newRecvBufEnd = 0;				// wrap around
      }

      // Start searching through the newly received data for an end-of-message-delimiter (\n)
      bool messageReceived = false;

      while(recvBufEnd != newRecvBufEnd) {
         while(recvBuf[recvBufEnd] != '\n' && recvBufEnd != newRecvBufEnd) {
            recvBufEnd++;

            if(recvBufEnd == recvBufSize) {
               recvBufEnd = 0;
            }
         }

         if(recvBufEnd != newRecvBufEnd) {	// Complete message found
            messageReceived = true;

            if(recvBufEnd >= recvBufStart) {
               char *command = new char[recvBufEnd - recvBufStart + 2];
               strncpy(command, &(recvBuf[recvBufStart]), recvBufEnd - recvBufStart + 1);
               command[recvBufEnd - recvBufStart + 1] = 0;

               std::string cmd = command;
               recvQueue.push(cmd);

               if(recvBufEnd != recvBufSize - 1) {
                  recvBufEnd++;
               } else {
                  recvBufEnd = 0;
               }

               recvBufStart = recvBufEnd;
            } else if(recvBufEnd < recvBufStart) {
               char *command = new char[recvBufSize - recvBufStart + recvBufEnd + 2];
               strncpy(command, &(recvBuf[recvBufStart]), recvBufSize - recvBufStart);
               strncpy((command + recvBufSize - recvBufStart), recvBuf, recvBufEnd + 1);
               command[recvBufSize - recvBufStart + recvBufEnd + 1] = 0;

               std::string cmd = command;
               recvQueue.push(cmd);

               recvBufEnd++;
               recvBufStart = recvBufEnd;
            }
         }
      }

      // If newRecvBufEnd is now equal to recvBufStart it means that the buffer is either full or empty.
      // If it is empty now that we have received data, messageReceived will be true, so if messageReceived
      // is false it means that the buffer is full and that there are no '\n' in the data currently in the
      // buffer. Return an error saying that we are receiving a message that is larger than the receive buffer.
      if(newRecvBufEnd == recvBufStart && !messageReceived) {
         return Result::ERROR_OVERRUN;
      }

      return Result::SUCCESS;
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
   // Since this is a Line-server, we want to make sure every message ends with a newline
   if (message[message.size() - 1] != '\n') {
      message = message + '\n';
   }

   sendQueue.push(message);
}

bool TCPLineServer::ClientConnected() {
   if(state == State::CLIENT_CONNECTED)
      return true;
   else
      return false;
}