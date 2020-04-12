/* Copyright (C) 2020 Christoph Theis */

// SyListView.syp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "SyListView.h"

#include "SyListStore.h"
#include "SyStore.h"
#include "SyItem.h"

#include "InfoSystem.h"
#include "Request.h"


IMPLEMENT_DYNAMIC_CLASS(CSyListView, CFormViewEx)


// -----------------------------------------------------------------------
// CSyListView
CSyListView::CSyListView() : CFormViewEx()
{
}


CSyListView::~CSyListView()
{
}


bool CSyListView::Edit(va_list vaList)
{
  SyListStore  syList;
  syList.SelectAll();

  while (syList.Next())
  {
    m_listCtrl->AddListItem(new SyItem(syList));
  }
  
  return true;
}



// -----------------------------------------------------------------------
// CSyListView diagnostics
void CSyListView::OnInitialUpdate() 
{
	CFormViewEx::OnInitialUpdate();
	
	m_listCtrl = XRCCTRL(*this, "Systems", CListCtrlEx);
	
  // Name
  m_listCtrl->InsertColumn(0, _("Name"), wxALIGN_LEFT, 6 * cW);

  // Description
  m_listCtrl->InsertColumn(1, _("Description"), wxALIGN_LEFT);
  
  m_listCtrl->SetResizeColumn(1);
}


void  CSyListView::OnAdd()
{
  CTT32App::instance()->OpenView(_("Edit Team System"), wxT("SyEdit"), 0);
}


void  CSyListView::OnEdit()
{
  ListItem *itemPtr = m_listCtrl->GetCurrentItem();
  if (!itemPtr)
    return;

  CTT32App::instance()->OpenView(_("Edit Team System"), wxT("SyEdit"), itemPtr->GetID());
}


void  CSyListView::OnDelete()
{
  ListItem *itemPtr = m_listCtrl->GetCurrentItem();
  if (!itemPtr)
    return;
    
  if ( infoSystem.Confirmation(_("Are you sure to delete selected team system(s)?")) == false )
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

    if (SyStore().Remove(itemPtr->GetID()))
      TTDbse::instance()->GetDefaultConnection()->Commit();
    else
      TTDbse::instance()->GetDefaultConnection()->Rollback();
  }
}


// -----------------------------------------------------------------------
void CSyListView::OnUpdate(CRequest *reqPtr) 
{
	if (!reqPtr)
  {
    CFormViewEx::OnUpdate(reqPtr);
    return;
  }

  if (reqPtr->rec != CRequest::SYREC)
    return;

  switch (reqPtr->type)
  {
    case CRequest::INSERT :
    case CRequest::UPDATE :
    {
      SyListStore  syList;
      if (!syList.SelectById(reqPtr->id))
        return;
      if (!syList.Next())
        return;

      // Add / Set Item Data
      m_listCtrl->RemoveListItem(reqPtr->id);
      m_listCtrl->AddListItem(new SyItem(syList));
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

