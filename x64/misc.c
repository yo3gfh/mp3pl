
#pragma warn(disable: 2008 2118 2228 2231 2030 2260)

#include        <windows.h>
#include        <strsafe.h>
#include        <bass.h>
#include        "resource.h"


const TCHAR * app_title     = 
    TEXT("Lil' MP3 player");

const TCHAR * about_txt     = 
    TEXT("Lil' MP3 player")
    TEXT("\t\tCopyright (c) 2003-2020, Adrian Petrila\n")
    TEXT("BASS sound system v%d.%d\tCopyright (c) 1999-2019, Ian Luck\n");

const TCHAR * hlp_txt       = 
    TEXT("Space, Enter, dblclick\t\tPlay/Resume\n")
    TEXT("Esc\t\t\tStop playback\n")
    TEXT("Ctrl+P\t\t\tPause/Resume\n")
    TEXT("Ctrl+B\t\t\tNext track\n")
    TEXT("Ctrl+Left Arrow\t\tRewind\n")
    TEXT("Ctrl+Right Arrow\t\tFast-forward\n\n")
    TEXT("Ctrl+O\t\t\tOpen playlist\n")
    TEXT("Ctrl+Ins\t\t\tAdd files to playlist\n")
    TEXT("Ctrl+S\t\t\tSave playlist\n")
    TEXT("Ctrl+A\t\t\tSelect all\n")
    TEXT("Shift+Del\t\t\tClear all\n")
    TEXT("Del\t\t\tDelete selected");

const TCHAR * g_err_messages[] = { 
    TEXT("Can't load accelerator table!"),                              //0
    TEXT("Program already up and running :)"),                          //1     
    TEXT("Wrong bass.dll version (%d.%d) - expecting %d.%d"),           //2
    TEXT("Failed to initialize sound system!"),                         //3
    TEXT("Can't create main window!"),                                  //4
    TEXT("Error initializing device list!"),                            //5
    TEXT("Error initializing volume/spectrum display!"),                //6
    TEXT("Error creating DIB for volume/spectrum display!"),            //7
    TEXT("Error initializing volume/position sliders!"),                //8
    TEXT("Error subclassing controls!"),                                //9
    TEXT("Error adding icon to systray!"),                              //10
    TEXT("Error initializing multimedia timer!"),                       //11
    TEXT("Unable to switch to this playback device!"),                  //12
    TEXT("Unable to start playback!")                                   //13
};

