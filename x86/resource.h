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

#ifndef _RES_H
#define _RES_H

#define IDD_MAIN                        101

#define IDR_MAINMNU                     105
#define IDR_ACC                         106

#define IDI_MAINICON                    500
#define IDI_LISTICON                    501
#define IDI_PLAYICON                    503

#define IDC_PLAYLIST                    1000
#define IDC_GRVOL                       1008
#define IDC_GRPOS                       1010
#define IDC_GRDEV                       4001
#define IDC_POS                         1009
#define IDC_VOLTRAK                     1011
#define IDC_VOLUME                      1012
#define IDC_TIME                        1013
#define IDC_SPEC                        1017
#define IDC_LEVELS                      1666
#define IDC_DEVLIST                     4002

#define IDM_QUIT                        40002
#define IDM_ADD                         40003
#define IDM_DEL                         40004
#define IDM_CLEAR                       40005
#define IDM_PLAY                        40006
#define IDM_ABOUT                       40007
#define IDM_STOP                        40008
#define IDM_PAUSE                       40009
#define IDM_SELALL                      40010
#define IDM_NEXT                        40011
#define IDM_REW                         40012
#define IDM_FFW                         40013
#define IDM_KEYHLP                      40014
#define IDM_OPEN                        40015
#define IDM_SAVE                        40016

#endif
