
#pragma warn(disable: 2008 2118 2228 2231 2030 2260)

#include        <windows.h>
#include        <commctrl.h>
#include        <bass.h>
#include        <strsafe.h>

#include        "playlist.h"
#include        "resource.h"
#include        "lv.h"
#include        "draw.h"


const TCHAR * add_title     = TEXT("Add file(s) to playlist...");
const TCHAR * opn_title     = TEXT("Open playlist...");
const TCHAR * sav_title     = TEXT("Save playlist...");

const TCHAR * add_defext    = TEXT("mp3");
const TCHAR * opn_defext    = TEXT("mpl");

const TCHAR * add_filter    = TEXT("MP3 files (*.mp3)\0*.mp3\0")
                              TEXT("FLAC files (*.flac)\0*.flac\0")  
                              TEXT("All Files (*.*)\0*.*\0");

const TCHAR * opn_filter    = TEXT("Playlist files (*.mpl)\0*.mpl\0")
                              TEXT("All Files (*.*)\0*.*\0");


/*-@@+@@--------------------------------------------------------------------*/
//       Function: Playlist_Init 
/*--------------------------------------------------------------------------*/
//           Type: void 
//    Param.    1: HWND hList        : 
//    Param.    2: HINSTANCE hInst   : 
//    Param.    3: HIMAGELIST * pIml : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: init playlist
/*--------------------------------------------------------------------@@-@@-*/
void Playlist_Init ( HWND hList, HINSTANCE hInst, HIMAGELIST * pIml )
/*--------------------------------------------------------------------------*/
{
    COLDATA cd[2] =
    {
        { TEXT("Track"), 700, LVCFMT_LEFT },
        { TEXT("Length"), 45, LVCFMT_LEFT }
    };

    int     i;

    ListView_SetExtendedListViewStyle ( hList,
        LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP );

    if ( ( *pIml = InitImgList( hInst ) ) != NULL )
        ListView_SetImageList ( hList, *pIml, LVSIL_SMALL );

    for ( i = 0; i < 2; i++ )
        LVInsertColumn ( hList, i, cd[i].text, cd[i].flags, cd[i].width, -1);
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: Playlist_LoadFromCmdl 
/*--------------------------------------------------------------------------*/
//           Type: BOOL 
//    Param.    1: HWND hList       : 
//    Param.    2: TCHAR ** filelist: 
//    Param.    3: int files        : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: fetch files from commandline
/*--------------------------------------------------------------------@@-@@-*/
BOOL Playlist_LoadFromCmdl ( HWND hList, TCHAR ** filelist, int files )
/*--------------------------------------------------------------------------*/
{
    int         i, last_index;
    HSTREAM     stream;
    DWORD       time;
    QWORD       len;
    TCHAR       temp[32];

    if ( filelist == NULL )
        return FALSE;

    last_index = LVGetCount ( hList );

    for ( i = 1; i < files; i++, last_index++ )
    {
        LVInsertItem ( hList, last_index, 0, filelist[i] );
        #ifdef  UNICODE
        stream = BASS_StreamCreateFile ( FALSE, filelist[i], 0, 0, 
            BASS_STREAM_DECODE | BASS_UNICODE | BASS_ASYNCFILE );
        #else
        stream = BASS_StreamCreateFile ( FALSE, filelist[i], 0, 0, 
            BASS_STREAM_DECODE | BASS_ASYNCFILE );
        #endif
        if ( stream )
        {
            len = BASS_ChannelGetLength ( stream, BASS_POS_BYTE );
            time = (DWORD)BASS_ChannelBytes2Seconds ( stream, len );
            BASS_StreamFree ( stream );
            StringCchPrintf ( temp, ARRAYSIZE(temp), 
                TEXT("%02d:%02d"), time/60, time%60 );
            LVSetItemText ( hList, last_index, 1, temp );
        }
    }

    return TRUE;
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: Playlist_LoadFromMem 
/*--------------------------------------------------------------------------*/
//           Type: BOOL 
//    Param.    1: HWND hList           : 
//    Param.    2: const TCHAR * filebuf: 
//    Param.    3: DWORD dirlen         : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: fetch files from the lump returned by OpenFileDlg
/*--------------------------------------------------------------------@@-@@-*/
BOOL Playlist_LoadFromMem ( HWND hList, const TCHAR * filebuf, DWORD dirlen )
/*--------------------------------------------------------------------------*/
{
    DWORD       i, j, time;
    HSTREAM     stream;
    QWORD       len;
    TCHAR       file[MAX_PATH];
    TCHAR       curdir[MAX_PATH];
    BOOL        result = TRUE;

    i = dirlen;
    j = LVGetCount ( hList );

    StringCchCopyN ( curdir, ARRAYSIZE(curdir), filebuf, dirlen );

    while ( !curdir[dirlen] )
        dirlen--;
        
    if ( curdir[dirlen] != TEXT('\\') )
        StringCchCat ( curdir, ARRAYSIZE(curdir), TEXT("\\") );

    __try
    {
        do
        {
            StringCchCopy ( file, ARRAYSIZE(file), curdir );
            StringCchCat ( file, ARRAYSIZE(file), (TCHAR*)(&(filebuf[i])) );
            LVInsertItem ( hList, j, 0, file );
            #ifdef  UNICODE
            stream = BASS_StreamCreateFile ( FALSE, file, 0, 0, 
                BASS_STREAM_DECODE | BASS_UNICODE | BASS_ASYNCFILE );
            #else
            stream = BASS_StreamCreateFile ( FALSE, file, 0, 0, 
                BASS_STREAM_DECODE | BASS_ASYNCFILE );
            #endif
            if ( stream )
            {
                len = BASS_ChannelGetLength ( stream, BASS_POS_BYTE );
                time = (DWORD)BASS_ChannelBytes2Seconds ( stream, len );
                BASS_StreamFree ( stream );
                StringCchPrintf ( file, ARRAYSIZE(file), 
                    TEXT("%02d:%02d"), time/60, time%60 );
                LVSetItemText ( hList, j, 1, file );
            }
            i += lstrlen ( (TCHAR*)(&(filebuf[i])) );
            i++;
            j++;
        } 
        while ( filebuf[i] != 0 );
    }

    __except ( TRUE )
    {
        result = FALSE;
    }

    return result;
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: Playlist_Add 
/*--------------------------------------------------------------------------*/
//           Type: BOOL 
//    Param.    1: HWND hList      : 
//    Param.    2: HWND hParent    : 
//    Param.    3: HINSTANCE hInst : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: display OpenFile dialog, add one or more files to playlist
/*--------------------------------------------------------------------@@-@@-*/
BOOL Playlist_Add ( HWND hList, HWND hParent, HINSTANCE hInst )
/*--------------------------------------------------------------------------*/
{
    OPENFILENAME    ofn;
    DWORD           dlgstyle;
    TCHAR           * filebuf;
    HGLOBAL         hmem;

    
    hmem = GlobalAlloc ( GMEM_FIXED, ALLOC_SIZE );

    if ( !hmem )
        return FALSE;

    filebuf = (TCHAR *)hmem;

    RtlZeroMemory ( &ofn, sizeof ( ofn ) );
    RtlZeroMemory ( filebuf, ALLOC_SIZE );
    dlgstyle = OFN_EXPLORER|OFN_HIDEREADONLY|
        OFN_ALLOWMULTISELECT|OFN_FILEMUSTEXIST;
    
    ofn.lStructSize     = sizeof ( ofn );
    ofn.hInstance       = hInst;
    ofn.hwndOwner       = hParent;
    ofn.Flags           = dlgstyle;
    ofn.lpstrFilter     = add_filter;
    ofn.nFilterIndex    = 1;
    ofn.lpstrFile       = filebuf;
    ofn.nMaxFile        = ALLOC_SIZE / sizeof(TCHAR);
    ofn.lpstrTitle      = add_title;
    ofn.lpstrDefExt     = add_defext;

    if ( !GetOpenFileName ( &ofn ) )
    {
        GlobalFree ( hmem );
        return FALSE;
    }
    
    BeginDraw ( hList );

    if ( !Playlist_LoadFromMem ( hList, filebuf, ofn.nFileOffset ) )
    {
        GlobalFree ( hmem );
        EndDraw ( hList );
        return FALSE;        
    }

    EndDraw ( hList );
    GlobalFree ( hmem );

    return TRUE;
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: Playlist_SaveToFile 
/*--------------------------------------------------------------------------*/
//           Type: BOOL 
//    Param.    1: HWND hList         : 
//    Param.    2: const TCHAR * name : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: save the playlist
/*--------------------------------------------------------------------@@-@@-*/
BOOL Playlist_SaveToFile ( HWND hList, const TCHAR * name )
/*--------------------------------------------------------------------------*/
{
    DWORD   count, i;
    TCHAR   temp[32];
    TCHAR   filename[255];
    TCHAR   length[32];

    count = LVGetCount ( hList );
    if ( count == 0 ) { return FALSE; }

    StringCchPrintf ( temp, ARRAYSIZE(temp), TEXT("%lu"), count ); 
    WritePrivateProfileString ( TEXT("playlist"), 
        TEXT("itemcount"), temp, name );

    for ( i = 0; i < count; i++ )
    {
        LVGetItemText ( hList, i, 0, filename, ARRAYSIZE(filename) );
        LVGetItemText ( hList, i, 1, length, ARRAYSIZE(length) );
        StringCchPrintf ( temp, ARRAYSIZE(temp), TEXT("item%lu"), i ); 
        WritePrivateProfileString ( temp, TEXT("filename"), filename, name );
        WritePrivateProfileString ( temp, TEXT("length"), length, name );
    }

    return TRUE;
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: Playlist_LoadFromFile 
/*--------------------------------------------------------------------------*/
//           Type: BOOL 
//    Param.    1: HWND hList         : 
//    Param.    2: const TCHAR * name : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: load a saved playlist
/*--------------------------------------------------------------------@@-@@-*/
BOOL Playlist_LoadFromFile ( HWND hList, const TCHAR * name )
/*--------------------------------------------------------------------------*/
{
    DWORD   count, i, crtcount;
    TCHAR   temp[32];
    TCHAR   filename[255];
    TCHAR   length[32];

    count = GetPrivateProfileInt ( TEXT("playlist"), 
        TEXT("itemcount"), 0, name );

    if ( count == 0 )
        return FALSE;

    crtcount = LVGetCount ( hList );

    for ( i = 0; i < count; i++ )
    {
        StringCchPrintf ( temp, ARRAYSIZE(temp), TEXT("item%lu"), i );
        GetPrivateProfileString ( temp, TEXT("filename"), 
            TEXT("get_laid.mp3"), filename, ARRAYSIZE(filename), name );

        GetPrivateProfileString ( temp, TEXT("length"), 
            TEXT("69:69"), length, ARRAYSIZE(length), name );

        LVInsertItem ( hList, crtcount+i, 0, filename );
        LVSetItemText ( hList, crtcount+i, 1, length );
    }

    return TRUE;
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: Playlist_Open 
/*--------------------------------------------------------------------------*/
//           Type: BOOL 
//    Param.    1: HWND hList      : 
//    Param.    2: HWND hParent    : 
//    Param.    3: HINSTANCE hInst : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: display the OpenFile dlg and load a playlist
/*--------------------------------------------------------------------@@-@@-*/
BOOL Playlist_Open ( HWND hList, HWND hParent, HINSTANCE hInst )
/*--------------------------------------------------------------------------*/
{
    OPENFILENAME    ofn;
    DWORD           dlgstyle;
    TCHAR           filebuf[MAX_PATH];

    RtlZeroMemory ( &ofn, sizeof ( ofn ) );
    RtlZeroMemory ( filebuf, sizeof ( filebuf ) );
    dlgstyle = OFN_EXPLORER|OFN_HIDEREADONLY|OFN_FILEMUSTEXIST;
    
    ofn.lStructSize     = sizeof ( ofn );
    ofn.hInstance       = hInst;
    ofn.hwndOwner       = hParent;
    ofn.Flags           = dlgstyle;
    ofn.lpstrFilter     = opn_filter;
    ofn.nFilterIndex    = 1;
    ofn.lpstrFile       = filebuf;
    ofn.nMaxFile        = ARRAYSIZE ( filebuf );
    ofn.lpstrTitle      = opn_title;
    ofn.lpstrDefExt     = opn_defext;

    if ( !GetOpenFileName ( &ofn ) )
        return FALSE;

    BeginDraw ( hList );

    if ( !Playlist_LoadFromFile ( hList, filebuf ) )
    {
        EndDraw ( hList );
        return FALSE;        
    }

    EndDraw ( hList );

    return TRUE;
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: Playlist_Save 
/*--------------------------------------------------------------------------*/
//           Type: BOOL 
//    Param.    1: HWND hList      : 
//    Param.    2: HWND hParent    : 
//    Param.    3: HINSTANCE hInst : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: display FileSave dlg and save aplaylist
/*--------------------------------------------------------------------@@-@@-*/
BOOL Playlist_Save ( HWND hList, HWND hParent, HINSTANCE hInst )
/*--------------------------------------------------------------------------*/
{
    OPENFILENAME    ofn;
    DWORD           dlgstyle;
    TCHAR           filebuf[MAX_PATH];

    RtlZeroMemory ( &ofn, sizeof ( ofn ) );
    RtlZeroMemory ( filebuf, sizeof ( filebuf ) );
    dlgstyle = OFN_EXPLORER|OFN_PATHMUSTEXIST|
        OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT;
    
    ofn.lStructSize     = sizeof ( ofn );
    ofn.hInstance       = hInst;
    ofn.hwndOwner       = hParent;
    ofn.Flags           = dlgstyle;
    ofn.lpstrFilter     = opn_filter;
    ofn.nFilterIndex    = 1;
    ofn.lpstrFile       = filebuf;
    ofn.nMaxFile        = ARRAYSIZE ( filebuf );
    ofn.lpstrTitle      = sav_title;
    ofn.lpstrDefExt     = opn_defext;

    if ( !GetSaveFileName ( &ofn ) )
        return FALSE;

    return Playlist_SaveToFile ( hList, filebuf );
}
