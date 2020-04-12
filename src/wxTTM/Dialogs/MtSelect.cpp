/* Copyright (C) 2020 Christoph Theis */

// MtSelect.cpp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "MtSelect.h"

#include "MtListStore.h"
#include "GrListStore.h"


IMPLEMENT_DYNAMIC_CLASS(CMtSelect, CFormViewEx)

BEGIN_EVENT_TABLE(CMtSelect, CFormViewEx)
  EVT_BUTTON(XRCID("ScoreSheet"), CMtSelect::OnScore)
  EVT_BUTTON(XRCID("Result"), CMtSelect::OnResult)  
END_EVENT_TABLE()


// -----------------------------------------------------------------------
// CMtSelect dialog
CMtSelect::CMtSelect() : CFormViewEx()
{
	m_mtNr = 0;
}


CMtSelect::~CMtSelect()
{
}


// -----------------------------------------------------------------------
// CMtSelect message handlers

void CMtSelect::OnResult(wxCommandEvent &)
{
  TransferDataFromWindow();

  MtListStore  mt;
  mt.SelectByNr(m_mtNr);
  if (mt.Next())
  {
    mt.Close();

    GrListStore gr;
    gr.grID = mt.mtEvent.grID;

    if (gr.QryCombined())
      CTT32App::instance()->OpenView(_("Edit Group Results"), wxT("MtListView"), mt.mtID, false);
    else if (mt.cpType != CP_TEAM)
      CTT32App::instance()->OpenView(_("Edit Result"), wxT("MtRes"), mt.mtID, 0);
    else
      CTT32App::instance()->OpenView(_("Edit Team Result"), wxT("MtTeam"), mt.mtID);
  }

  XRCCTRL(*this, "MatchNo", wxTextCtrl)->SetSelection(0, -1);
}


void CMtSelect::OnScore(wxCommandEvent &) 
{
  TransferDataFromWindow();

  MtListStore  mt;
  mt.SelectByNr(m_mtNr);
  if (mt.Next())
    mt.Close();

  GrListStore gr;
  gr.grID = mt.mtEvent.grID;

  CTT32App::instance()->OpenDialog(true, _("Print Scoresheet"), wxT("Score"), mt.mtID, gr.QryCombined() ? 4 : 0);
}


// -----------------------------------------------------------------------
void CMtSelect::OnInitialUpdate()
{
  CFormViewEx::OnInitialUpdate();

  FindWindow("MatchNo")->SetValidator(CLongValidator(&m_mtNr));  
  
  TransferDataToWindow();
  XRCCTRL(*this, "MatchNo", wxTextCtrl)->SetSelection(0, -1);
}