/*-@@+@@--------------------------------------------------------------------*/
//       Function: Extract_path 
/*--------------------------------------------------------------------------*/
//           Type: const TCHAR * 
//    Param.    1: const TCHAR * src: 
//    Param.    2: BOOL last_bslash : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: make a path from a filename
/*--------------------------------------------------------------------@@-@@-*/
const TCHAR * Extract_path ( const TCHAR * src, BOOL last_bslash )
/*--------------------------------------------------------------------------*/
{
    DWORD           idx;
    static TCHAR    temp[MAX_PATH];

    if ( src == NULL )
        return NULL;

    idx = lstrlen ( src )-1;

    if ( idx >= MAX_PATH )
        return NULL;

    while ( ( src[idx] != TEXT('\\') ) && ( idx != 0 ) )
        idx--;

    if ( idx == 0 )
        return NULL;

    if ( last_bslash )
        idx++;

    StringCchCopyN ( temp, ARRAYSIZE(temp), src, idx );

    return temp;
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: ShowMessage 
/*--------------------------------------------------------------------------*/
//           Type: int 
//    Param.    1: HWND howner          : 
//    Param.    2: const TCHAR * message: 
//    Param.    3: DWORD style          : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: custom messagebox
/*--------------------------------------------------------------------@@-@@-*/
int ShowMessage ( HWND howner, const TCHAR * message, DWORD style )
/*--------------------------------------------------------------------------*/
{
    MSGBOXPARAMS    mp;

    mp.cbSize               = sizeof ( mp );
    mp.dwStyle              = MB_USERICON | style;
    mp.hInstance            = GetModuleHandle ( NULL );
    mp.lpszIcon             = MAKEINTRESOURCE ( IDI_MAINICON );
    mp.hwndOwner            = howner;
    mp.lpfnMsgBoxCallback   = NULL;
    mp.lpszCaption          = app_title;
    mp.lpszText             = message;
    mp.dwContextHelpId      = 0;
    mp.dwLanguageId         = MAKELANGID ( LANG_NEUTRAL, SUBLANG_DEFAULT );
    return MessageBoxIndirect ( &mp );
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: Load_BASS_Plugins 
/*--------------------------------------------------------------------------*/
//           Type: BOOL 
//    Param.    1: void : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: load any present plugins
/*--------------------------------------------------------------------@@-@@-*/
BOOL Load_BASS_Plugins ( void )
/*--------------------------------------------------------------------------*/
{
    WIN32_FIND_DATA     fd;
    HANDLE              fh;
    TCHAR               path[MAX_PATH], temp[MAX_PATH];
    #ifdef UNICODE
    char                plug_name[MAX_PATH];
    #endif

    GetModuleFileName ( 0, path, ARRAYSIZE( path ));
    StringCchCopyN ( temp, ARRAYSIZE(temp), 
        Extract_path ( path, TRUE ), MAX_PATH-1 );
    StringCchCat ( temp, ARRAYSIZE(temp), TEXT("bass*.dll") );

    fh = FindFirstFile ( temp, &fd );

    if ( fh == INVALID_HANDLE_VALUE )
        return FALSE;

    do
    {
        #ifdef UNICODE
        WideCharToMultiByte ( CP_ACP, 0, fd.cFileName, -1, 
            plug_name, sizeof ( plug_name ), NULL, NULL );
        BASS_PluginLoad ( plug_name, 0 );
        #else
        BASS_PluginLoad ( fd.cFileName, 0 );
        #endif    
    }
    while ( FindNextFile ( fh, &fd ) );

    FindClose ( fh );
    return TRUE;
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: BASS_Error 
/*--------------------------------------------------------------------------*/
//           Type: void 
//    Param.    1: HWND howner           : 
//    Param.    2: const TCHAR * message : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: fetch last BASS error
/*--------------------------------------------------------------------@@-@@-*/
void BASS_Error ( HWND howner, const TCHAR * message )
/*--------------------------------------------------------------------------*/
{
    TCHAR   temp[128];
#ifdef UNICODE
    StringCchPrintf ( temp, ARRAYSIZE(temp), 
        L"%ls\nError code: %d", message, BASS_ErrorGetCode() );
#else
    StringCchPrintf ( temp, ARRAYSIZE(temp), 
        "%s\nError code: %d", message, BASS_ErrorGetCode() );
#endif
    ShowMessage ( howner, temp, MB_OK );
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: IsThereAnotherInstance 
/*--------------------------------------------------------------------------*/
//           Type: BOOL 
//    Param.    1: const TCHAR * classname : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: Is there another :-)
/*--------------------------------------------------------------------@@-@@-*/
BOOL IsThereAnotherInstance ( const TCHAR * classname )
/*--------------------------------------------------------------------------*/
{
    return ( FindWindow ( classname, NULL ) != NULL );
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: FILE_CommandLineToArgv
/*--------------------------------------------------------------------------*/
//           Type: TCHAR **
//    Param.    1: TCHAR * CmdLine: pointer to app commandline
//    Param.    2: int * _argc    : pointer to var to receive arg. count
/*--------------------------------------------------------------------------*/
//         AUTHOR: Thanks to Alexander A. Telyatnikov,
//                 http://alter.org.ua/en/docs/win/args/
//           DATE: 28.09.2020
//    DESCRIPTION: Nice little function! Takes a command line and breaks it
//                 into null term. strings (_argv) indexed by an array of
//                 pointers (argv). All necessary mem. is allocated
//                 dynamically, _argv and argv being stored one after the
//                 other. It returns the pointers table, which you must free
//                 with GlobalFree after getting the job done.
/*--------------------------------------------------------------------@@-@@-*/
TCHAR ** FILE_CommandLineToArgv ( TCHAR * CmdLine, int * _argc )
/*--------------------------------------------------------------------------*/
{
    TCHAR       ** argv;
    TCHAR       *  _argv;
    ULONG       len;
    ULONG       argc;
    TCHAR       a;
    ULONG       i, j;

    BOOLEAN     in_QM;
    BOOLEAN     in_TEXT;
    BOOLEAN     in_SPACE;

    if ( CmdLine == NULL || _argc == NULL )
        return NULL;

    len = lstrlen ( CmdLine );
    i = ((len+2)/2)*sizeof(PVOID) + sizeof(PVOID);

    argv = (TCHAR**)GlobalAlloc(GMEM_FIXED, 
        (i + (len+2)*sizeof(TCHAR) + 4096 + 1) & (~4095) );

    if ( argv == NULL )
        return NULL; 

    _argv = (TCHAR*)(((UCHAR*)argv)+i);

    argc = 0;
    argv[argc] = _argv;
    in_QM = FALSE;
    in_TEXT = FALSE;
    in_SPACE = TRUE;
    i = 0;
    j = 0;

    while( a = CmdLine[i] ) {
        if(in_QM) {
            if(a == TEXT('\"')) {
                in_QM = FALSE;
            } else {
                _argv[j] = a;
                j++;
            }
        } else {
            switch(a) {
            case TEXT('\"'):
                in_QM = TRUE;
                in_TEXT = TRUE;
                if(in_SPACE) {
                    argv[argc] = _argv+j;
                    argc++;
                }
                in_SPACE = FALSE;
                break;
            case TEXT(' '):
            case TEXT('\t'):
            case TEXT('\n'):
            case TEXT('\r'):
                if(in_TEXT) {
                    _argv[j] = TEXT('\0');
                    j++;
                }
                in_TEXT = FALSE;
                in_SPACE = TRUE;
                break;
            default:
                in_TEXT = TRUE;
                if(in_SPACE) {
                    argv[argc] = _argv+j;
                    argc++;
                }
                _argv[j] = a;
                j++;
                in_SPACE = FALSE;
                break;
            }
        }
        i++;
    }
    _argv[j] = TEXT('\0');
    argv[argc] = NULL;

    (*_argc) = argc;
    return argv;
}
