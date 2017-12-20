#include "server_tcp.h"
#include "server_functions.h"
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <queue>

///Form requests and send answers
//-----------------------------------------------------------------------------

//Function FormSendRequest forms request and sends anwswer to it
#if defined (_WIN32) || (_WIN64)
 void FormSendRequest(void* ptr)  //because such function takes as parametr windows thread
#elif defined (__unix__)
 void* FormSendRequest(void* ptr) //because such function takes as parametr linux thread
#endif
{
     //got pointer to object ServerTCP
    ServerTCP* serverPointer = (ServerTCP*)ptr;
    int id = 1;
    //form request and answer for them while server working
    while (true)
    {

        while (serverPointer->connectionsQueue.empty())
        { 
            
#if defined (_WIN32) || (_WIN64)
            Sleep(1000);
#else
            sleep(1);
#endif
        }; //wait for clients

        //take the first client from the begining of the queue
        serverPointer->connectionSocket = serverPointer->connectionsQueue.front();
        serverPointer->connectionsQueue.pop();
        
        //send if to client
        serverPointer->Send(&id, sizeof(id));
        std::vector<std::string> listOfFiles;
        std::string path;

        std::cout << "Client connected" << std::endl;

        //receive path to folder with files
        serverPointer->ReceivePathToFolder(&path);
        //receive list of file names to check which is available
        serverPointer->ReceiveListOfFiles(&listOfFiles);
        std::cout << "List of received file names:" << std::endl;

        if (!listOfFiles.size())
        {
            std::cout << "Client disconnected" << std::endl;
            ++id;
            continue;
        }

        for (int i(0); i < listOfFiles.size(); ++i)
        {
            std::cout << listOfFiles[i] << std::endl;
        }

        //get mask of true/false to list of file names
        char* availableFiles = FindFiles(path, listOfFiles);

        //reconnect to 7505 port to send available files
        serverPointer->CatchAnswerConnection();

        //send available files to client
        std::cout << "Number of sended files is " << serverPointer->SendFiles(listOfFiles, availableFiles, path) << std::endl;
        
        std::cout << "Client disconnected" << std::endl;

        ++id;
    }
}

//-----------------------------------------------------------------------------


///Networking methods
//-----------------------------------------------------------------------------

//Constructor ServerTCP initialise sockets
ServerTCP::ServerTCP() :listenSocketRequest(INVALID_SOCKET), listenSocketAnswer(INVALID_SOCKET),
connectionSocket(INVALID_SOCKET) { }

//Method SetAddress returns address for sockets
sockaddr_in ServerTCP::SetAddress(const unsigned int port) //to 2save transmitted port
{
    sockaddr_in address = { AF_INET, htons(port) };
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    return address;
}

//Method ListenSocketRequestOpen open listen socket for requests
void ServerTCP::ListenSocketRequestOpen()
{
    //take address for bind
    sockaddr_in addrForRequst = this->SetAddress(7500);

    //initial values for socket
    ErrorChecker(listenSocketRequest = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP), SocketForRequest, Validation);

    //bind socket with 7500 port
    ErrorChecker(bind(listenSocketRequest, (sockaddr*)&addrForRequst, sizeof(addrForRequst)), SocketForRequest, Binding);

    //listen socket for connections
    ErrorChecker(listen(listenSocketRequest, SOMAXCONN), SocketForRequest, Listening);
}

//Method ListenSocketAnswerOpen open listen socket for answers
void ServerTCP::ListenSocketAnswerOpen()
{
    //take address for bind
    sockaddr_in addrForAnswer = this->SetAddress(7505);

    //initial values for socket
    ErrorChecker((listenSocketAnswer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)), SocketForAnswer, Validation);

    //bind socket with 7505 port
    ErrorChecker(bind(listenSocketAnswer, (sockaddr*)&addrForAnswer, sizeof(addrForAnswer)), SocketForAnswer, Binding);

    //listen socket for connections
    ErrorChecker(listen(listenSocketAnswer, SOMAXCONN), SocketForAnswer, Listening);
}


