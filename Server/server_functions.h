#ifndef SERVER_FUNCTIONS_H
#define SERVER_FUNCTIONS_H
#include "server_tcp.h"


char* FindFiles(const std::string& path,                          //functions to find available files on WIN and Linux  
                const std::vector<std::string>& listOfFiles);     
                                                                                          
                    
#endif