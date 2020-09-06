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
        
    Please note that this is cca. 20 years old code, not particullary something
    to write home about :-))
    
    It's taylored to my own needs, modify it to suit your own. I'm not a professional programmer,
    so this isn't the best code you'll find on the web, you have been warned :-))

    All the bugs are guaranteed to be genuine, and are exclusively mine =)
*/
#pragma warn(disable: 2008 2118 2228 2231 2030 2260)

#include        <windows.h>
#include        <commctrl.h>
#include        "resource.h"

//
// listview management functions
//

int LVInsertColumn ( HWND hList, int nCol, const TCHAR * lpszColumnHeading, int nFormat, int nWidth, int nSubItem )
/*****************************************************************************************************************/
/* insert a column into a LV control                                                                             */
{
    LVCOLUMN column;

    column.mask     = LVCF_TEXT|LVCF_FMT;
    column.pszText  = (TCHAR *)lpszColumnHeading;
    column.fmt      = nFormat;
    if ( nWidth != -1 )
    {
        column.mask |= LVCF_WIDTH;
        column.cx = nWidth;
    }
    if ( nSubItem != -1 )
    {
        column.mask |= LVCF_SUBITEM;
        column.iSubItem = nSubItem;
    }
    return ( int ) SendMessage ( hList, LVM_INSERTCOLUMN, nCol, ( LPARAM )&column );
}

int LVInsertItem ( HWND hList, int nItem, int nImgIndex, const TCHAR * lpszItem )
/*****************************************************************************************************************/
/* insert an item into a LV control                                                                              */
{
    LVITEM  item;
    
    item.mask       = LVIF_TEXT;
    if ( nImgIndex != -1 ) { item.mask |= LVIF_IMAGE; }
    item.iItem      = nItem;
    item.iSubItem   = 0;
    item.pszText    = (TCHAR *)lpszItem;
    item.state      = 0;
    item.stateMask  = 0;
    item.iImage     = nImgIndex;
    item.lParam     = 0;

    return ( int ) SendMessage ( hList, LVM_INSERTITEM, 0, ( LPARAM )&item );
}

BOOL LVSetItemText ( HWND hList, int nItem, int nSubItem, const TCHAR * lpszText )
/*****************************************************************************************************************/
/* set item text                                                                                                 */
{
    LVITEM  item;

    item.iSubItem   = nSubItem;
    item.pszText    = (TCHAR *)lpszText;
    return ( BOOL ) SendMessage ( hList, LVM_SETITEMTEXT, nItem, ( LPARAM )&item );
}

BOOL LVSetItemImg ( HWND hList, int nItem, int nImgidx )
/*****************************************************************************************************************/
/* set item icon                                                                                                 */
{
    LVITEM  item;

    item.iItem      = nItem;
    item.iImage     = nImgidx;
    item.iSubItem   = 0;
    item.mask       = LVIF_IMAGE;
    return ( BOOL ) SendMessage ( hList, LVM_SETITEM, 0, ( LPARAM )&item );
}

int LVGetItemImgIdx ( HWND hList, int nItem )
/*****************************************************************************************************************/
/* get a item image index                                                                                        */
{
    LVITEM  item;

    item.iItem      = nItem;
    item.iSubItem   = 0;
    item.mask       = LVIF_IMAGE;
    
    if ( SendMessage ( hList, LVM_GETITEM, 0, ( LPARAM )&item ) )
        return item.iImage;

    return -1;
}

BOOL LVGetItemText ( HWND hList, int nItem, int nSubItem, TCHAR * lpszText, int size )
/*****************************************************************************************************************/
/* get the text from a LV item                                                                                   */
{
    LVITEM  item;

    item.iSubItem   = nSubItem;
    item.cchTextMax = size;
    item.pszText    = lpszText;
    return (BOOL) SendMessage(hList, LVM_GETITEMTEXT, nItem, ( LPARAM )&item);
}

BOOL LVClear ( HWND hList )
/*****************************************************************************************************************/
/* clear all items                                                                                               */
{
    return (BOOL)SendMessage ( hList, LVM_DELETEALLITEMS, 0, 0 );
}

