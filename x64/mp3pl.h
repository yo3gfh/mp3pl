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
        
    Please note that this is cca. 20 years old code, not particullary 
    something to write home about :-))
    
    It's taylored to my own needs, modify it to suit your own. 
    I'm not a professional programmer, so this isn't the best code you'll find
    on the web, you have been warned :-))

    All the bugs are guaranteed to be genuine, and are exclusively mine =)
*/

#ifndef _MP3PL_H
#define _MP3PL_H

// timer update interval; greater values,
// jerkier volume & spectrum - smaller values,
// possible blue screen savers :))))

#define     UPDATE_INTERVAL 25
                             
// position trackbar maximum range            
#define     MAX_RANGE       0x4000          
#define     POS_INCREMENT   500

// DIB header size
#define     BMP_STUFF_SIZE  ((sizeof(BITMAPINFOHEADER)) + \
                            (256 * (sizeof(RGBQUAD)))) 

#define     FFTS            1024
#define     APP_CLASSNAME   TEXT("MPL_CLASS_666")
#define     COPYDATA_MAGIC  0x0666DEAD

#endif
