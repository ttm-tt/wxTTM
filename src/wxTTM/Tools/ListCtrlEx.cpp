/* Copyright (C) 2020 Christoph Theis */

// ListCtrlEx.cpp : implementation file
//

#include "stdafx.h"
#include "ListCtrlEx.h"
#include "ListItem.h"

#include "TT32App.h"
#include "FormViewEx.h"
#include "MainFrm.h"  // Fuer den Suchstring
#include "Profile.h"
#include "Res.h"


// -----------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS(CListCtrlEx, wxListCtrl)

BEGIN_EVENT_TABLE(CListCtrlEx, wxListCtrl)
  EVT_LIST_COL_CLICK(wxID_ANY, CListCtrlEx::OnColumnClick)
  EVT_LIST_COL_RIGHT_CLICK(wxID_ANY, CListCtrlEx::OnColumnRightClick)
  EVT_WINDOW_DESTROY(CListCtrlEx::OnDestroy)
  EVT_KILL_FOCUS(CListCtrlEx::OnKillFocus)
  EVT_MENU(wxID_SELECTALL, CListCtrlEx::OnSelectAll)
  EVT_CHAR(CListCtrlEx::OnChar)
  EVT_KEY_DOWN(CListCtrlEx::OnKeyDown)  
  EVT_LEFT_DCLICK(CListCtrlEx::OnLButtonDblClk)
  EVT_SIZE(CListCtrlEx::OnSize)
END_EVENT_TABLE()


// -----------------------------------------------------------------------
// CListCtrlEx

int  wxCALLBACK CListCtrlEx::Compare(wxIntPtr item1, wxIntPtr item2, wxIntPtr param)
{
  return ((ListItem *) item1)->Compare( ((const ListItem *) item2), param);
}


CListCtrlEx::CListCtrlEx()
{
  m_itemHeight = 1;
  m_lastIdx = -1;
  m_resizeColumn = -1;
  m_sortIdx = -1;
}


CListCtrlEx::~CListCtrlEx()
{
}


// -----------------------------------------------------------------------
// CListCtrlEx message handlers

void CListCtrlEx::OnSize(wxSizeEvent &evt) 
{
  if (GetWindowStyleFlag() & wxLC_NO_HEADER)
  {
    if (GetColumnCount() == 0)    
    {
      InsertColumn(0, wxT(""));      
      
      m_resizeColumn = 0;
    }
  }
  
  ResizeColumn(m_resizeColumn);

  evt.Skip();
}


void CListCtrlEx::OnKeyDown(wxKeyEvent &evt)
{
  wxChar nChar = evt.GetKeyCode();
  
  switch (nChar)
  {
    case 'A' :
      // CTRL+A : Select All
      if (evt.GetModifiers() == wxMOD_CONTROL)
      {
        SelectAll();
        return;
      }
      
      break;
      
    case 'I' :
      // CTRL+I : Invert Selection
      if (evt.GetModifiers() == wxMOD_CONTROL)
      {
        for (int idx = 0; idx < GetItemCount(); idx++)
          IsSelected(idx) ? ClearSelected(idx) : SetSelected(idx);
          
        return;
      }
      
      break;
      
    case WXK_ESCAPE :
      // ESC : Stop find
      if (m_lastIdx >= 0 || m_editString.Length())
      {
        ClearSelection();
        EnsureVisible(m_lastIdx);
    
        m_editString[0] = 0;
        m_lastIdx = -1;

        ((CMainFrame *) wxGetApp().GetTopWindow())->SetFindString("");
        
        return;
      }
      
      break;      

    case WXK_BACK :
      // BSP : Letztes Zeichen vom Suchstring loeschen
      if (!m_editString.Length())
        return;
        
      m_editString.RemoveLast();

      if (m_editString.Length() == 0)
      {
        ClearSelection();
        EnsureVisible(m_lastIdx);

        m_lastIdx = -1;

        ((CMainFrame *) wxGetApp().GetTopWindow())->SetFindString("");
        
        return;
      }

      SetCurrentIndex(m_lastIdx);

      ((CMainFrame *) wxGetApp().GetTopWindow())->SetFindString(m_editString);

      for (int idx = m_lastIdx; idx--; )
      {
        ListItem *itemPtr = GetListItem(idx);
        if (!itemPtr)
          break;

        // Nicht abbrechen. Es wird das oberste passende Item markiert
        if (itemPtr->HasString(m_editString))
        {
          m_lastIdx = idx;
          
          SetCurrentIndex(m_lastIdx);
        }
      }

      return;

    case WXK_F3 :
      // F3 : Naechstes Vorkommen
    {
      for (int idx = m_lastIdx + 1; idx < GetItemCount(); idx++)
      {
        ListItem *itemPtr = GetListItem(idx);
        if (!itemPtr)
          break;

        if (itemPtr->HasString(m_editString))
        {
          m_lastIdx = idx;
          SetCurrentIndex(m_lastIdx);

          return;
        }
      }

      // Not found
      wxBell();

      return;
    }

    case WXK_INSERT :
    case WXK_DELETE :
    {
      // INS u. DEL : Einfuegen u. Loeschen
      wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, nChar == WXK_INSERT ? IDC_ADD : IDC_DELETE);
      GetEventHandler()->ProcessEvent(evt);

      return;
    }
  }
  
  // Was hier landet wurde nicht behandelt
  evt.Skip();
}      


