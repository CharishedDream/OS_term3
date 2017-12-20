#include "server_functions.h"
#include "server_tcp.h"


int main()
{
    ServerTCP server;

    //connect networking block for windows
#if defined (_WIN32) || (_WIN64)
    server.ConnectSubTool();
#endif

    //open listen socket for requests through (7500 port)
    server.ListenSocketRequestOpen();
  
    //open listen socket for answers through (7505 port)
    server.ListenSocketAnswerOpen();
  
    //create queue of connections
    server.CreateQueue();
   

    return 0;
}

