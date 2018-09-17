#ifndef _BSTNODE_H_
#define _BSTNODE_H_

#include <string>
#include <windows.h>

class BSTnode;

class BSTnode
{
public:
    std::string filename, 
                showName,
                pathAndFile,
                resString;

    bool ignored;

    void UpdateText()
    {
        resString.clear();
        char buffer[100];
        itoa(programMode->dmPelsWidth, buffer, 10);
        resString += buffer;
        resString += "x";
        itoa(programMode->dmPelsHeight, buffer, 10);
        resString += buffer;
        resString += " ";
        itoa(programMode->dmBitsPerPel, buffer, 10);
        resString += buffer;
        resString += "bits ";
        itoa(programMode->dmDisplayFrequency, buffer, 10);
        resString += buffer;
        resString += "hz";
        //resString = buffer;
    }

    LPDEVMODE programMode;
    unsigned int key;
    BSTnode * left, *right;
};

#endif