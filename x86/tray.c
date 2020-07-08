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
#include        <shellapi.h>
#include        "tray.h"

BOOL Tray_Add ( HWND hwnd, UINT uID, HICON hicon, const TCHAR * lpszTip )
/*****************************************************************************************************************/
/* add a tray icon to systray                                                                                    */
{
    NOTIFYICONDATA  ni;

    ni.cbSize           = sizeof ( NOTIFYICONDATA );
    ni.hWnd             = hwnd;
    ni.uID              = uID;
    ni.uFlags           = NIF_MESSAGE|NIF_ICON|NIF_TIP;
    ni.uCallbackMessage = WM_TRAY;
    ni.hIcon            = hicon;
    if ( lpszTip )
        lstrcpyn( ni.szTip, lpszTip, sizeof ( ni.szTip ) );
    else
        ni.szTip[0] = '\0';

    return Shell_NotifyIcon ( NIM_ADD, &ni );
}

BOOL Tray_Remove ( HWND hwnd, UINT uID )
/*****************************************************************************************************************/
/* remove it                                                                                                     */
{
    NOTIFYICONDATA  ni;

    ni.cbSize   = sizeof ( NOTIFYICONDATA );
    ni.hWnd     = hwnd;
    ni.uID      = uID;

    return Shell_NotifyIcon ( NIM_DELETE, &ni );
}

