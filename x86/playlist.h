
#ifndef _PLAYLIST_H
#define _PLAYLIST_H
#include <windows.h>

// size for filename buffer in open dialog;
#define     ALLOC_SIZE      0x0000FFFF

typedef struct
{
    TCHAR   * text;
    int     width;
    int     flags;
}   COLDATA;

void Playlist_Init 
    ( HWND hList, HINSTANCE hInst, HIMAGELIST * pIml );

BOOL Playlist_LoadFromMem
    ( HWND hList, const TCHAR * filebuf, DWORD dirlen );

BOOL    Playlist_LoadFromCmdl   ( HWND hList, TCHAR ** filelist, int files );
BOOL    Playlist_Add            ( HWND hList, HWND hParent, HINSTANCE hInst );
BOOL    Playlist_Save           ( HWND hList, HWND hParent, HINSTANCE hInst );
BOOL    Playlist_Open           ( HWND hList, HWND hParent, HINSTANCE hInst );
BOOL    Playlist_LoadFromFile   ( HWND hList, const TCHAR * name );
BOOL    Playlist_SaveToFile     ( HWND hList, const TCHAR * name );

#endif
