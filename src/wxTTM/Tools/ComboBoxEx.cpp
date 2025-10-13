/* Copyright (C) 2020 Christoph Theis */

// ComboBoxEx.cpp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "ComboBoxEx.h"
#include "ListItem.h"

#include "MainFrm.h"  // Fuer den Suchstring


// -----------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS(CComboBoxEx, wxOwnerDrawnComboBox)

BEGIN_EVENT_TABLE(CComboBoxEx, wxOwnerDrawnComboBox)
  EVT_KEY_DOWN(CComboBoxEx::OnKeyDown)
  EVT_CHAR(CComboBoxEx::OnChar)
  EVT_KILL_FOCUS(CComboBoxEx::OnKillFocus)
END_EVENT_TABLE()
// CComboBoxEx


// Um so viel Pixel soll die CB groesser werden als der default,
// damit sie die gleiche Hoehe hat wie eine normale CB
static const int OFFSET = 5;

CComboBoxEx::CComboBoxEx()
{
  // SetPopupMaxHeight(100);
  m_itemHeight = 1.0;
  m_lastIdx = -1;
}


CComboBoxEx::~CComboBoxEx()
{
  RemoveAllListItems();
}  


// -----------------------------------------------------------------------
// Add Item

void  CComboBoxEx::AddListItem(const ListItem *itemPtr, int idx)
{
  if (idx == -1)
    idx = Append(itemPtr->GetLabel(), (void *) itemPtr);
  else
    idx = Insert(itemPtr->GetLabel(), idx, (void *) itemPtr);
}


// Get Item idx by ID / Label
ListItem *  CComboBoxEx::FindListItem(long id)
{
  if (GetCount() == 0)
    return 0;

  ListItem *itemPtr;
  for (int idx = 0; (itemPtr = GetListItem(idx)); idx++)
  {
    if (itemPtr->GetID() == id)
      return itemPtr;
  }

  return 0;
}


ListItem * CComboBoxEx::FindListItem(const wxString &str)
{
  if (GetCount() == 0)
    return 0;

  ListItem *itemPtr;
  for (int idx = 0; (itemPtr = GetListItem(idx)); idx++)
  {
    if ( !wxStrcoll(itemPtr->GetLabel(), str) )
      return itemPtr;
  }

  return 0;
}


ListItem * CComboBoxEx::GetListItem(int idx)
{
  if (idx < 0 || GetCount() <= (unsigned) idx)
    return 0;

  ListItem *itemPtr = (ListItem *) GetClientData(idx);
  if (itemPtr == (ListItem *) CB_ERR)
    return 0;

  return itemPtr;  
}


// Get Current Item
ListItem *  CComboBoxEx::GetCurrentItem()
{
  int idx = GetSelection();
  if (idx != wxNOT_FOUND)
    return (ListItem *) GetClientData(idx);
  else
    return 0;
}


// Cut current item
ListItem *  CComboBoxEx::CutCurrentItem()
{
  int idx = GetSelection();
  if (idx != wxNOT_FOUND)
  {
    ListItem * itemPtr = (ListItem *) GetClientData(idx);
    SetClientData(idx, 0);
    Delete(idx);

    return itemPtr;
  }
  else
    return 0;
}


// Cut Item with ID / label
ListItem * CComboBoxEx::CutListItem(long id)
{
  return 0;
}


ListItem * CComboBoxEx::CutListItem(const wxString &str)
{
  return 0;
}


// Remove Item by id
void  CComboBoxEx::RemoveListItem(long id)
{
  return;
}


void CComboBoxEx::RemoveListItem(int idx)
{
  Delete(idx);
}

// Remove Item by label
void  CComboBoxEx::RemoveListItem(const wxString &str)
{
  if (GetCount() == 0)
    return;

  ListItem *itemPtr;
  for (int idx = 0; (itemPtr = GetListItem(idx)); idx++)
  {
    if (!wxStrcoll(itemPtr->GetLabel(), str))
    {
      Delete(idx);
      return;
    }
  }
}


void  CComboBoxEx::RemoveAllListItems()
{
  if (GetCount() == 0)
    return;
    
  ListItem *itemPtr;
  for (int idx = 0; (itemPtr = GetListItem(idx)); idx++)
  {
    SetClientData(idx, 0);
    delete itemPtr;
  }

  Clear();
}


// Set current item to label "str"
void  CComboBoxEx::SetCurrentItem(const wxString &str)
{
  if (GetCount() == 0)
    return;

  ListItem *itemPtr;
  for (int idx = 0; (itemPtr = GetListItem(idx)); idx++)
  {
    if (!wxStrcoll(itemPtr->GetLabel(), str))
    {
      SetSelection(idx);
      return;
    }
  }
}


void  CComboBoxEx::SetCurrentItem(long id)
{
  if (GetCount() == 0)
    return;

  ListItem *itemPtr;
  for (int idx = 0; (itemPtr = GetListItem(idx)); idx++)
  {
    if (itemPtr->GetID() == id)
    {
      SetSelection(idx);
      return;
    }
  }
}


void CComboBoxEx::SetCurrentItem(int idx)
{
  if (idx < 0 || idx >= GetCount())
    return;

  SetSelection(idx);
}


void  CComboBoxEx::SetCurrentItem(const ListItem *ptr)
{
  if (GetCount() == 0)
    return;

  ListItem *itemPtr;
  for (int idx = 0; (itemPtr = GetListItem(idx)); idx++)
  {
    if (itemPtr == ptr)
    {
      SetSelection(idx);
      return;
    }
  }
}


