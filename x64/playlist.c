/*
    MP3PL, a small mp3 and flac player 
    -------------------------------------------------------------------
    Copyright (c) 2002-2020 Adrian Petrila, YO3GFH
    Uses the BASS sound system by Ian Luck (http://www.un4seen.com/)
    Inspired by the examples included with the bass library.
    
    This was my "most ambitious" project at the time, right before being
    drafted in the army. Dugged out recently and dusted off to compile with
    Pelle's C compiler.
    
                                * * *
                                
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

                                * * *

    Features
    ---------
    
        - play mp3 and flac files
        - save and load playlists
        - volume, spectrum analyzer and waveform (scope) display
        
    Please note that this is cca. 20 years old code, not particullary something
    to write home about :-))
    
    It's taylored to my own needs, modify it to suit your own. I'm not a professional programmer,
    so this isn't the best code you'll find on the web, you have been warned :-))

    All the bugs are guaranteed to be genuine, and are exclusively mine =)
*/

#include        <windows.h>
#include        <commctrl.h>
#include        <bass.h>

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


void Playlist_Init ( HWND hList, HINSTANCE hInst, HIMAGELIST * pIml )
/*****************************************************************************************************************/
/* init playlist                                                                                                 */
{
    COLDATA cd[2] =
    {
        { TEXT("Track"), 700, LVCFMT_LEFT },
        { TEXT("Length"), 45, LVCFMT_LEFT }
    };

    int     i;

    ListView_SetExtendedListViewStyle ( hList, LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP );

    if ( ( *pIml = InitImgList( hInst ) ) != NULL )
        ListView_SetImageList ( hList, *pIml, LVSIL_SMALL );

    for ( i = 0; i < 2; i++ )
        LVInsertColumn ( hList, i, cd[i].text, cd[i].flags, cd[i].width, -1 );
}

BOOL Playlist_LoadFromMem ( HWND hList, const TCHAR * filebuf, DWORD dirlen )
/*****************************************************************************************************************/
/* fetch files from the lump return by OpenFileDlg                                                               */
{
    DWORD       i, j, time;
    HSTREAM     stream;
    QWORD       len;
    TCHAR       file[MAX_PATH];
    TCHAR       curdir[MAX_PATH];
    BOOL        result = TRUE;

    i = dirlen;
    j = LVGetCount ( hList );

    lstrcpyn ( curdir, filebuf, dirlen+1 );

    while ( !curdir[dirlen] )
        dirlen--;
        
    if ( curdir[dirlen] != TEXT('\\') )
        lstrcat ( curdir, TEXT("\\") );

    __try
    {
        do
        {
            lstrcpy ( file, curdir );
            lstrcat ( file, (TCHAR*)(&(filebuf[i])) );
            LVInsertItem ( hList, j, 0, file );
            #ifdef  UNICODE
            stream = BASS_StreamCreateFile ( FALSE, file, 0, 0, BASS_STREAM_DECODE | BASS_UNICODE | BASS_ASYNCFILE );
            #else
            stream = BASS_StreamCreateFile ( FALSE, file, 0, 0, BASS_STREAM_DECODE | BASS_ASYNCFILE );
            #endif
            if ( stream )
            {
                len = BASS_ChannelGetLength ( stream, BASS_POS_BYTE );
                time = (DWORD)BASS_ChannelBytes2Seconds ( stream, len );
                BASS_StreamFree ( stream );
                wsprintf ( file, TEXT("%02d:%02d"), time/60, time%60 );
                LVSetItemText ( hList, j, 1, file );
            }
            i += lstrlen ( (TCHAR*)(&(filebuf[i])) );
            i++;
            j++;
        } while ( filebuf[i] != 0 );
    }

    __except ( TRUE )
    {
        result = FALSE;
    }

    return result;
}

