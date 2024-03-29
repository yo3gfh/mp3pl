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

#define INITGUID
#define COBJMACROS

#include "mp3plSH.h"
#include <windows.h>
#include <objbase.h>
#include <initguid.h>
#include <shlobj.h>
#include <shlwapi.h>
#include "ClassFactory.h"

#include <stdio.h>

/* 
    This CLSID has been generated with GUIDGEN.EXE - if you want to use
    this context menu extension DLL for another purpose you should use 
    GUIDGEN.EXE to create another unique CLSID.
*/

/* {3339EEAF-B6FB-4D3D-8465-3B45DA7B31DA} */
DEFINE_GUID(CLSID_Shell_TextEditSH,0x3339eeaf,0xb6fb,0x4d3d,0x84,0x65,0x3b,0x45,0xda,0x7b,0x31,0xda);

#define SZ_GUID                 TEXT("{3339EEAF-B6FB-4D3D-8465-3B45DA7B31DA}")

#define SZ_CLSID                TEXT("CLSID\\{3339EEAF-B6FB-4D3D-8465-3B45DA7B31DA}")
#define SZ_INPROCSERVER32       TEXT("CLSID\\{3339EEAF-B6FB-4D3D-8465-3B45DA7B31DA}\\InprocServer32")
#define SZ_DEFAULT              TEXT("")
#define SZ_THREADINGMODEL       TEXT("ThreadingModel")
#define SZ_APARTMENT            TEXT("Apartment")
#define SZ_APPROVED             TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved")
#define SZ_ERRMSG               TEXT("Unable to add ClassId {3339EEAF-B6FB-4D3D-8465-3B45DA7B31DA} to Registry\nAdministrative Privileges Needed")
#define SZ_ERROR                TEXT("Error")
//#define SZ_DIRCONTEXTMENUEXT    TEXT("Directory\\shellex\\ContextMenuHandlers\\TextEdit") //we don't process folders
#define SZ_FILECONTEXTMENUEXT_MP3    TEXT("*\\shellex\\ContextMenuHandlers\\Mp3plSH")
//#define SZ_FILECONTEXTMENUEXT_FLAC   TEXT("Mp3plSH.flac\\shellex\\ContextMenuHandlers\\Mp3plSH")
//#define SZ_FOLDERCONTEXTMENUEXT    TEXT("Folder\\shellex\\ContextMenuHandlers\\TextEdit") //we don't process folders

UINT            g_uiRefThisDll = 0;         // Reference count for this DLL
HINSTANCE       g_hInstance;                // Instance handle for this DLL

BOOL WINAPI DllMain ( HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved )
/*****************************************************************************************************************/
{
    if ( dwReason == DLL_PROCESS_ATTACH )
    {
        // SAVE this :-) GetModuleHandle(NULL) doesn't work in dll's
        g_hInstance = hInstance;
    }
    return TRUE;
}

STDAPI DllGetClassObject ( REFCLSID rclsid, REFIID riid, LPVOID *ppv )
/*****************************************************************************************************************/
{
    *ppv = NULL;
    if ( !IsEqualCLSID ( rclsid, &CLSID_Shell_TextEditSH ) )
    {
        return CLASS_E_CLASSNOTAVAILABLE;
    }

    IClassFactory * pClassFactory = CClassFactory_Create();
    if ( !pClassFactory )
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = pClassFactory->lpVtbl->QueryInterface ( pClassFactory, riid, ppv );
    pClassFactory->lpVtbl->Release ( pClassFactory );
    return hr;
}

STDAPI DllCanUnloadNow ( void )
/*****************************************************************************************************************/
{
    return ( g_uiRefThisDll == 0 ) ? S_OK : S_FALSE;
}

