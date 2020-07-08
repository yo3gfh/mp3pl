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

#ifndef _PLAYLIST_H
#define _PLAYLIST_H
#include <windows.h>

#define     ALLOC_SIZE      0x0000FFFF      // size for filename buffer in open dialog;
                                            // please tell me why in the name of jesus
                                            // this value cannot be greater than 64K :( 

typedef struct
{
    TCHAR   * text;
    int     width;
    int     flags;
}   COLDATA;

void    Playlist_Init           ( HWND hList, HINSTANCE hInst, HIMAGELIST * pIml );
BOOL    Playlist_LoadFromMem    ( HWND hList, const TCHAR * filebuf, DWORD dirlen );
BOOL    Playlist_Add            ( HWND hList, HWND hParent, HINSTANCE hInst );
BOOL    Playlist_Save           ( HWND hList, HWND hParent, HINSTANCE hInst );
BOOL    Playlist_Open           ( HWND hList, HWND hParent, HINSTANCE hInst );
BOOL    Playlist_LoadFromFile   ( HWND hList, const TCHAR * name );
BOOL    Playlist_SaveToFile     ( HWND hList, const TCHAR * name );

#endif