// Eine Funktion fuer die normalen Zeichen.
// OnKeyDown geht nicht, weil das die KbdCodes sind.
// Alles in OnChar geht auch nicht, weil mir sonst die VK_XXX 
// durch die Lappen gehen.
void CListCtrlEx::OnChar(wxKeyEvent &evt)
{
  if ( (evt.GetModifiers() & ~wxMOD_SHIFT) != 0 )
  {
    evt.Skip();
    return;
  }

  wxChar nChar = evt.GetKeyCode();
  
  // int    nFlags = evt.GetModifiers();
  int    idx;
  
  switch (evt.GetKeyCode())
  {
    case 0x0 :
      break; // To make compilers happy
      
    // 0x7F: Liegt in den Grenzen unten
    case WXK_DELETE :
    case WXK_INSERT :
      break;

    default :
    {
      if ( nChar >= 0x20 && nChar < 0xFF ) //  (stuerzt ab?)
      {
        m_editString.Append(nChar);

        ((CMainFrame *) wxGetApp().GetTopWindow())->SetFindString(m_editString);

        for (idx = (m_lastIdx < 0 ? 0 : m_lastIdx); idx < GetItemCount(); idx++)
        {
          ListItem *itemPtr = GetListItem(idx);
          if (!itemPtr)
            break;

          if (itemPtr->HasString(m_editString))
          {
            m_lastIdx = idx;
            SetCurrentIndex(m_lastIdx);

            if (idx < GetItemCount() - 1)
              EnsureVisible(idx + 1);

            return;
          }
        }

        // Not found
        m_editString.RemoveLast();
        SetCurrentIndex(m_lastIdx);

        if (idx < GetItemCount() - 1)
          EnsureVisible(idx + 1);

        ((CMainFrame *) wxGetApp().GetTopWindow())->SetFindString(m_editString);

        wxBell();

        return;
      }
    }

    break;
  }
  
  evt.Skip();
}


void CListCtrlEx::OnLButtonDblClk(wxMouseEvent &evt) 
{
  int flags = 0;
  int idx = HitTest(evt.GetPosition(), flags);

  if (idx == wxNOT_FOUND)  
  {
    evt.Skip();
    return;
  }
  
  SetSelected(idx);
  
  ListItem * itemPtr = GetListItem(idx);  
  
  wxCommandEvent cmd(wxEVT_COMMAND_BUTTON_CLICKED, itemPtr ? itemPtr->GetType() : IDC_EDIT);
  ProcessCommand(cmd);
}


void CListCtrlEx::OnColumnClick(wxListEvent &evt) 
{
  SortItems(GetIdxForColumn(evt.GetColumn()));
}


void CListCtrlEx::OnColumnRightClick(wxListEvent &evt)
{
  wxMenu menu(_("Select Columns"));

  for (size_t i = 0; i < columnInfo.size(); i++)
  {
    if (columnInfo[i].enableHide)
      menu.AppendCheckItem(2000 + i, columnInfo[i].m_text)->Check(HasColumn(i));
  }

  if (menu.GetMenuItemCount() == 0)
    return;

  wxPoint popupPos = ScreenToClient(wxGetMousePosition());
  
  int rc = GetPopupMenuSelectionFromUser(menu, popupPos);

  if (rc != wxID_NONE)
    ShowColumn(rc - 2000, menu.IsChecked(rc));
}


