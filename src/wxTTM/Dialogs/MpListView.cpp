/* Copyright (C) Christoph Theis 2022 */

#include "stdafx.h"
#include "TT32App.h"

#include "MpListView.h"

#include "MpListStore.h"
#include "MpStore.h"
#include "MpItem.h"

#include "InfoSystem.h"
#include "Request.h"


IMPLEMENT_DYNAMIC_CLASS(CMpListView, CFormViewEx)


// -----------------------------------------------------------------------
CMpListView::CMpListView() : CFormViewEx()
{
}


CMpListView::~CMpListView()
{
}


bool CMpListView::Edit(va_list vaList)
{
  MpListStore  mpList;
  mpList.SelectAll();

  while (mpList.Next())
  {
    m_listCtrl->AddListItem(new MpItem(mpList));
  }
  
  return true;
}



// -----------------------------------------------------------------------
// CMpListView diagnostics
void CMpListView::OnInitialUpdate() 
{
	CFormViewEx::OnInitialUpdate();
	
	m_listCtrl = XRCCTRL(*this, "Items", CListCtrlEx);
	
  // Name
  m_listCtrl->InsertColumn(0, _("Name"), wxALIGN_LEFT, 6 * cW);

  // Description
  m_listCtrl->InsertColumn(1, _("Description"), wxALIGN_LEFT);
  
  m_listCtrl->SetResizeColumn(1);
}


void  CMpListView::OnAdd()
{
  CTT32App::instance()->OpenView(_("Edit Match Points"), wxT("MpEdit"), 0);
}


void  CMpListView::OnEdit()
{
  ListItem *itemPtr = m_listCtrl->GetCurrentItem();
  if (!itemPtr)
    return;

  CTT32App::instance()->OpenView(_("Edit Match Points"), wxT("MpEdit"), itemPtr->GetID());
}


void  CMpListView::OnDelete()
{
  ListItem *itemPtr = m_listCtrl->GetCurrentItem();
  if (!itemPtr)
    return;
    
  if ( infoSystem.Confirmation(_("Are you sure to delete selected match point(s))?")) == false )
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

    if (MpStore().Remove(itemPtr->GetID()))
      TTDbse::instance()->GetDefaultConnection()->Commit();
    else
      TTDbse::instance()->GetDefaultConnection()->Rollback();
  }
}


// -----------------------------------------------------------------------
void CMpListView::OnUpdate(CRequest *reqPtr) 
{
	if (!reqPtr)
  {
    CFormViewEx::OnUpdate(reqPtr);
    return;
  }

  if (reqPtr->rec != CRequest::MPREC)
    return;

  switch (reqPtr->type)
  {
    case CRequest::INSERT :
    case CRequest::UPDATE :
    {
      MpListStore  mpList;
      if (!mpList.SelectById(reqPtr->id))
        return;
      if (!mpList.Next())
        return;

      // Add / Set Item Data
      m_listCtrl->RemoveListItem(reqPtr->id);
      m_listCtrl->AddListItem(new MpItem(mpList));
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


