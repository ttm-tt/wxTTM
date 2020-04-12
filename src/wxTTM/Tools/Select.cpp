/* Copyright (C) 2020 Christoph Theis */

// Select.cpp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "Select.h"

#include "ListItem.h"
#include "Profile.h"
#include "Res.h"


IMPLEMENT_DYNAMIC_CLASS(CSelect, wxDialog)

// -----------------------------------------------------------------------
// CSelect dialog
CSelect::CSelect(const wxString &title) : wxDialog()
{
  wxXmlResource::Get()->LoadDialog(this, wxGetApp().GetTopWindow(), "Select");
  
  listBox = XRCCTRL(*this, "List", CListCtrlEx);
  
  listBox->Connect(wxEVT_LEFT_DCLICK, wxMouseEventHandler(CSelect::OnCommand), NULL, this);
  
  listBox->Connect(wxEVT_CHAR, wxCharEventHandler(CSelect::OnChar), NULL, this);
    
  if (title != "")
    SetTitle(title);

  listBox->InsertColumn(0, wxT(""));
  listBox->ResizeColumn(0);

  listBox->SetItemHeight(1.5);
}


CSelect::~CSelect()
{
}


ListItem *  CSelect::Select(long id)
{
  itemPtr = NULL;

  listBox->SetSelected(0);

  listBox->SetFocus();

  RestoreSize();

  if (ShowModal() == wxID_OK)
    itemPtr = listBox->GetCurrentItem();

  SaveSize();

  return itemPtr;
}


void  CSelect::AddListItem(ListItem *itemPtr, int idx)
{
  listBox->AddListItem(itemPtr, idx);
}


void CSelect::InsertListItem(ListItem *itemPtr)
{
  listBox->InsertListItem(itemPtr);
}


void  CSelect::RemoveListItem(long id)
{
  listBox->RemoveListItem(id);
}


ListItem *  CSelect::CutListItem(long id)
{
  return listBox->CutListItem(id);
}


// -----------------------------------------------------------------------
void CSelect::OnCommand(wxMouseEvent &) 
{
  if ( (itemPtr = listBox->GetCurrentItem()) )
    EndModal(wxID_OK);
}



void CSelect::OnChar(wxKeyEvent &evt)
{
  if (evt.GetRawKeyCode() == WXK_ESCAPE)
    EndModal(wxID_CANCEL);
  else if (evt.GetRawKeyCode() == WXK_RETURN)
  {
    if ( (itemPtr = listBox->GetCurrentItem()) )
      EndModal(wxID_OK);
  }
    
  evt.Skip();
}



// -----------------------------------------------------------------------
void CSelect::SaveSize()
{
  wxSize size = GetSize();
  wxPoint pos = GetPosition();

  if (wxPoint(origX, origY) != pos || wxSize(origW, origH) != size)
  {
    wxString tmp = wxString::Format("%d,%dx%d,%d", std::max(0, pos.x), std::max(0, pos.y), size.x, size.y);
    ttProfile.AddString(PRF_GLOBAL_SETTINGS, GetClassInfo()->GetClassName(), tmp);
  }
}


void  CSelect::RestoreSize()
{
  wxString tmp = ttProfile.GetString(PRF_GLOBAL_SETTINGS, GetClassInfo()->GetClassName());
  if (!tmp.IsEmpty())
  {
    int x, y, w, h;
    sscanf(tmp.c_str(), "%d,%dx%d,%d", &x, &y, &w, &h);

    SetSize(w, h);

    SetPosition(wxPoint(std::max(0, x), std::max(0, y)));
  }
  else
    SetSize(400, 600);

  wxSize size = GetSize();
  wxPoint pos = GetPosition();

  origX = pos.x;
  origY = pos.y;
  origW = size.x;
  origH = size.y;
}