// Bei Focusverlust den Editstring zuruecksetzen
void CListCtrlEx::OnKillFocus(wxFocusEvent &evt)
{
  if (m_lastIdx >= 0 || m_editString.Length())
  {
    m_editString = "";
    m_lastIdx = -1;
    
    ((CMainFrame *) wxGetApp().GetTopWindow())->SetFindString("");
  }
  
  evt.Skip();
}


// Brauch ich, um die Items zu loeschen. 
// Fuer dieses Ctrl gibt es leider kein WM_DELETEITEM
void CListCtrlEx::OnDestroy(wxWindowDestroyEvent &) 
{
  RemoveAllListItems();
}


// -----------------------------------------------------------------------
// Add Item
void  CListCtrlEx::AddListItem(const ListItem *itemPtr, int idx)
{
  if (idx == -1)
    idx = GetItemCount();

  wxListItem li;
  li.SetMask(wxLIST_MASK_TEXT | wxLIST_MASK_DATA);
  li.SetId(idx);
  li.SetText(itemPtr->GetLabel());
  li.SetData((void *) itemPtr);

  InsertItem(li);
}


// Get Item idx by ID
ListItem * CListCtrlEx::FindListItem(long id)
{
  if (!m_hWnd)
    return 0;

  wxIntPtr  param;
  ListItem * itemPtr = 0;

  for (int idx = 0; idx < GetItemCount(); idx++)
  {
    param = GetItemData(idx);
    if ( param && ((ListItem *) param)->GetID() == id)
    {
      itemPtr = (ListItem *) param;
      return itemPtr;
    }
  }

  return 0;
}


// Get Item idx by ID
ListItem * CListCtrlEx::FindListItem(const wxString &label)
{
  if (!m_hWnd)
    return 0;

  wxIntPtr  param;
  ListItem * itemPtr = 0;

  for (int idx = 0; idx < GetItemCount(); idx++)
  {
    param = GetItemData(idx);
    if (param && ((ListItem *)param)->GetLabel() == label)
    {
      itemPtr = (ListItem *)param;
      return itemPtr;
    }
  }

  return 0;
}


ListItem * CListCtrlEx::GetListItem(int idx)
{
  if (!m_hWnd)
    return 0;
    
  if (idx >= GetCount())
    return NULL;

  ListItem *itemPtr = (ListItem *) GetItemData(idx);
  return itemPtr;
}


// Get Current Item
ListItem *  CListCtrlEx::GetCurrentItem()
{
  if (!m_hWnd)
    return 0;

  ListItem * itemPtr = 0;

  int idx = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  if (idx == -1)
    return 0;

  wxIntPtr  lParam = GetItemData(idx);
  itemPtr = (ListItem *) lParam;
  return itemPtr;
}


void  CListCtrlEx::SetCurrentItem(long id)
{
  for (int idx = 0; idx < GetItemCount(); idx++)
  {
    if (GetListItem(idx) && GetListItem(idx)->GetID() == id)
    {
      SetCurrentIndex(idx);
      break;
    }
  }
}


void  CListCtrlEx::SetCurrentIndex(long idx)
{
  if (idx < 0 || idx > GetItemCount())
    return;
    
  ClearSelection();
    
  SetItemState(idx, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
  EnsureVisible(idx);
}


// Get index of current item
long  CListCtrlEx::GetCurrentIndex()
{
  long ret = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
  if (ret == -1) // not found
    ret = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_FOCUSED);

  return ret;
}


long CListCtrlEx::GetListIndex(const ListItem *itemPtr)
{
  for (int idx = GetCount(); idx--; )
  {
    if (GetListItem(idx) == itemPtr)
      return (long) idx;
  }

  return (long) -1;
}


// Entfernt alle Selektionen
void CListCtrlEx::ClearSelection() 
{
  long idx = -1;
  
  while ( (idx = GetNextItem(idx, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED)) != -1)
    SetItemState(idx, 0, wxLIST_STATE_SELECTED);
}


// Anzahl selektierte Eintraege
int  CListCtrlEx::GetSelectedCount()
{
  return GetSelectedItemCount();
}


