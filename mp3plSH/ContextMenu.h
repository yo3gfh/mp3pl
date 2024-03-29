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
// contextmenu.h

#ifndef _CONTEXTMENU_H_
#define _CONTEXTMENU_H_

// IContextMenu methods
STDMETHODIMP                CContextMenuExt_QueryInterface     ( IContextMenu * this,REFIID riid, LPVOID *ppvOut );
/*STDMETHODIMP_(ULONG)*/
ULONG STDMETHODCALLTYPE     CContextMenuExt_AddRef             ( IContextMenu * this );
/*STDMETHODIMP_(ULONG)*/
ULONG STDMETHODCALLTYPE     CContextMenuExt_Release            ( IContextMenu * this );
STDMETHODIMP                CContextMenuExt_QueryContextMenu   ( IContextMenu * this, HMENU, UINT, UINT, UINT, UINT );
STDMETHODIMP                CContextMenuExt_InvokeCommand      ( IContextMenu * this, LPCMINVOKECOMMANDINFO );
STDMETHODIMP                CContextMenuExt_GetCommandString   ( IContextMenu * this, UINT_PTR, UINT, UINT *, LPSTR, UINT );

// IContextMenu constructor
IContextMenu        * CContextMenuExt_Create                    ( void );

// This struct acts somewhat like a pseudo class in that you have
// variables accociated with an instance of this interface.
typedef struct _ContextMenuExtStruct
{
    // Two interfaces
    IContextMenu            cm;
    IShellExtInit           si;

    // second part of the struct for the variables
    LPTSTR                  m_pszSource;
    // let's put this so CContextMenuExt_InvokeCommand
    // will know how many bytes to alloc for the cmdline
    // buffer
    size_t                  m_pszSource_bsize;
    ULONG                   m_ulRef;
} ContextMenuExtStruct;

// IShellExtInit methods
STDMETHODIMP                CShellInitExt_QueryInterface       ( IShellExtInit * this, REFIID riid, LPVOID* ppvObject );
/*STDMETHODIMP_(ULONG)*/
ULONG STDMETHODCALLTYPE     CShellInitExt_AddRef               ( IShellExtInit * this );
/*STDMETHODIMP_(ULONG)*/
ULONG STDMETHODCALLTYPE     CShellInitExt_Release              ( IShellExtInit * this );
STDMETHODIMP                CShellInitExt_Initialize           ( IShellExtInit * this, LPCITEMIDLIST pidlFolder, LPDATAOBJECT lpdobj, HKEY hKeyProgID );

#endif // _CONTEXTMENU_H_


