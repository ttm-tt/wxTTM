/* Copyright (C) 2020 Christoph Theis */

// TmList.cpp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"

#include "TmListView.h"

#include "TmEdit.h"
#include "CpItem.h"
#include "TdItem.h"

#include "TmEntryStore.h"
#include "TmStore.h"

#include "InfoSystem.h"
#include "Request.h"
#include "Rec.h"


// -----------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS(CTmListView, CFormViewEx)

BEGIN_EVENT_TABLE(CTmListView, CFormViewEx)
  EVT_COMBOBOX(XRCID("Event"), OnSelChangeCp)
  EVT_BUTTON(XRCID("Groups"), OnGroups)
END_EVENT_TABLE()


// -----------------------------------------------------------------------
// CTmListView

CTmListView::CTmListView() : CFormViewEx(), m_cbCp(0)
{
}

CTmListView::~CTmListView()
{
}


bool CTmListView::Edit(va_list vaList)
{
  naID = va_arg(vaList, long);

  if (naID)
  {
    // "All Events" entry
    cp.cpID = 0;
    wxStrncpy(cp.cpDesc, _("All Events").c_str(), sizeof(cp.cpDesc) / sizeof(wxChar) - 1);
    m_cbCp->AddListItem(new CpItem(cp));
  }

  // If we select by naID add otption to show show teams for all events, 
  // but add column for event
  // Otherwise hide that column and default to first / default event
  CpListStore  cp;
  cp.SelectAll();
  while (cp.Next())
  {
    if (cp.cpType == CP_TEAM)
      m_cbCp->AddListItem(new CpItem(cp));
  }

  wxString cpName = CTT32App::instance()->GetDefaultCP();

  ListItem *cpItemPtr = m_cbCp->FindListItem(cpName);
  if (naID || !cpItemPtr)
    cpItemPtr = m_cbCp->GetListItem(0);
    
  if (cpItemPtr)
  {
    m_cbCp->SetCurrentItem(cpItemPtr->GetID());

    OnSelChangeCp(wxCommandEvent_);
  }
  
  TransferDataToWindow();
  
  return true;
}


// -----------------------------------------------------------------------
void  CTmListView::OnAdd()
{
  CTT32App::instance()->OpenView(_("Edit Team"), wxT("TmEdit"), 0, cp.cpID, NULL);
}


void  CTmListView::OnEdit()
{
  ListItem *itemPtr = m_listCtrl->GetCurrentItem();
  if (!itemPtr)
    return;

  long  tmID = itemPtr->GetID();
  long  cpID = cp.cpID;
  
  m_listCtrl->SetCurrentIndex(m_listCtrl->GetCurrentIndex() + 1);

  CTT32App::instance()->OpenView(_("Edit Team"), wxT("TmEdit"), tmID, cpID, NULL);
}


void  CTmListView::OnDelete()
{
  if (m_listCtrl->GetSelectedCount() == 0)
    return;
    
  if (infoSystem.Confirmation(_("Are you sure to delete selected team(s)?")) == false)
    return;
    
  TTDbse::instance()->GetDefaultConnection()->StartTransaction();
  
  bool success = true;

  for (int idx = m_listCtrl->GetItemCount(); success && idx--; )
  {
    if (m_listCtrl->IsSelected(idx))
    {
      TmItem *itemPtr = (TmItem *) m_listCtrl->GetListItem(idx);

      if (!itemPtr)
        continue;
        
      success = success && TmStore().Remove(itemPtr->GetID());
    }
  }
  
  if (success)
    TTDbse::instance()->GetDefaultConnection()->Commit();
  else
    TTDbse::instance()->GetDefaultConnection()->Rollback();  
}


void CTmListView::OnInitialUpdate() 
{
	CFormViewEx::OnInitialUpdate();
	
	m_cbCp = XRCCTRL(*this, "Event", CComboBoxEx);
	m_listCtrl = XRCCTRL(*this, "Teams", CListCtrlEx);
	
  // Name, Desc., Assoc.
  m_listCtrl->InsertColumn(0, _("Name"), wxALIGN_LEFT);
  m_listCtrl->InsertColumn(1, _("Description"), wxALIGN_LEFT, GetClientSize().GetWidth());
  m_listCtrl->InsertColumn(2, _("Assoc."), wxALIGN_LEFT);
  
  m_listCtrl->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
  m_listCtrl->SetColumnWidth(2, wxLIST_AUTOSIZE_USEHEADER);
 
  m_listCtrl->SetResizeColumn(1);
}


void CTmListView::OnSelChangeCp(wxCommandEvent &) 
{
  m_listCtrl->RemoveAllListItems();

  ListItem *itemPtr = m_cbCp->GetCurrentItem();
  if (!itemPtr)
    return;

  cp.SelectById(itemPtr->GetID());
  cp.Next();

  TmEntryStore  tmList;
  tmList.SelectTeamByCp(cp);

  while (tmList.Next())
  {
    m_listCtrl->AddListItem(new TdItem(tmList));
  }
  
  m_listCtrl->SortItems();
    
  m_listCtrl->SetCurrentIndex(0);

  CTT32App::instance()->SetDefaultCP(cp.cpName);
}


void CTmListView::OnGroups(wxCommandEvent&)
{
  ListItem* itemPtr = m_listCtrl->GetCurrentItem();
  if (!itemPtr)
    return;

  long  tmID = itemPtr->GetID();

  CTT32App::instance()->OpenView(_("Team Groups"), wxT("TmGroups"), tmID, 0, NULL);
}



// Update einer Mannschaft
void CTmListView::OnUpdate(CRequest *reqPtr) 
{
	if (!reqPtr)
  {
    CFormViewEx::OnUpdate(reqPtr);
    return;
  }

  if (reqPtr->rec != CRequest::TMREC)
    return;

  switch (reqPtr->type)
  {
    case CRequest::INSERT :
    case CRequest::UPDATE :
    {
      TmEntryStore  tmList;
      if (!tmList.SelectTeamById(reqPtr->id, cp.cpType))
        return;
      if (!tmList.Next() || tmList.cpID != cp.cpID)
        return;
        
      tmList.Close();
      
      long id = m_listCtrl->GetCurrentItem() ? 
                m_listCtrl->GetCurrentItem()->GetID() : 0;      

      // Add / Set Item Data
      if (reqPtr->type == CRequest::INSERT)
      {      
        m_listCtrl->AddListItem(new TdItem(tmList));
      }
      else
      {
        TmItem *itemPtr = (TmItem *) m_listCtrl->FindListItem(tmList.tmID);
        if (!itemPtr)
          m_listCtrl->AddListItem(new TdItem(tmList));
        else
          itemPtr->entry = tmList;
      }
      
      m_listCtrl->SortItems();
      
      if (id)
        m_listCtrl->SetCurrentItem(id);
        
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
}