// Remove Item by id
void  CListCtrlEx::RemoveListItem(long id)
{
  for (int idx = 0; idx < GetItemCount(); idx++)
  {
    wxIntPtr param = GetItemData(idx);
    if ( param && ((ListItem *) param)->GetID() == id)
    {
      delete (ListItem *) param;
      DeleteItem(idx);
      return;
    }
  }
}


void  CListCtrlEx::RemoveAllListItems()
{
  for (int idx = GetItemCount(); idx--; )
    delete GetListItem(idx);

  DeleteAllItems();
}


ListItem * CListCtrlEx::CutListItem(int idx)
{
  ListItem *itemPtr = (ListItem *) GetItemData(idx);
  DeleteItem(idx);
  
  return itemPtr;
}


ListItem * CListCtrlEx::CutListItem(long id)
{
  ListItem *itemPtr;

  for (int idx = 0; itemPtr = GetListItem(idx); idx++)
  {
    if (itemPtr->GetID() == id)
      return CutListItem(idx);
  }

  return 0;
}  


ListItem * CListCtrlEx::CutCurrentItem()
{
  return CutListItem((int) GetCurrentIndex() );
}
  
  
void  CListCtrlEx::SortItems(int col)
{
  if (col < 0)
    col = m_sortIdx;

  m_sortIdx = (col == -1 ? 0 : col);

  wxListCtrl::SortItems(CListCtrlEx::Compare, m_sortIdx);

  int idx = GetCurrentIndex();
  if (idx >= 0)
    EnsureVisible(idx);
}


long CListCtrlEx::InsertColumn(long col, const wxString &heading, int fmt, int width)
{
  if (width < 0)
    width = GetTextExtent(heading).GetWidth() + 2 * GetTextExtent("M").GetWidth();

  ColumnInfo item;
  item.m_mask = wxLIST_MASK_TEXT | wxLIST_MASK_FORMAT;
  item.m_text = heading;
  if ( width >= 0 || width == wxLIST_AUTOSIZE || width == wxLIST_AUTOSIZE_USEHEADER )
  {
    item.m_mask |= wxLIST_MASK_WIDTH;
    item.m_width = width;
  } 

  item.m_format = fmt;
  item.column = -1;
  item.enableHide = false;

  if (col >= (long) columnInfo.size())
    columnInfo.push_back(item);
  else
    columnInfo.insert(columnInfo.begin() + col, item);

  return ShowColumn(col);
}

bool CListCtrlEx::DeleteColumn(long col)
{
  if (!wxListCtrl::DeleteColumn(col))
    return false;

  for (size_t idx = 0; idx < columnInfo.size(); idx++)
  {
    if (columnInfo[idx].column > columnInfo[col].column)
      --columnInfo[idx].column;
  }

  columnInfo.erase(columnInfo.begin() + col);

  return true;
}


void CListCtrlEx::HideColumn(long col)
{
  ShowColumn(col, false);
}


long CListCtrlEx::ShowColumn(long col, bool show)
{
  if (col >= (long) columnInfo.size())
    return -1;

  if (!show && columnInfo[col].column < 0)
    return col;

  if (show && columnInfo[col].column >= 0)
    return col;

  if (!show)
  {
    for (size_t idx = 0; idx < columnInfo.size(); idx++)
    {
      if (columnInfo[idx].column > columnInfo[col].column)
        --columnInfo[idx].column;
    }

    GetColumn(columnInfo[col].column, columnInfo[col]);

    wxListCtrl::DeleteColumn(columnInfo[col].column);

    columnInfo[col].column = -1;
  }
  else
  {
    int count = 0;
    for (long idx = 0; idx < col; idx++)
    {
      if (columnInfo[idx].column >= 0)
        ++count;
    }

    for (size_t idx = 0; idx < columnInfo.size(); idx++)
    {
      if (columnInfo[idx].column >= count)
        ++columnInfo[idx].column;
    }

    columnInfo[col].column = count;

    wxListCtrl::InsertColumn(columnInfo[col].column, columnInfo[col]);
  }

  ResizeColumn();

  return col;
}


void CListCtrlEx::AllowHideColumn(long col, bool allow)
{
  if (col < 0 || col >= (long) columnInfo.size())
    return;

  columnInfo[col].enableHide = allow;
}


bool CListCtrlEx::HasColumn(long col)
{
  if (col < 0 || col >= (long) columnInfo.size())
    return false;

  return columnInfo[col].column >= 0;
} 


