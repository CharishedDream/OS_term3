#include "server_functions.h"
#include <string>
#include <vector>
#include <fstream>
#include <iostream>



//Function FindFiles determine which files are available in needed folder
char* FindFiles(const std::string& path,                          //to save path to folder      
                const std::vector<std::string>& listOfFiles)      //to save list of file names for comparison 
{
   //mask of true/false fot list of file names
    char* availableFiles = new char[listOfFiles.size()]{ 0 };

    //check files on windows os
#if defined (_WIN32) || (_WIN64)

    WIN32_FIND_DATA file;
    //path to folder with files
    std::string temporaryPath = path + "\\*";
    HANDLE check = FindFirstFile((temporaryPath.c_str()), &file);

    if (check != INVALID_HANDLE_VALUE)
    {
        for (int i(0); i<listOfFiles.size(); ++i)
        {
            check = FindFirstFile((temporaryPath.c_str()), &file);
            for (int j(0); FindNextFile(check, &file); ++j)
            {
                //check files on windows os
                if (listOfFiles[i] == file.cFileName)
                {
                    availableFiles[i] = (char)1;
                    break;
                }
            }
        }
    }
   //check files on linux os
#elif defined (__unix__)

    DIR *directory = opendir(path.c_str());
    dirent *file;

    if (directory)
    {
            for (int i(0); i<listOfFiles.size(); ++i)
            {
                directory = opendir(path.c_str());
                for (int j(0); (file = readdir(directory)); ++j)
                {
                    //compare transmited file name with file in folder
                    if (listOfFiles[i] == file->d_name)
                    {
                        availableFiles[i] = (char)1;
                        break;
                    }
                }
            }
    }
    else
    {
        std::cout << "Error while opening folder" << std::endl;
    }

#endif
    return availableFiles;
}


