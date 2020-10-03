
#pragma warn(disable: 2008 2118 2228 2231 2030 2260)

#include        <windows.h>
#include        <strsafe.h>
#include        <shellapi.h>
#include        "tray.h"


/*-@@+@@--------------------------------------------------------------------*/
//       Function: Tray_Add 
/*--------------------------------------------------------------------------*/
//           Type: BOOL 
//    Param.    1: HWND hwnd             : 
//    Param.    2: UINT uID              : 
//    Param.    3: HICON hicon           : 
//    Param.    4: const TCHAR * lpszTip : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: add a tray icon to systray
/*--------------------------------------------------------------------@@-@@-*/
BOOL Tray_Add ( HWND hwnd, UINT uID, HICON hicon, const TCHAR * lpszTip )
/*--------------------------------------------------------------------------*/
{
    NOTIFYICONDATA  ni;

    ni.cbSize           = sizeof ( NOTIFYICONDATA );
    ni.hWnd             = hwnd;
    ni.uID              = uID;
    ni.uFlags           = NIF_MESSAGE|NIF_ICON|NIF_TIP;
    ni.uCallbackMessage = WM_TRAY;
    ni.hIcon            = hicon;
    if ( lpszTip )
        StringCchCopyN ( ni.szTip, ARRAYSIZE(ni.szTip), 
            lpszTip, ARRAYSIZE(ni.szTip)-1 );
    else
        ni.szTip[0] = TEXT('\0');

    return Shell_NotifyIcon ( NIM_ADD, &ni );
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: Tray_Remove 
/*--------------------------------------------------------------------------*/
//           Type: BOOL 
//    Param.    1: HWND hwnd: 
//    Param.    2: UINT uID : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: remove it
/*--------------------------------------------------------------------@@-@@-*/
BOOL Tray_Remove ( HWND hwnd, UINT uID )
/*--------------------------------------------------------------------------*/
{
    NOTIFYICONDATA  ni;

    ni.cbSize   = sizeof ( NOTIFYICONDATA );
    ni.hWnd     = hwnd;
    ni.uID      = uID;

    return Shell_NotifyIcon ( NIM_DELETE, &ni );
}

