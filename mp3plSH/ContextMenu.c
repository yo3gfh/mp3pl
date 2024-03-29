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

// contextmenu.c

/*--------------------------------------------------------------
    The IShellExtInit interface is incorporated into the 
    IContextMenu interface
--------------------------------------------------------------*/

#include "mp3plSH.h"
#include <windows.h>
#include <shlobj.h>
#include <strsafe.h>
#include <stdarg.h>
#include <stdio.h>
#include "ContextMenu.h"

#define SEVERITY_SUCCESS    0
#define IDM_OWMPL            0x0002
#define IDB_OWMPL            8001
#define ALLOC_MULT          (sizeof(TCHAR)+2)                   // sizeof TCHAR would do, but let's be generous
#define MAX_CHARS           (0x00010000 / sizeof(TCHAR))        // OTOH, let's put a limit on how much filenames buf we process 
#define COPYDATA_MAGIC      0x0666DEAD

#pragma warn(disable: 2231 2030 2260)                           //enum not used in switch, = used in conditional, etc..

const TCHAR * FILE_Extract_path ( const TCHAR * src, BOOL last_bslash );

// Keep a count of DLL references
extern UINT g_uiRefThisDll;
extern HINSTANCE g_hInstance;
HBITMAP g_hMenuBmp;

// The virtual table of functions for IContextMenu interface
IContextMenuVtbl icontextMenuVtbl = {
    CContextMenuExt_QueryInterface,
    CContextMenuExt_AddRef,
    CContextMenuExt_Release,
    CContextMenuExt_QueryContextMenu,
    CContextMenuExt_InvokeCommand,
    CContextMenuExt_GetCommandString
};

// The virtual table of functions for IShellExtInit interface
IShellExtInitVtbl ishellInitExtVtbl = {
    CShellInitExt_QueryInterface,
    CShellInitExt_AddRef,
    CShellInitExt_Release,
    CShellInitExt_Initialize
};

//--------------------------------------------------------------
// IContextMenu constructor
//--------------------------------------------------------------
IContextMenu * CContextMenuExt_Create ( void )
/*****************************************************************************************************************/
{
    // Create the ContextMenuExtStruct that will contain interfaces and vars
    ContextMenuExtStruct * pCM = malloc ( sizeof(ContextMenuExtStruct) );
    if(!pCM)
        return NULL;

    // Point to the IContextMenu and IShellExtInit Vtbl's
    pCM->cm.lpVtbl = &icontextMenuVtbl;
    pCM->si.lpVtbl = &ishellInitExtVtbl;

    // increment the class reference count
    pCM->m_ulRef = 1;
    pCM->m_pszSource = NULL;

    g_uiRefThisDll++;

    // Return the IContextMenu virtual table
    return &pCM->cm;
}

