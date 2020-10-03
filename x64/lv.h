

#ifndef _LV_H
#define _LV_H

#include <windows.h>

BOOL LVSetItemText 
    ( HWND hList, int nItem, int nSubItem, const TCHAR * lpszText );

BOOL LVSetItemImg ( HWND hList, int nItem, int nImgidx );

BOOL LVGetItemText 
    ( HWND hList, int nItem, int nSubItem, TCHAR * lpszText, int cchMax );

int     LVGetItemImgIdx     ( HWND hList, int nItem );
BOOL    LVClear             ( HWND hList );
BOOL    LVDeleteItem        ( HWND hList, int item );
void    LVDeleteSelection   ( HWND hList );
void    LVSelectAll         ( HWND hList );
void    LVSelectItem        ( HWND hList, int index );
void    LVFocusItem         ( HWND hList, int index );
void    LVUnselectItem      ( HWND hList, int index );
BOOL    LVEnsureVisible     ( HWND hList, int index );

int LVInsertColumn 
    ( HWND hList, int nCol, const TCHAR * lpszColumnHeading, 
    int nFormat, int nWidth, int nSubItem );

int LVInsertItem 
    ( HWND hList, int nItem, int nImgIndex, const TCHAR * lpszItem );

int     LVGetSelIndex       ( HWND hList );
int     LVGetCount          ( HWND hList );
HIMAGELIST InitImgList      ( HINSTANCE hInst );

#endif // _LV_H

