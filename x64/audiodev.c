
#pragma warn(disable: 2008 2118 2228 2231 2030 2260)

#include        <windows.h>
#include        <strsafe.h>
#include        <tchar.h>
#include        <stdlib.h>
#include        <bass.h>


/*-@@+@@--------------------------------------------------------------------*/
//       Function: AudioDevListInit 
/*--------------------------------------------------------------------------*/
//           Type: BOOL 
//    Param.    1: HWND hList : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: init a listbox with the active audio devices
/*--------------------------------------------------------------------@@-@@-*/
BOOL AudioDevListInit ( HWND hList )
/*--------------------------------------------------------------------------*/
{
    TCHAR               buf[256];
    BASS_DEVICEINFO     devinfo;
    int                 i, enabled;

    enabled = 0;
    for ( i = 1; BASS_GetDeviceInfo ( i, &devinfo ); i++ )
    {
        if ( devinfo.flags & BASS_DEVICE_ENABLED )
        {
            // a small hack, because BASS_SetConfig 
            // ( BASS_CONFIG_UNICODE, TRUE ) does not seem to work correctly
            // so we let it be ANSI, and cvt. to WCHAR when doing a 
            // UNICODE build
#ifdef UNICODE
            int     wchars;
            WCHAR   wname[256];
            wchars = MultiByteToWideChar 
                ( CP_ACP, 0, devinfo.name, -1, NULL, 0 );
            MultiByteToWideChar 
                ( CP_ACP, 0, devinfo.name, -1, wname, wchars );
            StringCchPrintf ( buf, ARRAYSIZE(buf), L"%d %ls", i, wname );
#else
            StringCchPrintf ( buf, ARRAYSIZE(buf), "%d %s", i, devinfo.name );
#endif
            if ( devinfo.flags & BASS_DEVICE_DEFAULT )
            {
                StringCchCat ( buf, ARRAYSIZE(buf), TEXT(" (default)") );
                SendMessage ( hList, LB_ADDSTRING, 0, (LPARAM)buf );
                SendMessage ( hList, LB_SETCURSEL, enabled, 0 );
            }
            else
            {
                SendMessage ( hList, LB_ADDSTRING, 0, (LPARAM)buf );
            }
            enabled++;
        }
    }

    return TRUE;
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: AudioDevChange 
/*--------------------------------------------------------------------------*/
//           Type: BOOL 
//    Param.    1: HWND hList : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: switch to another playback device
/*--------------------------------------------------------------------@@-@@-*/
BOOL AudioDevChange ( HWND hList )
/*--------------------------------------------------------------------------*/
{
    INT_PTR     sel;
    DWORD       sel_dev, old_dev;
    TCHAR       buf[64];
    
    sel = SendMessage ( hList, LB_GETCURSEL, 0, 0 );
    
    if ( sel == LB_ERR )
        return FALSE;

    old_dev = BASS_GetDevice();

    if ( old_dev == ( DWORD )-1 )
        return FALSE;

    SendMessage ( hList, LB_GETTEXT, ( WPARAM )sel, ( LPARAM )buf );

    sel_dev = _ttoi ( buf );
    BASS_Free();

    if ( !BASS_Init ( sel_dev, 44100, 0, 0, NULL ) )
    {
        BASS_Init ( old_dev, 44100, 0, 0, NULL );
        return FALSE;
    }

    return TRUE;
}