int LVGetCount ( HWND hList )
/*****************************************************************************************************************/
/* item count in a LV control                                                                                    */
{
    return (int)SendMessage ( hList, LVM_GETITEMCOUNT, 0, 0 );
}

int LVGetSelIndex ( HWND hList )
/*****************************************************************************************************************/
/* which item is selected                                                                                        */
{
    int         i, count;
    INT_PTR     state;

    count = LVGetCount ( hList );
    
    for ( i = 0; i < count; i++ )
    {
        state = SendMessage ( hList, LVM_GETITEMSTATE, i, LVIS_SELECTED );
        if ( state & LVIS_SELECTED ) { return i; }
    }
    return -1;
}

BOOL LVDeleteItem ( HWND hList, int item )
/*****************************************************************************************************************/
/* remove an item                                                                                                */
{
    return ( BOOL )SendMessage ( hList, LVM_DELETEITEM, item, 0 );
}

void LVDeleteSelection ( HWND hList )
/*****************************************************************************************************************/
/* remove a bunch of selected items                                                                              */
{
    int         i, count;
    INT_PTR     state;

    i = 0;
    count = LVGetCount ( hList );

    do
    {
        state = SendMessage ( hList, LVM_GETITEMSTATE, i, LVIS_SELECTED );
        if ( state & LVIS_SELECTED )
        {
            LVDeleteItem ( hList, i );
            count--;
        }
        else
            i++;
    } while ( i < count );
}

void LVSelectAll ( HWND hList )
/*****************************************************************************************************************/
/* select all items                                                                                              */
{
    LVITEM  item;

    item.stateMask  = LVIS_SELECTED;
    item.state      = LVIS_SELECTED;

    SendMessage ( hList, LVM_SETITEMSTATE, (WPARAM)-1, ( LPARAM )&item );
}

void LVSelectItem ( HWND hList, int index )
/*****************************************************************************************************************/
/* select one                                                                                                    */
{
    LVITEM  item;

    item.stateMask  = LVIS_SELECTED | LVIS_FOCUSED;
    item.state      = LVIS_SELECTED | LVIS_FOCUSED;

    SendMessage ( hList, LVM_SETITEMSTATE, index, ( LPARAM )&item );
}

void LVFocusItem ( HWND hList, int index )
/*****************************************************************************************************************/
/* focus item                                                                                                    */
{
    LVITEM  item;

    item.stateMask  = LVIS_FOCUSED;
    item.state      = LVIS_FOCUSED;

    SendMessage ( hList, LVM_SETITEMSTATE, index, ( LPARAM )&item );
}


void LVUnselectItem ( HWND hList, int index )
/*****************************************************************************************************************/
/* unselect one                                                                                                  */
{
    LVITEM  item;

    item.stateMask  =  LVIS_SELECTED;
    item.state      &= ~LVIS_SELECTED;

    SendMessage ( hList, LVM_SETITEMSTATE, index, ( LPARAM )&item );
}

BOOL LVEnsureVisible ( HWND hList, int index )
/*****************************************************************************************************************/
/* scroll a bit to ensure item visibility                                                                        */
{
    return (BOOL)SendMessage ( hList, LVM_ENSUREVISIBLE, index, (LPARAM)FALSE );
}

HIMAGELIST InitImgList ( HINSTANCE hInst )
/*****************************************************************************************************************/
/* make IMG list for the LV, from resource icons                                                                 */
{
    HIMAGELIST  img;

    img = ImageList_Create ( 16, 16, ILC_COLOR32 | ILC_MASK, 0, 2 );

    if ( !img ) { return NULL; }

    if ( ImageList_AddIcon ( img, LoadIcon ( hInst, MAKEINTRESOURCE ( IDI_LISTICON ) ) ) == -1 ) return NULL;
    if ( ImageList_AddIcon ( img, LoadIcon ( hInst, MAKEINTRESOURCE ( IDI_PLAYICON ) ) ) == -1 ) return NULL;

    return img;
}


