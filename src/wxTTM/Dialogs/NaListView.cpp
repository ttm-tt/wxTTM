/* Copyright (C) 2020 Christoph Theis */

// NaListView.cpp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "NaListView.h"

#include "NaListStore.h"
#include "NaStore.h"
#include "NaItem.h"

#include "PlListStore.h"
#include "TmListStore.h"

#include "InfoSystem.h"
#include "Request.h"


// -----------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS(CNaListView, CFormViewEx)

BEGIN_EVENT_TABLE(CNaListView, CFormViewEx)
EVT_BUTTON(XRCID("Players"), CNaListView::OnPlayers)
EVT_BUTTON(XRCID("Teams"), CNaListView::OnTeams)

END_EVENT_TABLE()

// -----------------------------------------------------------------------
// CNaListView

CNaListView::CNaListView() : CFormViewEx(), m_listCtrl(0)
{
}


CNaListView::~CNaListView()
{
}


// -----------------------------------------------------------------------
void CNaListView::SaveSettings()
{
  m_listCtrl->SaveColumnInfo(GetClassInfo()->GetClassName());

  CFormViewEx::SaveSettings();
}


void CNaListView::RestoreSettings()
{
  CFormViewEx::RestoreSettings();

  m_listCtrl->RestoreColumnInfo(GetClassInfo()->GetClassName());
}


bool CNaListView::Edit(va_list vaList)
{
  // Insert Items
  int row = 0;

  NaListStore  naList;
  naList.SelectAll();

  while (naList.Next())
  {
    m_listCtrl->AddListItem(new NaItem(naList), row++);
  }
  
  m_listCtrl->SetCurrentIndex(0);

  return true;
}  


// -----------------------------------------------------------------------
// CNaListView message handlers

void CNaListView::OnInitialUpdate() 
{
	CFormViewEx::OnInitialUpdate();

  // TODO: Show only when we have teams
  // XRCCTRL(*this, "Teams", wxButton)->Hide();
	
	m_listCtrl = XRCCTRL(*this, "Associations", CListCtrlEx);
	
  // Name
  m_listCtrl->InsertColumn(0, _("Name"), wxALIGN_LEFT, wxLIST_AUTOSIZE_USEHEADER);

  // Description
  m_listCtrl->InsertColumn(1, _("Description"), wxALIGN_LEFT);

  // Region
  m_listCtrl->InsertColumn(2, _("Region"), wxALIGN_LEFT, m_listCtrl->GetClientSize().GetWidth() / 3.5);

  m_listCtrl->HideColumn(2);
  m_listCtrl->AllowHideColumn(2, true);
  
  m_listCtrl->SetResizeColumn(1);  
}


// -----------------------------------------------------------------------
void  CNaListView::OnAdd()
{
  CTT32App::instance()->OpenView(_("Edit Association"), wxT("NaEdit"), 0);
}


void  CNaListView::OnEdit()
{
  ListItem *itemPtr = m_listCtrl->GetCurrentItem();
  if (!itemPtr)
    return;

  CTT32App::instance()->OpenView(_("Edit Association"), wxT("NaEdit"), itemPtr->GetID());
}


void  CNaListView::OnDelete()
{
  if (m_listCtrl->GetSelectedCount() == 0)
    return;
    
  if (infoSystem.Confirmation(_("Are you sure to delete selected association(s)?")) == false)
    return;

  for (int idx = 0; idx < m_listCtrl->GetItemCount(); idx++)
  {
    if (!m_listCtrl->IsSelected(idx))
      continue;
      
    ListItem *itemPtr = m_listCtrl->GetListItem(idx);
    if (!itemPtr)
      continue;

    TTDbse::instance()->GetDefaultConnection()->StartTransaction();
  
    if ( NaStore().Remove(itemPtr->GetID()) )
      TTDbse::instance()->GetDefaultConnection()->Commit();
    else
      TTDbse::instance()->GetDefaultConnection()->Rollback();
  }
}


void CNaListView::OnPlayers(wxCommandEvent&)
{
  NaItem * itemPtr = (NaItem *) m_listCtrl->GetCurrentItem();
  if (!itemPtr)
    return;

  long  naID = itemPtr->GetID();

  CTT32App::instance()->OpenView(_("Players from ") + itemPtr->na.naDesc, wxT("PlListView"), naID);
}



void CNaListView::OnTeams(wxCommandEvent&)
{
  NaItem * itemPtr = (NaItem *) m_listCtrl->GetCurrentItem();
  if (!itemPtr)
    return;

  long  naID = itemPtr->GetID();

  CTT32App::instance()->OpenView(_("Teams from ") + itemPtr->na.naDesc, wxT("TmListView"), naID);
}

// -----------------------------------------------------------------------
void CNaListView::OnUpdate(CRequest *reqPtr) 
{
	if (!reqPtr)
  {
    CFormViewEx::OnUpdate(reqPtr);
    return;
  }

  if (reqPtr->rec != CRequest::NAREC)
    return;

  long id = m_listCtrl->GetCurrentItem() ? 
            m_listCtrl->GetCurrentItem()->GetID() : 0;

  switch (reqPtr->type)
  {
    case CRequest::INSERT :
    case CRequest::UPDATE :
    {
      NaListStore  naList;
      if (!naList.SelectById(reqPtr->id))
        return;
      if (!naList.Next())
        return;

      // Add / Set Item Data
      m_listCtrl->RemoveListItem(reqPtr->id);
      m_listCtrl->AddListItem(new NaItem(naList));

      break;
    }

    case CRequest::REMOVE :
    {
      m_listCtrl->RemoveListItem(reqPtr->id);
      break;
    }

    default :
      break;
  }

  m_listCtrl->SortItems();

  if (id)
    m_listCtrl->SetCurrentItem(id);
}

