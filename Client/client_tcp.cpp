#include "client_tcp.h"
#include <iostream>
#include <string>




//Constructor ClientTCP set initial value to socket
ClientTCP::ClientTCP() : connectionSocket(INVALID_SOCKET) { }


//Method ConnectSubTool connect block for networking on windows os
#if defined (_WIN32) || (_WIN64)
void ClientTCP::ConnectSubTool()
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


//Method ErrorChecker checks on error and inform what kind of error it is
int ClientTCP::ErrorChecker(int error, Action action)
{
    if (error < 0)
    {
        //inform while which process was an error
        switch (action)
        {
        case Validation: { std::cout << "Error, socket isn`t valid "; break; }
        case Connection: { std::cout << "Error while connecting "; break; }
        case Sending: { std::cout << "Error while sending "; break; }
        case Requesting: { std::cout << "Error while receiving "; break; }
        default: { break; }
        }

#if defined (_WIN32) || (_WIN64)
        //close socket which got an error
        std::cout << WSAGetLastError() << std::endl;
        closesocket(connectionSocket);
#elif (__unix__)
        //close socket which got an error
        perror("");
        close(connectionSocket);
#endif
    }
    return error;
}


//Method SetAddress returns address for sockets
sockaddr_in ClientTCP::SetAddress(const std::string& ip,   //to store ip of server
    const unsigned int port) //to store transmitted port
{
    sockaddr_in address = { AF_INET, htons(port) };
    address.sin_addr.s_addr = inet_addr(ip.c_str());
    return address;
}


//Method CatchConnection 
int ClientTCP::CatchConnection(const std::string& ip,   //to store ip of server
    const unsigned int port) //to store transmitted port
{
    int checker = 0;
    //take address for connection
    sockaddr_in addressForRequest = this->SetAddress(ip, port);

    //initialise socket
    checker=ErrorChecker(connectionSocket = socket(AF_INET, SOCK_STREAM, 0), Validation);

    //connect to server
    checker=ErrorChecker(connect(connectionSocket, (sockaddr*)&addressForRequest, sizeof(addressForRequest)), Connection);

    return checker;
}



//Method Send acts like usual send but has some checks
int ClientTCP::Send(void* buffer,   //to store buffer for transmission
    size_t size)    //size of buffer
{
    int number = 0;
    //check if socket is valid
    ErrorChecker(connectionSocket, Validation);

    //check for errors while sending
    ErrorChecker((number = send(connectionSocket, (const char*)buffer, size, 0)), Sending);

    return number;
}

//Method Receive acts like usual send but has some checks
int ClientTCP::Receive(void* buffer,
    size_t size)
{
    int number = 0;
    //check if socket is valid
    ErrorChecker(connectionSocket, Validation);

    //check for errors while receiving
    ErrorChecker((number = recv(connectionSocket, (char*)buffer, size, 0)), Requesting);

    return number;
}


//Finction SendPathToServer sends path to folder to server
void ClientTCP::SendPathToServer()   //clientSide for sending info of path to server
{
    std::string path;
    size_t sizeOfPath;

    std::cout << "Enter path to the folder on the server" << std::endl;
    std::cin >> path;
    sizeOfPath = path.size();
    //fills buufer with size of path

    if (path.size())
    {
       Send(&sizeOfPath, sizeof(sizeOfPath));
       Send((char*)path.c_str(), path.size());
    }
}



//Function SendListOfFilesToServer sends number of files and list of file names
void ClientTCP::SendListOfFilesToServer(std::vector<std::string>* listOfFiles)  //listOfFile to save file names
{

    int sizeOfList = 0;
    size_t sizeOfName = 0;

    
        std::cout << "Enter number of files you want to download" << std::endl;
        std::cin >> sizeOfList;
   
     
    //send size of list to server
   
    Send(&sizeOfList, sizeof(sizeOfList));

    //resize container for file names
    listOfFiles->resize(sizeOfList);

    if (sizeOfList)
    {
        std::cout << "Enter file names: " << std::endl;
    }
    else
    {
        std::cout << "The program is about to end as you entered 0 number of files to download, bye." << std::endl;
        system("pause");
        exit(1);
    }

    for (int i(0); i < sizeOfList; ++i)
    {

        //send name size and name to server
        std::cin >> (*listOfFiles)[i];
        sizeOfName = (*listOfFiles)[i].size();
        Send(&sizeOfName, sizeof(sizeOfName));
        Send((char*)(*listOfFiles)[i].c_str(), (*listOfFiles)[i].size());
    }
}


//Function ReceiveAvailableFiles receives mask true/false of available files on server
int ClientTCP::ReceiveAvailableFiles(const std::vector<std::string>& listOfFiles,  //listOfFiles to create files with proper name
                                     const std::string& pathToSave)                //pathToSave to save files in needed place
{
        int receiveCounter = 0;
        std::ofstream fileWriter;
        char* bufferForFile = NULL;
        size_t fileSize = 0;
        char* availableFiles = new char[listOfFiles.size()];

        //receive mask of true/false to know which files are available or not
        Receive((char*)availableFiles, listOfFiles.size());

        for (int i(0); i<listOfFiles.size(); ++i)
        {
            if ((int)availableFiles[i])
            {
                //open file to copy in buffer on windows os
#if defined (_WIN32) || (_WIN64)
                fileWriter.open((pathToSave + "\\" + listOfFiles[i]).c_str(), std::ios::binary);
                //open file to copy in buffer on linux os
#elif defined (__unix__)
                fileWriter.open((pathToSave + "/" + listOfFiles[i]).c_str(), std::ios::binary);
#endif
                if (!fileWriter)
                {
                    std::cout << "Error while writing " << listOfFiles[i] << std::endl;
                    break;
                }
                //receive file size from server
                Receive(&fileSize,sizeof(fileSize));

                //create buffer for file
                bufferForFile = new char[fileSize];

                //send file by parts if it`s bigger than buffer of socket
                size_t currentPosition = 0;
                int numberOfBytes = 0;
                while (fileSize > currentPosition)
                {
                    numberOfBytes = this->Receive(bufferForFile + currentPosition, fileSize - currentPosition);

                    if (numberOfBytes <= 0)
                    {
                        ErrorChecker(numberOfBytes, Requesting);
                        break;
                    }
                    else { currentPosition += numberOfBytes; }

                }

                fileWriter.write(bufferForFile, fileSize);
                fileWriter.close();
                delete[] bufferForFile;
                ++receiveCounter;
            }
        }

        //show which files are available or not
        std::cout << std::endl << "List of files that are available or not:" << std::endl;
        for (int i(0); i < listOfFiles.size(); ++i)
        {
            std::cout << listOfFiles[i] << " - " << ((int)availableFiles[i] ? "available" : "not available") << std::endl;
        }

        //delete mask
        delete[] availableFiles;
        return receiveCounter;
    
}




