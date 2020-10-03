
#pragma warn(disable: 2008 2118 2228 2231 2030 2260)

#include        <windows.h>
#include        <commctrl.h>
#include        "resource.h"

//
// listview management functions
//

/*-@@+@@--------------------------------------------------------------------*/
//       Function: LVInsertColumn 
/*--------------------------------------------------------------------------*/
//           Type: int 
//    Param.    1: HWND hList                     : Listview item
//    Param.    2: int nCol                       : col. index
//    Param.    3: const TCHAR * lpszColumnHeading: col. title
//    Param.    4: int nFormat                    : indentation
//    Param.    5: int nWidth                     : col. width
//    Param.    6: int nSubItem                   : index of subitem
//                                                  assoc. wth column
//                                                  (-1 if none)
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: Insert a column into a LV control
/*--------------------------------------------------------------------@@-@@-*/
int LVInsertColumn ( HWND hList, int nCol, const TCHAR * lpszColumnHeading, 
    int nFormat, int nWidth, int nSubItem )
/*--------------------------------------------------------------------------*/
{
    LVCOLUMN column;

    column.mask         = LVCF_TEXT|LVCF_FMT;
    column.pszText      = (TCHAR *)lpszColumnHeading;
    column.cchTextMax   = lstrlen ( lpszColumnHeading );
    column.fmt          = nFormat;

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

    return (int)
        SendMessage ( hList, LVM_INSERTCOLUMN, nCol, ( LPARAM )&column );
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: LVInsertItem 
/*--------------------------------------------------------------------------*/
//           Type: int 
//    Param.    1: HWND hList             : listview item
//    Param.    2: int nItem              : item index
//    Param.    3: int nImgIndex          : img. list index
//    Param.    4: const TCHAR * lpszItem : item caption
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: insert an item into a LV control
/*--------------------------------------------------------------------@@-@@-*/
int LVInsertItem ( HWND hList, int nItem, 
    int nImgIndex, const TCHAR * lpszItem )
/*--------------------------------------------------------------------------*/
{
    LVITEM  item;
    
    item.mask       = LVIF_TEXT;

    if ( nImgIndex != -1 )
        item.mask |= LVIF_IMAGE;

    item.iItem      = nItem;
    item.iSubItem   = 0;
    item.pszText    = (TCHAR *)lpszItem;
    item.state      = 0;
    item.stateMask  = 0;
    item.iImage     = nImgIndex;
    item.lParam     = 0;

    return (int)
        SendMessage ( hList, LVM_INSERTITEM, 0, ( LPARAM )&item );
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: LVSetItemText 
/*--------------------------------------------------------------------------*/
//           Type: BOOL 
//    Param.    1: HWND hList             : listview control
//    Param.    2: int nItem              : item index
//    Param.    3: int nSubItem           : subitem index
//    Param.    4: const TCHAR * lpszText : item text
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: set item text
/*--------------------------------------------------------------------@@-@@-*/
BOOL LVSetItemText ( HWND hList, int nItem, 
    int nSubItem, const TCHAR * lpszText )
/*--------------------------------------------------------------------------*/
{
    LVITEM  item;

    item.iSubItem   = nSubItem;
    item.pszText    = (TCHAR *)lpszText;

    return (BOOL)
        SendMessage ( hList, LVM_SETITEMTEXT, nItem, ( LPARAM )&item );
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: LVSetItemImg 
/*--------------------------------------------------------------------------*/
//           Type: BOOL 
//    Param.    1: HWND hList  : listview control
//    Param.    2: int nItem   : item index
//    Param.    3: int nImgidx : img. list index
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: set item icon
/*--------------------------------------------------------------------@@-@@-*/
BOOL LVSetItemImg ( HWND hList, int nItem, int nImgidx )
/*--------------------------------------------------------------------------*/
{
    LVITEM  item;

    item.iItem      = nItem;
    item.iImage     = nImgidx;
    item.iSubItem   = 0;
    item.mask       = LVIF_IMAGE;
    return ( BOOL ) SendMessage ( hList, LVM_SETITEM, 0, ( LPARAM )&item );
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: LVGetItemImgIdx 
/*--------------------------------------------------------------------------*/
//           Type: int 
//    Param.    1: HWND hList: listview control
//    Param.    2: int nItem : item index
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: get a item image index
/*--------------------------------------------------------------------@@-@@-*/
int LVGetItemImgIdx ( HWND hList, int nItem )
/*--------------------------------------------------------------------------*/
{
    LVITEM  item;

    item.iItem      = nItem;
    item.iSubItem   = 0;
    item.mask       = LVIF_IMAGE;
    
    if ( SendMessage ( hList, LVM_GETITEM, 0, ( LPARAM )&item ) )
        return item.iImage;

    return -1;
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: LVGetItemText 
/*--------------------------------------------------------------------------*/
//           Type: BOOL 
//    Param.    1: HWND hList      : listview control
//    Param.    2: int nItem       : item index
//    Param.    3: int nSubItem    : subitem index
//    Param.    4: TCHAR * lpszText: buffer to receive text
//    Param.    5: int size        : buffer size, in chars
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: get the text from a LV item
/*--------------------------------------------------------------------@@-@@-*/
BOOL LVGetItemText ( HWND hList, int nItem, 
    int nSubItem, TCHAR * lpszText, int cchMax )
/*--------------------------------------------------------------------------*/
{
    LVITEM  item;

    item.iSubItem   = nSubItem;
    item.cchTextMax = cchMax;
    item.pszText    = lpszText;

    return (BOOL)
        SendMessage(hList, LVM_GETITEMTEXT, nItem, ( LPARAM )&item);
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: LVClear 
/*--------------------------------------------------------------------------*/
//           Type: BOOL 
//    Param.    1: HWND hList : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: clear all items
/*--------------------------------------------------------------------@@-@@-*/
BOOL LVClear ( HWND hList )
/*--------------------------------------------------------------------------*/
{
    return (BOOL)SendMessage ( hList, LVM_DELETEALLITEMS, 0, 0 );
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: LVGetCount 
/*--------------------------------------------------------------------------*/
//           Type: int 
//    Param.    1: HWND hList : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: item count in a LV control
/*--------------------------------------------------------------------@@-@@-*/
int LVGetCount ( HWND hList )
/*--------------------------------------------------------------------------*/
{
    return (int)SendMessage ( hList, LVM_GETITEMCOUNT, 0, 0 );
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: LVGetSelIndex 
/*--------------------------------------------------------------------------*/
//           Type: int 
//    Param.    1: HWND hList : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: which item is selected
/*--------------------------------------------------------------------@@-@@-*/
int LVGetSelIndex ( HWND hList )
/*--------------------------------------------------------------------------*/
{
    int         i, count;
    INT_PTR     state;

    count = LVGetCount ( hList );
    
    for ( i = 0; i < count; i++ )
    {
        state = SendMessage ( hList, LVM_GETITEMSTATE, i, LVIS_SELECTED );

        if ( state & LVIS_SELECTED )
            return i;
    }

    return -1;
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: LVDeleteItem 
/*--------------------------------------------------------------------------*/
//           Type: BOOL 
//    Param.    1: HWND hList: 
//    Param.    2: int item  : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: remove an item
/*--------------------------------------------------------------------@@-@@-*/
BOOL LVDeleteItem ( HWND hList, int item )
/*--------------------------------------------------------------------------*/
{
    return ( BOOL )SendMessage ( hList, LVM_DELETEITEM, item, 0 );
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: LVDeleteSelection 
/*--------------------------------------------------------------------------*/
//           Type: void 
//    Param.    1: HWND hList : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: remove a bunch of selected items
/*--------------------------------------------------------------------@@-@@-*/
void LVDeleteSelection ( HWND hList )
/*--------------------------------------------------------------------------*/
{
    int         i, count;
    INT_PTR     state;

    i           = 0;
    count       = LVGetCount ( hList );

    do
    {
        state   = SendMessage ( hList, LVM_GETITEMSTATE, i, LVIS_SELECTED );

        if ( state & LVIS_SELECTED )
        {
            LVDeleteItem ( hList, i );
            count--;
        }
        else
            i++;

    } 
    while ( i < count );
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: LVSelectAll 
/*--------------------------------------------------------------------------*/
//           Type: void 
//    Param.    1: HWND hList : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: select all items
/*--------------------------------------------------------------------@@-@@-*/
void LVSelectAll ( HWND hList )
/*--------------------------------------------------------------------------*/
{
    LVITEM  item;

    item.stateMask  = LVIS_SELECTED;
    item.state      = LVIS_SELECTED;

    SendMessage ( hList, LVM_SETITEMSTATE, (WPARAM)-1, ( LPARAM )&item );
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: LVSelectItem 
/*--------------------------------------------------------------------------*/
//           Type: void 
//    Param.    1: HWND hList: 
//    Param.    2: int index : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: select one
/*--------------------------------------------------------------------@@-@@-*/
void LVSelectItem ( HWND hList, int index )
/*--------------------------------------------------------------------------*/
{
    LVITEM  item;

    item.stateMask  = LVIS_SELECTED | LVIS_FOCUSED;
    item.state      = LVIS_SELECTED | LVIS_FOCUSED;

    SendMessage ( hList, LVM_SETITEMSTATE, index, ( LPARAM )&item );
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: LVFocusItem 
/*--------------------------------------------------------------------------*/
//           Type: void 
//    Param.    1: HWND hList: 
//    Param.    2: int index : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: focus item
/*--------------------------------------------------------------------@@-@@-*/
void LVFocusItem ( HWND hList, int index )
/*--------------------------------------------------------------------------*/
{
    LVITEM  item;

    item.stateMask  = LVIS_FOCUSED;
    item.state      = LVIS_FOCUSED;

    SendMessage ( hList, LVM_SETITEMSTATE, index, ( LPARAM )&item );
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: LVUnselectItem 
/*--------------------------------------------------------------------------*/
//           Type: void 
//    Param.    1: HWND hList: 
//    Param.    2: int index : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: unselect one
/*--------------------------------------------------------------------@@-@@-*/
void LVUnselectItem ( HWND hList, int index )
/*--------------------------------------------------------------------------*/
{
    LVITEM  item;

    item.stateMask  =  LVIS_SELECTED;
    item.state      &= ~LVIS_SELECTED;

    SendMessage ( hList, LVM_SETITEMSTATE, index, ( LPARAM )&item );
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: LVEnsureVisible 
/*--------------------------------------------------------------------------*/
//           Type: BOOL 
//    Param.    1: HWND hList: 
//    Param.    2: int index : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: scroll a item into view
/*--------------------------------------------------------------------@@-@@-*/
BOOL LVEnsureVisible ( HWND hList, int index )
/*--------------------------------------------------------------------------*/
{
    return (BOOL)SendMessage ( hList, LVM_ENSUREVISIBLE, index, (LPARAM)FALSE);
}

/*-@@+@@--------------------------------------------------------------------*/
//       Function: InitImgList 
/*--------------------------------------------------------------------------*/
//           Type: HIMAGELIST 
//    Param.    1: HINSTANCE hInst : 
/*--------------------------------------------------------------------------*/
//         AUTHOR: Adrian Petrila, YO3GFH
//           DATE: 03.10.2020
//    DESCRIPTION: make IMG list for the LV, from resource icons
/*--------------------------------------------------------------------@@-@@-*/
HIMAGELIST InitImgList ( HINSTANCE hInst )
/*--------------------------------------------------------------------------*/
{
    HIMAGELIST  img;

    img = ImageList_Create ( 16, 16, ILC_COLOR32 | ILC_MASK, 0, 2 );

    if ( !img )
        return NULL;

    if ( ImageList_AddIcon ( img, LoadIcon 
        ( hInst, MAKEINTRESOURCE ( IDI_LISTICON ) ) ) == -1 )
            return NULL;

    if ( ImageList_AddIcon ( img, LoadIcon 
        ( hInst, MAKEINTRESOURCE ( IDI_PLAYICON ) ) ) == -1 )
            return NULL;

    return img;
}