BOOL Playlist_Add ( HWND hList, HWND hParent, HINSTANCE hInst )
/*****************************************************************************************************************/
/* display OpenFile dialog, add one or more files to playlist                                                    */
{
    OPENFILENAME    ofn;
    DWORD           dlgstyle;
    TCHAR           * filebuf;
    HGLOBAL         hmem;

    
    hmem = GlobalAlloc ( GMEM_FIXED, ALLOC_SIZE );
    if ( !hmem ) { return FALSE; }
    filebuf = (TCHAR *)hmem;

    RtlZeroMemory ( &ofn, sizeof ( ofn ) );
    RtlZeroMemory ( filebuf, ALLOC_SIZE );
    dlgstyle = OFN_EXPLORER|OFN_HIDEREADONLY|OFN_ALLOWMULTISELECT|OFN_FILEMUSTEXIST;
    
    ofn.lStructSize     = sizeof ( ofn );
    ofn.hInstance       = hInst;
    ofn.hwndOwner       = hParent;
    ofn.Flags           = dlgstyle;
    ofn.lpstrFilter     = add_filter;
    ofn.nFilterIndex    = 1;
    ofn.lpstrFile       = filebuf;
    ofn.nMaxFile        = ALLOC_SIZE;
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

BOOL Playlist_SaveToFile ( HWND hList, const TCHAR * name )
/*****************************************************************************************************************/
/* save the playlist                                                                                             */
{
    DWORD   count, i;
    TCHAR   temp[32];
    TCHAR   filename[255];
    TCHAR   length[32];

    count = LVGetCount ( hList );
    if ( count == 0 ) { return FALSE; }

    wsprintf ( temp, TEXT("%lu"), count );
    WritePrivateProfileString ( TEXT("playlist"), TEXT("itemcount"), temp, name );

    for ( i = 0; i < count; i++ )
    {
        LVGetItemText ( hList, i, 0, filename, sizeof(filename) );
        LVGetItemText ( hList, i, 1, length, sizeof(length) );
        wsprintf ( temp, TEXT("item%lu"), i );
        WritePrivateProfileString ( temp, TEXT("filename"), filename, name );
        WritePrivateProfileString ( temp, TEXT("length"), length, name );
    }
    return TRUE;
}

BOOL Playlist_LoadFromFile ( HWND hList, const TCHAR * name )
/*****************************************************************************************************************/
/* load a saved playlist                                                                                         */
{
    DWORD   count, i, crtcount;
    TCHAR   temp[32];
    TCHAR   filename[255];
    TCHAR   length[32];

    count = GetPrivateProfileInt ( TEXT("playlist"), TEXT("itemcount"), 0, name );
    if ( count == 0 ) { return FALSE; }

    crtcount = LVGetCount ( hList );

    for ( i = 0; i < count; i++ )
    {
        wsprintf ( temp, TEXT("item%lu"), i );
        GetPrivateProfileString ( temp, TEXT("filename"), TEXT("get_laid.mp3"), filename, sizeof(filename), name );
        GetPrivateProfileString ( temp, TEXT("length"), TEXT("69:69"), length, sizeof(length), name );
        LVInsertItem ( hList, crtcount+i, 0, filename );
        LVSetItemText ( hList, crtcount+i, 1, length );
    }
    return TRUE;
}

BOOL Playlist_Open ( HWND hList, HWND hParent, HINSTANCE hInst )
/*****************************************************************************************************************/
/* display the OpenFile dlg and load a playlist                                                                  */
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
    ofn.nMaxFile        = sizeof ( filebuf );
    ofn.lpstrTitle      = opn_title;
    ofn.lpstrDefExt     = opn_defext;

    if ( !GetOpenFileName ( &ofn ) ) { return FALSE; }

    BeginDraw ( hList );
    if ( !Playlist_LoadFromFile ( hList, filebuf ) )
    {
        EndDraw ( hList );
        return FALSE;        
    }
    EndDraw ( hList );
    return TRUE;
}

BOOL Playlist_Save ( HWND hList, HWND hParent, HINSTANCE hInst )
/*****************************************************************************************************************/
/* display FileSave dlg and save aplaylist                                                                       */
{
    OPENFILENAME    ofn;
    DWORD           dlgstyle;
    TCHAR           filebuf[MAX_PATH];

    RtlZeroMemory ( &ofn, sizeof ( ofn ) );
    RtlZeroMemory ( filebuf, sizeof ( filebuf ) );
    dlgstyle = OFN_EXPLORER|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT;
    
    ofn.lStructSize     = sizeof ( ofn );
    ofn.hInstance       = hInst;
    ofn.hwndOwner       = hParent;
    ofn.Flags           = dlgstyle;
    ofn.lpstrFilter     = opn_filter;
    ofn.nFilterIndex    = 1;
    ofn.lpstrFile       = filebuf;
    ofn.nMaxFile        = sizeof ( filebuf );
    ofn.lpstrTitle      = sav_title;
    ofn.lpstrDefExt     = opn_defext;

    if ( !GetSaveFileName ( &ofn ) ) { return FALSE; }

    return Playlist_SaveToFile ( hList, filebuf );
}
