#include <windows.h>
#include <list>
#include <vector>
#include "Resource.h"

#include "bsttree.h"
#include "bstnode.h"
#include "commctrl.h"
#pragma comment(lib,"comctl32.lib")
#include <Psapi.h.>
#pragma comment(lib, "Psapi.lib")

#define MAX_LENGTH 1024
/* Application Text
 ******************/

const char * OwnerString = "'OWNER'";
const char * ApplicationNameString = "Resolution Changer 1.0";




/* Global Variables
 ******************/
std::vector<DEVMODE> DisplayModesVec;
#define OPENPROGRAM 0x1000
#define EXITPROGRAM 0x2000
#define APPLICATION 0x3000
BSTTree tree;
BSTnode * currentNode;
static HWND listBox = 0;
std::string currentApplication;
unsigned int currentImage = 0;
DEVMODE current, defaultMode;
HIMAGELIST hImageListSmall; // a list for the images ..



/* WARNING: The listboxes can not be sorted, if you want them to be
 * sorted you have to get the selected string from the listbox and use that
 * to figure out what the user selected. 
*/
INT_PTR CALLBACK SetupProc(HWND hWnd, UINT msg,WPARAM wParam,LPARAM lParam)
{
    std::string buffer;
    char strBuffer[100];
    int selected;
    switch(msg)
    {
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case ID_SETUPOK:
            selected = (int) SendDlgItemMessage(hWnd,
                                          IDC_MODELIST,
                                          LB_GETCURSEL,
                                          0,
                                          0);
            if(selected >= 0 && (unsigned int)selected < DisplayModesVec.size())
            {
                // a valid one was selected
                memcpy(&defaultMode, &DisplayModesVec[selected], sizeof(DEVMODE));
                EndDialog(hWnd, 0);
                return TRUE;
            }
            else
            {
                MessageBox(hWnd, "Please select a resolution to use","Error",MB_OK);
                return TRUE;
            }

        }
        return TRUE;
    case WM_INITDIALOG:
        /* The setup text */

        buffer = "Welcome to ";
        buffer += ApplicationNameString;
        buffer += ". To start using ";
        buffer += ApplicationNameString;    
        buffer += " you must first select a default desktop resolution to use. \nPlease select a mode from the list below and press the 'ok' button to continue\n\n";
        buffer += ApplicationNameString;
        buffer += " © 2005 ";
        buffer += OwnerString;
        buffer += ". All rights reserved.";
        SendDlgItemMessage(hWnd, 
                           IDC_SETUPTEXT,
                           WM_SETTEXT,
                           0,
                           (LPARAM)buffer.c_str());
        /* Fill the listbox with the modes */
        for(unsigned int i=0;i<DisplayModesVec.size();i++)
        {
            sprintf(strBuffer,
                    "%dx%d %dbit %dhz",
                    DisplayModesVec[i].dmPelsWidth,
                    DisplayModesVec[i].dmPelsHeight,
                    DisplayModesVec[i].dmBitsPerPel,
                    DisplayModesVec[i].dmDisplayFrequency);

            SendDlgItemMessage(hWnd, IDC_MODELIST,LB_ADDSTRING, 0, (LPARAM) strBuffer);

        }
        return TRUE;
                
    case WM_DESTROY:
    case WM_CLOSE:
        return TRUE;
    }
    return FALSE;
}

