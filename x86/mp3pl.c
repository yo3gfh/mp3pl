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

#pragma warn(disable: 2008 2118 2228 2231 2030 2260)

#include                <windows.h>
#include                <strsafe.h>
#include                <commctrl.h>
#include                <shellapi.h>
#include                <math.h>
#include                <bass.h>
#include                <stdlib.h>

#include                "mp3pl.h"
#include                "lv.h"
#include                "draw.h"
#include                "playlist.h"
#include                "audiodev.h"
#include                "misc.h"
#include                "tray.h"

#include                "resource.h"


//
// FUNCTION PROTOTYPES
//

static void             LVDrag              ( HWND hList, DWORD x, DWORD y );
static void             LVEndDrag           ( HWND hList );

static INT_PTR CALLBACK DlgProc
    ( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

static void Process_WM_COMMAND  ( HWND hDlg, WPARAM wParam, LPARAM lParam );
static void Process_WM_NOTIFY   ( HWND hDlg, WPARAM wParam, LPARAM lParam );
static void Process_WM_HSCROLL  ( HWND hDlg, WPARAM wParam, LPARAM lParam );
static void Process_WM_TRAY     ( HWND hDlg, WPARAM wParam, LPARAM lParam );
static BOOL Process_WM_COPYDATA ( HWND hwnd, WPARAM wParam, LPARAM lParam );
static INT_PTR CALLBACK LVSubclassProc 
    ( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam );
static INT_PTR CALLBACK SpecSubclassProc 
    ( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam );
static INT_PTR CALLBACK VolSubclassProc 
    ( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam );

static void CALLBACK MP3_UpdateStatus 
    ( UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2 );

static void CALLBACK StreamEndProc 
    ( HSYNC handle, DWORD channel, DWORD data, void * user );
static BOOL Create_DIB 
    ( HDC * hdc, HBITMAP * hbmp, BYTE ** bmpdata, int width, int height );
static void             UpdateSpec          ( void );
static void             UpdateScope         ( void );
static void             UpdateLevels        ( void );
static void             App_Shutdown        ( void );
static INIT_STATUS      App_Init            ( HWND hDlg );
static INIT_STATUS      Global_Init         ( void );

static BOOL             MP3_Play            ( void );
static BOOL             MP3_Stop            ( void );
static BOOL             MP3_Pause           ( void );
static BOOL             MP3_Next            ( void );
static BOOL             MP3_SetPos          ( void );
static BOOL             MP3_Ffw             ( void );
static BOOL             MP3_Rew             ( void );

static void             ToggleMenus         ( HMENU hmenu );
static void             ContextMenu         ( HWND hwnd, DWORD flags );
    
//
// GLOBALS
//

 HINSTANCE              g_hInst             = NULL;
 HWND                   g_hList             = NULL;
 HWND                   g_hDevList          = NULL;
 HWND                   g_hDlg              = NULL;
 HWND                   g_hPosTrack         = NULL;
 HWND                   g_hVolTrack         = NULL;
 HWND                   g_hSpec             = NULL;
 HWND                   g_hVol              = NULL;
 HACCEL                 g_hAccel            = NULL;
 HIMAGELIST             g_hIml              = NULL;
 HICON                  g_hIcon             = NULL;
 HICON                  g_hTray             = NULL;
 HSTREAM                g_hStream           = 0;
 HMENU                  g_hMainmenu         = NULL;

// playback flags & globals
 BOOL                   g_Minimized         = FALSE;
 BOOL                   g_Playing           = FALSE;
 BOOL                   g_Paused            = FALSE;
 BOOL                   g_Busy              = FALSE;
 BOOL                   g_havescope         = FALSE;
 int                    g_CurIndex          = 0;
 DWORD                  g_ScaleFactor;

// used for item dragging
 BOOL                   g_Dragging          = FALSE;
 TCHAR                  g_Item[MAX_PATH];
 TCHAR                  g_SubItem[32];
 int                    g_DragStart         = 0;
 int                    g_DragPos           = 0;
 int                    g_ItemImgIdx        = 0;
 int                    g_ItemCount         = 0;
 int                    g_LastPlayed        = -1;

// for spectrum analyser / scope
 HBITMAP                g_SpecBmp           = NULL;
 BYTE                   * g_SpecBuf         = NULL;
 HDC                    g_SpecDC            = NULL;
 int                    g_SpecWidth         = 0;
 int                    g_SpecHeight        = 0;
 int                    g_SpecScaleTop      = 0;

// for vu-meter
 HBITMAP                g_VolBmp            = NULL;
 BYTE                   * g_VolBuf          = NULL;
 HDC                    g_VolDC             = NULL;
 DWORD                  g_VolWidth          = 0;
 DWORD                  g_VolHeight         = 0;
 DWORD                  g_VolScaleTop       = 0;

// pointers to old window procs
 WNDPROC                g_OldListProc       = NULL;
 WNDPROC                g_OldSpecProc       = NULL;
 WNDPROC                g_OldVolProc        = NULL;

 DWORD                  g_Timer             = 0;


/*-@@+@@--------------------------------------------------------------------*/
//       Function: WinMain 
/*--------------------------------------------------------------------------*/
//           Type: int WINAPI 
//    Param.    1: HINSTANCE hInstance    : 
//    Param.    2: HINSTANCE hPrevInstance: 
//    Param.    3: PSTR szCmdLine         : 
//    Param.    4: int iCmdShow           : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: Main program loop
/*--------------------------------------------------------------------@@-@@-*/
int WINAPI WinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, 
    PSTR szCmdLine, int iCmdShow )
