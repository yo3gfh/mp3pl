
#ifndef _MISC_H
#define _MISC_H

typedef enum {
    ERR_INIT_ACC_TABLE,                          //0
    ERR_APP_RUNNING,                             //1
    ERR_BASS_VER,                                //2
    ERR_BASS_INIT,                               //3
    ERR_DLG_CREATE,                              //4
    ERR_DEV_LIST,                                //5
    ERR_SCOPE_INIT,                              //6
    ERR_DIB_INIT,                                //7
    ERR_SLIDE_INIT,                              //8
    ERR_SUBCLASS,                                //9
    ERR_TRAY_INIT,                               //10
    ERR_TIMER_INIT,                              //11
    ERR_CHG_PBDEV,                               //12
    ERR_PB_START,                                //13
    ERR_SUCCESS
} INIT_STATUS;


extern const TCHAR  * app_title;
extern const TCHAR  * about_txt;
extern const TCHAR  * hlp_txt;
extern const TCHAR  * g_err_messages[];

extern const TCHAR * Extract_path ( const TCHAR * src, BOOL last_bslash );
extern int ShowMessage ( HWND howner, const TCHAR * message, DWORD style );
extern BOOL Load_BASS_Plugins ( void );
extern void BASS_Error ( HWND howner, const TCHAR * message );
extern BOOL IsThereAnotherInstance ( const TCHAR * classname );
extern TCHAR ** FILE_CommandLineToArgv ( TCHAR * CmdLine, int * _argc );

#endif