INT_PTR CALLBACK DlgProc(HWND hWnd, UINT msg, WPARAM wParam,LPARAM lParam)
{
    WORD wD;
    static std::string name;
    int selected;
    char buffer[100];
    HICON icon;
    int id;

    switch(msg)
    {
    case WM_COMMAND:
        
        // Load the Icon for the Application
        //***********************************
        if((icon = (HICON)ExtractAssociatedIcon((HINSTANCE) GetWindowLong(hWnd, GWL_HINSTANCE), 
                                               (LPSTR)currentApplication.c_str(),
                                                &wD)) == NULL)
        {
            // error, could not load icon ..
        }
        // The user wants to ignore this program
        //**************************************
        if(LOWORD(wParam) == IDIGNORE)
        {
            id = ImageList_AddIcon(hImageListSmall,icon);
            DestroyIcon(icon);
            tree.addItem(&DisplayModesVec[selected], currentApplication.c_str(), "", listBox, true,id);
            EndDialog(hWnd, 0);
        }
        //
        // The user wants to set a custom mode for this application
        //*********************************************************
        else if(LOWORD(wParam) == IDOK)
        {
            // Make sure that the user has choosen a mode.
            //********************************************
            selected = (int) SendDlgItemMessage(hWnd, MODELIST, LB_GETCURSEL, 0,0);
            if(selected < 0)
            {
                // Cleanup
                DestroyIcon(icon);
                MessageBox(hWnd, "Please select a resolution", 0, MB_OK);
                return TRUE;
            }
            else
            {   
                // The user has selected a mode from the list,
                // now add the application to the Tree and update the
                // Screen mode
                //***************************************************
                id = ImageList_AddIcon(hImageListSmall,icon);
                DestroyIcon(icon);
                tree.addItem(&DisplayModesVec[selected], currentApplication.c_str(), "", listBox, false,id);
                
                // Fist perform a test to see if the mode can be used by the current
                // configuration.
                //******************************************************************
                if(ChangeDisplaySettings(&DisplayModesVec[selected], CDS_TEST) == DISP_CHANGE_SUCCESSFUL)
                {
                    if(ChangeDisplaySettings(&DisplayModesVec[selected], CDS_SET_PRIMARY) == DISP_CHANGE_SUCCESSFUL)
                    {
                        EnumDisplaySettings(0,ENUM_CURRENT_SETTINGS, &current);
                    }
                    else
                    {
                        MessageBox(hWnd, "Failed to set the screen resolution", "Error", MB_OK | MB_ICONEXCLAMATION);
                    }
                }
                EndDialog(hWnd, 0);
            }
        }
        return 0;
    case WM_INITDIALOG:
        SendDlgItemMessage(hWnd, ApplicationName, WM_SETTEXT, 0, (LPARAM)currentApplication.c_str());
        for(unsigned int index = 0;index < DisplayModesVec.size();index++)
        {
            sprintf(buffer, "%dx%d %dbits %dhz",DisplayModesVec[index].dmPelsWidth,
                    DisplayModesVec[index].dmPelsHeight,
                    DisplayModesVec[index].dmBitsPerPel,
                    DisplayModesVec[index].dmDisplayFrequency);
            SendDlgItemMessage(hWnd, MODELIST, LB_ADDSTRING, 0, (LPARAM) buffer);                   
        }
        return 0;
    case WM_CLOSE:
    case WM_DESTROY:
        return TRUE;
    }
    return FALSE;
}

/*
 * Function:    CreateListView
 * Description: Creates the ListView control
 *********************************************/
HWND CreateListView(HWND hWnd)
{
    std::vector<BSTnode *> vec = tree.getVector();
    HWND hWndListView;
    RECT rcl; 
    INITCOMMONCONTROLSEX icex;

    // Ensure that the common control DLL is loaded. 
    
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC  = ICC_LISTVIEW_CLASSES; 
    InitCommonControlsEx(&icex);

    GetClientRect(hWnd, &rcl);

    // Create the window
    hWndListView = CreateWindow(WC_LISTVIEW,
                                "",
                                LVS_NOSORTHEADER | LVS_REPORT | WS_VISIBLE | WS_CHILD | LVS_EDITLABELS,
                                0,
                                0,
                                rcl.right,
                                rcl.bottom,
                                hWnd,
                                0,
                                (HINSTANCE) GetWindowLong(hWnd, GWL_HINSTANCE),
                                0);

    if(!hWndListView)
    {
        MessageBox(0, "Could not create ListView, aborting", 0,0);
        return 0;
    }

    // Create the columns for the ListView
    LVCOLUMN col;
    col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT |
    LVCF_SUBITEM; 
    col.iSubItem = 0;

    // The first column
    col.pszText = "Application Name";
    col.cx = 140;
    col.fmt = LVCFMT_LEFT;
    ListView_InsertColumn(hWndListView, 0, &col);

    // The second column
    col.pszText = "Screen Resolution";
    col.cx = rcl.right - 160;
    col.iSubItem = 1;
    ListView_InsertColumn(hWndListView, 1, &col);
    
    LVITEM item;
    
    // Add the Items ..
    for(unsigned int icurrent=0;icurrent < vec.size();icurrent++)
    {
        item.iImage = 0;
        item.iItem = vec[icurrent]->key;
        item.pszText = LPSTR_TEXTCALLBACK;
        item.lParam = (LPARAM) vec[icurrent]->filename.c_str();
        item.iSubItem = 0;
        item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE; 
        item.state = 0; 
        item.stateMask = 0; 
        ListView_InsertItem(hWndListView, &item);
    }
    return hWndListView;
}

