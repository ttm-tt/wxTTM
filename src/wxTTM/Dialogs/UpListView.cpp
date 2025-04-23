/* Copyright (C) 2020 Christoph Theis */

// UpListView.cpp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "UpListView.h"

#include "UpListStore.h"
#include "UpItem.h"
#include "UpStore.h"

#include "InfoSystem.h"
#include "Request.h"

#include <list>

// -----------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS(CUpListView, CFormViewEx)

BEGIN_EVENT_TABLE(CUpListView, CFormViewEx)
  EVT_BUTTON(XRCID("Restore"), CUpListView::OnRestore)
END_EVENT_TABLE()


const char * CUpListView::headers[] = 
{
  wxTRANSLATE("Ump. Nr."),
  wxTRANSLATE("Name"),
  wxTRANSLATE("Assoc."),
  wxTRANSLATE("Sex"),
  NULL
};

// -----------------------------------------------------------------------
CUpListView::CUpListView() : CFormViewEx(), m_listCtrl(0)
{
}

CUpListView::~CUpListView()
{
}



// -----------------------------------------------------------------------
void CUpListView::SaveSettings()
{
  m_listCtrl->SaveColumnInfo(GetClassInfo()->GetClassName());

  CFormViewEx::SaveSettings();
}


void CUpListView::RestoreSettings()
{
  CFormViewEx::RestoreSettings();

  m_listCtrl->RestoreColumnInfo(GetClassInfo()->GetClassName());
}


// -----------------------------------------------------------------------
bool CUpListView::Edit(va_list vaList)
{
  UpListStore  upList;
  upList.SelectAll();

  while (upList.Next())
  {
    m_listCtrl->AddListItem(new UpItem(upList));
  }
  
  m_listCtrl->ResizeColumn(1);
  

  m_listCtrl->SortItems();
  
  m_listCtrl->SetCurrentIndex(0);
  
  return true;
}


// -----------------------------------------------------------------------
void  CUpListView::OnAdd()
{
  if (!CTT32App::instance()->IsLicenseValid())
    return;

  CTT32App::instance()->OpenView(_("Add Umpire"), wxT("UpEdit"));
}


void  CUpListView::OnEdit()
{
  ListItem *itemPtr = m_listCtrl->GetCurrentItem();
  if (!itemPtr)
    return;

  CTT32App::instance()->OpenView(_("Edit Umpire"), wxT("UpEdit"), itemPtr->GetID());
}


void  CUpListView::OnDelete()
{
  if (!CTT32App::instance()->IsLicenseValid())
    return;

  if (m_listCtrl->GetSelectedItemCount() > 0)
  {
    if (infoSystem.Confirmation(_("Are you sure to delete selected umpires?")) == false)
      return;

    bool doIt = true;

    for (int idx = m_listCtrl->GetItemCount(); doIt && idx--; )
    {
      if (m_listCtrl->IsSelected(idx))
      {
        UpItem *itemPtr = (UpItem *) m_listCtrl->GetListItem(idx);

        if (!itemPtr)
          continue;
              
        TTDbse::instance()->GetDefaultConnection()->StartTransaction();
        
        if ( UpStore().Remove( ((UpItem *) itemPtr)->up.upID ) )
          TTDbse::instance()->GetDefaultConnection()->Commit();
        else
          TTDbse::instance()->GetDefaultConnection()->Rollback();
      }
    }
  }
}


void CUpListView::OnRestore(wxCommandEvent&)
{
  bool doIt = true;

  for (int idx = m_listCtrl->GetItemCount(); doIt && idx--; )
  {
    if (m_listCtrl->IsSelected(idx))
    {
      UpItem *itemPtr = (UpItem *) m_listCtrl->GetListItem(idx);

      if (!itemPtr)
        continue;

      UpStore up;
      up.SelectById(itemPtr->up.upID);
      up.Next();
      up.Close();

      up.Restore();
    }
  }
}