/*--------------------------------------------------------------------------*/
{
    MSG             msg;
    INIT_STATUS     is;

    is = Global_Init();

    switch ( is )
    {    
        case ERR_SUCCESS:
            break;

        case ERR_INIT_ACC_TABLE:
        case ERR_APP_RUNNING:
        case ERR_DLG_CREATE:
            ShowMessage ( NULL, g_err_messages[is], MB_OK );
            return 1;
            break;

        case ERR_BASS_VER:
            return 1;
            break;

        case ERR_BASS_INIT:
            BASS_Error ( NULL, g_err_messages[is] );
            return 1;
            break;

        default:
            break;
    }

    while ( GetMessage ( &msg, NULL, 0, 0 ) > 0 )
    {
        if ( !TranslateAccelerator ( g_hDlg, g_hAccel, &msg ) )
        {
            TranslateMessage ( &msg );
            DispatchMessage ( &msg );
        }
    }

    return 0;
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: DlgProc 
/*--------------------------------------------------------------------------*/
//           Type: static INT_PTR CALLBACK 
//    Param.    1: HWND hDlg     : 
//    Param.    2: UINT uMsg     : 
//    Param.    3: WPARAM wParam : 
//    Param.    4: LPARAM lParam : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: DLG procedure for the main window
/*--------------------------------------------------------------------@@-@@-*/
static INT_PTR CALLBACK DlgProc ( HWND hDlg, UINT uMsg, WPARAM wParam, 
    LPARAM lParam )
/*--------------------------------------------------------------------------*/
{
    INIT_STATUS     is; // see misc.h

    switch ( uMsg )
    {
        case WM_INITDIALOG:
            is = App_Init( hDlg );
            if ( is != ERR_SUCCESS )
            {
                // g_err_messages in "misc.c"
                ShowMessage ( hDlg, g_err_messages[is], MB_OK ); 
                SendMessage ( hDlg, WM_CLOSE, 0, 0 );
            }
            break;

        case WM_COPYDATA:
            Process_WM_COPYDATA ( hDlg, wParam, lParam );
            break;

        // process WM_HSCROLL for vol and position sliders
        case WM_HSCROLL:
            Process_WM_HSCROLL ( hDlg, wParam, lParam );
            break;

        case WM_LBUTTONDBLCLK:
            g_havescope = !g_havescope;
            break;

        case WM_SETFOCUS:
            SetFocus ( g_hList );
            break;

        case WM_INITMENUPOPUP:
            if ( LOWORD ( lParam ) == 1 )
                ToggleMenus ( ( HMENU )wParam );
            break;

        case WM_CLOSE:
            // remove tray icon here, while still having a valid dlg handle
            Tray_Remove ( g_hDlg, IDI_LISTICON );
            DestroyWindow ( hDlg );
            break;

        case WM_DESTROY:
            App_Shutdown();
            PostQuitMessage ( 0 );
            break;

        case WM_SIZE:
            if ( wParam == SIZE_MINIMIZED )
            {
                ShowWindow ( hDlg, SW_HIDE );
                g_Minimized = TRUE;
            }
            break;

        case WM_TRAY:
            // tray icon related
            Process_WM_TRAY ( hDlg, wParam, lParam );
            break;

        case WM_NOTIFY:
            Process_WM_NOTIFY ( hDlg, wParam, lParam );
            break;

        case WM_COMMAND:
            // menus and such
            Process_WM_COMMAND ( hDlg, wParam, lParam );
            break;

        default:
            return 0;
            break;
    }
    return 1;
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: MP3_Play 
/*--------------------------------------------------------------------------*/
//           Type: static BOOL 
//    Param.    1: void : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: play a file or unpause a paused stream
/*--------------------------------------------------------------------@@-@@-*/
static BOOL MP3_Play ( void )
/*--------------------------------------------------------------------------*/
{
    TCHAR   file[MAX_PATH];
    HSYNC   hsync;
    QWORD   len;

    g_Busy = TRUE;

    if ( g_Paused )
    {
        BASS_ChannelPlay ( g_hStream, FALSE );
        g_Paused = FALSE;
        g_Busy = FALSE;
        return TRUE;
    }

    if ( ( LVGetCount ( g_hList ) ) == 0 )
    {
        g_Playing = FALSE;
        g_Busy = FALSE;
        return FALSE;
    }

    BASS_StreamFree ( g_hStream );
    g_hStream = 0;
    g_CurIndex = LVGetSelIndex ( g_hList );
    //change crt played item icon...
    LVGetItemText ( g_hList, g_CurIndex, 0, file, ARRAYSIZE(file) );
    LVSetItemImg ( g_hList, g_CurIndex, 1 ); 
    EnableWindow ( g_hDevList, FALSE );

    // ...and reset the previous item icon
    if ( ( g_LastPlayed != -1 ) && ( g_LastPlayed != g_CurIndex ) )
        LVSetItemImg ( g_hList, g_LastPlayed, 0 ); 

    g_LastPlayed = g_CurIndex;
    
    #ifdef  UNICODE
    g_hStream = BASS_StreamCreateFile ( FALSE, file, 0, 0, BASS_UNICODE );
    #else
    g_hStream = BASS_StreamCreateFile ( FALSE, file, 0, 0, 0 );
    #endif

    if ( !g_hStream )
    {
        g_Busy = FALSE;
        return FALSE;
    }
    
    // set a "stream end" callback
    hsync = BASS_ChannelSetSync ( g_hStream,
                                  BASS_SYNC_END|BASS_SYNC_ONETIME,
                                  0,
                                  StreamEndProc,
                                  NULL
                                );

    if ( !hsync )
    {
        BASS_StreamFree ( g_hStream );
        g_hStream = 0;
        g_Busy = FALSE;
        return FALSE;
    }

    len = BASS_ChannelGetLength ( g_hStream, BASS_POS_BYTE );
    g_ScaleFactor = ( DWORD )( len >> 14 );

    if ( !BASS_ChannelPlay ( g_hStream, FALSE ) )
    {
        BASS_ChannelRemoveSync ( g_hStream, hsync );
        BASS_StreamFree ( g_hStream );
        g_hStream = 0;
        g_Busy = FALSE;
        return FALSE;
    }
    
    g_Busy      = FALSE;
    g_Playing   = TRUE;

    return TRUE;
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: MP3_Stop 
/*--------------------------------------------------------------------------*/
//           Type: static BOOL 
//    Param.    1: void : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: stop a playing stream
/*--------------------------------------------------------------------@@-@@-*/
static BOOL MP3_Stop ( void )
/*--------------------------------------------------------------------------*/
{
    MSG     msg;

    g_Busy = TRUE;
    
    PeekMessage ( &msg, g_hDlg, TBM_SETPOS, TBM_SETPOS, PM_REMOVE );
    RtlZeroMemory ( g_VolBuf, g_VolWidth * g_VolHeight );
    RtlZeroMemory ( g_SpecBuf, g_SpecWidth * g_SpecHeight );
    RedrawWindow( g_hSpec, NULL, NULL, RDW_INVALIDATE );
    RedrawWindow( g_hVol, NULL, NULL, RDW_INVALIDATE );
    SendMessage ( g_hPosTrack, TBM_SETPOS, TRUE, 0 );
    SetDlgItemText ( g_hDlg, IDC_TIME, TEXT("00:00") );
    EnableWindow ( g_hDevList, TRUE );

    Sleep ( 50 );

    if ( !BASS_ChannelStop ( g_hStream ) )
    {
        g_Playing = FALSE;
        g_Paused = FALSE;
        g_Busy = FALSE;
        return FALSE;
    }

    LVSetItemImg ( g_hList, g_LastPlayed, 0 );

    g_Playing = FALSE;
    g_Paused = FALSE;
    g_Busy = FALSE;

    return TRUE;
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: MP3_Pause 
/*--------------------------------------------------------------------------*/
//           Type: static BOOL 
//    Param.    1: void : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: pause a playing stream
/*--------------------------------------------------------------------@@-@@-*/
static BOOL MP3_Pause ( void )
/*--------------------------------------------------------------------------*/
{
    BOOL    result;
    
    g_Busy = TRUE;

    if ( g_Paused )
        result = BASS_ChannelPlay ( g_hStream, FALSE );
    else
        result = BASS_ChannelPause ( g_hStream );

    if ( !result )
    {
        g_Busy = FALSE;
        return FALSE;
    }

    g_Paused = !g_Paused;
    g_Busy = FALSE;

    return TRUE;
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: MP3_Next 
/*--------------------------------------------------------------------------*/
//           Type: static BOOL 
//    Param.    1: void : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: play the next in line
/*--------------------------------------------------------------------@@-@@-*/
static BOOL MP3_Next ( void )
/*--------------------------------------------------------------------------*/
{
    int     count;
    
    g_Busy = TRUE;
    count = LVGetCount ( g_hList );

    if ( count == 0 )
    {
        g_Playing = FALSE;
        g_Paused = FALSE;
        g_Busy = FALSE;
        return FALSE;
    }

    LVUnselectItem ( g_hList, -1 );
    g_CurIndex++;
    g_CurIndex %= count;
    LVSelectItem ( g_hList, g_CurIndex );
    LVEnsureVisible ( g_hList, g_CurIndex );
    g_Playing = FALSE;
    g_Paused = FALSE;
    g_Busy = FALSE;

    return MP3_Play();
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: MP3_Ffw 
/*--------------------------------------------------------------------------*/
//           Type: static BOOL 
//    Param.    1: void : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: fast forward into a stream
/*--------------------------------------------------------------------@@-@@-*/
static BOOL MP3_Ffw ( void )
/*--------------------------------------------------------------------------*/
{
    INT_PTR         trackpos;
    QWORD           streampos;

    g_Busy = TRUE;
    trackpos = SendMessage ( g_hPosTrack, TBM_GETPOS, 0, 0 );
    trackpos += POS_INCREMENT;

    if ( trackpos >= MAX_RANGE )
    {
        g_Busy = FALSE;
        return FALSE;
    }

    streampos = UInt32x32To64 ( trackpos, g_ScaleFactor );

    if ( !BASS_ChannelSetPosition ( g_hStream, streampos, BASS_POS_BYTE ) )
    {
        g_Busy = FALSE;
        return FALSE;
    }
    
    g_Busy = FALSE;

    return TRUE;
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: MP3_Rew 
/*--------------------------------------------------------------------------*/
//           Type: static BOOL 
//    Param.    1: void : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: rewind into a stream
/*--------------------------------------------------------------------@@-@@-*/
static BOOL MP3_Rew ( void )
/*--------------------------------------------------------------------------*/
{
    INT_PTR         trackpos;
    QWORD           streampos;

    g_Busy      = TRUE;
    trackpos    = SendMessage ( g_hPosTrack, TBM_GETPOS, 0, 0 );
    
    if ( trackpos < POS_INCREMENT )
    {
        g_Busy = FALSE;
        return FALSE;
    }

    trackpos    -= POS_INCREMENT;
    streampos   = UInt32x32To64 ( trackpos, g_ScaleFactor );

    if ( !BASS_ChannelSetPosition ( g_hStream, streampos, BASS_POS_BYTE ) )
    {
        g_Busy = FALSE;
        return FALSE;
    }
    
    g_Busy = FALSE;

    return TRUE;
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: MP3_SetPos 
/*--------------------------------------------------------------------------*/
//           Type: static BOOL 
//    Param.    1: void : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: position somewhere into a stream
/*--------------------------------------------------------------------@@-@@-*/
static BOOL MP3_SetPos ( void )
/*--------------------------------------------------------------------------*/
{
    INT_PTR         trackpos;
    QWORD           streampos;

    if ( !g_Playing || g_Paused ) { return FALSE; }

    g_Busy      = TRUE;
    trackpos    = SendMessage ( g_hPosTrack, TBM_GETPOS, 0, 0 );
    streampos   = UInt32x32To64 ( trackpos, g_ScaleFactor );

    if ( !BASS_ChannelSetPosition ( g_hStream, streampos, BASS_POS_BYTE ) )
    {
        g_Busy  = FALSE;
        return FALSE;
    }
    
    g_Busy      = FALSE;
    return TRUE;
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: MP3_UpdateStatus 
/*--------------------------------------------------------------------------*/
//           Type: static void CALLBACK 
//    Param.    1: UINT uTimerID: 
//    Param.    2: UINT uMsg    : 
//    Param.    3: DWORD dwUser : 
//    Param.    4: DWORD dw1    : 
//    Param.    5: DWORD dw2    : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: called on the mmtimer event, every 25ms; keep it short :
/*--------------------------------------------------------------------@@-@@-*/
static void CALLBACK MP3_UpdateStatus ( UINT uTimerID, UINT uMsg, 
    DWORD dwUser, DWORD dw1, DWORD dw2 )
/*--------------------------------------------------------------------------*/
{
    DWORD       time;
    TCHAR       temp[16];
    QWORD       pos;
    // function pointers array
    void        ( *spec_scope[] )( void ) = { UpdateSpec, UpdateScope }; 

    if ( !g_hStream || !g_SpecBuf || !g_VolBuf || g_Minimized || g_Busy || 
            !g_Playing || g_Paused )
                return; 

    pos = BASS_ChannelGetPosition ( g_hStream, BASS_POS_BYTE );
    SendMessage ( g_hPosTrack, TBM_SETPOS, TRUE, (DWORD)(pos/g_ScaleFactor) );
    time = (DWORD)BASS_ChannelBytes2Seconds ( g_hStream, pos );
    // we keep wsprintf here; I think it's faster than StringCchPrintf
    wsprintf ( temp, TEXT("%02d:%02d"), time/60, time%60 ); 
    SetDlgItemText ( g_hDlg, IDC_TIME, temp );
    ( *spec_scope[g_havescope] )(); // select the appropriate function
    UpdateLevels();
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: StreamEndProc 
/*--------------------------------------------------------------------------*/
//           Type: static void CALLBACK 
//    Param.    1: HSYNC handle : 
//    Param.    2: DWORD channel: 
//    Param.    3: DWORD data   : 
//    Param.    4: void * user  : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: callback when a stream ends
/*--------------------------------------------------------------------@@-@@-*/
static void CALLBACK StreamEndProc ( HSYNC handle, DWORD channel, 
    DWORD data, void * user )
/*--------------------------------------------------------------------------*/
{
    if ( ( LVGetCount ( g_hList ) ) > 0 ) { MP3_Next(); }
    else { MP3_Stop(); }
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: ToggleMenus 
/*--------------------------------------------------------------------------*/
//           Type: static void 
//    Param.    1: HMENU hmenu : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: enable/disable menu items according to conditions
/*--------------------------------------------------------------------@@-@@-*/
static void ToggleMenus ( HMENU hmenu )
/*--------------------------------------------------------------------------*/
{
    DWORD       states[2] = { MF_GRAYED, MF_ENABLED };
    BOOL        state;

    state = ( LVGetSelIndex ( g_hList ) != -1 );
    EnableMenuItem ( hmenu, IDM_DEL, states[state] );
    state = state || g_Paused;
    EnableMenuItem ( hmenu, IDM_PLAY, states[state] );
    
    state = ( LVGetCount ( g_hList ) > 0 );
    EnableMenuItem ( hmenu, IDM_CLEAR, states[state] );
    EnableMenuItem ( hmenu, IDM_SELALL, states[state] );
    EnableMenuItem ( hmenu, IDM_NEXT, states[state] );
    EnableMenuItem ( hmenu, IDM_STOP, states[g_Playing] );
    EnableMenuItem ( hmenu, IDM_PAUSE, states[g_Playing] );
    EnableMenuItem ( hmenu, IDM_REW, states[(g_Playing && !g_Paused)] );
    EnableMenuItem ( hmenu, IDM_FFW, states[(g_Playing && !g_Paused)] );
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: ContextMenu 
/*--------------------------------------------------------------------------*/
//           Type: static void 
//    Param.    1: HWND hwnd   : 
//    Param.    2: DWORD flags : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: playlist/trayicon popup menu
/*--------------------------------------------------------------------@@-@@-*/
static void ContextMenu ( HWND hwnd, DWORD flags )
/*--------------------------------------------------------------------------*/
{
    POINT       pt;
    HMENU       hmain, hsub;

    GetCursorPos ( &pt );
    hmain = GetMenu ( hwnd );
    hsub = GetSubMenu ( hmain, 1 );
    TrackPopupMenuEx ( hsub, flags, pt.x, pt.y, hwnd, NULL );
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: LVSubclassProc 
/*--------------------------------------------------------------------------*/
//           Type: static INT_PTR CALLBACK 
//    Param.    1: HWND hwnd     : 
//    Param.    2: UINT message  : 
//    Param.    3: WPARAM wParam : 
//    Param.    4: LPARAM lParam : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: subclassing the playlist (listview) control so that we can 
//                 access certain events and custom handle them
/*--------------------------------------------------------------------@@-@@-*/
static INT_PTR CALLBACK LVSubclassProc ( HWND hwnd, UINT message, 
    WPARAM wParam, LPARAM lParam )
/*--------------------------------------------------------------------------*/
{
    switch ( message )
    {
        case WM_MOUSEMOVE:
            if ( g_Dragging )
                LVDrag ( hwnd, LOWORD ( lParam ), HIWORD ( lParam ) );
            break;

        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
            if ( g_Dragging )
            {
                LVEndDrag ( hwnd );
                g_Dragging = FALSE;
                ReleaseCapture();
            }
            break;
    }
    // don't forget to call the old handler :))
    return CallWindowProc ( g_OldListProc, hwnd, message, wParam, lParam );
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: SpecSubclassProc 
/*--------------------------------------------------------------------------*/
//           Type: static INT_PTR CALLBACK 
//    Param.    1: HWND hwnd     : 
//    Param.    2: UINT message  : 
//    Param.    3: WPARAM wParam : 
//    Param.    4: LPARAM lParam : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: subclassing the spectrum analyzer display control so that 
//                 we can access certain events and custom handle them
/*--------------------------------------------------------------------@@-@@-*/
static INT_PTR CALLBACK SpecSubclassProc ( HWND hwnd, UINT message, 
    WPARAM wParam, LPARAM lParam )
/*--------------------------------------------------------------------------*/
{
    PAINTSTRUCT     p;
    HDC             dc;

    switch ( message )
    {
        // see when we need to be refreshed 
        case WM_PAINT:
            if ( GetUpdateRect ( hwnd, 0, 0 ) )
            {
                dc = BeginPaint ( hwnd, &p );

                if ( dc )
                    BitBlt ( dc, 0, 0, g_SpecWidth, g_SpecHeight,
                        g_SpecDC, 0, 0, SRCCOPY );

                EndPaint ( hwnd, &p );
            }
            break;
    }

    return CallWindowProc ( g_OldSpecProc, hwnd, message, wParam, lParam );
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: VolSubclassProc 
/*--------------------------------------------------------------------------*/
//           Type: static INT_PTR CALLBACK 
//    Param.    1: HWND hwnd     : 
//    Param.    2: UINT message  : 
//    Param.    3: WPARAM wParam : 
//    Param.    4: LPARAM lParam : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: subclassing the bargraph control so that we can access 
//                 certain events and custom handle them
/*--------------------------------------------------------------------@@-@@-*/
static INT_PTR CALLBACK VolSubclassProc ( HWND hwnd, UINT message, 
    WPARAM wParam, LPARAM lParam )
/*--------------------------------------------------------------------------*/
{
    PAINTSTRUCT     p;
    HDC             dc;

    switch ( message )
    {
        case WM_PAINT:
            if ( GetUpdateRect ( hwnd, 0, 0 ) )
            {
                dc = BeginPaint ( hwnd, &p );

                if ( dc )
                    BitBlt ( dc, 0, 0, g_VolWidth, g_VolHeight, 
                        g_VolDC, 0, 0, SRCCOPY );

                EndPaint ( hwnd, &p );
            }
            break;
    }

    return CallWindowProc ( g_OldVolProc, hwnd, message, wParam, lParam );
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: LVDrag 
/*--------------------------------------------------------------------------*/
//           Type: static void 
//    Param.    1: HWND hList: 
//    Param.    2: DWORD x   : 
//    Param.    3: DWORD y   : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: drag a playlist item up/down
/*--------------------------------------------------------------------@@-@@-*/
static void LVDrag ( HWND hList, DWORD x, DWORD y )
/*--------------------------------------------------------------------------*/
{
    LVHITTESTINFO   hti;
    TCHAR           item[MAX_PATH];
    TCHAR           subitem[32];
    int             idx;

    hti.pt.x = x;
    hti.pt.y = y;
    SendMessage ( hList, LVM_HITTEST, 0, (LPARAM)&hti );

    if ( hti.flags & LVHT_ONITEM )
    {
        if ( ( hti.iItem == g_DragPos ) ) { return; }

        LVFocusItem ( hList, hti.iItem );

        LVGetItemText ( hList, hti.iItem, 0, item, ARRAYSIZE(item) );
        LVGetItemText ( hList, hti.iItem, 1, subitem, ARRAYSIZE(subitem) );
        idx = LVGetItemImgIdx ( hList, hti.iItem );
        
        LVSetItemText ( hList, hti.iItem, 0, g_Item );
        LVSetItemText ( hList, hti.iItem, 1, g_SubItem );
        LVSetItemImg ( hList, hti.iItem, g_ItemImgIdx );

        if ( g_ItemImgIdx == 1 ) { g_LastPlayed = hti.iItem; }

        if ( hti.iItem < g_DragPos )
        {
            LVSetItemText ( hList, hti.iItem+1, 0, item );
            LVSetItemText ( hList, hti.iItem+1, 1, subitem );
            LVSetItemImg ( hList, hti.iItem+1, idx );
            if ( idx == 1 ) { g_LastPlayed++; }
        }
        else
        {
            LVSetItemText ( hList, hti.iItem-1, 0, item );
            LVSetItemText ( hList, hti.iItem-1, 1, subitem );
            LVSetItemImg ( hList, hti.iItem-1, idx );
            if ( idx == 1 ) { g_LastPlayed--; }
        }
        LVEnsureVisible ( hList, hti.iItem ); // see that it's on view
        g_DragPos = hti.iItem;
        g_CurIndex = g_LastPlayed;
    }
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: LVEndDrag 
/*--------------------------------------------------------------------------*/
//           Type: static void 
//    Param.    1: HWND hList : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: finish dragging a playlist item
/*--------------------------------------------------------------------@@-@@-*/
static void LVEndDrag ( HWND hList )
/*--------------------------------------------------------------------------*/
{
    LVUnselectItem ( hList, g_DragStart );
    LVSelectItem ( hList, g_DragPos );
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: Create_DIB 
/*--------------------------------------------------------------------------*/
//           Type: static BOOL 
//    Param.    1: HDC * hdc      : 
//    Param.    2: HBITMAP * hbmp : 
//    Param.    3: BYTE ** bmpdata: 
//    Param.    4: int width      : 
//    Param.    5: int height     : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: create a device independent bitmap (dib) in memory
//                 for spectrum/scope/vol bargraph 
/*--------------------------------------------------------------------@@-@@-*/
static BOOL Create_DIB ( HDC * hdc, HBITMAP * hbmp, BYTE ** bmpdata, 
    int width, int height )
/*--------------------------------------------------------------------------*/
{
    BYTE                head[BMP_STUFF_SIZE];
    BITMAPINFOHEADER    * bh;
    RGBQUAD             * pal;
    HBITMAP             bmp;
    HDC                 dc;
    BYTE                a;
    DWORD               color;

    RtlZeroMemory ( head, BMP_STUFF_SIZE );
    bh  = ( BITMAPINFOHEADER *)head;
    pal = ( RGBQUAD *)( head + sizeof ( BITMAPINFOHEADER ) );

    bh->biSize          = sizeof( BITMAPINFOHEADER );
    bh->biWidth         = width;
    bh->biHeight        = height;
    bh->biPlanes        = 1;
    bh->biBitCount      = 8;
    bh->biClrUsed       = 256;
    bh->biClrImportant  = 256;
    bh->biCompression   = BI_RGB;

    // this will be the bgcolor
    color               = GetSysColor ( COLOR_BTNFACE );        
    pal[0].rgbRed       = GetRValue ( color );
    pal[0].rgbGreen     = GetGValue ( color );
    pal[0].rgbBlue      = GetBValue ( color );
    // this will be fgcolor
    color               = GetSysColor ( COLOR_WINDOWTEXT );     
    pal[1].rgbRed       = GetRValue ( color );
    pal[1].rgbGreen     = GetGValue ( color );
    pal[1].rgbBlue      = GetBValue ( color );

    // initialize the rest of the pallette
    for ( a = 2; a < 128; a++ )                                 
    {
		pal[a].rgbGreen = 256 - 2 * a;
		pal[a].rgbRed = 2 * a;
    }

	for ( a = 0; a < 32; a++ )
    {
		pal[128 + a].rgbBlue = 8 * a;
		pal[128 + 32 + a].rgbBlue = 255;
		pal[128 + 32 + a].rgbRed = 8 * a;
		pal[128 + 64 + a].rgbRed = 255;
		pal[128 + 64 + a].rgbBlue = 8 * ( 31 - a );
		pal[128 + 64 + a].rgbGreen = 8 * a;
		pal[128 + 96 + a].rgbRed = 255;
		pal[128 + 96 + a].rgbGreen = 255;
		pal[128 + 96 + a].rgbBlue = 8 * a;
	}

    bmp = CreateDIBSection ( 0, ( BITMAPINFO *)bh, 
        DIB_RGB_COLORS, ( void **)bmpdata, NULL, 0 );
    
    if ( !bmp )
        return FALSE;

    dc  = CreateCompatibleDC ( 0 );
    
    if ( !dc )
    {
        DeleteObject ( bmp );
        return FALSE;
    }    

    SelectObject ( dc, bmp );
    *hdc    = dc;
    *hbmp   = bmp;

    return TRUE;
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: UpdateSpec 
/*--------------------------------------------------------------------------*/
//           Type: static void 
//    Param.    1: void : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: update spectrum ananlyzer
/*--------------------------------------------------------------------@@-@@-*/
static void UpdateSpec ( void )
/*--------------------------------------------------------------------------*/
{
    HDC         dc;
    int         x, y, sc, b0, b1;
    static int  y1;
    float       fft[FFTS];
    float       sum;

    if ( !g_SpecBuf || g_Minimized || g_Busy || !g_Playing || g_Paused )
        return;

    // get 1024 FFT's
    if ( BASS_ChannelGetData ( g_hStream, fft, BASS_DATA_FFT2048 | 
        BASS_DATA_FFT_REMOVEDC ) == (DWORD)-1 )
            return;

    RtlZeroMemory ( g_SpecBuf, g_SpecWidth * g_SpecHeight );

    b0 = 0;

    for ( x = 0; x < 24; x++ )
    {
        sum     = 0;
        b1      = pow ( 2, x * 10.0 / ( 24 - 1 ) );

        if ( b1 > FFTS-1 )
            b1 = FFTS-1;

        // make sure it uses at least 1 FFT bin
        if ( b1 <= b0 )
            b1 = b0 + 1; 

        sc      = 7 + b1 - b0; //5 + b1 - b0;

        for ( ; b0 < b1; b0++ )
            sum += fft [ 1 + b0 ];
        
        y = ( sqrt ( sum / log10 ( sc ) ) * 1.7 * g_SpecScaleTop ) - 4;

        if ( y > g_SpecScaleTop )
            y = g_SpecScaleTop; // cap it
        
        if ( x && ( y1 = ( y + y1 ) / 2 ) )
        {
            while ( --y1 >= 0 )
            {
                if ( y1 & 1 ) // make nice zebra :)
                {
                    g_SpecBuf[y1 * g_SpecWidth + x * 6]      = 1;
                    g_SpecBuf[y1 * g_SpecWidth + x * 6 + 1]  = 1;
                    g_SpecBuf[y1 * g_SpecWidth + x * 6 + 2]  = 0;
                    g_SpecBuf[y1 * g_SpecWidth + x * 6 + 3]  = 1;
                }
            }
        }
            
        y1 = y;
    }

    dc = GetDC ( g_hSpec );

    if ( dc )
    {
        BitBlt ( dc, 0, 0, g_SpecWidth, 
            g_SpecHeight, g_SpecDC, 0, 0, SRCCOPY );

        ReleaseDC ( g_hSpec, dc );
    }
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: UpdateScope 
/*--------------------------------------------------------------------------*/
//           Type: static void 
//    Param.    1: void : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: update scope
/*--------------------------------------------------------------------@@-@@-*/
static void UpdateScope ( void )
/*--------------------------------------------------------------------------*/
{
    HDC                 dc;
    float               * buf;
    int                 x, v, idx;
    static int          y;
    DWORD               c;
    BASS_CHANNELINFO    ci;

    if ( !g_SpecBuf || g_Minimized || g_Busy || 
        !g_Playing || g_Paused )
            return;

    if ( !BASS_ChannelGetInfo ( g_hStream, &ci ) )
        return; 

    buf = alloca ( ci.chans * g_SpecWidth * sizeof(float) );

    if ( BASS_ChannelGetData ( g_hStream, buf, 
        ( ci.chans * g_SpecWidth * sizeof(float) ) | 
            BASS_DATA_FLOAT ) == (DWORD)-1 )
                return;

    RtlZeroMemory ( g_SpecBuf, g_SpecWidth * g_SpecHeight );

	for ( c = 0; c < ci.chans; c++ ) 
    {
		for ( x = 0; x < g_SpecWidth; x++ ) 
        {
            // invert and scale to fit display
			v = ( 1 - buf[x * ci.chans + c] ) * g_SpecHeight / 2;

			if ( v < 0 )
                v = 0;
			else 
                if ( v >= g_SpecHeight )
                    v = g_SpecHeight - 1;

			if ( !x )
                y = v;
			do 
            {   // draw line from previous sample...
				if ( y < v )
                    y++;
				else
                    if ( y > v )
                        y--;

                idx = y * g_SpecWidth + x;
                // draw every odd pixel (remove for full line draw)
                // left=black, right=red (could add more colours 
                // to palette for more chans)
                if ( idx & 1 )                          
				    g_SpecBuf[idx] = c & 1 ? 127 : 1;

			}
            while ( y != v );
		}
	}

    dc = GetDC ( g_hSpec );

    if ( dc )
    {
        BitBlt ( dc, 0, 0, g_SpecWidth, 
            g_SpecHeight, g_SpecDC, 0, 0, SRCCOPY );

        ReleaseDC ( g_hSpec, dc );
    }
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: UpdateLevels 
/*--------------------------------------------------------------------------*/
//           Type: static void 
//    Param.    1: void : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: update bargraphs
/*--------------------------------------------------------------------@@-@@-*/
static void UpdateLevels ( void )
/*--------------------------------------------------------------------------*/
{
    DWORD           temp, left, right;
    static DWORD    l1, r1;
    HDC             dc;

    if ( !g_VolBuf || g_Minimized || g_Busy || 
        !g_Playing || g_Paused )
            return;

    RtlZeroMemory ( g_VolBuf, g_VolWidth * g_VolHeight );

    temp    = BASS_ChannelGetLevel ( g_hStream );
    left    = ( LOWORD ( temp ) ) >> 8;
    right   = ( HIWORD ( temp ) ) >> 8;
    
    if ( left > g_VolWidth )
        left   = g_VolWidth;

    if ( right > g_VolWidth )
        right = g_VolWidth;

    l1 = ( left + l1 ) / 2;
    r1 = ( right + r1 ) / 2;

    // "< right" - no interpolation
    for ( temp = 0; temp < r1; temp++ ) 
    {
        if ( temp & 1 ) // make nice zebra :)
        {
            g_VolBuf[0 * g_VolWidth + temp] = 1;
            g_VolBuf[1 * g_VolWidth + temp] = 1;
            g_VolBuf[2 * g_VolWidth + temp] = 0;
            g_VolBuf[3 * g_VolWidth + temp] = 1;
            g_VolBuf[4 * g_VolWidth + temp] = 1;
        }
    }

    // "< left" - no interpolation
    for ( temp = 0; temp < l1; temp++ ) 
    {
        if ( temp & 1 )
        {
            g_VolBuf[14 * g_VolWidth + temp] = 1;
            g_VolBuf[15 * g_VolWidth + temp] = 1;
            g_VolBuf[16 * g_VolWidth + temp] = 0;
            g_VolBuf[17 * g_VolWidth + temp] = 1;
            g_VolBuf[18 * g_VolWidth + temp] = 1;
        }
    }

    l1 = left;
    r1 = right;

    dc = GetDC ( g_hVol );

    if ( dc )
    {
        BitBlt ( dc, 0, 0, g_VolWidth, g_VolHeight, g_VolDC, 0, 0, SRCCOPY );
        ReleaseDC ( g_hVol, dc );
    }
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: App_Shutdown 
/*--------------------------------------------------------------------------*/
//           Type: static void 
//    Param.    1: void : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: closing time
/*--------------------------------------------------------------------@@-@@-*/
static void App_Shutdown ( void )
/*--------------------------------------------------------------------------*/
{
    HANDLE      hIOMutex;

    hIOMutex    = CreateMutex ( NULL, FALSE, NULL );
    WaitForSingleObject ( hIOMutex, INFINITE );

    g_Busy      = TRUE;

    g_SpecBuf   = NULL;
    g_VolBuf    = NULL;
    
    if ( g_hStream )
    {
        BASS_ChannelStop ( g_hStream );
        BASS_StreamFree ( g_hStream );
    }        

    BASS_Free();
    BASS_PluginFree( 0 );

    if ( g_hIml )
        ImageList_Destroy ( g_hIml );

    if ( g_SpecDC )
        DeleteDC ( g_SpecDC );

    if ( g_VolDC )
        DeleteDC ( g_VolDC );
    
    if ( g_SpecBmp )
        DeleteObject ( g_SpecBmp );

    if ( g_VolBmp )
        DeleteObject ( g_VolBmp );

    ReleaseMutex( hIOMutex);
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: App_Init 
/*--------------------------------------------------------------------------*/
//           Type: static INIT_STATUS 
//    Param.    1: HWND hDlg : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: app initialization
/*--------------------------------------------------------------------@@-@@-*/
static INIT_STATUS App_Init ( HWND hDlg )
/*--------------------------------------------------------------------------*/
{
    DWORD   vol;
    RECT    rect;
    TCHAR   temp[64];

    SendMessage ( hDlg, WM_SETICON, ICON_BIG, ( LPARAM )g_hIcon );

    g_hDevList   = GetDlgItem ( hDlg, IDC_DEVLIST );
    if ( !g_hDevList )
        return ERR_DEV_LIST;
        
    if ( !AudioDevListInit ( g_hDevList ) )
        return ERR_DEV_LIST;

    g_hSpec     = GetDlgItem ( hDlg, IDC_SPEC );
    if ( !g_hSpec )
        return ERR_SCOPE_INIT;

    g_hVol      = GetDlgItem ( hDlg, IDC_LEVELS );
    if ( !g_hVol )
        return ERR_SCOPE_INIT;

    if ( !GetClientRect ( g_hSpec, &rect ) )
        return ERR_SCOPE_INIT;

    g_SpecWidth     = 4 * ( ( rect.right * 8 + 31 ) / 32 );
    g_SpecHeight    = rect.bottom;
    g_SpecScaleTop  = g_SpecHeight-8;

    if ( !Create_DIB ( &g_SpecDC, &g_SpecBmp, &g_SpecBuf, 
            rect.right, g_SpecHeight ) )
                return ERR_DIB_INIT;

    if ( !GetClientRect ( g_hVol, &rect ) )
        return ERR_SCOPE_INIT;

    g_VolWidth      = 4 * ( ( rect.right * 8 + 31 ) / 32 );
    g_VolHeight     = rect.bottom;
    g_VolScaleTop   = g_VolHeight-8;

    if ( !Create_DIB ( &g_VolDC, &g_VolBmp, &g_VolBuf, 
            rect.right, g_VolHeight ) )
                return ERR_DIB_INIT;

    RtlZeroMemory ( g_SpecBuf, g_SpecWidth * g_SpecHeight );
    RtlZeroMemory ( g_VolBuf, g_VolWidth * g_VolHeight );

    g_hList         = GetDlgItem ( hDlg, IDC_PLAYLIST );
    if ( !g_hList )
        return ERR_SLIDE_INIT;

    g_hPosTrack     = GetDlgItem ( hDlg, IDC_POS );
    if ( !g_hPosTrack )
        return ERR_SLIDE_INIT;

    SendMessage ( g_hPosTrack, TBM_SETPAGESIZE, 0, POS_INCREMENT );

    g_hVolTrack     = GetDlgItem ( hDlg, IDC_VOLTRAK );
    if ( !g_hVolTrack )
        return ERR_SLIDE_INIT;

    g_OldListProc   = ( WNDPROC ) 
        SetWindowLongPtr ( g_hList, GWLP_WNDPROC, 
            ( LONG_PTR ) LVSubclassProc );

    if ( !g_OldListProc )
        return ERR_SUBCLASS;

    g_OldSpecProc   = ( WNDPROC ) 
        SetWindowLongPtr ( g_hSpec, GWLP_WNDPROC, 
            ( LONG_PTR ) SpecSubclassProc );

    if ( !g_OldSpecProc )
        return ERR_SUBCLASS;

    g_OldVolProc    = ( WNDPROC ) 
        SetWindowLongPtr ( g_hVol, GWLP_WNDPROC, 
            ( LONG_PTR ) VolSubclassProc );

    if ( !g_OldVolProc )
        return ERR_SUBCLASS;

    Playlist_Init ( g_hList, g_hInst, &g_hIml );
    SetWindowText ( hDlg, app_title );
    SendMessage ( g_hPosTrack, TBM_SETRANGE, TRUE, 
        (LPARAM) MAKELONG(0, MAX_RANGE) );
    SendMessage ( g_hVolTrack, TBM_SETRANGE, TRUE, 
        (LPARAM) MAKELONG(0, 100) );
    vol = BASS_GetConfig ( BASS_CONFIG_GVOL_STREAM ) / 100;
    SendMessage ( g_hVolTrack, TBM_SETPOS, TRUE, (LPARAM) vol );
    StringCchPrintf ( temp, ARRAYSIZE(temp), TEXT("%lu%%"), vol );
    SetDlgItemText ( hDlg, IDC_VOLUME, temp );
    
    if ( !Tray_Add ( hDlg, IDI_LISTICON, g_hTray, app_title ) )
        return ERR_TRAY_INIT;

    g_Timer = timeSetEvent ( UPDATE_INTERVAL, UPDATE_INTERVAL, 
        (LPTIMECALLBACK)&MP3_UpdateStatus, 0, TIME_PERIODIC );

    if ( !g_Timer )
        return ERR_TIMER_INIT;

    return ERR_SUCCESS;
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: Global_Init 
/*--------------------------------------------------------------------------*/
//           Type: static INIT_STATUS 
//    Param.    1: void : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: init global vars and BASS lib
/*--------------------------------------------------------------------@@-@@-*/
static INIT_STATUS Global_Init ( void )
/*--------------------------------------------------------------------------*/
{
    WORD        current_ver;
    TCHAR       buf[128];
    TCHAR       ** arglist;
    int         argc;
    WNDCLASSEX  wc;

    g_hInst     = GetModuleHandle ( NULL );
    g_hAccel    = LoadAccelerators ( g_hInst, MAKEINTRESOURCE ( IDR_ACC ) );

    if ( !g_hAccel )
        return ERR_INIT_ACC_TABLE;

    g_hIcon     = LoadIcon ( g_hInst, MAKEINTRESOURCE ( IDI_MAINICON ) );
    g_hTray     = LoadIcon ( g_hInst, MAKEINTRESOURCE ( IDI_LISTICON ) );
    
    if ( IsThereAnotherInstance ( APP_CLASSNAME ))
        return ERR_APP_RUNNING;

    current_ver = HIWORD ( BASS_GetVersion() );
    
    if ( current_ver != BASSVERSION )
    {
        StringCchPrintf ( buf, ARRAYSIZE(buf), g_err_messages[ERR_BASS_VER], 
            HIBYTE ( current_ver ), LOBYTE ( current_ver ), 
                HIBYTE ( BASSVERSION ), LOBYTE ( BASSVERSION ) );

        BASS_Error ( NULL, buf );

        return ERR_BASS_VER;
    }
    
    if ( !BASS_Init ( -1, 44100, 0, 0, NULL ) )
        return ERR_BASS_INIT;

    Load_BASS_Plugins();

    if ( !BASS_SetConfig ( BASS_CONFIG_BUFFER, 2500 ) )
        return ERR_BASS_INIT;

    InitCommonControls();

    // register our custom wnd class (MPL_CLASS_666)
    wc.cbSize   = sizeof (WNDCLASSEX);
    GetClassInfoEx ( 0, WC_DIALOG, &wc );
    wc.lpszClassName = APP_CLASSNAME;
    wc.style    &= ~CS_GLOBALCLASS;

    if ( !RegisterClassEx ( &wc ) )
        return ERR_DLG_CREATE;

    g_hDlg      = CreateDialog ( g_hInst, MAKEINTRESOURCE ( IDD_MAIN ), NULL, 
        ( DLGPROC )DlgProc );

    if ( !g_hDlg )
        return ERR_DLG_CREATE;

    g_hMainmenu = GetMenu ( g_hDlg );
    BASS_Start();

    arglist = FILE_CommandLineToArgv ( GetCommandLine(), &argc );

    if ( argc >= 2 && arglist != NULL )
    {
        Playlist_LoadFromCmdl ( g_hList, arglist, argc );
    }

    if ( arglist != NULL )
        GlobalFree ( arglist );

    return ERR_SUCCESS;
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: Process_WM_COPYDATA 
/*--------------------------------------------------------------------------*/
//           Type: static BOOL 
//    Param.    1: HWND hwnd     : 
//    Param.    2: WPARAM wParam : 
//    Param.    3: LPARAM lParam : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: handle WM_COPYDATA message
/*--------------------------------------------------------------------@@-@@-*/
static BOOL Process_WM_COPYDATA ( HWND hwnd, WPARAM wParam, LPARAM lParam )
/*--------------------------------------------------------------------------*/
{
    BOOL            result = FALSE;
    COPYDATASTRUCT  * pcd;
    TCHAR           ** arglist;
    int             argc;

    pcd = (COPYDATASTRUCT *)(lParam);

    if ( pcd == NULL )
        return result;

    // check for our secret preshared key with 
    // the shell ctx menu extension :-))
    if ( pcd->dwData != COPYDATA_MAGIC )
        return result;

    __try
    {
        // use the same function from cmdline processing, 
        // since the received data is the same
        arglist = FILE_CommandLineToArgv ( (TCHAR *)(pcd->lpData), &argc );

        if ( argc >= 2 && arglist != NULL )
        {
            result = Playlist_LoadFromCmdl ( g_hList, arglist, argc );

            // ok, so now let's bring our window to front...
            if ( IsIconic ( hwnd ) )
                // simulate a click on the tray icon
                PostMessage ( hwnd, WM_TRAY, (WPARAM)0, 
                    (LPARAM)WM_LBUTTONDOWN );
            else
            {
                // minimize to tray and simulate a click on the tray icon
                ShowWindow ( hwnd, SW_MINIMIZE );
                PostMessage ( hwnd, WM_TRAY, (WPARAM)0, 
                    (LPARAM)WM_LBUTTONDOWN );
            }
            // bring program window to foreground by momentarily making it topmost
            // ...but only if it isn't already 8-)
            SetWindowPos ( hwnd, HWND_TOPMOST, 0, 0, 0, 0, 
                SWP_SHOWWINDOW| SWP_NOMOVE | SWP_NOSIZE );
            SetWindowPos ( hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, 
                SWP_SHOWWINDOW| SWP_NOMOVE | SWP_NOSIZE );
            SetFocus ( g_hList );
            g_Minimized = FALSE;
        }
    }
    __except ( EXCEPTION_EXECUTE_HANDLER )
    {
        result = FALSE;
    }

    if ( arglist != NULL )
        GlobalFree ( arglist );

    return result;
}


/*-@@+@@--------------------------------------------------------------------*/
//       Function: Process_WM_COMMAND 
/*--------------------------------------------------------------------------*/
//           Type: static void 
//    Param.    1: HWND hDlg     : 
//    Param.    2: WPARAM wParam : 
//    Param.    3: LPARAM lParam : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: process WM_COMMAND message
/*--------------------------------------------------------------------@@-@@-*/
static void Process_WM_COMMAND ( HWND hDlg, WPARAM wParam, LPARAM lParam )
/*--------------------------------------------------------------------------*/
{
    DWORD   ver;
    TCHAR   temp[256];

    if ( LOWORD ( wParam ) == IDCANCEL )
    {
        SendMessage ( hDlg, WM_CLOSE, 0, 0 ); 
        return; 
    }

    if ( LOWORD ( wParam ) == IDC_DEVLIST )
    {
        if ( HIWORD ( wParam ) == LBN_DBLCLK )
        {
            if ( !AudioDevChange ( g_hDevList ) )
            {
                BASS_Error ( hDlg, g_err_messages[ERR_CHG_PBDEV] );
                return;
            }
        }
    }

    if ( !lParam )
    {
        switch ( LOWORD ( wParam ) )
        {
            case IDM_QUIT:
                SendMessage ( hDlg, WM_CLOSE, 0, 0 );
                break;

            case IDM_OPEN:
                Playlist_Open ( g_hList, hDlg, g_hInst );
                break;

            case IDM_SAVE:
                Playlist_Save ( g_hList, hDlg, g_hInst );
                break;

            case IDM_ADD:
                Playlist_Add ( g_hList, hDlg, g_hInst );
                break;

            case IDM_PLAY:
                if ( !MP3_Play() )
                    BASS_Error ( hDlg, g_err_messages[ERR_PB_START] );
                break;

            case IDM_STOP:
                MP3_Stop();
                break;

            case IDM_PAUSE:
                MP3_Pause();
                break;

            case IDM_NEXT:
                if ( !MP3_Next() )
                    BASS_Error ( hDlg, g_err_messages[ERR_PB_START] );
                break;

            case IDM_FFW:
                MP3_Ffw();
                break;

            case IDM_REW:
                MP3_Rew();
                break;

            case IDM_CLEAR:
                if ( g_Paused )
                    MP3_Stop();

                LVClear ( g_hList );
                g_CurIndex = 0;
                g_LastPlayed = -1;
                break;

            case IDM_SELALL:
                LVSelectAll ( g_hList );
                break;

            case IDM_DEL:
                LVDeleteSelection ( g_hList );
                if ( g_CurIndex > LVGetCount ( g_hList ) )
                {
                    g_CurIndex = 0;

                    if ( g_Paused )
                        MP3_Stop();
                }
                break;

            case IDM_KEYHLP:
                ShowMessage ( hDlg, hlp_txt, MB_OK );
                break;

            case IDM_ABOUT:
                ver = HIWORD ( BASS_GetVersion() );
                StringCchPrintf ( temp, ARRAYSIZE(temp), 
                    about_txt, HIBYTE ( ver ), LOBYTE ( ver ) );
                ShowMessage ( hDlg, temp, MB_OK );
                break;
        }
    }

    return;
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: Process_WM_NOTIFY 
/*--------------------------------------------------------------------------*/
//           Type: static void 
//    Param.    1: HWND hDlg     : 
//    Param.    2: WPARAM wParam : 
//    Param.    3: LPARAM lParam : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: process WM_NOTIFY message 
/*--------------------------------------------------------------------@@-@@-*/
static void Process_WM_NOTIFY ( HWND hDlg, WPARAM wParam, LPARAM lParam )
/*--------------------------------------------------------------------------*/
{
    if ( wParam == IDC_PLAYLIST )
    {
        switch ( ( ( NMHDR * )lParam )->code )
        {
            case NM_DBLCLK:
                if ( LVGetSelIndex ( g_hList ) != -1 )
                {
                    if ( !MP3_Play() )
                        BASS_Error ( hDlg, g_err_messages[ERR_PB_START] );
                }
                break;

            case NM_RCLICK:
                ToggleMenus ( GetSubMenu ( g_hMainmenu, 1 ) );
                ContextMenu ( hDlg, TPM_LEFTALIGN|TPM_RIGHTBUTTON );
                break;

            case LVN_BEGINDRAG:
                g_Dragging = TRUE;
                g_DragStart = ( ( NMLISTVIEW *) lParam )->iItem;
                g_DragPos = g_DragStart;
                LVGetItemText ( g_hList, g_DragStart, 0, 
                    g_Item, ARRAYSIZE(g_Item) );
                LVGetItemText ( g_hList, g_DragStart, 1, 
                    g_SubItem, ARRAYSIZE(g_SubItem) );
                g_ItemCount = LVGetCount ( g_hList );
                g_ItemImgIdx = LVGetItemImgIdx ( g_hList, g_DragStart );
                SetCapture ( g_hList );
                break;
        }
    }
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: Process_WM_HSCROLL 
/*--------------------------------------------------------------------------*/
//           Type: static void 
//    Param.    1: HWND hDlg     : 
//    Param.    2: WPARAM wParam : 
//    Param.    3: LPARAM lParam : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: process WM_HSCROLL message
/*--------------------------------------------------------------------@@-@@-*/
static void Process_WM_HSCROLL ( HWND hDlg, WPARAM wParam, LPARAM lParam )
/*--------------------------------------------------------------------------*/
{
    TCHAR       temp[32];
    INT_PTR     vol;

    if ( (HWND)lParam == g_hPosTrack )
    {
        switch ( LOWORD ( wParam ) )
        {
            case TB_THUMBTRACK:

                if ( g_Playing || !g_Paused )
                    g_Busy = TRUE;

                break;

            case TB_BOTTOM:
            case TB_TOP:
            case TB_PAGEDOWN:
            case TB_PAGEUP:
            case TB_LINEDOWN:
            case TB_LINEUP:
            case TB_THUMBPOSITION:

                if ( g_Playing || !g_Paused )
                    MP3_SetPos();

                break;
        }
        return;
    }
    if ( (HWND)lParam == g_hVolTrack )
    {
        switch ( LOWORD  (wParam ) )
        {
            case TB_PAGEDOWN:
            case TB_PAGEUP:
            case TB_LINEDOWN:
            case TB_LINEUP:
            case TB_BOTTOM:
            case TB_TOP:
            case TB_THUMBTRACK:
            case TB_THUMBPOSITION:
                vol = SendMessage ( g_hVolTrack, TBM_GETPOS, 0, 0 );
                BASS_SetConfig ( BASS_CONFIG_GVOL_STREAM, (DWORD)(vol * 100));
                StringCchPrintf ( temp, ARRAYSIZE(temp), TEXT("%zd%%"), vol);
                SetDlgItemText ( hDlg, IDC_VOLUME, temp );
                break;
        }
    }
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: Process_WM_TRAY 
/*--------------------------------------------------------------------------*/
//           Type: static void 
//    Param.    1: HWND hDlg     : 
//    Param.    2: WPARAM wParam : 
//    Param.    3: LPARAM lParam : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: process WM_TRAY message
/*--------------------------------------------------------------------@@-@@-*/
static void Process_WM_TRAY ( HWND hDlg, WPARAM wParam, LPARAM lParam )
/*--------------------------------------------------------------------------*/
{
    switch ( lParam )
    {
        case WM_LBUTTONDOWN:
            SetForegroundWindow ( hDlg );
            g_Minimized = FALSE;
            ShowWindow ( hDlg, SW_RESTORE );
            break;

        case WM_RBUTTONUP:
            SetForegroundWindow ( hDlg );
            ToggleMenus ( GetSubMenu ( g_hMainmenu, 1 ) );
            ContextMenu ( hDlg, TPM_LEFTALIGN|TPM_RIGHTBUTTON );
            PostMessage ( hDlg, WM_NULL, 0, 0 );
            break;
    }
}