long CListCtrlEx::GetColumnForIdx(long idx)
{
  if (idx < 0 || idx >= (long) columnInfo.size())
    return -1;

  return columnInfo[idx].column;
}


long CListCtrlEx::GetIdxForColumn(long col)
{
  for (size_t idx = 0; idx < columnInfo.size(); idx++)
    if (columnInfo[idx].column == col)
      return idx;

  return -1;
}


bool CListCtrlEx::SaveColumnInfo(const wxString &prefix) const
{
  wxString key = prefix + "." + GetName() + ".ColumnInfo";

  wxString value;
  for (size_t idx = 0; idx < columnInfo.size(); idx++)
  {
    value << columnInfo[idx].column << ";";
  }

  ttProfile.AddString(PRF_DEFAULTS, key, value);

  return true;
}


bool CListCtrlEx::RestoreColumnInfo(const wxString &prefix)
{
  wxString key = prefix + "." + GetName() + ".ColumnInfo";

  wxArrayString value = wxStringTokenize(ttProfile.GetString(PRF_DEFAULTS, key, ""), ";");
  if (value.Count() != columnInfo.size())
    return false;

  long val;
  for (size_t idx = 0; idx < value.Count(); idx++)
  {
    if (value[idx].ToLong(&val))
      ShowColumn(idx, val >= 0);
  }

  return true;
}


void CListCtrlEx::SetResizeColumn(int col)
{      
  m_resizeColumn = col; 
  ResizeColumn();
}

    
void  CListCtrlEx::ResizeColumn(int col)
{
  if (col < 0 || col >= GetColumnCount())
    return;
    
  // 8 ist ein heuristisches Mass
  unsigned width = GetClientSize().GetWidth(); //  - 8;

  for (int idx = GetColumnCount(); idx--; )
  {
    if (idx == col)
      continue;

    width -= GetColumnWidth(idx);
  }

  SetColumnWidth(col, width);
}


void CListCtrlEx::SetItemHeight(double height)
{
  m_itemHeight = height;
  
  // SetSize(wxRect(GetPosition(), GetSize()), wxSIZE_FORCE);
  // Eine neue MEASUREITEM-Message erzwingen. 
  // Dies geschieht in Folge einer WINDOWSPOSCHANGED-Message
  wxSize rc = GetSize();

  WINDOWPOS wp;
  wp.hwnd = (HWND) GetHandle();
  wp.cx = rc.GetWidth();
  wp.cy = rc.GetHeight();
  wp.flags = SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER;
  ::SendMessage( wp.hwnd, WM_WINDOWPOSCHANGED, 0, (LPARAM)&wp );  
}


// -----------------------------------------------------------------------
// CListCtrlEx drawing

// offsets for first and other columns
#define OFFSET_FIRST	2
#define OFFSET_OTHER	6


wxString CListCtrlEx::MakeShortString(wxDC &dc, const wxString &str, const wxRect &rect) const
{
  static const wxString ellipses = wxT("...");
  
  if (dc.GetTextExtent(str).GetWidth() <= rect.GetWidth())
    return str;
    
  int ellipsesLen = dc.GetTextExtent(ellipses).GetWidth();
  wxString ret = str;
  
  do
  {
    ret.RemoveLast();
  } while (dc.GetTextExtent(ret).GetWidth() + ellipsesLen > rect.GetWidth());
  
  return ret + ellipses;  
}


// -----------------------------------------------------------------------
#ifndef wxLVS_OWNERDRAW
# define wxLVS_OWNERDRAW 0x400
#endif

WXDWORD  CListCtrlEx::MSWGetStyle(long style, WXDWORD *extstyle) const
{
  return wxListCtrl::MSWGetStyle(style, extstyle) | wxLVS_OWNERDRAW;
}


bool CListCtrlEx::MSWOnMeasure(WXMEASUREITEMSTRUCT *item)
{
    // only owner-drawn control should receive this message
    // wxCHECK( ((m_windowStyle & wxLVS_OWNERDRAWFIXED) == wxLVS_OWNERDRAWFIXED), false );

    MEASUREITEMSTRUCT *pStruct = (MEASUREITEMSTRUCT *)item;

#ifdef __WXWINCE__
    HDC hdc = GetDC(NULL);
#else
    HDC hdc = CreateIC(wxT("DISPLAY"), NULL, NULL, 0);
#endif

    {
        wxDCTemp dc((WXHDC)hdc);
        dc.SetFont(GetFont());

        pStruct->itemHeight = dc.GetCharHeight() + 2;
        pStruct->itemWidth  = dc.GetCharWidth();
    }

#ifdef __WXWINCE__
    ReleaseDC(NULL, hdc);
#else
    DeleteDC(hdc);
#endif

    pStruct->itemHeight *= m_itemHeight;
    pStruct->itemHeight += pStruct->itemHeight / 2;

    return true;
}