/* Function:    GetDisplaySettings
 * Description: Retrives all the DEVMODE the current
 *              display/graphicscard can handle.
 ***************************************************/
void GetDisplaySettings()
{
    DEVMODE devMode;
    devMode.dmSize = sizeof(devMode);
    int currentMode = 0;
    while(EnumDisplaySettingsEx(NULL, // use the primary device
                              currentMode++,
                              &devMode,
                              0) != 0)
    {
        // Filter out the DEVMODES with the Screen Area < (480 * 640)
        // and the modes where the frequency is less than 60 and
        // the modes where the colors is less than 16 bits/pixel
        //***********************************************************
        if(devMode.dmBitsPerPel < 16 ||
            devMode.dmDisplayFrequency < 60 ||
            ((devMode.dmPelsHeight * devMode.dmPelsWidth) < 480*640)) continue;

        DisplayModesVec.push_back(devMode);
        //DisplayModes.push_back(devMode);
    }
    /*EnumDisplaySettings(0,ENUM_CURRENT_SETTINGS, &current);
    EnumDisplaySettings(0,ENUM_CURRENT_SETTINGS, &defaultMode); 
*/
}

/* Function:    CreateMenu
 * Description: Creates the main Application menu
 ************************************************/

DEVMODE * createMenu(HWND hWnd, int x, int y)
{
    MENUITEMINFO item, item2;
    HMENU menu = CreatePopupMenu(), menu2 = CreatePopupMenu();
    ZeroMemory(&item, sizeof(item));
    item.cbSize = sizeof(item);
    item.fMask = MIIM_STRING | MIIM_CHECKMARKS | MIIM_FTYPE | MIIM_STATE | MIIM_DATA;
    item.fType = MFT_STRING;
    item.fState = MFS_CHECKED;

    item2.cbSize = sizeof(item);
    item2.fMask = MIIM_STRING | MIIM_CHECKMARKS | MIIM_FTYPE | MIIM_STATE | MIIM_DATA;
    item2.fType = MFT_STRING;
    item2.fState = MFS_CHECKED;

    
    std::string temp;

    HMENU Res = CreatePopupMenu();
    HMENU mainMenu = CreatePopupMenu();

    // The "Open Application" item
    AppendMenu(mainMenu,MF_UNCHECKED | MF_STRING, OPENPROGRAM, "Open Application");
    // The "Exit" item
    AppendMenu(mainMenu,MF_UNCHECKED | MF_STRING, EXITPROGRAM, "Exit");
    

    MENUITEMINFO submenuItem, submenuItem2;
    ZeroMemory(&submenuItem, sizeof(MENUITEMINFO));
    ZeroMemory(&submenuItem2, sizeof(MENUITEMINFO));
    submenuItem.cbSize = sizeof(submenuItem);
    submenuItem.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_STATE | MIIM_SUBMENU;
    submenuItem.fType = MFT_STRING;
    
    submenuItem.hSubMenu = menu;

    // The submenu for the 32 bit modes.
    submenuItem.dwTypeData = "32 bits";
    submenuItem.cch = (int) strlen("32 bits");
    // insert the submenu
    InsertMenuItem(Res, 0, TRUE, &submenuItem);

    // The submenu for the 16 bit modes.
    submenuItem.dwTypeData = "16 bits";
    submenuItem.hSubMenu = menu2;
    // insert the submenu
    InsertMenuItem(Res, 0, TRUE, &submenuItem);
    
    submenuItem.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_STATE | MIIM_SUBMENU;
    submenuItem.hSubMenu = Res;
    submenuItem.dwTypeData = "Change Desktop Resolution";
    submenuItem.cch = (int) strlen("Change Desktop Resolution");
    InsertMenuItem(mainMenu, 0, TRUE, &submenuItem);

    char stringBuffer[256];
    int pos = 0;
    for(unsigned int i=0;i<DisplayModesVec.size();i++)
    {
        // Create the string for the menu
        sprintf(stringBuffer,
                "%dx%d %dhz",
                DisplayModesVec[i].dmPelsWidth, 
                DisplayModesVec[i].dmPelsHeight, 
                DisplayModesVec[i].dmDisplayFrequency);

        item.dwTypeData = (LPSTR) stringBuffer;
        item.cch = (int) strlen(stringBuffer);
        item.dwItemData = 2;

        // Insert the new item in the correct menu.
        if(DisplayModesVec[i].dmBitsPerPel == 32) 
        {
            if(defaultMode.dmBitsPerPel == DisplayModesVec[i].dmBitsPerPel &&
                defaultMode.dmDisplayFrequency == DisplayModesVec[i].dmDisplayFrequency &&
                defaultMode.dmPelsHeight == DisplayModesVec[i].dmPelsHeight &&
                defaultMode.dmPelsWidth == DisplayModesVec[i].dmPelsWidth)
            {
                AppendMenu(menu,MF_CHECKED | MF_STRING  ,pos++,stringBuffer);
            }
            else
            {
                AppendMenu(menu,MF_UNCHECKED | MF_STRING  ,pos++,stringBuffer);
            }

        }
        else 
        {
            if(defaultMode.dmBitsPerPel == DisplayModesVec[i].dmBitsPerPel &&
                defaultMode.dmDisplayFrequency == DisplayModesVec[i].dmDisplayFrequency &&
                defaultMode.dmPelsHeight == DisplayModesVec[i].dmPelsHeight &&
                defaultMode.dmPelsWidth == DisplayModesVec[i].dmPelsWidth)
            {
                AppendMenu(menu2,MF_CHECKED | MF_STRING  ,pos++,stringBuffer);
            }
            else AppendMenu(menu2,MF_UNCHECKED | MF_STRING  ,pos++,stringBuffer);
        }
    }
    
        // Show the menu
        TrackPopupMenu(mainMenu,
                   TPM_BOTTOMALIGN,
                   x,
                   y,
                   0,
                   hWnd,
                   0);

    return 0;
}