STDAPI DllRegisterServerInt ( void )
/*****************************************************************************************************************/
{
    HRESULT         hr          = E_UNEXPECTED;
    LONG            regRet      = 0;
    static TCHAR    szDescr[]   = TEXT("Mp3plSH Extension");        
    TCHAR           szFilePath[MAX_PATH];

    GetModuleFileName ( g_hInstance, szFilePath, ARRAYSIZE(szFilePath) );

    regRet = SHSetValue ( HKEY_CLASSES_ROOT, SZ_CLSID, SZ_DEFAULT, REG_SZ, szDescr, sizeof(szDescr) );
    if ( ERROR_SUCCESS != regRet )
    {
        return E_UNEXPECTED;
    }

    regRet  = SHSetValue ( HKEY_CLASSES_ROOT, SZ_INPROCSERVER32, SZ_DEFAULT, REG_SZ, szFilePath, (lstrlen ( szFilePath )+1) * sizeof(TCHAR) );
    regRet |= SHSetValue ( HKEY_CLASSES_ROOT, SZ_INPROCSERVER32, SZ_THREADINGMODEL, REG_SZ, SZ_APARTMENT, sizeof(SZ_APARTMENT) );

    if ( regRet != ERROR_SUCCESS )
    {
        return E_FAIL;
    }

    HKEY hKeyApproved = NULL;
    LONG lRet = RegOpenKeyEx ( HKEY_LOCAL_MACHINE, SZ_APPROVED, 0, KEY_SET_VALUE, &hKeyApproved );
    if ( lRet == ERROR_ACCESS_DENIED )
    {
        MessageBox ( NULL, SZ_ERRMSG, SZ_ERROR, MB_OK );
        hr = E_UNEXPECTED;
    }
    else if ( lRet == ERROR_FILE_NOT_FOUND )
    {
        // Desn't exist
    }
    if ( hKeyApproved )
    {
        if ( RegSetValueEx ( hKeyApproved, SZ_GUID, 0, REG_SZ, (const BYTE*) szDescr, (lstrlen ( szDescr ) + 1) * sizeof(TCHAR) ) == ERROR_SUCCESS )
        {
            hr = S_OK;
        }
        else
        {
            hr = E_UNEXPECTED;
        }
    }

    if ( FAILED ( hr ) )
    {
        return hr;
    }
/*
    regRet = SHSetValue(HKEY_CLASSES_ROOT, SZ_DIRCONTEXTMENUEXT, SZ_DEFAULT, REG_SZ, 
        SZ_GUID, sizeof(SZ_GUID));
    regRet |= SHSetValue(HKEY_CLASSES_ROOT, SZ_FILECONTEXTMENUEXT, SZ_DEFAULT, REG_SZ,
        SZ_GUID, sizeof(SZ_GUID));
    regRet |= SHSetValue(HKEY_CLASSES_ROOT, SZ_FOLDERCONTEXTMENUEXT, SZ_DEFAULT, 
        REG_SZ, SZ_GUID, sizeof(SZ_GUID));
*/
    regRet = SHSetValue ( HKEY_CLASSES_ROOT, SZ_FILECONTEXTMENUEXT_MP3, SZ_DEFAULT, REG_SZ, SZ_GUID, sizeof(SZ_GUID) );
    //regRet |= SHSetValue ( HKEY_CLASSES_ROOT, SZ_FILECONTEXTMENUEXT_FLAC, SZ_DEFAULT, REG_SZ, SZ_GUID, sizeof(SZ_GUID) );
    return (regRet != ERROR_SUCCESS) ? E_FAIL : S_OK;
}

STDAPI DllUnregisterServer ( void )
/*****************************************************************************************************************/
{
    LONG regRet = SHDeleteKey ( HKEY_CLASSES_ROOT, SZ_INPROCSERVER32 );
    regRet |= SHDeleteKey ( HKEY_CLASSES_ROOT, SZ_CLSID );
    regRet |= SHDeleteValue ( HKEY_LOCAL_MACHINE, SZ_APPROVED, SZ_GUID );
    //regRet |= SHDeleteKey(HKEY_CLASSES_ROOT, SZ_FOLDERCONTEXTMENUEXT);
    //regRet |= SHDeleteKey(HKEY_CLASSES_ROOT, SZ_DIRCONTEXTMENUEXT);
    regRet |= SHDeleteKey ( HKEY_CLASSES_ROOT, SZ_FILECONTEXTMENUEXT_MP3 );
    //regRet |= SHDeleteKey ( HKEY_CLASSES_ROOT, SZ_FILECONTEXTMENUEXT_FLAC );

    return regRet == ERROR_SUCCESS ? S_OK : E_FAIL;
}

STDAPI DllRegisterServer ( void )
/*****************************************************************************************************************/
{
    HRESULT hr = DllRegisterServerInt();
    if ( FAILED ( hr ) )
    {
        DllUnregisterServer();
    }
    return hr;
}