bool CListCtrlEx::MSWOnDraw(WXDRAWITEMSTRUCT *item)
{
  DRAWITEMSTRUCT *pStruct = (DRAWITEMSTRUCT *) item;
  
  int itemID = pStruct->itemID;
  
  if (itemID < 0 || itemID >= GetItemCount())
    return false;
  
  ListItem *itemPtr = GetListItem(pStruct->itemID);
  
  if (!itemPtr)
    return false;

  wxDCTemp dc(pStruct->hDC);
  
  wxRect itemRect;
  GetItemRect(pStruct->itemID, itemRect, wxLIST_RECT_BOUNDS);
  int state = GetItemState(pStruct->itemID, wxLIST_STATE_FOCUSED | wxLIST_STATE_SELECTED | wxLIST_STATE_DROPHILITED);

  if (itemPtr->DrawSeparatingLine())
  {
    dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT)));
    dc.DrawLine(itemRect.GetBottomLeft(), itemRect.GetBottomRight());
    itemRect.Deflate(0, 1);
  }    
  
  if (state & wxLIST_STATE_SELECTED)
  {
    // Eigentlich will ich immer highlight anzeigen, wenn das Fenster den Focus hat
    // und nicht nur, wenn das ctrl den Focus hat.
    if (HasFocus() || true)
    {
      dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT)));
      dc.SetTextBackground(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
      dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));
    }
    else
    {
      dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE)));
      dc.SetTextBackground(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
      dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
      dc.SetTextForeground(itemPtr->GetForeground());
    }
  }
  else
  {
    dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW)));
    dc.SetTextBackground(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
    dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
    dc.SetTextForeground(itemPtr->GetForeground());
  }

  if (itemPtr->GetForeground() != wxNullColour)
    dc.SetTextForeground(itemPtr->GetForeground());

  if (itemPtr->IsDeleted())
  {
    wxFont font = dc.GetFont();
    font.SetStrikethrough(true);
    dc.SetFont(font);
  }

  dc.SetPen(*wxTRANSPARENT_PEN);
  dc.DrawRectangle(itemRect);
  
  if ( (state & wxLIST_STATE_FOCUSED) && HasFocus() )
    wxRendererNative::Get().DrawFocusRect(this, dc, itemRect);
    
  if (GetWindowStyle() & wxLC_NO_HEADER)
  {
    // Ganzes Item zeichnen, wenn es keinen Header gibt
    wxRect rect = itemRect;
    rect.Deflate(OFFSET_OTHER, 1);
    itemPtr->DrawItem(&dc, rect);
  }
  else
  {
    wxRect rect;
    GetItemRect(pStruct->itemID, rect, wxLIST_RECT_LABEL);
    
    rect.Deflate(OFFSET_FIRST, 1);

    int idx = GetIdxForColumn(0);
    
    if (itemPtr)
      itemPtr->DrawColumn(&dc, idx, rect);
    else
      ListItem::DrawString(&dc, rect, GetItemText(pStruct->itemID));
      
    for (int i = 1; i < GetColumnCount(); i++)
    {
      idx = GetIdxForColumn(i);

      GetSubItemRect(pStruct->itemID, i, rect, wxLIST_RECT_BOUNDS);
    
      rect.Deflate(OFFSET_OTHER, 1);
      
      if (itemPtr)
      {
        itemPtr->DrawColumn(&dc, idx, rect);
      }
      else
      {
        wxListItem info;
        info.m_itemId = pStruct->itemID;
        info.m_col = i;
        info.m_mask = wxLIST_MASK_TEXT;
        
        GetItem(info);
        
        ListItem::DrawString(&dc, rect, info.m_text);        
      }
    }      
  }

  if (itemPtr->IsDeleted())
  {
    wxFont font = dc.GetFont();
    font.SetStrikethrough(false);
    dc.SetFont(font);
  }

  return true;
}  