/* Function:    createApplicationMenu
 * Description: Creates the menu for the applications
 ***************************************************/
void createApplicationMenu(HWND hWnd, int x, int y, BSTnode * node)
{
    MENUITEMINFO item;
    HMENU menu = CreatePopupMenu(), menu2 = CreatePopupMenu();
    ZeroMemory(&item, sizeof(item));
    item.cbSize = sizeof(item);
    item.fMask = MIIM_STRING | MIIM_CHECKMARKS | MIIM_FTYPE | MIIM_STATE | MIIM_DATA;
    item.fType = MFT_STRING;
    item.fState = MFS_CHECKED;

    std::string temp;

    HMENU Res = CreatePopupMenu();

    MENUITEMINFO submenuItem, submenuItem2;
    ZeroMemory(&submenuItem, sizeof(MENUITEMINFO));
    ZeroMemory(&submenuItem2, sizeof(MENUITEMINFO));
    submenuItem.cbSize = sizeof(submenuItem);
    submenuItem.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_STATE | MIIM_SUBMENU;
    submenuItem.fType = MFT_STRING;
    
    submenuItem.hSubMenu = menu;
    submenuItem.dwTypeData = "32 bits";
    submenuItem.cch = (int) strlen("32 bits");
        
    InsertMenuItem(Res, 0, TRUE, &submenuItem);
    submenuItem.dwTypeData = "16 bits";
    submenuItem.hSubMenu = menu2;
    InsertMenuItem(Res, 0, TRUE, &submenuItem);
    

    int pos = 0;
    char stringBuffer[256];
    for(unsigned int i = 0;i<DisplayModesVec.size();i++)
    {   
        sprintf(stringBuffer,
                "%dx%d %dhz", 
                DisplayModesVec[i].dmPelsWidth, 
                DisplayModesVec[i].dmPelsHeight,
                DisplayModesVec[i].dmDisplayFrequency);
        item.dwTypeData = (LPSTR)stringBuffer;
        item.cch = (unsigned int) strlen(stringBuffer);
        item.dwItemData = 2;
        if(DisplayModesVec[i].dmBitsPerPel == 32) 
        {
            if(node->ignored == true)
            {
                AppendMenu(menu,MF_UNCHECKED | MF_STRING  ,APPLICATION+pos++,stringBuffer);
            }
            else if(node->programMode->dmBitsPerPel == DisplayModesVec[i].dmBitsPerPel &&
                node->programMode->dmDisplayFrequency == DisplayModesVec[i].dmDisplayFrequency &&
                node->programMode->dmPelsHeight == DisplayModesVec[i].dmPelsHeight &&
                node->programMode->dmPelsWidth == DisplayModesVec[i].dmPelsWidth)
            {
                AppendMenu(menu,MF_CHECKED | MF_STRING  ,APPLICATION + pos++,stringBuffer);
            }
            else
            {
                AppendMenu(menu,MF_UNCHECKED | MF_STRING  ,APPLICATION+pos++,stringBuffer);
            }

        }
        else 
        {
            if(node->ignored) 
            {
                AppendMenu(menu2,MF_UNCHECKED | MF_STRING  ,APPLICATION+pos++,stringBuffer);
            }
            else if(node->programMode->dmBitsPerPel == DisplayModesVec[i].dmBitsPerPel &&
                node->programMode->dmDisplayFrequency == DisplayModesVec[i].dmDisplayFrequency &&
                node->programMode->dmPelsHeight == DisplayModesVec[i].dmPelsHeight &&
                node->programMode->dmPelsWidth == DisplayModesVec[i].dmPelsWidth)
            {
                AppendMenu(menu2,MF_CHECKED | MF_STRING  ,APPLICATION + pos++,stringBuffer);
            }
            else AppendMenu(menu2,MF_UNCHECKED | MF_STRING  ,APPLICATION+pos++,stringBuffer);
        }
    }
    
    
        currentNode = node;
        TrackPopupMenu(Res,
                       TPM_BOTTOMALIGN,
                       x,
                       y,
                       0,
                       hWnd,
                       0);
}

 
/*
 * Function:    WndProc
 * Description: The callback function for the 
 *              main window and the menus.
 ********************************************/

