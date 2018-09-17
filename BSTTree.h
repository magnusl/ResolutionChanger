#ifndef _BSTTREE_H_
#define _BSTTREE_H_

#include "BSTnode.h"
#include <windows.h>
#include <vector>

class BSTTree
{
public:
    BSTTree();
    ~BSTTree();
    
    // adds a elemnt to the BST..
    void addItem(LPDEVMODE,const char *, const char *, HWND hWnd, bool, int);
    BSTnode * findItem(const char * name);
    BSTnode * findItem(unsigned int);
    BSTnode * getRoot() {return root;}
    int save(const char *);
    int load(const char *_filename, std::vector<DEVMODE> &vec, HWND hWnd, unsigned int *currentImage);
    std::vector<BSTnode *> & getVector() {return nodes;}
private:
    int mainKey;

    void AddNode(BSTnode *node);
    void cleanup(BSTnode *node);
    void HelpAddNode(BSTnode *node, BSTnode *curr);
    std::vector<BSTnode *> nodes;
    BSTnode * HelpFindItem(const char * name, BSTnode *current);
    BSTnode *root;
};

#endif