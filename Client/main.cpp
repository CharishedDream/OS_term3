#include <iostream>
#include "client_tcp.h"


int main()
{
    int id = 0;
    std::string pathToSave, ip;
    ClientTCP client;
    std::vector<std::string> listOfFiles;

#if defined (_WIN32) || (_WIN64)
    client.ConnectSubTool();
#endif
  
    std::cout << "Enter the ip of the server to connect." << std::endl;
    std::cin >> ip;


    //create connection through "7500" port
    if (client.CatchConnection(ip, 7500) == -1)
    {
        std::cout << "Connection error, program is about to end, bye" << std::endl;
        system("pause");
        exit(1);
    }

    std::cout << "You are connected to server." << std::endl;

    client.Receive(&id, sizeof(id));

    std::cout << " id " << (int)id << " | connected" << std::endl;
    std::cout << "You can send your request." << std::endl;

    //send info about files you need to server
    client.SendPathToServer();
    client.SendListOfFilesToServer(&listOfFiles);

    std::cout << "Enter path to save files." << std::endl;
    std::cin >> pathToSave;

    //create connection through "7505" port
    client.CatchConnection(ip, 7505);
    std::cout << "Number of received: " << client.ReceiveAvailableFiles(listOfFiles, pathToSave) << std::endl;


    system("pause");
    return 0;

}
