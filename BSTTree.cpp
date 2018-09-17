#include "BSTTree.h"
#include "shlwapi.h"
#include <string>
#include "commctrl.h"
#include <fstream>
#pragma comment(lib, "shlwapi.lib")

extern HIMAGELIST hImageListSmall;

/*
 * Function:    ConvertToLowerCase
 * Desceiption: Converts a string to
 *              lowercase.
 **********************************/
void ConvertToLowerCase(char * c)
{
    for(unsigned int i=0;i<strlen(c)-1;i++)
    {
        if(c[i] >= 'A' && c[i] <= 'Z') c[i] += 32;
    }
}

/*
 * Function:    BSTTree
 * Description: The constructor for
 *              the BSTTree class.
 ***********************************/
BSTTree::BSTTree()
{
    this->root = 0;
    mainKey = 0;
    
}

/*
 * Function:    ~BSTTree
 * Description: The destructor for 
 *              the BSTTree class.
 ***********************************/
BSTTree::~BSTTree()
{
    this->cleanup(root);
}

/*
 * Function:    cleanup
 * Description: Cleanup after the 
 *              tree.
 ***********************************/
void BSTTree::cleanup(BSTnode *node)
{
    if(!node)
    {
        return;
    }
    if(node->left)
    {
        cleanup(node->left);
        node->left = 0;
    }
    if(node->right)
    {
        cleanup(node->right);
        node->right = 0;
    }
    delete node;
}

/*
 * Function:    AddNode
 * Description: Adds a node to the 
 *              tree.
 ***********************************/
void BSTTree::AddNode(BSTnode *node)
{
    if(!root)
    {
        root = node;
        nodes.push_back(node);
        return;
    }
    this->HelpAddNode(node, root);
}

/*
 * Function:    HelpAddNode
 * Description: A help function for 
 *              AddNode.
 ***********************************/
void BSTTree::HelpAddNode(BSTnode * node, BSTnode *curr)
{
    if(node->filename.compare(curr->filename) < 0)
    {
        if(curr->left == 0)
        {
            curr->left = node;
            nodes.push_back(node);
            return;
        }
        else
        {
            HelpAddNode(node, curr->left);
        }
    }
    else
    {
        if(curr->right == 0)
        {
            curr->right = node;
            nodes.push_back(node);
            return;
        }
        else
        {
            HelpAddNode(node, curr->right);
        }
    }
}

/*
 * Function:    finditem
 * Description: Finds and returns a 
 *              from the tree by the
 *              key.
 ***********************************/
BSTnode * BSTTree::findItem(unsigned int key)
{
    for(unsigned int currentP = 0;currentP < nodes.size();currentP++)
    {
        if(nodes[currentP]->key == key)
        {
            return nodes[currentP];
        }
    }
    return 0;
}


/*
 * Function:    finditem
 * Description: Finds and returns a 
 *              from the tree by the
 *              name.
 ***********************************/
BSTnode * BSTTree::findItem(const char * name)
{
    char * buffer = new char[strlen(name) + 1];
    ZeroMemory(buffer, strlen(name)+1);
    strcpy(buffer, name);
    ConvertToLowerCase(buffer);
    BSTnode * n = BSTTree::HelpFindItem(buffer, this->root);
    delete [] buffer;
    return n;
}


/*
 * Function:    HelpFinditem
 * Description: Help function for 
 *              finditem.
 ***********************************/
BSTnode * BSTTree::HelpFindItem(const char * name, BSTnode * current)
{
    if(current == 0) return 0;
    if (current->filename == name) return current;
    else if (current->filename.compare(name) > 0)
        return HelpFindItem(name, current->left);
    else return HelpFindItem(name, current->right);
}

/*
 * Function:    addItem
 * Description: Adds a item to the tree
 **************************************/
void BSTTree::addItem(LPDEVMODE mode, const char * name, const char * dName, HWND hWnd, bool isIgnored, int icon)
{
    
    int length = (unsigned int) strlen(name);
    char * buffer = new char[length+1];
    ZeroMemory(buffer, length+1);
    strncpy(buffer, name,length);
    PathStripPath(buffer);
    BSTnode * temp = new BSTnode;
    temp->ignored = isIgnored;
    temp->showName = buffer;
    
    // Create the "Resolution string" ..
    
    if(isIgnored == false)
    {
        itoa(mode->dmPelsWidth, buffer, 10);
        temp->resString += buffer;
        temp->resString += "x";
        itoa(mode->dmPelsHeight, buffer, 10);
        temp->resString += buffer;
        temp->resString += " ";
        itoa(mode->dmBitsPerPel, buffer, 10);
        temp->resString += buffer;
        temp->resString += "bits ";
        itoa(mode->dmDisplayFrequency, buffer, 10);
        temp->resString += buffer;
        temp->resString += "hz";
    }
    else
    {
        temp->resString = "Ignored";
    }

    strncpy(buffer, name,length);
    ConvertToLowerCase(buffer);
    temp->filename = buffer;
    temp->programMode = mode;
    temp->left = temp->right = 0;
    delete [] buffer;

    LVITEM item;
    temp->key = mainKey++;
    item.iImage = icon;
    item.iItem = temp->key;
    item.pszText = LPSTR_TEXTCALLBACK;
    item.lParam = (LPARAM) temp->filename.c_str();
    item.iSubItem = 0;
    item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE; 
    item.state = 0; 
    item.stateMask = 0; 
    // insert the item
    ListView_InsertItem(hWnd, &item);
    
    // Add the node.
    return AddNode(temp);
}

