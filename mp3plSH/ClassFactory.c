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

// classfactory.c

#include "mp3plSH.h"
#include <windows.h>
#include <shlobj.h>
#include <stdio.h>
#include "contextMenu.h"
#include "classfactory.h"


// declared in texteditsh.c
extern UINT             g_uiRefThisDll; // Reference count for this DLL
extern HINSTANCE        g_hInstance;    // Reference count for this DLL

// The virtual table for the ClassFactory
IClassFactoryVtbl iclassFactoryVtbl = {
    CClassFactory_QueryInterface,
    CClassFactory_AddRef,
    CClassFactory_Release,
    CClassFactory_CreateInstance,
    CClassFactory_LockServer
};

//--------------------------------------------------------------
// ClassFactoryEx constructor
//--------------------------------------------------------------
IClassFactory * CClassFactory_Create ( void )
/*****************************************************************************************************************/
{
     // Create the ClassFactoryStruct that will contain interfaces and vars
    ClassFactoryStruct * pCF = malloc ( sizeof(*pCF) );

    if(!pCF)
        return NULL;

    pCF->fc.lpVtbl = &iclassFactoryVtbl;

    // init the vars
    pCF->m_hInstance  = g_hInstance;    // Instance handle for this DLL
    pCF->m_ulRef = 1;                    // increment the reference

    ++g_uiRefThisDll;

    // Return the IClassFactory virtual table
    return &pCF->fc;
}

STDMETHODIMP CClassFactory_QueryInterface ( IClassFactory *this, REFIID riid, LPVOID *ppv )
/*****************************************************************************************************************/
{
    // The address of the struct is the same as the address
    // of the IClassFactory Virtual table. 
    ClassFactoryStruct * pCF = (ClassFactoryStruct*)this;

    if ( IsEqualIID ( riid, &IID_IUnknown ) || IsEqualIID ( riid, &IID_IClassFactory ))
    {
        *ppv = this;
        ++pCF->m_ulRef;
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}

/*STDMETHODIMP_(ULONG)*/
ULONG STDMETHODCALLTYPE CClassFactory_AddRef ( IClassFactory *this )
/*****************************************************************************************************************/
{
    ClassFactoryStruct * pCF = (ClassFactoryStruct*)this;
    return ++pCF->m_ulRef;
}

/*STDMETHODIMP_(ULONG)*/
ULONG STDMETHODCALLTYPE CClassFactory_Release ( IClassFactory *this )
/*****************************************************************************************************************/
{
    ClassFactoryStruct * pCF = (ClassFactoryStruct*)this;
    if ( --pCF->m_ulRef == 0 )
    {
        free(this);
        --g_uiRefThisDll;
        return 0;
    }
    return pCF->m_ulRef;
}

STDMETHODIMP CClassFactory_CreateInstance ( IClassFactory *this, LPUNKNOWN pUnkOuter, REFIID riid,  LPVOID *ppv )
/*****************************************************************************************************************/
{
    *ppv = NULL;
    
    if ( pUnkOuter )
        return CLASS_E_NOAGGREGATION;

    HRESULT hr = S_OK;
    // Creates the IContextMenu incorperating IShellExtInit interfaces
    IContextMenu * pIContextMenu = CContextMenuExt_Create();
    if (NULL == pIContextMenu)
    {
        return E_OUTOFMEMORY;
    }

    // This puts the IContextMenu interface into 'ppv'
    hr = pIContextMenu->lpVtbl->QueryInterface ( pIContextMenu, riid, ppv );
    pIContextMenu->lpVtbl->Release ( pIContextMenu );
    return hr;
}

STDMETHODIMP CClassFactory_LockServer ( IClassFactory *this, BOOL fLock )
/*****************************************************************************************************************/
{
    return E_NOTIMPL;
}