//Method ErrorChecker checks on error and inform what kind of error it is
int ServerTCP::ErrorChecker(const int error, const int flag, const int action)
{
    if (error < 0)
    {
        //inform which action gave an error
        switch (action)
        {
        case Accepting: { std::cout << "Error while accepting." << std::endl;  break; }
        case Listening: { std::cout << "Error while listennig." << std::endl; break; }
        case Binding: { std::cout << "Error while binding." << std::endl; break; }
        case Validation: { std::cout << "Error, socket isn`t valid." << std::endl; break; }
        case Sending: { std::cout << "Error while sending." << std::endl; break; }
        case Requesting: { std::cout << "Error while receiving" << std::endl; break; }
        default:break;
        }
        //close socket for windows os 
#if defined (_WIN32) || (_WIN64)
        //close socket which got an error
        switch (flag)
        {
        case SocketForRequest:
        {
            shutdown(listenSocketRequest, 2);
            std::cout << "Listen socket for requests was closed with error: " << WSAGetLastError() << std::endl;
            break;
        }
        case SocketForAnswer:
        {
            shutdown(listenSocketAnswer,2);
            std::cout << "Listen socket for answers was closed with error: " << WSAGetLastError() << std::endl;
            break;
        }
        case SocketForConnection:
        {
            shutdown(connectionSocket,2);
            connectionSocket = INVALID_SOCKET;
            std::cout << "Client socket was closed with error: " << WSAGetLastError() << std::endl;
            break;
        }
        default: break;
        }
        //close socket for windows os 
#elif defined (__unix__)
        //close socket which got an error
        switch (flag)
        {
        case SocketForRequest:
        {
            shutdown(listenSocketRequest,2);
            perror("Listen socket for requests was closed with error");
        }
        case SocketForAnswer:
        {
            shutdown(listenSocketAnswer,2);
            perror("Listen socket for answers was closed with error");
        }
        case SocketForConnection:
        {
            shutdown(connectionSocket,2);
            connectionSocket = INVALID_SOCKET;
            perror("Client socket was closed with error");
        }
        }
#endif

    }
    return error;
}

//Method CatchAnswerConnection reconnect connection socket to send formed request
void ServerTCP::CatchAnswerConnection()
{
    int valid = ErrorChecker(connectionSocket, SocketForConnection, Validation);

    if (valid >= 0)
    {
        ErrorChecker(connectionSocket = accept(listenSocketAnswer, NULL, NULL), SocketForConnection, Accepting);
    }
}


//Method Send acts like usual send but has some checks
int ServerTCP::Send(void* buffer,   //to store buffer for transmission
                    size_t size)    //size of buffer
{
    int number = 0;
    //check if socket is valid
    ErrorChecker(connectionSocket, SocketForConnection, Validation);

    //check for errors while sending
    ErrorChecker((number = send(connectionSocket, (const char*)buffer, size, 0)), SocketForConnection, Sending);

    return number;
}



//Method Receive acts like usual send but has some checks
int ServerTCP::Receive(void* buffer,
    size_t size)
{
    int number = 0;
    //check if socket is valid
    ErrorChecker(connectionSocket, SocketForConnection, Validation);

    //check for errors while receiving
    ErrorChecker((number = recv(connectionSocket, (char*)buffer, size, 0)), SocketForConnection, Requesting);

    return number;
}

//Method CreateQueue creates queue of connections
void ServerTCP::CreateQueue()
{
    //create thread to form requests and send answers for windows os 
#if defined (_WIN32) || (_WIN64)
    _beginthread(FormSendRequest, 0, this);
    //create thread to form requests and send answers for linux os
#elif defined (__unix__)

    pthread_t threadForFormSendRequest;

     //check if creation of thread was successful
    if ((pthread_create(&threadForFormSendRequest, NULL, FormSendRequest, this)))
    {
        std::cout << "Unable to create thread for forming and sending request" << std::endl;
    }
#endif
    //waits and creates queue of connections
    while (true)
    {
        SOCKET temporary = accept(listenSocketRequest, NULL, NULL);
        ErrorChecker(temporary, 'C', Validation);
        connectionsQueue.push(temporary);
    }
}


