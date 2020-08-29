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

#ifndef _MISC_H
#define _MISC_H

typedef enum {
    ERR_INIT_ACC_TABLE,                          //0
    ERR_APP_RUNNING,                             //1
    ERR_BASS_VER,                                //2
    ERR_BASS_INIT,                               //3
    ERR_DLG_CREATE,                              //4
    ERR_DEV_LIST,                                //5
    ERR_SCOPE_INIT,                              //6
    ERR_DIB_INIT,                                //7
    ERR_SLIDE_INIT,                              //8
    ERR_SUBCLASS,                                //9
    ERR_TRAY_INIT,                               //10
    ERR_TIMER_INIT,                              //11
    ERR_CHG_PBDEV,                               //12
    ERR_PB_START,                                //13
    ERR_SUCCESS
} INIT_STATUS;


extern const TCHAR  * app_title;
extern const TCHAR  * about_txt;
extern const TCHAR  * hlp_txt;
extern const TCHAR  * g_err_messages[];

extern const TCHAR         * Extract_path               ( const TCHAR * src, BOOL last_bslash );
extern int                 ShowMessage                  ( HWND howner, const TCHAR * message, DWORD style );
extern BOOL                Load_BASS_Plugins            ( void );
extern void                BASS_Error                   ( HWND howner, const TCHAR * message );
extern BOOL                IsThereAnotherInstance       ( const TCHAR * classname );
extern TCHAR               ** FILE_CommandLineToArgv    ( TCHAR * CmdLine, int * _argc );

#endif