// Neue Itemhoehe setzen
void  CComboBoxEx::SetItemHeight(unsigned newHeight)
{
  if (m_itemHeight != newHeight)
  {
    SetSize(-1, ((GetSize().GetHeight() - OFFSET) * newHeight) / m_itemHeight + OFFSET);
    
    m_itemHeight = newHeight;
  }
}



// -----------------------------------------------------------------------
// CComboBoxEx message handlers

void CComboBoxEx::OnDrawItem(wxDC &dc, const wxRect &rect, int item, int flags) const
{
  ListItem *itemPtr = (ListItem *) GetClientData(item);
  if (itemPtr->GetForeground() != wxNullColour)
    dc.SetTextForeground(itemPtr->GetForeground());
  itemPtr->DrawItem(&dc, const_cast<wxRect &>(rect));
}


wxCoord CComboBoxEx::OnMeasureItem(size_t item) const
{
  if (m_itemHeight == 1)
    return GetCharHeight() + 2;
    // return wxOwnerDrawnComboBox::OnMeasureItem(item);
  else
    return 2 * (GetCharHeight() + 2); // GetSize().GetHeight();
}


wxString CComboBoxEx::MakeShortString(wxDC &dc, const wxString & str, const wxRect &rect) const
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
void CComboBoxEx::OnKeyDown(wxKeyEvent &evt)
{
  switch (evt.GetKeyCode())
  {
    case WXK_ESCAPE :
      if (m_lastIdx >= 0 || m_editString.Length())
      {
        SetSelection(wxNOT_FOUND);

        m_editString = "";
        m_lastIdx = -1;                

        ((CMainFrame *) wxGetApp().GetTopWindow())->SetFindString(m_editString);

        return;
      }
        
      break;

    case WXK_BACK :
      if (m_editString.Length() == 0)
        break;
        
      m_editString.RemoveLast();

      ((CMainFrame *) wxGetApp().GetTopWindow())->SetFindString(m_editString);

      if (m_editString.Length() == 0)
      {
        m_lastIdx = -1;
        SetSelection(wxNOT_FOUND);

        return;
      }

      SetSelection(m_lastIdx);

      for (int idx = m_lastIdx; idx--; )
      {
        ListItem *itemPtr = GetListItem(idx);
        if (!itemPtr)
          break;

        // Nicht abbrechen. Es wird das oberste passende Item markiert
        if (itemPtr->HasString(m_editString))
        {
          SetSelection(idx);
          
          wxCommandEvent evt(wxEVT_COMMAND_COMBOBOX_SELECTED, GetId());

          if (idx != m_lastIdx)
            GetEventHandler()->ProcessEvent(evt);
                
          m_lastIdx = idx;
        }
      }

      return;

    case WXK_F3 :
    {
      if (m_lastIdx < 0 || m_editString.Length() == 0)
        break;

      for (size_t idx = m_lastIdx + 1; idx < GetCount(); idx++)
      {
        ListItem *itemPtr = GetListItem(idx);
        if (!itemPtr)
          return;

        if (itemPtr->HasString(m_editString))
        {
          SetSelection(idx);
          
          wxCommandEvent evt(wxEVT_COMMAND_COMBOBOX_SELECTED, GetId());

          if (idx != m_lastIdx)
            GetEventHandler()->ProcessEvent(evt);
                
          m_lastIdx = idx;

          break;
        }
      }

      return;
    }
  }
  
  evt.Skip();
}


// OnChar brauch ich, weil die Codes in Zeichen uebersetzt werden muessen
void CComboBoxEx::OnChar(wxKeyEvent &evt)
{
  wxChar nChar = evt.GetKeyCode();
  
  switch (nChar)
  {
    // Make compiler happy
    case 0x0 :
      break;
      
    // 0x7F: Liegt in den Grenzen unten
    case WXK_DELETE :
      break;
      
    default :
    {
      if ( nChar >= 0x20 && nChar < 0xFF ) // && isprint(nChar)
      {
        m_editString.Append(nChar);

        ((CMainFrame *) wxGetApp().GetTopWindow())->SetFindString(m_editString);

        for (size_t idx = (m_lastIdx < 0 ? 0 : m_lastIdx); 
             idx < GetCount(); idx++)
        {
          ListItem *itemPtr = GetListItem(idx);
          if (!itemPtr)
            break;

          if (itemPtr->HasString(m_editString))
          {
            SetSelection(idx);            

            wxCommandEvent evt(wxEVT_COMMAND_COMBOBOX_SELECTED, GetId());
            
            if (idx != m_lastIdx)
              GetEventHandler()->ProcessEvent(evt);
              
            m_lastIdx = idx;

            return;
          }
        }

        // Not found
        m_editString.RemoveLast();

        ((CMainFrame *) wxGetApp().GetTopWindow())->SetFindString(m_editString);

        return;
      }
    }

    break;
  }
  
  // Wenn man hier landet wurde das Event nicht behandelt
  evt.Skip();
}


// Bei Focusverlust den Editstring zuruecksetzen
void CComboBoxEx::OnKillFocus(wxFocusEvent &evt)
{
  if (m_lastIdx >= 0 || m_editString.Length())
  {
    m_editString = wxT("");
    m_lastIdx = -1;
    
    ((CMainFrame *) wxGetApp().GetTopWindow())->SetFindString(m_editString);
  }
  
  evt.Skip();
}


wxSize CComboBoxEx::DoGetBestClientSize() const
{
  // Ein Hack, damit die CB die gleiche Groesse hat wie eine normale CB.
  // Nur weiss ich nicht, wie gross eine normale CB ist, drum einfach mal 4 draufaddiert.
  wxSize size = wxOwnerDrawnComboBox::DoGetBestSize();
  size.y += OFFSET;

  return size;
}
