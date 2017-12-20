#ifndef CLIENT_TCP_H
#define CLIENT_TCP_H

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#if defined (_WIN32) || (_WIN64)


#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma warning(disable : 4996)

#elif defined (__unix__)

#define INVALID_SOCKET -1

#include <sys/types.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>

typedef int SOCKET;

#endif

enum Action { Validation, Connection, Sending, Requesting};

//this class is a wrapper around networking
class ClientTCP //totally not ClientNetwork!
{
public:

    ///Networking methods
    ClientTCP(); //initialise sockets

    void ConnectSubTool(); //connect networking tool for windows

    int ErrorChecker(const int error, Action action); //check functions on errors

    sockaddr_in SetAddress(const std::string& ip, const unsigned int port); //to set address for sockets

    int CatchConnection(const std::string& ip, const unsigned int port); //to reconnect through 7505 port

    int  Send(void* buffer,const size_t size);  //wrapper for usual send with checks

    int  Receive(void* buffer,const size_t size); //wrapper for usual receive with checks


    ///Functional methods
    void SendListOfFilesToServer(std::vector<std::string>* listOfFiles); //send list of file names to server

    void SendPathToServer();  //send path to folder with needed files

    int  ReceiveAvailableFiles(const std::vector<std::string>& listOfFiles,  //download available files from server
                               const std::string& pathToSave);

private:
    SOCKET connectionSocket;
};


#endif   //CLIENT_TCP_H
