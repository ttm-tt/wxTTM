/* Copyright (C) 2020 Christoph Theis */

// MdListView.mdp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "MdListView.h"

#include "MdListStore.h"
#include "MdStore.h"
#include "MdItem.h"

#include "InfoSystem.h"
#include "Request.h"



IMPLEMENT_DYNAMIC_CLASS(CMdListView, CFormViewEx)

// -----------------------------------------------------------------------
// CMdListView

CMdListView::CMdListView() : CFormViewEx()
{
}


CMdListView::~CMdListView()
{
}


bool CMdListView::Edit(va_list vaList)
{
  MdListStore  mdList;
  mdList.SelectAll();

  while (mdList.Next())
  {
    m_listCtrl->AddListItem(new MdItem(mdList));
  }

  return true;
}


// -----------------------------------------------------------------------
// CMdListView message handlers

void CMdListView::OnInitialUpdate() 
{
	CFormViewEx::OnInitialUpdate();
	
	m_listCtrl = XRCCTRL(*this, "Modi", CListCtrlEx);
	
  // Name
  m_listCtrl->InsertColumn(0, _("Name"), wxALIGN_LEFT, 6 * cW);

  // Description
  m_listCtrl->InsertColumn(1, _("Description"));

  m_listCtrl->SetResizeColumn(1);
}


void  CMdListView::OnAdd()
{
  CTT32App::instance()->OpenView(_("Edit Group Modus"), wxT("MdEdit"), 0);
}


void  CMdListView::OnEdit()
{
  ListItem *itemPtr = m_listCtrl->GetCurrentItem();
  if (!itemPtr)
    return;

  CTT32App::instance()->OpenView(_("Edit Group Modus"), wxT("MdEdit"), itemPtr->GetID());
}


void  CMdListView::OnDelete()
{
  ListItem *itemPtr = m_listCtrl->GetCurrentItem();
  if (!itemPtr)
    return;
    
  if ( infoSystem.Confirmation(_("Are you sure to delete selected group modi?")) == false )
    return;

  TTDbse::instance()->GetDefaultConnection()->StartTransaction();

  for (int idx = m_listCtrl->GetItemCount(); idx--;)
  {
    if (!m_listCtrl->IsSelected(idx))
      continue;
      
    ListItem *itemPtr = m_listCtrl->GetListItem(idx);
    if (!itemPtr)
      continue;

    TTDbse::instance()->GetDefaultConnection()->StartTransaction();

    if (MdStore().Remove(itemPtr->GetID()))
      TTDbse::instance()->GetDefaultConnection()->Commit();
    else
      TTDbse::instance()->GetDefaultConnection()->Rollback();
  }
}


// -----------------------------------------------------------------------
void CMdListView::OnUpdate(CRequest *reqPtr) 
{
	if (!reqPtr)
  {
    CFormViewEx::OnUpdate(reqPtr);
    return;
  }

  if (reqPtr->rec != CRequest::MDREC)
    return;

  switch (reqPtr->type)
  {
    case CRequest::INSERT :
    case CRequest::UPDATE :
    {
      MdListStore  mdList;
      if (!mdList.SelectById(reqPtr->id))
        return;
      if (!mdList.Next())
        return;

      // Add / Set Item Data
      m_listCtrl->RemoveListItem(reqPtr->id);
      m_listCtrl->AddListItem(new MdItem(mdList));
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