//-----------------------------------------------------------------------------



///Functional methods
//-----------------------------------------------------------------------------

//Method ReceivePathToFolder receives path to folder where to look for files
void ServerTCP::ReceivePathToFolder(std::string* path)
{
    char* temporaryPath = NULL;
    size_t sizeOfPath = 0;

    //receive size of path 
    this->Receive(&sizeOfPath, sizeof(size_t));

    //create buffer for path
    temporaryPath = new char[sizeOfPath + 1];
    temporaryPath[sizeOfPath] = '\0';

    //receive path
    this->Receive(temporaryPath, sizeOfPath);

    *path = temporaryPath;
}


//Method ReceiveListOfFiles receives list of file names which client what to download
void ServerTCP::ReceiveListOfFiles(std::vector<std::string>* listOfFiles) //to store transmitted file names
{
    size_t sizelistOfFiles = 0;
    size_t nameSize = 0;
    char* bufferForName = NULL;

    //receive size of list
    this->Receive(&sizelistOfFiles, sizeof(size_t));

    //resize container for list of file names
    listOfFiles->resize(sizelistOfFiles);

    for (int i(0); i < sizelistOfFiles; ++i)
    {
        //receive size of file name
        this->Receive(&nameSize, sizeof(size_t));

        //create buffer for file name
        bufferForName = new char[nameSize + 1];
        bufferForName[nameSize] = '\0';

        //receivce file name
        this->Receive(bufferForName, nameSize);

        (*listOfFiles)[i] = bufferForName;
    }
}


//Method SendFiles sends available files to client
int ServerTCP::SendFiles(const std::vector<std::string>& listOfFiles, //to open files 
                         const char* const availableFiles,            //to know which file is available
                         const std::string& path)                     //to open files
{
    std::ifstream fileReader;
    int sendCounter = 0;
    size_t sizeOfFile = 0;
    char* bufferForFile = NULL;

    //send mask of true/false to client
    this->Send((char*)availableFiles, listOfFiles.size());

    for (int i(0); i < listOfFiles.size(); ++i)
    {
        if ((int)availableFiles[i])
        {
            //open file to copy in buffer on windows os
#if defined (_WIN32) || (_WIN64)
            fileReader.open((path + "\\" + listOfFiles[i]).c_str(), std::ios::binary);
            //open file to copy in buffer on linux os
#elif defined (__unix)
            fileReader.open((path + "/" + listOfFiles[i]).c_str(), std::ios::binary);
#endif
            if (!fileReader)
            {
                std::cout << "Error while readaing the file." << std::endl;
                break;
            }

            //determine file size
            fileReader.seekg(0, std::ios_base::end);
            sizeOfFile = (size_t)fileReader.tellg();
            fileReader.seekg(0);

            //create buffer for file
            bufferForFile = new char[sizeOfFile];
            fileReader.read(bufferForFile, sizeOfFile);

            //send file size to client
            this->Send(&sizeOfFile, sizeof(sizeOfFile));

            //send file by parts if it`s bigger than buffer of socket
            size_t currentPosition = 0;
            int numberOfBytes = 0;
            while (sizeOfFile > currentPosition)
            {
                //send file by parts if needed
                numberOfBytes = this->Send(bufferForFile + currentPosition, sizeOfFile - currentPosition);

                if (numberOfBytes <= 0)
                {
                    ErrorChecker(numberOfBytes, 'C', Sending);
                    break;
                }
                else { currentPosition += numberOfBytes; }

            }

            fileReader.close();
            ++sendCounter;
            delete[] bufferForFile;
        }
    }

    return sendCounter;
}


//Method ConnectSubTool connect block for networking on windows os
#if defined (_WIN32) || (_WIN64)
void ServerTCP::ConnectSubTool()
{
    WSADATA data;
    WORD version = MAKEWORD(2, 2);

    //sub tool networking start with types of sockets
    if (WSAStartup(version, &data))
    {
        std::cout << "WSAStartup error: " << WSAGetLastError << std::endl;
    }
}
#endif









































