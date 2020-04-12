/* Copyright (C) 2020 Christoph Theis */

// CpListView.cpp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "CpListView.h"

#include "CpListStore.h"
#include "CpItem.h"

#include "InfoSystem.h"
#include "Request.h"


IMPLEMENT_DYNAMIC_CLASS(CCpListView, CFormViewEx)

BEGIN_EVENT_TABLE(CCpListView, CFormViewEx)
  EVT_BUTTON(XRCID("Entries"), CCpListView::OnEntries)
END_EVENT_TABLE()


// -----------------------------------------------------------------------
// CCpListView
CCpListView::CCpListView() : CFormViewEx()
{
}


CCpListView::~CCpListView()
{
}


// -----------------------------------------------------------------------
void CCpListView::SaveSettings()
{
  m_listCtrl->SaveColumnInfo(GetClassInfo()->GetClassName());

  CFormViewEx::SaveSettings();
}


void CCpListView::RestoreSettings()
{
  CFormViewEx::RestoreSettings();

  m_listCtrl->RestoreColumnInfo(GetClassInfo()->GetClassName());
}


bool CCpListView::Edit(va_list vaList)
{
  CpListStore  cpList;
  cpList.SelectAll();

  m_listCtrl->AllowHideColumn(2, false);

  while (cpList.Next())
  {
    m_listCtrl->AddListItem(new CpItem(cpList));
    if (cpList.cpYear)
      m_listCtrl->AllowHideColumn(2, true);
  }
  
  TransferDataToWindow();
  
  return true;
}
  

// -----------------------------------------------------------------------
// CCpListView message handlers
void CCpListView::OnInitialUpdate() 
{
	CFormViewEx::OnInitialUpdate();
	
	m_listCtrl = XRCCTRL(*this, "Events", CListCtrlEx);	

  // Name
  m_listCtrl->InsertColumn(0, _("Name"), wxALIGN_LEFT, 6 * cW);

  // Description
  m_listCtrl->InsertColumn(1, _("Description"), wxALIGN_LEFT);

  // Year
  m_listCtrl->InsertColumn(2, _("Year"), wxALIGN_LEFT);
  m_listCtrl->HideColumn(2);
  m_listCtrl->AllowHideColumn(2, true);

  // Type
  m_listCtrl->InsertColumn(3, _("Type"), wxALIGN_LEFT);
  m_listCtrl->HideColumn(3);
  m_listCtrl->AllowHideColumn(3, true);

  // Ssex
  m_listCtrl->InsertColumn(4, _("Sex"), wxALIGN_LEFT);
  m_listCtrl->HideColumn(4);
  m_listCtrl->AllowHideColumn(4, true);

  m_listCtrl->SetResizeColumn(1);
}


void  CCpListView::OnAdd()
{
  CTT32App::instance()->OpenView(_("Edit Event"), wxT("CpEdit"), 0);
}


void  CCpListView::OnEdit()
{
  ListItem *itemPtr = m_listCtrl->GetCurrentItem();
  if (!itemPtr)
    return;

  CTT32App::instance()->OpenView(_("Edit Event"), wxT("CpEdit"), itemPtr->GetID());
}


void  CCpListView::OnDelete()
{
  if (m_listCtrl->GetSelectedCount() == 0)
    return;
    
  if (infoSystem.Confirmation(_("Are you sure to delete selected event(s)?")) == false)
    return;

  for (int idx = m_listCtrl->GetItemCount(); idx--;)
  {
    if (!m_listCtrl->IsSelected(idx))
      continue;
      
    ListItem *itemPtr = m_listCtrl->GetListItem(idx);
    if (!itemPtr)
      continue;

    TTDbse::instance()->GetDefaultConnection()->StartTransaction();

    if (CpStore().Remove(itemPtr->GetID()))
      TTDbse::instance()->GetDefaultConnection()->Commit();
    else
      TTDbse::instance()->GetDefaultConnection()->Rollback();
  }
}


void CCpListView::OnEntries(wxCommandEvent &) 
{
  ListItem *itemPtr = m_listCtrl->GetCurrentItem();
  if (!itemPtr)
    return;

  CTT32App::instance()->OpenView(_("Edit Entries"), wxT("CpEntries"), itemPtr->GetID());
}


void CCpListView::OnUpdate(CRequest *reqPtr) 
{
	if (!reqPtr)
  {
    CFormViewEx::OnUpdate(reqPtr);
    return;
  }

  if (reqPtr->rec != CRequest::CPREC)
    return;

  switch (reqPtr->type)
  {
    case CRequest::INSERT :
    case CRequest::UPDATE :
    {
      long id = m_listCtrl->GetCurrentItem() ? m_listCtrl->GetCurrentItem()->GetID() : 0;

      CpListStore  cpList;
      if (!cpList.SelectById(reqPtr->id))
        return;
      if (!cpList.Next())
        return;

      // Add / Set Item Data
      m_listCtrl->RemoveListItem(reqPtr->id);
      m_listCtrl->AddListItem(new CpItem(cpList));

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

