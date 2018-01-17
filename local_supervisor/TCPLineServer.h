#ifndef TCPLINESERVER_H
#define TCPLINESERVER_H

#include <WinSock2.h>
#include <queue>
#include <string>


/* Class TCPLineServer
   
   A TCP server that accepts one client. The server listens for incoming messages ending with a
   newline character. When a full message has been received it may be fetched by the caller. Messages
   may also be added to the send queue and then sent when the Update()-function is called. The Update()-
   function shall be called periodically to send and receive data. If no client is currently connected
   it also handles listening for and accepting an incoming connection by using an internal state-machine.

   When a client connects, both the internal receive buffer and send and receive queues are cleared.
   
   PREREQUISITES:
   The caller must initialize the WinSock-library with a call to WSAStartup() before calling Update()
   for the first time.
*/
class TCPLineServer {
public:
   enum class Result { SUCCESS, ERROR_SOCKET_NOT_CREATED, ERROR_ACCEPT, ERROR_CONNECTION_CLOSED, ERROR_OVERRUN };

   /* Contructor
      ARGS:
         port (in):
            The TCP port number to which the server shall listen for incoming connections
         recvbuf_size (in):
            The size of the internal receive buffer. This needs to be greater than the
            longest message that the client can send, including newline character.
   */
   TCPLineServer(unsigned int port, unsigned int recvbuf_size = 128);

   /* Needs to be called periodically. Handles sending and receiving of data and accepting
      incoming connections.
      RETVAL:
         Result::SUCCESS on success, otherwise one of the error values
   */
   Result Update();

   /* Used to poll if there are any received lines in the internal receive queue.
      RETVAL:
         True if there are lines waiting to be fetched, false otherwise.
   */
   bool HasData();

   /* Used to fetch one line received from the client. Can be called as long
      as HasData() returns true.
      RETVAL:
         A string containing a line received from the client.
   */
   std::string Get();

   /* Add a string to the send queue. If the string doesn't end with a newline character
      this function will add one.
   */
   void AddToSendQueue(std::string message);
   
   /* Used to ask if there is currently a client connected.
      RETVAL:
         True if a client is connected, false otherwise.
   */
   bool ClientConnected();

private:
   Result CreateSocket();
   Result Accept();
   Result SendAndReceive();
   Result Send();
   Result Receive();

   std::queue<std::string> sendQueue;
   std::queue<std::string> recvQueue;
   
   char * recvBuf;
   unsigned int recvBufSize;
   unsigned int recvBufStart, recvBufEnd;

   unsigned int port;
   SOCKET serverSocket;
   SOCKET clientSocket;
   enum class State { CLIENT_CONNECTED, SOCKET_NOT_CREATED, WAITING_FOR_CLIENT };
   State state;
};

#endif