//===============================================
// IContextMenu interface routines
//===============================================
STDMETHODIMP CContextMenuExt_QueryInterface ( IContextMenu * this, REFIID riid, LPVOID *ppv )
/*****************************************************************************************************************/
{
    // The address of the struct is the same as the address
    // of the IContextMenu Virtual table. 
    ContextMenuExtStruct * pCM = (ContextMenuExtStruct*)this;
    
    if ( IsEqualIID ( riid, &IID_IUnknown ) || IsEqualIID ( riid, &IID_IContextMenu ) )
    {
        *ppv = this;
        pCM->m_ulRef++;
        return S_OK;
    }
    else if ( IsEqualIID ( riid, &IID_IShellExtInit ) )
    {
        // Give the IShellInitExt interface
        *ppv = &pCM->si;
        pCM->m_ulRef++;
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}

/*STDMETHODIMP_(ULONG)*/ // this breaks the Pelle function browser 
ULONG STDMETHODCALLTYPE CContextMenuExt_AddRef ( IContextMenu * this )
/*****************************************************************************************************************/
{
    ContextMenuExtStruct * pCM = (ContextMenuExtStruct*)this;
    return ++pCM->m_ulRef;
}

/*STDMETHODIMP_(ULONG)*/
ULONG STDMETHODCALLTYPE CContextMenuExt_Release ( IContextMenu * this )
/*****************************************************************************************************************/
{
    ContextMenuExtStruct * pCM = (ContextMenuExtStruct*)this;

    if ( --pCM->m_ulRef == 0 )
    {
        free ( pCM->m_pszSource );
        free ( this );
        DeleteObject ( g_hMenuBmp );
        g_uiRefThisDll--;
        return 0;
    }

    return pCM->m_ulRef;
}

STDMETHODIMP CContextMenuExt_GetCommandString ( IContextMenu * this, UINT_PTR idCmd, UINT uFlags, UINT *pwReserved, LPSTR pszName, UINT cchMax )
/*****************************************************************************************************************/
{
    HRESULT hr = S_OK;

    switch ( uFlags )
    {
        case GCS_HELPTEXTA:
        switch ( idCmd )
        {
            case IDM_OWMPL:
            StringCchCopyA ( (LPSTR)pszName, cchMax, "Add to Mp3pl playlist" );
            hr = S_OK;
            break;
            
            default:
            hr = E_NOTIMPL;
            break;
        }
        break;

        case GCS_HELPTEXTW:
        switch ( idCmd )
        {
            case IDM_OWMPL:
            StringCchCopyW ( (LPWSTR)pszName, cchMax, L"Add to Mp3pl playlist" );
            hr = S_OK;
            break;

            default:
            hr = E_NOTIMPL;
            break;
        }
        break;
    }
    return hr;
}

STDMETHODIMP CContextMenuExt_QueryContextMenu ( IContextMenu * this, HMENU hMenu, UINT uiIndexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags )
/*****************************************************************************************************************/
{
    HINSTANCE        hinst;
    HBITMAP          hbmp;
    UINT             ix;
    ULONG            sev;
    MENUITEMINFO     mi;

    hinst            = g_hInstance;
    hbmp             = LoadBitmap ( hinst, MAKEINTRESOURCE(IDB_OWMPL) );
    g_hMenuBmp       = hbmp;

    ix               = uiIndexMenu;
    sev              = SEVERITY_SUCCESS;

    RtlZeroMemory ( &mi, sizeof (MENUITEMINFO) );
    mi.cbSize        = sizeof (MENUITEMINFO);
    mi.fMask         = MIIM_STRING | MIIM_STATE | MIIM_BITMAP | MIIM_FTYPE | MIIM_ID | MIIM_CHECKMARKS;
    mi.dwTypeData    = TEXT("Add to Mp3pl playlist");
    mi.wID           = idCmdFirst + IDM_OWMPL;
    mi.fType         = MFT_STRING;
    mi.fState        = MFS_ENABLED;
    mi.hbmpItem      = hbmp;
    mi.hbmpChecked   = hbmp;
    mi.hbmpUnchecked = hbmp;

    __try
    {
        InsertMenu(hMenu, ix++, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);
        InsertMenuItem ( hMenu, ix++, TRUE, &mi );
        InsertMenu(hMenu, ix++, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);
    }
    __except ( EXCEPTION_EXECUTE_HANDLER )
    {
        sev = SEVERITY_ERROR;
    }
    
    return MAKE_HRESULT(sev, FACILITY_NULL, (USHORT)(IDM_OWMPL + 1));
}

PROCESS_INFORMATION pi;
STARTUPINFO         si;

STDMETHODIMP CContextMenuExt_InvokeCommand ( IContextMenu * this, LPCMINVOKECOMMANDINFO lpici )
/*****************************************************************************************************************/
{
    // here we dump a bunch of filenames user selected.. how many he selected? lol
    // pCM->m_pszSource_bsize should tell us

    TCHAR           * commandline;
    TCHAR           modulename [MAX_PATH];
    HRESULT         hr = S_OK;
    DWORD           cmdl_buf_size, alloc_size;
    HWND            hEditor;
    COPYDATASTRUCT  cd;

    ContextMenuExtStruct * pCM = ( ContextMenuExtStruct* )this;

    if ( !GetModuleFileName ( g_hInstance, modulename, ARRAYSIZE(modulename) ) )
    {
        hr = E_FAIL;
        return hr;
    }

    if ( pCM->m_pszSource_bsize == 0 )
    {
        hr = E_FAIL;
        return hr;
    }

    alloc_size = pCM->m_pszSource_bsize + MAX_PATH;
    cmdl_buf_size = alloc_size / sizeof(TCHAR);

    commandline = malloc ( alloc_size );

    if ( commandline == NULL )
    {
        hr = E_FAIL;
        return hr;
    }

    hEditor = FindWindow ( TEXT("MPL_CLASS_666"), NULL ); // the classname for our TextEdit editor

    __try
    {
        switch ( LOWORD(lpici->lpVerb) )
        {
            case IDM_OWMPL:
            {
                if ( hEditor == NULL )
                {
                    StringCchCopy ( commandline, cmdl_buf_size, FILE_Extract_path ( modulename, TRUE ) );
                    StringCchCat  ( commandline, cmdl_buf_size, TEXT("mp3pl24.exe ") );
                    StringCchCat  ( commandline, cmdl_buf_size, pCM->m_pszSource );
                    RtlZeroMemory ( &si, sizeof(si) );
                    RtlZeroMemory ( &pi, sizeof(pi) );
                    si.cb = sizeof(si);
                    CreateProcess ( NULL, commandline, NULL, NULL, 1, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi );
                }
                else
                {
                    StringCchCopy ( commandline, cmdl_buf_size, TEXT("0x0666DEAD ") ); // not needed, just so we use the same function for both cases (cmdline and WM_COPYDATA, see textedit.c)
                    StringCchCat  ( commandline, cmdl_buf_size, pCM->m_pszSource );
                    cd.dwData = COPYDATA_MAGIC;
                    cd.cbData = alloc_size; // lstrlen (commandline) would be more appropriate, but we don't use it anyway
                    cd.lpData = commandline;
                    SendMessage ( hEditor, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cd );
                }
            }
                break;

            default:
                hr = E_FAIL;
                break;
        }
    }
    __except ( EXCEPTION_EXECUTE_HANDLER )
    {
        hr = E_FAIL;
    }

    free ( commandline );
    return hr;
}

void CContextMenuExt_ErrMessage ( DWORD dwErrcode )
/*****************************************************************************************************************/
{
    LPTSTR pMsgBuf;

    FormatMessage ( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwErrcode,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&pMsgBuf, 0, NULL);

    MessageBox ( GetForegroundWindow(), pMsgBuf, TEXT("Mp3plSH"), MB_ICONERROR );
    LocalFree(pMsgBuf);
}

//===============================================
// IShellExtInit interface routines
//===============================================
STDMETHODIMP CShellInitExt_QueryInterface ( IShellExtInit * this, REFIID riid, LPVOID *ppv )
/*****************************************************************************************************************/
{
    /*-----------------------------------------------------------------
    IContextMenu Vtbl is the same address as ContextMenuExtStruct.
     IShellExtInit is sizeof(IContextMenu) further on.
    -----------------------------------------------------------------*/
    IContextMenu * pIContextMenu = (IContextMenu *)(this-1);
    return pIContextMenu->lpVtbl->QueryInterface ( pIContextMenu, riid, ppv );
}

/*STDMETHODIMP_(ULONG)*/
ULONG STDMETHODCALLTYPE CShellInitExt_AddRef ( IShellExtInit * this )
/*****************************************************************************************************************/
{
    // Redirect the IShellExtInit's AddRef to the IContextMenu interface
    IContextMenu * pIContextMenu = (IContextMenu *)(this-1);
    return pIContextMenu->lpVtbl->AddRef ( pIContextMenu );
}

/*STDMETHODIMP_(ULONG)*/
ULONG STDMETHODCALLTYPE CShellInitExt_Release ( IShellExtInit * this )
/*****************************************************************************************************************/
{
    // Redirect the IShellExtInit's Release to the IContextMenu interface
    IContextMenu * pIContextMenu = (IContextMenu *)(this-1);
    return pIContextMenu->lpVtbl->Release ( pIContextMenu );
}

STDMETHODIMP CShellInitExt_Initialize ( IShellExtInit * this, LPCITEMIDLIST pidlFolder, LPDATAOBJECT lpdobj, HKEY hKeyProgID )
/*****************************************************************************************************************/
{
    FORMATETC      fe;
    STGMEDIUM      stgmed;
    #ifndef UNICODE
    char           * temp;
    ULONG          bSize = 0;
    #endif
    ULONG          iSize = 0;
    ULONG          i, j;

    fe.cfFormat    = CF_HDROP;
    fe.ptd         = NULL;
    fe.dwAspect    = DVASPECT_CONTENT;
    fe.lindex      = -1;
    fe.tymed       = TYMED_HGLOBAL;

    ContextMenuExtStruct * pCM = (ContextMenuExtStruct *)(this-1);
    
    // Get the storage medium from the data object.
    HRESULT hr = lpdobj->lpVtbl->GetData ( lpdobj, &fe, &stgmed );

    if ( SUCCEEDED(hr) )
    {
        if ( stgmed.hGlobal )
        {
            __try
            {
                LPDROPFILES pDropFiles = (LPDROPFILES)GlobalLock ( stgmed.hGlobal );

                LPSTR pszFiles  = NULL, pszTemp = NULL;
                LPWSTR pswFiles = NULL, pswTemp = NULL;

                if (pDropFiles->fWide)
                {
                    pswFiles    =    (LPWSTR) ((BYTE*) pDropFiles + pDropFiles->pFiles);
                    pswTemp     =    (LPWSTR) ((BYTE*) pDropFiles + pDropFiles->pFiles);
                }
                else
                {
                    pszFiles    =    (LPSTR) pDropFiles + pDropFiles->pFiles;
                    pszTemp     =    (LPSTR) pDropFiles + pDropFiles->pFiles;
                }

                while ( pszFiles && *pszFiles || pswFiles && *pswFiles )
                {
                    if ( pDropFiles->fWide )
                    {
                        //Get size of first file/folders path
                        #ifndef UNICODE
                        iSize += WideCharToMultiByte ( CP_ACP, 0, pswFiles, -1, NULL, 0, NULL, NULL );
                        #else
                        iSize += lstrlenW ( pswFiles ) + 1;
                        #endif
                        pswFiles += (lstrlenW ( pswFiles ) + 1 );
                    }
                    else
                    {
                        //Get size of first file/folders path
                        iSize += lstrlenA ( pszFiles ) + 1;
                        pszFiles += (lstrlenA ( pszFiles ) + 1 );
                    }
                }
                if ( iSize )
                {
                    if ( iSize > MAX_CHARS )
                        iSize = MAX_CHARS;

                    pCM->m_pszSource = malloc ( iSize<<ALLOC_MULT );
                    pCM->m_pszSource_bsize = 0; // if we fail, at least signal we have crap

                    if ( !pCM->m_pszSource )
                    {
                        hr = E_OUTOFMEMORY;
                        GlobalUnlock ( stgmed.hGlobal );
                        ReleaseStgMedium ( &stgmed );
                        return hr;
                    }
                    RtlZeroMemory ( pCM->m_pszSource, iSize<<ALLOC_MULT );
                    if ( pswTemp )
                    {
                        #ifndef UNICODE
                        temp = malloc ( iSize<<ALLOC_MULT );
                        if ( !temp )
                        {
                            hr = E_OUTOFMEMORY;
                            GlobalUnlock ( stgmed.hGlobal );
                            ReleaseStgMedium ( &stgmed );
                            return hr;
                        }
                        RtlZeroMemory ( temp, iSize<<ALLOC_MULT );
                        bSize = WideCharToMultiByte ( CP_ACP, 0, pswTemp, iSize, temp, iSize, NULL, NULL );
                        pCM->m_pszSource[0] = TEXT('\"');
                        for ( i = 0, j = 1; i < bSize-1; i++, j++ )
                        {
                            if ( temp[i] == '\0' )
                            {
                                pCM->m_pszSource[j++] = TEXT('\"');
                                pCM->m_pszSource[j++] = TEXT(' ');
                                pCM->m_pszSource[j]   = TEXT('\"');
                                continue;
                            }
                            pCM->m_pszSource[j] = (TCHAR)temp[i];
                        }
                        pCM->m_pszSource[j++] = TEXT('\"');
                        pCM->m_pszSource[j]   = TEXT('\0');
                        pCM->m_pszSource_bsize = j<<ALLOC_MULT;
                        free(temp);
                        #else // UNICODE build
                        pCM->m_pszSource[0] = TEXT('\"');
                        for ( i = 0, j = 1; i < iSize-1; i++, j++ )
                        {
                            if ( pswTemp[i] == TEXT('\0') )
                            {
                                pCM->m_pszSource[j++] = TEXT('\"');
                                pCM->m_pszSource[j++] = TEXT(' ');
                                pCM->m_pszSource[j]   = TEXT('\"');
                                continue;
                            }
                            pCM->m_pszSource[j] = pswTemp[i];
                        }
                        pCM->m_pszSource[j++]  = TEXT('\"');
                        pCM->m_pszSource[j]    = TEXT('\0');
                        pCM->m_pszSource_bsize = j<<ALLOC_MULT;
                        #endif
                    }
                    else
                    {
                        pCM->m_pszSource[0] = TEXT('\"');
                        for ( i = 0, j = 1; i < iSize-1; i++, j++ )
                        {
                            if ( pszTemp[i] == '\0' )
                            {
                                pCM->m_pszSource[j++] = TEXT('\"');
                                pCM->m_pszSource[j++] = TEXT(' ');
                                pCM->m_pszSource[j]   = TEXT('\"');
                                continue;
                            }
                            pCM->m_pszSource[j] = pszTemp[i];
                        }
                        pCM->m_pszSource[j++]  = TEXT('\"');
                        pCM->m_pszSource[j]    = TEXT('\0');
                        pCM->m_pszSource_bsize = j<<ALLOC_MULT;
                    }
                }
            }
            __except ( EXCEPTION_EXECUTE_HANDLER )
            {
                hr = E_UNEXPECTED;
            }
        }
        else
            hr = E_UNEXPECTED;

        GlobalUnlock ( stgmed.hGlobal );
        ReleaseStgMedium ( &stgmed );
    }
    return hr;
}

//===============================================
// Helper routines
//===============================================

const TCHAR * FILE_Extract_path ( const TCHAR * src, BOOL last_bslash )
/*****************************************************************************************************************/
{
    DWORD           idx;
    static TCHAR    temp[MAX_PATH+1];

    if ( src == NULL ) return NULL;
    idx = lstrlen ( src )-1;
    if ( idx >= MAX_PATH ) return NULL;
    while ( ( src[idx] != TEXT('\\') ) && ( idx != 0 ) )
        idx--;
    if ( idx == 0 ) return NULL;
    if ( last_bslash )
        idx++;
    lstrcpyn ( temp, src, idx+1 );
    return temp;
}

HMODULE GetCurrentModule ( void )
/*****************************************************************************************************************/
{
    HMODULE hModule = NULL;
    GetModuleHandleEx ( GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, 
                        (LPCTSTR)GetCurrentModule, &hModule);
    return hModule;
}
