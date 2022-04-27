/* Copyright (C) 2020 Christoph Theis */

// RkListView.cpp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "RkListView.h"

#include "NaListStore.h"
#include "RkListStore.h"

#include "CpItem.h"
#include "NaItem.h"

#include "Request.h"


class NaItemEx : public NaItem
{
  public:
    NaItemEx(const NaListRec &na) : NaItem(na) 
    {
      de = qu = 0;
    }
    
    void SetDirectEntries(int d) {de = d;}
    void SetQualifiers(int q)    {qu = q;}
    
    void DrawColumn(wxDC *pDC, int col, wxRect &rect)
    {
      switch (col)
      {
        case 0 :
        case 1 :
          NaItem::DrawColumn(pDC, col, rect);
          break;
          
        case 2 :
          DrawLong(pDC, rect, de);
          break;
          
        case 3 :
          DrawLong(pDC, rect, qu);
          break;
      }
    }
    
  private:
    int  de;
    int  qu;
};



// -----------------------------------------------------------------------
// CRkListView

IMPLEMENT_DYNAMIC_CLASS(CRkListView, CFormViewEx)

BEGIN_EVENT_TABLE(CRkListView, CFormViewEx)
  EVT_COMBOBOX(XRCID("Event"), CRkListView::OnSelChangeCp)
  EVT_BUTTON(XRCID("Change"), CRkListView::OnChangeDECount)
END_EVENT_TABLE()

CRkListView::CRkListView() : CFormViewEx()
{
}


CRkListView::~CRkListView()
{
}


bool CRkListView::Edit(va_list vaList)
{
  CpListStore  cp;
  cp.SelectAll();
  while (cp.Next())
    m_cbCp->AddListItem(new CpItem(cp));

  wxString cpName = CTT32App::instance()->GetDefaultCP();

  ListItem *cpItemPtr = m_cbCp->FindListItem(cpName);
  if (!cpItemPtr)
    cpItemPtr = m_cbCp->GetListItem(0);
  if (!cpItemPtr)
    return true;
    
  m_cbCp->SetCurrentItem(cpItemPtr->GetID());

  TransferDataToWindow();
  
  OnSelChangeCp(wxCommandEvent_);
  
  return true;
}  


// -----------------------------------------------------------------------
void CRkListView::OnInitialUpdate() 
{
	CFormViewEx::OnInitialUpdate();
	
	m_cbCp = XRCCTRL(*this, "Event", CComboBoxEx);
	m_listCtrl = XRCCTRL(*this, "Associations", CListCtrlEx);
	
  // Name (absichtlich NALIST, da es Nationen sind)
  m_listCtrl->InsertColumn(0, _("Name"), wxALIGN_LEFT, 8*cW);

  // Description
  m_listCtrl->InsertColumn(1, _("Description"));
  
  // Direct Entries
  m_listCtrl->InsertColumn(2, "DE", wxALIGN_LEFT, 4 * cW);
  
  // Qualifiers
  m_listCtrl->InsertColumn(3, "QU", wxALIGN_LEFT, 4 * cW);

  m_listCtrl->ResizeColumn(1);
  
  // Heuristisch
  m_listCtrl->SetColumnWidth(1, m_listCtrl->GetColumnWidth(1) - 18);
}


// -----------------------------------------------------------------------
// CRkListView message handlers
void CRkListView::OnSelChangeCp(wxCommandEvent &) 
{
  ListItem *itemPtr = m_cbCp->GetCurrentItem();
  if (!itemPtr)
    return;

  cp.SelectById(itemPtr->GetID());
  cp.Next();

  CTT32App::instance()->SetDefaultCP(cp.cpName);

  m_listCtrl->RemoveAllListItems();

  NaListStore  na;
  na.SelectAll();

  while (na.Next())
  {
    m_listCtrl->AddListItem(new NaItemEx(na));
  }
  
  int de = 0, qu = 0;
  for (int idx = m_listCtrl->GetItemCount(); idx--; )
  {
    NaItemEx *itemPtr = (NaItemEx *) m_listCtrl->GetListItem(idx);
    int tmpDE = RkListStore().CountDirectEntries(cp, itemPtr->na);
    int tmpQU = RkListStore().CountQualifiers(cp, itemPtr->na);
    
    if (tmpDE == 0 && tmpQU == 0)
    {
      m_listCtrl->DeleteItem(idx);
    }
    else
    {
      itemPtr->SetDirectEntries(tmpDE);
      itemPtr->SetQualifiers(tmpQU);
      
      de += tmpDE;
      qu += tmpQU;
    }
  }
  
  wxString tmp;
  
  tmp = wxString::Format("%d", de);
  XRCCTRL(*this, "DirectEntries", wxTextCtrl)->SetValue(tmp);

  tmp = wxString::Format("%d", qu);
  XRCCTRL(*this, "Qualifiers", wxTextCtrl)->SetValue(tmp);
}


void CRkListView::OnChangeDECount(wxCommandEvent &)
{
  CTT32App::instance()->OpenView(_("Edit Ranking"), wxT("RkEdit"), cp.cpID, 0);
}


void  CRkListView::OnEdit()
{
  ListItem *itemPtr = m_listCtrl->GetCurrentItem();
  if (!itemPtr)
    return;
    
  m_listCtrl->SetCurrentIndex(m_listCtrl->GetCurrentIndex()+1);

  CTT32App::instance()->OpenView(_("Edit Ranking"), wxT("RkEdit"), cp.cpID, itemPtr->GetID());

}

// -----------------------------------------------------------------------
void CRkListView::OnUpdate(CRequest *reqPtr) 
{
	if (!reqPtr)
  {
    CFormViewEx::OnUpdate(reqPtr);
    return;
  }
  
  // Absichtlich NAREC (siehe RkEdit)
  if (reqPtr->rec != CRequest::NAREC)
    return;
    
  int de = RkListStore().CountDirectEntries(cp, NaRec());
  int qu = RkListStore().CountQualifiers(cp, NaRec());
  
  wxString tmp;
  
  tmp = wxString::Format("%d", de);
  XRCCTRL(*this, "DirectEntries", wxTextCtrl)->SetValue(tmp);

  tmp = wxString::Format("%d", qu);
  XRCCTRL(*this, "Qualifiers", wxTextCtrl)->SetValue(tmp);
  
  if (reqPtr->type == CRequest::UPDATE)
  {
    NaItemEx * itemPtr = (NaItemEx *) m_listCtrl->FindListItem(reqPtr->id);
    if (itemPtr)
    {
      int tmpDE = RkListStore().CountDirectEntries(cp, itemPtr->na);
      int tmpQU = RkListStore().CountQualifiers(cp, itemPtr->na);
    
      itemPtr->SetDirectEntries(tmpDE);
      itemPtr->SetQualifiers(tmpQU); 
      
      m_listCtrl->Refresh();     
    }
  }
}
