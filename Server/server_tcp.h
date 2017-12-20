#ifndef SERVER_TCP_H
#define SERVER_TCP_H

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <queue>

#if defined (_WIN32) || (_WIN64)

#pragma comment (lib, "Ws2_32.lib")
#pragma warning(disable : 4996)

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <process.h>


#elif defined (__unix__)

#define INVALID_SOCKET -1

#include <sys/types.h>
#include <dirent.h>
#include <pthread.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef int SOCKET;

#endif 

enum Action { Listening, Accepting, Binding, Requesting, Sending, Validation };
enum Socket { SocketForRequest, SocketForAnswer, SocketForConnection };

//this class is a wrapper around networking 
class ServerTCP //totally not ServerNetwork!
{
public:
   
    ///Networking methods
    ServerTCP();  //intialise sockets

    sockaddr_in SetAddress(const unsigned int port); //to set address for sockets

    void ListenSocketRequestOpen(); //open listen socket for requests (7500 port)

    void ListenSocketAnswerOpen(); //open listen sokcet for answers (7505 port)

    int ErrorChecker(const int error, const  int  flag, const int action);  //check functions on errors

    void CatchAnswerConnection();  //reconnect main socket for answers through (7505 port)

    int Send(void* buffer, const  size_t size); //wrapper for usual send with checks

    int Receive(void* buffer, const size_t size); //wrapper for usual receive with checks

    void CreateQueue();  //creates thread to form requests and queue of client connections

    ///Functional methods
    void ReceivePathToFolder(std::string* path);  //receive path to folder from client

    void ReceiveListOfFiles(std::vector<std::string>* listOfFiles); //receive list of file names from client

    int SendFiles(const std::vector<std::string>& listOfFiles,    //send available files to client
                  const char* const availableFiles,
                  const std::string& path);

    ///Form requests and send answers
#if defined (_WIN32) || (_WIN64)
    void ConnectSubTool();         //connect sub system networking for windows

                                   //I made it as friend function because threads need static address of function
    friend void FormSendRequest(void*);  //form requests in windows thread
#elif defined (__unix__)
    friend void* FormSendRequest(void*); //form requests in linux thread
#endif

private:
    std::queue<SOCKET> connectionsQueue;            //queue of client connections
    SOCKET  ,listenSocketAnswer;  //two sockets for different ports
    SOCKET connectionSocket;                        //socket for connection with client
};

#endif      //SERVER_TCP_H