LRESULT CALLBACK WndProc(HWND hWnd,
                         UINT msg,
                         WPARAM wParam,
                         LPARAM lParam)
{
    
    RECT rect;
    std::string temp;
    char buffer[100];
    std::list<DEVMODE>::iterator it;
    static POINT point;
    DEVMODE mode;
    int index;
    std::vector<BSTnode *> vec;
    static LVHITTESTINFO lvHit;
    BSTnode *node;
    int numberOfElements;
    switch(msg)
    {
    case WM_NOTIFY:
        switch(((LPNMHDR)lParam)->code)
        {
        case NM_RCLICK:
                    
            ZeroMemory(&lvHit, sizeof(lvHit));
            GetCursorPos(&lvHit.pt);
            GetWindowRect(listBox,&rect);
            lvHit.pt.x -= rect.left;
            lvHit.pt.y -= rect.top; 
            lvHit.flags = LVHT_ABOVE;
        
        
            /* Make sure the user clicked on a item in the listview */

            index = ListView_SubItemHitTest(listBox, &lvHit);
            if(index >= 0) 
            {
                // find the item the user clicked on ..
                node = tree.findItem((unsigned int)lvHit.iItem);
                if(node)
                {
                    GetCursorPos(&point);
                    // create the "application menu" for the item
                    createApplicationMenu(hWnd, point.x, point.y, node);
                }

            }
            
            return 0;
        case LVN_GETDISPINFO:
            vec = tree.getVector();
            
            switch(((NMLVDISPINFO *)lParam)->item.iSubItem)
            {
            case 1:
                // Set the 2:d column in the ListView (resolution string)
                ((NMLVDISPINFO *)lParam)->item.pszText = (LPSTR) vec[((NMLVDISPINFO *)lParam)->item.iItem]->resString.c_str();
                return 0;
            case 0:
                // Set the 1:st column in the ListView (application name)
                ((NMLVDISPINFO *)lParam)->item.pszText = (LPSTR) vec[((NMLVDISPINFO *)lParam)->item.iItem]->showName.c_str();
                return 0;
            }
            return 0;
        }
        return 0;

    case WM_SIZE:
        if(wParam == SIZE_MINIMIZED)
        {
            // The window was minimized, hide it!
            ShowWindow(hWnd, SW_HIDE);
        }
        else
        {
            // Update the window size
            GetClientRect(hWnd, &rect);
            MoveWindow(listBox, 0,0, rect.right, rect.bottom, TRUE);
        }
        return 0;
    case WM_USER + 2:
        // the windwos should be vivible
        ShowWindow(hWnd, SW_SHOW);
    
        return 0;
    case WM_USER + 1:
        switch(LOWORD(lParam))
        {
        case WM_RBUTTONDOWN:
            // the user rightcklicked on the tray icon, show the menu
            GetCursorPos(&point);
            SetForegroundWindow(hWnd);
            PostMessage(hWnd, WM_NULL,0,0);
            createMenu(hWnd, point.x, point.y);
            return 0;
        case WM_LBUTTONDBLCLK:
            ShowWindow(hWnd, SW_RESTORE);
            SetForegroundWindow(hWnd);
            // This line must be here for it to work!
            PostMessage(hWnd, WM_NULL, 0,0);
            // Don't remove it!
            return 0;
        }
        return 0;
    case WM_COMMAND:
        if(HIWORD(wParam) == 0)
        {
        
            if(LOWORD(wParam) >= APPLICATION)
            {
                currentNode->programMode = &DisplayModesVec[LOWORD(wParam) - APPLICATION];
                numberOfElements = (int)SendMessage(listBox, LVM_GETITEMCOUNT, 0,0);
                for(int i=0;i<numberOfElements;i++)
                {
                    currentNode->UpdateText();
                    currentNode->ignored = false;
                    SendMessage(listBox, LVM_UPDATE, 0,(LPARAM)i);
                }
                return 0;
            }
            if(LOWORD(wParam) == EXITPROGRAM)
            {
                PostQuitMessage(0);
            }
            else if(LOWORD(wParam) == OPENPROGRAM)
            {
                ShowWindow(hWnd, SW_RESTORE);
            }

            else if(LOWORD(wParam) >= DisplayModesVec.size())
            {
                MessageBox(hWnd, "Display Vector out of range", 0,0);

            }
            else
            {
                // Get the selected mode ..
                mode = DisplayModesVec[(LOWORD(wParam))];

                // Create the message tp display..
                temp = "Are you sure you want to change the default screen resoultion to:\n";
                itoa(mode.dmPelsWidth, buffer, 10);
                temp += buffer;
                temp += "x";
                itoa(mode.dmPelsHeight, buffer, 10);
                temp += buffer;
                temp += " ";
                itoa(mode.dmBitsPerPel, buffer, 10);
                temp += buffer;
                temp += "bits ";
                itoa(mode.dmDisplayFrequency, buffer, 10);
                temp += buffer;
                temp += "hz?";

                // Show the confirmation dialog
                if(MessageBox(0, temp.c_str(), "Confirmation", MB_YESNO | MB_ICONQUESTION) == IDYES)
                {
                    // set the choosen default mode..
                    memcpy(&defaultMode, &DisplayModesVec[(LOWORD(wParam))], sizeof(defaultMode));
                }
                        
            }
        }
        return 0;

    case WM_CREATE:
        return 0;                          
    case WM_CLOSE:
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd,msg,wParam,lParam);
}

