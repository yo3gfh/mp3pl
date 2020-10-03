
#pragma warn(disable: 2008 2118 2228 2231 2030 2260)

#include        <windows.h>

/*-@@+@@--------------------------------------------------------------------*/
//       Function: BeginDraw 
/*--------------------------------------------------------------------------*/
//           Type: void 
//    Param.    1: HWND hwnd : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: disable paint on lengthy operations 
//                 (list updates, for example)
/*--------------------------------------------------------------------@@-@@-*/
void BeginDraw ( HWND hwnd )
/*--------------------------------------------------------------------------*/
{
    SendMessage ( hwnd, WM_SETREDRAW, FALSE, 0 );
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: EndDraw 
/*--------------------------------------------------------------------------*/
//           Type: void 
//    Param.    1: HWND hwnd : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: reenable paint processing after a lenghty operation
/*--------------------------------------------------------------------@@-@@-*/
void EndDraw ( HWND hwnd )
/*--------------------------------------------------------------------------*/
{
    SendMessage ( hwnd, WM_SETREDRAW, TRUE, 0 );
    InvalidateRect ( hwnd, NULL, TRUE );
}