// -----------------------------------------------------------------------
void CUpListView::OnInitialUpdate() 
{
  m_listCtrl = XRCCTRL(*this, "Umpires", CListCtrlEx);

  for (size_t i = 0; headers[i]; i++)
  {
    m_listCtrl->InsertColumn(i, wxGetTranslation(headers[i]), wxLIST_FORMAT_LEFT, i == 1 ? GetClientSize().GetWidth() : wxLIST_AUTOSIZE_USEHEADER);
  }

  for (size_t i = 0; headers[i]; i++)
  {
    if (i != 1)
      m_listCtrl->SetColumnWidth(i, wxLIST_AUTOSIZE_USEHEADER);
  }

  m_listCtrl->AllowHideColumn(0);
  m_listCtrl->AllowHideColumn(2);
  m_listCtrl->AllowHideColumn(3);
  m_listCtrl->AllowHideColumn(4);

  m_listCtrl->SetResizeColumn(1);

  FindWindow("Add")->Enable(CTT32App::instance()->IsLicenseValid());
  FindWindow("Delete")->Enable(CTT32App::instance()->IsLicenseValid());
  // FindWindow("Events")->Enable(CTT32App::instance()->IsLicenseValid());
}


void CUpListView::OnUpdate(CRequest *reqPtr) 
{
	if (!reqPtr)
  {
    CFormViewEx::OnUpdate(reqPtr);
    return;
  }

  switch (reqPtr->type)
  {
    case CRequest::INSERT :
    case CRequest::UPDATE :
    {
      if (reqPtr->rec != CRequest::UPREC)
        return;
        
      UpListStore  upList;
      if (!upList.SelectById(reqPtr->id))
        return;
      if (!upList.Next())
        return;
        
      upList.Close();
        
      long id = m_listCtrl->GetCurrentItem() ? 
                m_listCtrl->GetCurrentItem()->GetID() : 0;

      // Add / Set Item Data
      if (reqPtr->type == CRequest::INSERT)
      {
        m_listCtrl->AddListItem(new UpItem(upList));
        id = reqPtr->id;
      }
      else
      {
        UpItem *itemPtr = (UpItem *) m_listCtrl->FindListItem(reqPtr->id);
        if (itemPtr)
          itemPtr->up = upList;
      }
      
      m_listCtrl->SortItems();
      
      if (id)
        m_listCtrl->SetCurrentItem(id);
      else
        m_listCtrl->SetCurrentItem(reqPtr->id);
        
      break;
    }

    case CRequest::REMOVE :
    {
      int idx = m_listCtrl->GetCurrentIndex();

      switch (reqPtr->rec)
      {
        case CRequest::PSREC :
        {
          // Es koennte das aktuelle sein
          if ( m_listCtrl->GetCurrentItem() && 
               ((UpItem *) m_listCtrl->GetCurrentItem())->up.psID == reqPtr->id)
          {
            m_listCtrl->DeleteItem(m_listCtrl->GetCurrentIndex());
            break;
          }
        
          // Ansonsten suchen
          for (int i = m_listCtrl->GetItemCount(); i--; )
          {
            if ( ((UpItem *) m_listCtrl->GetListItem(i))->up.psID == reqPtr->id)
            {
              m_listCtrl->DeleteItem(i);
              break;
            }
          }
        }

        break;

        case CRequest::UPREC :
        {      
          // Hier sucht m_listCtrl selbst
          m_listCtrl->RemoveListItem(reqPtr->id);
          break;
        }

        default :
          break;
      }

      if (m_listCtrl->GetSelectedCount() > 1)
        ;
      else if (idx < m_listCtrl->GetCount())
        m_listCtrl->SetCurrentIndex(idx);
      else if (idx > 0)
        m_listCtrl->SetCurrentIndex(idx - 1);
      else if (m_listCtrl->GetCount())
        m_listCtrl->SetCurrentIndex(0);

      break;
    }

    default :
      break;
  }
}

