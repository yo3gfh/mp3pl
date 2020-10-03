

#ifndef _TRAY_H
#define _TRAY_H

// notification message for tray icon
#define     WM_TRAY         WM_USER + 69    

BOOL Tray_Add ( HWND hwnd, UINT uID, HICON hicon, const TCHAR * lpszTip );
BOOL Tray_Remove ( HWND hwnd, UINT uID );

#endif