/*
 * Function:    BSTHelpSave
 * Description: Help function for the
 *              save funcion.
 ***********************************/
int BSTHelpSave(const char * filename, FILE *file, BSTnode *node)
{
    char buffer[1024+1];
    const DWORD data = 0;
    ZeroMemory(buffer, 1024+1);
    strncpy(buffer, node->filename.c_str(), 1024);
    fwrite(buffer, 1024, 1, file);
    if(node->ignored)
    {
        fwrite(&data, sizeof(DWORD), 1, file);
        fwrite(&data, sizeof(DWORD), 1, file);
        fwrite(&data, sizeof(DWORD), 1, file);
        fwrite(&data, sizeof(DWORD), 1, file);
    }
    else 
    {
        fwrite(&node->programMode->dmPelsWidth, sizeof(DWORD), 1 ,file);
        fwrite(&node->programMode->dmPelsHeight, sizeof(DWORD), 1 ,file);
        fwrite(&node->programMode->dmBitsPerPel, sizeof(DWORD), 1 ,file);
        fwrite(&node->programMode->dmDisplayFrequency, sizeof(DWORD), 1 ,file);
    }
    if(node->left)
    {
        BSTHelpSave(filename, file, node->left);
    }
    if(node->right)
    {
        BSTHelpSave(filename, file, node->right);
    }
    return 0;
}

/*
 * Function:    save
 * Description: saves the tree info to
 *              a file.
 ***********************************/

int BSTTree::save(const char *filename)
{
    FILE * file = fopen(filename,"wb");
    if(!file) return -1;
    BSTHelpSave(filename, file, this->root);
    fclose(file);
    return 0;
}

/*
 * Function:    load
 * Description: Load info from a file.
 ***********************************/
int BSTTree::load(const char *_filename, std::vector<DEVMODE> &vec, HWND hWnd, unsigned int *currentImage)
{
    FILE * file = fopen(_filename, "rb");
    if(!file) 
    {
        return -1;
    }
    char buffer[1024+1];
    WORD wD;
    DWORD width, height, hz, color;
    LPDEVMODE mode = 0;
    int nret;
    int id;
    while(1)
    {
        mode = 0;
        bool found = false;
        ZeroMemory(buffer, 1024+1);
        
        nret = (int) fread(buffer, 1024, 1, file);
        if(nret != 1)
        {
            break;
        }
        nret = (int) fread(&width, sizeof(DWORD), 1, file);
        if(nret != 1)
        {
            MessageBox(hWnd, "Mailformed settings file", "Error", MB_OK | MB_ICONEXCLAMATION);
            break;
        }
        nret = (int) fread(&height, sizeof(DWORD), 1, file);
        if(nret != 1)
        {
            MessageBox(hWnd, "Mailformed settings file", "Error", MB_OK | MB_ICONEXCLAMATION);
            break;
        }
        nret = (int) fread(&color, sizeof(DWORD), 1, file);
        if(nret != 1)
        {
            MessageBox(hWnd, "Mailformed settings file", "Error", MB_OK | MB_ICONEXCLAMATION);  
            break;
        }
        nret = (int) fread(&hz, sizeof(DWORD), 1, file);
        if(nret != 1)
        {
            MessageBox(hWnd, "Mailformed settings file", "Error", MB_OK | MB_ICONEXCLAMATION);
            break;
        }

        // Make sure that the application exists.
        //***************************************
        HANDLE h = CreateFile((LPCSTR) buffer,0,0,0, OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);

        if(h == INVALID_HANDLE_VALUE)
        {
            if(GetLastError() == ERROR_FILE_NOT_FOUND)
            {
                continue; // skip this file ..
            }
        }
        else
        {
            // Do some cleanup
            CloseHandle(h);
        }
        HICON icon;

        // Extract the icon
        if((icon = (HICON)ExtractAssociatedIcon((HINSTANCE) GetWindowLong(hWnd, GWL_HINSTANCE),
                                                    (LPSTR)buffer,
                                                    &wD)) == NULL)
        {
            MessageBox(0, "failed to load icon",0,0);
        }
        else
        {
            // add the icon to the list ..
            id = ImageList_AddIcon(hImageListSmall,icon);
            DestroyIcon(icon);
        }
        if(width == 0 || height == 0 || color == 0 || hz == 0)
        {
            this->addItem(mode,buffer, 0, hWnd, true, id);

            continue;
        }
        
        found = false;

        for(unsigned int i=0;i<vec.size();i++)
        {
            if(vec[i].dmBitsPerPel == color &&
               vec[i].dmPelsWidth == width &&
               vec[i].dmPelsHeight == height &&
               vec[i].dmDisplayFrequency == hz)
            {
                found = true;
                mode = (DEVMODE *)&vec[i];
            }
        }
        if(found == false)
        {
            this->addItem(mode,buffer, 0, hWnd, true, id);
        }
        else
        {
            this->addItem(mode,buffer, 0, hWnd, false, id);
        }   
    } 

    // cleanup ..
    fclose(file);
}

