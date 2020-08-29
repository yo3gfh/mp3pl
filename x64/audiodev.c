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
#include        <strsafe.h>
#include        <tchar.h>
#include        <stdlib.h>
#include        <bass.h>

#pragma warn(disable: 2231 2030 2260) //enum not used in switch, = used in conditional

BOOL AudioDevListInit ( HWND hList )
/**************************************************************************************************************/
/* init a listbox with the active audio devices                                                               */
{
    TCHAR               buf[256];
    BASS_DEVICEINFO     devinfo;
    int                 i, enabled;

    enabled = 0;
    for ( i = 1; BASS_GetDeviceInfo ( i, &devinfo ); i++ )
    {
        if ( devinfo.flags & BASS_DEVICE_ENABLED )
        {
            // a small hack, because BASS_SetConfig ( BASS_CONFIG_UNICODE, TRUE ) does not seem to work correctly
            // so we let it be ANSI, and cvt. to WCHAR when doing a UNICODE build
#ifdef UNICODE
            int   wchars;
            TCHAR wname[256];
            wchars = MultiByteToWideChar ( CP_ACP, 0, devinfo.name, -1, NULL, 0 );
            MultiByteToWideChar ( CP_ACP, 0, devinfo.name, -1, wname, wchars );
            //wsprintf ( buf, TEXT("%d %s"), i, wname );
            StringCchPrintf ( buf, ARRAYSIZE(buf), L"%d %ls", i, wname );
#else
            //wsprintf ( buf, TEXT("%d %s"), i, devinfo.name );
            StringCchPrintf ( buf, ARRAYSIZE(buf), "%d %s", i, wname );
#endif
            if ( devinfo.flags & BASS_DEVICE_DEFAULT )
            {
                //lstrcat ( buf, TEXT(" (default)") );
                StringCchCat ( buf, ARRAYSIZE(buf), TEXT(" (default)") );
                SendMessage ( hList, LB_ADDSTRING, 0, (LPARAM)buf );
                SendMessage ( hList, LB_SETCURSEL, enabled, 0 );
            }
            else
            {
                SendMessage ( hList, LB_ADDSTRING, 0, (LPARAM)buf );
            }
            enabled++;
        }
    }

    return TRUE;
}

BOOL AudioDevChange ( HWND hList )
/**************************************************************************************************************/
/* switch to another playback device                                                                          */
{
    int     sel;
    DWORD   sel_dev, old_dev;
    TCHAR   buf[64];
    
    sel = SendMessage ( hList, LB_GETCURSEL, 0, 0 );
    
    if ( sel == LB_ERR ) { return FALSE; }

    old_dev = BASS_GetDevice();

    if ( old_dev == ( DWORD )-1 ) { return FALSE; }

    SendMessage ( hList, LB_GETTEXT, ( WPARAM )sel, ( LPARAM )buf );

    sel_dev = _ttoi ( buf );
    BASS_Free();

    if ( !BASS_Init ( sel_dev, 44100, 0, 0, NULL ) )
    {
        BASS_Init ( old_dev, 44100, 0, 0, NULL );
        return FALSE;
    }

    return TRUE;
}

