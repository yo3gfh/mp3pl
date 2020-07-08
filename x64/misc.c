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
        
    Please note that this is cca. 20 yesrs old code, not particullary something
    to write home about :-))
    
    It's taylored to my own needs, modify it to suit your own. I'm not a professional programmer,
    so this isn't the best code you'll find on the web, you have been warned :-))

    All the bugs are guaranteed to be genuine, and are exclusively mine =)
*/

#include        <windows.h>
#include        <bass.h>
#include        "resource.h"

const TCHAR * app_title     = TEXT("Lil' MP3 player");

const TCHAR * about_txt     = TEXT("Lil' MP3 player")
                              TEXT("\t\tCopyright (c) 2003-2020, Adrian Petrila\n")
                              TEXT("BASS sound system v%d.%d\tCopyright (c) 1999-2019, Ian Luck\n");

const TCHAR * hlp_txt       = TEXT("Space, Enter, dblclick\t\tPlay/Resume\n")
                              TEXT("Esc\t\t\tStop playback\n")
                              TEXT("Ctrl+P\t\t\tPause/Resume\n")
                              TEXT("Ctrl+B\t\t\tNext track\n")
                              TEXT("Ctrl+Left Arrow\t\tRewind\n")
                              TEXT("Ctrl+Right Arrow\t\tFast-forward\n\n")
                              TEXT("Ctrl+O\t\t\tOpen playlist\n")
                              TEXT("Ctrl+Ins\t\t\tAdd files to playlist\n")
                              TEXT("Ctrl+S\t\t\tSave playlist\n")
                              TEXT("Ctrl+A\t\t\tSelect all\n")
                              TEXT("Shift+Del\t\t\tClear all\n")
                              TEXT("Del\t\t\tDelete selected");

const TCHAR * g_err_messages[] = { 
    TEXT("Can't load accelerator table!"),                              //0
    TEXT("Program already up and running :)"),                          //1     
    TEXT("Wrong bass.dll version (%d.%d) - expecting %d.%d"),           //2
    TEXT("Failed to initialize sound system!"),                         //3
    TEXT("Can't create main window!n"),                                 //4
    TEXT("Error initializing device list!"),                            //5
    TEXT("Error initializing volume/spectrum display!"),                //6
    TEXT("Error creating DIB for volume/spectrum display!"),            //7
    TEXT("Error initializing volume/position sliders!"),                //8
    TEXT("Error subclassing controls!"),                                //9
    TEXT("Error adding icon to systray!"),                              //10
    TEXT("Error initializing multimedia timer!"),                       //11
    TEXT("Unable to switch to this playback device!"),                  //12
    TEXT("Unable to start playback!")                                   //13
};

const TCHAR * Extract_path ( const TCHAR * src, BOOL last_bslash )
/*****************************************************************************************************************/
/* make a path from a filename                                                                                   */
{
    DWORD           idx;
    static TCHAR    temp[MAX_PATH];

    if ( src == NULL ) { return NULL; }
    idx = lstrlen ( src )-1;
    if ( idx >= MAX_PATH ) { return NULL; }
    while ( ( src[idx] != TEXT('\\') ) && ( idx != 0 ) )
        idx--;
    if ( idx == 0 ) { return NULL; }
    if ( last_bslash ) { idx++; }
    lstrcpyn ( temp, src, idx+1 );
    return temp;
}

int ShowMessage ( HWND howner, const TCHAR * message, DWORD style )
/*****************************************************************************************************************/
/* custom messagebox                                                                                             */
{
    MSGBOXPARAMS    mp;

    mp.cbSize               = sizeof ( mp );
    mp.dwStyle              = MB_USERICON | style;
    mp.hInstance            = GetModuleHandle ( NULL );
    mp.lpszIcon             = MAKEINTRESOURCE ( IDI_MAINICON );
    mp.hwndOwner            = howner;
    mp.lpfnMsgBoxCallback   = NULL;
    mp.lpszCaption          = app_title;
    mp.lpszText             = message;
    mp.dwContextHelpId      = 0;
    mp.dwLanguageId         = MAKELANGID ( LANG_NEUTRAL, SUBLANG_DEFAULT );
    return MessageBoxIndirect ( &mp );
}

BOOL Load_BASS_Plugins ( void )
/*****************************************************************************************************************/
/* load plugin dlls                                                                                              */
{
    WIN32_FIND_DATA     fd;
    HANDLE              fh;
    TCHAR               path[MAX_PATH], temp[MAX_PATH];
    #ifdef UNICODE
    char                plug_name[MAX_PATH];
    #endif

    GetModuleFileName ( 0, path, sizeof ( path ) );
    lstrcpyn ( temp, Extract_path ( path, TRUE ), MAX_PATH );
    lstrcat ( temp, TEXT("bass*.dll") );

    fh = FindFirstFile ( temp, &fd );

    if ( fh == INVALID_HANDLE_VALUE ) { return FALSE; }

    do
    {
        #ifdef UNICODE
        WideCharToMultiByte ( CP_ACP, 0, fd.cFileName, -1, plug_name, sizeof ( plug_name ), NULL, NULL );
        BASS_PluginLoad ( plug_name, 0 );
        #else
        BASS_PluginLoad ( fd.cFileName, 0 );
        #endif    
    }
    while ( FindNextFile ( fh, &fd ) );

    FindClose ( fh );
    return TRUE;
}

void BASS_Error ( HWND howner, const TCHAR * message )
/*****************************************************************************************************************/
/* fetch last BASS error                                                                                         */
{
    TCHAR   temp[128];

    wsprintf ( temp, TEXT("%s\nError code: %d"), message, BASS_ErrorGetCode() );
    ShowMessage ( howner, temp, MB_OK );
}