/*
 * Function:    WinMain
 * Description: The Entry point of the application
 ***************************************************/

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{

    // The Window Class used to create the window
    WNDCLASS wc;
    ZeroMemory(&wc, sizeof(WNDCLASS));
    wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
    wc.hInstance = hInstance;
    wc.lpfnWndProc = WndProc;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wc.lpszClassName = "classname";
    wc.style = CS_HREDRAW | CS_VREDRAW;

    // Create a Image List ..
    hImageListSmall = ImageList_Create(GetSystemMetrics(SM_CXICON),
    GetSystemMetrics(SM_CYICON), ILC_MASK | ILC_COLOR32, 1, 1); 
    
    
    if(!RegisterClass(&wc))
    {
        MessageBox(0,
                   "FAILED to register class, abortin..",
                   "Error",
                   MB_OK | MB_ICONEXCLAMATION);
        // Destroy the ImageList ..
        ImageList_Destroy(hImageListSmall);
        return -1;
    }

    HWND mainWnd = CreateWindow("classname",
                                ApplicationNameString,
                                WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                                CW_USEDEFAULT,
                                CW_USEDEFAULT,
                                420,
                                300,
                                0,
                                0,
                                hInstance,
                                0);
    if(!mainWnd)
    {
        MessageBox(0,
                   "FAILED to create window, aborting..",
                   "Error",
                   MB_OK | MB_ICONEXCLAMATION);
        ImageList_Destroy(hImageListSmall);
        return -1;
    }

    // Get the display settings..
    GetDisplaySettings();

    // Create the ListView..
    listBox = CreateListView(mainWnd);
    ListView_SetImageList(listBox, hImageListSmall, LVSIL_SMALL);

    // Load the data file with the application data
    tree.load("data.dat", DisplayModesVec, listBox, &currentImage);
    MSG msg;

    // Create the trayicon ..
    NOTIFYICONDATA trayData;
    ZeroMemory(&trayData, sizeof(trayData));
    trayData.cbSize = sizeof(trayData);
    trayData.hWnd = mainWnd;
    trayData.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    trayData.uCallbackMessage = (WM_USER) + 1;
    trayData.uFlags =  NIF_MESSAGE  | NIF_ICON;
    trayData.uID = 101;
    // Add the trayicon
    Shell_NotifyIcon(NIM_ADD, &trayData);

    HWND oldWnd = 0, currentWnd;
    DWORD id;
    HANDLE process;

    // Open the settings file ..
    FILE * settingsFile = fopen("settings.dat","rb");
    
        if(!settingsFile)
        {
            // The file does not exist
            // Get the default resolution ..
            // Set the default ..
            DialogBox(hInstance,
                MAKEINTRESOURCE(IDD_SETUP),
                0,
                SetupProc);
        }
        else
        {
            // the file exists ..
            // read the data ..
            unsigned int data[4]; //width,height, color, hz
            if(fread(data, 4 * sizeof(unsigned int),1,settingsFile) != 1)
            {
                //Failed to read the data from the file ..
                // use the current mode ..
                EnumDisplaySettings(0,ENUM_CURRENT_SETTINGS, &defaultMode); 
            }
            else
            {
                
                // find the wanted mode 
                for(unsigned int i=0;i<DisplayModesVec.size();i++)
                {
                    if(DisplayModesVec[i].dmPelsWidth == data[0] &&
                        DisplayModesVec[i].dmPelsHeight == data[1] &&
                        DisplayModesVec[i].dmBitsPerPel == data[2] &&
                        DisplayModesVec[i].dmDisplayFrequency == data[3])
                    {
                        // found the wanted one ..
                        // set it as the default.
                        memcpy(&defaultMode, &DisplayModesVec[i], sizeof(DEVMODE));
                    }

                }
            }
            fclose(settingsFile);

        }
    DWORD currentProcessID = GetCurrentProcessId();
    DWORD idDesktop;
    char filebuffer[MAX_LENGTH+1];
    ShowWindow(mainWnd, SW_HIDE);
    while(1)
    {
        if(listBox)
        {
            currentWnd = GetForegroundWindow();
            if(IsMenu((HMENU)currentWnd) == FALSE && 
               currentWnd != oldWnd && 
               IsWindowVisible(currentWnd))
            {
                oldWnd = currentWnd;
                
                GetWindowThreadProcessId(GetShellWindow(), &idDesktop);
                // different window..
                GetWindowThreadProcessId(currentWnd, &id);
                char bufferClass[256];
                
                if(id == idDesktop)
                {
                    // This is the Desktop window               
                    GetClassName(GetForegroundWindow(), bufferClass, 256);

                    // WARNING: To pull this off I had to make a small hack
                    // the names "Shell_TrayWnd" and DV2ControlHost were obtained
                    // using Spy++ and may not be the same in future versions of windows.
                    if(strcmp(bufferClass, "Shell_TrayWnd") == 0 ||
                      (strcmp(bufferClass, "DV2ControlHost") == 0)) continue;
                    if(ChangeDisplaySettings(&defaultMode, CDS_TEST) == DISP_CHANGE_SUCCESSFUL)
                    {
                        if(ChangeDisplaySettings(&defaultMode, CDS_SET_PRIMARY) == DISP_CHANGE_SUCCESSFUL)
                        {
                            // Update the current saved resoultion.
                            EnumDisplaySettings(0,ENUM_CURRENT_SETTINGS, &current);
                        }
                        else
                        {
                            MessageBox(mainWnd, "Failed to set the screen resolution", "Error", MB_OK | MB_ICONEXCLAMATION);
                        }
                    }
                }
                else if(id != currentProcessID) // don't care about this application
                {
                    
                    process = OpenProcess(PROCESS_QUERY_INFORMATION |PROCESS_VM_READ,
                                          FALSE,
                                          id);
                    
                    if(process)
                    {
                        // Could open the process, not a "system" process
                        if(GetModuleFileNameEx(process, 0, filebuffer, MAX_LENGTH) != 0) // Get the filename
                        {   
                            BSTnode * node;
                            // ignor
                                // got the name ..
                                if((node = tree.findItem(filebuffer)) == 0)
                                {
                                    currentApplication = filebuffer;
                                    DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1),0, DlgProc);
                                }
                                else if(node->ignored == false)
                                {
                                    
                                    // the application does exist ..
                                    
                                    if(ChangeDisplaySettings(node->programMode, CDS_TEST) == DISP_CHANGE_SUCCESSFUL)
                                    {
                                        if(ChangeDisplaySettings(node->programMode, CDS_SET_PRIMARY) == DISP_CHANGE_SUCCESSFUL)
                                        {
                                            // Update the current saved resoultion.
                                            EnumDisplaySettings(0,ENUM_CURRENT_SETTINGS, &current);
                                        }
                                        else
                                        {
                                            MessageBox(mainWnd, "Failed to set the screen resolution", "Error", MB_OK | MB_ICONEXCLAMATION);
                                        }
                                    }
                                
                            }
                        }
                        // close the process handle
                        CloseHandle(process);
                    }
                }
            }
        }
        if(PeekMessage(&msg,0,0,0,PM_REMOVE))
        {
    
            if(msg.message == WM_QUIT)
            {
                break;      
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    
        // Sleep for 5 millis, to prevent the application from stealing 99% of the
        // CPU time ..
        Sleep(5);
    
    }

    // Save the application data
    tree.save("data.dat");
    // save the default screen mode ..
    FILE * saveSettings = fopen("settings.dat","wb");
    if(saveSettings)
    {
        fwrite((void *)&defaultMode.dmPelsWidth,sizeof(DWORD),1,saveSettings);
        fwrite(&defaultMode.dmPelsHeight,sizeof(DWORD),1,saveSettings);
        fwrite(&defaultMode.dmBitsPerPel,sizeof(DWORD),1,saveSettings);
        fwrite(&defaultMode.dmDisplayFrequency,sizeof(DWORD),1,saveSettings);
        fclose(saveSettings);
    }
    // remove the shell icon
    Shell_NotifyIcon(NIM_DELETE, &trayData);
    // Destroy the Image list ..
    ImageList_Destroy(hImageListSmall);
    return 0;
}
