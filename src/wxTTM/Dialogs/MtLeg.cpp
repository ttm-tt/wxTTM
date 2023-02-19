/* Copyright (C) 2020 Christoph Theis */

#include "stdafx.h"
#include "TT32App.h"

#include "MtLeg.h"

#include "CpItem.h"
#include "GrItem.h"
#include "MtItem.h"


IMPLEMENT_DYNAMIC_CLASS(CMtLeg, CFormViewEx)


BEGIN_EVENT_TABLE(CMtLeg, CFormViewEx)
  EVT_COMBOBOX(XRCID("Event"), CMtLeg::OnSelChangeCp)
  EVT_COMBOBOX(XRCID("Group"), CMtLeg::OnSelChangeGr)
END_EVENT_TABLE()

CMtLeg::CMtLeg() : CFormViewEx()
{

}


CMtLeg::~CMtLeg()
{

}


bool CMtLeg::Edit(va_list vaList)
{
  long id = va_arg(vaList, long);

  wxString cpName = CTT32App::instance()->GetDefaultCP();
  wxStrncpy(cp.cpName, cpName, sizeof(cp.cpName) / sizeof(wxChar) - 1);

  wxString grName = CTT32App::instance()->GetDefaultGR();
  wxStrncpy(gr.grName, grName, sizeof(gr.grName) / sizeof(wxChar) - 1);

  // Liste der WB
  CpListStore cpList;
  cpList.SelectAll();
  while (cpList.Next())
    m_cbCp->AddListItem(new CpItem(cpList));

  ListItem* cpItemPtr = m_cbCp->FindListItem(cp.cpName);
  if (!cpItemPtr)
    cpItemPtr = m_cbCp->GetListItem(0);
  if (!cpItemPtr)
    return true;

  m_cbCp->SetCurrentItem(cpItemPtr->GetID());

  OnSelChangeCp(wxCommandEvent_);

  return true;
}


// -----------------------------------------------------------------------
void CMtLeg::OnInitialUpdate()
{
  m_cbCp = XRCCTRL(*this, "Event", CComboBoxEx);
  m_cbGr = XRCCTRL(*this, "Group", CComboBoxEx);
  m_listCtrl = XRCCTRL(*this, "MtList", CListCtrlEx);
}


void CMtLeg::OnSelChangeCp(wxCommandEvent&)
{
  m_cbGr->RemoveAllListItems();
  m_listCtrl->RemoveAllListItems();

  CpItem* cpItemPtr = (CpItem*)m_cbCp->GetCurrentItem();
  if (!cpItemPtr)
    return;

  cp = cpItemPtr->cp;

  CTT32App::instance()->SetDefaultCP(cp.cpName);

  GrListStore grList;
  grList.SelectAll(cp);

  while (grList.Next())
    m_cbGr->AddListItem(new GrItem(grList));

  // Erste Gruppe auswaehlen
  ListItem* grItemPtr = m_cbGr->FindListItem(GrItem(gr).GetLabel());
  if (!grItemPtr)
    grItemPtr = m_cbGr->GetListItem(0);
  if (!grItemPtr)
    return;

  m_cbGr->SetCurrentItem(grItemPtr->GetID());

  OnSelChangeGr(wxCommandEvent_);
}


void CMtLeg::OnSelChangeGr(wxCommandEvent&)
{

}