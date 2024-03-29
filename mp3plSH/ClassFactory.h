/*
    Mp3plSH, support (in)utility for Mp3pl player
    -----------------------------------------------
    Copyright (c) 2002-2020 Adrian Petrila, YO3GFH
    
    MS Explorer context menu shell extension to open
    files in the Mp3pl audio player.

    Original example from forum.pellesc.de

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

        - select one or more files to open with mp3pl. Amazing =)

*/

// classfactory.h

#ifndef _CLASSFACTORY_H_
#define _CLASSFACTORY_H_

// IClassFactory methods
STDMETHODIMP                CClassFactory_QueryInterface   ( IClassFactory *this,REFIID riid, LPVOID *ppvOut );
/*STDMETHODIMP_(ULONG)*/
ULONG STDMETHODCALLTYPE     CClassFactory_AddRef           ( IClassFactory *this );
/*STDMETHODIMP_(ULONG)*/
ULONG STDMETHODCALLTYPE     CClassFactory_Release          ( IClassFactory *this );
STDMETHODIMP                CClassFactory_CreateInstance   ( IClassFactory *, LPUNKNOWN, REFIID, LPVOID * );
STDMETHODIMP                CClassFactory_LockServer       ( IClassFactory *this, BOOL );

// IClassFactory constructor
IClassFactory               * CClassFactory_Create         ( void );

// This struct acts somewhat like a pseudo class in that you have
// variables accociated with an instance of this interface.
typedef struct _ClassFactoryStruct
{
    // first part of the struct for the vtable must be fc
    IClassFactory           fc;

    // second part of tye struct for the variables
    HINSTANCE               m_hInstance;            // Instance handle for this DLL
    ULONG                   m_ulRef;                // Object reference count
} ClassFactoryStruct;

#endif //CLASSFACTORY

