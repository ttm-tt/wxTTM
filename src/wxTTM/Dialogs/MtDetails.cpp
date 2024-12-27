/* Copyright (C) 2024 Christoph Theis */

// MtDetails.cpp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "MtDetails.h"

#include "CpItem.h"
#include "GrItem.h"
#include "TmItem.h"

#include "MtListStore.h"

#include "InfoSystem.h"


// -----------------------------------------------------------------------
// CMtDetails

IMPLEMENT_DYNAMIC_CLASS(CMtDetails, CFormViewEx)

BEGIN_EVENT_TABLE(CMtDetails, CFormViewEx)
END_EVENT_TABLE()


CMtDetails::CMtDetails() : CFormViewEx()
{
}

CMtDetails::~CMtDetails()
{
}


bool  CMtDetails::Edit(va_list vaList)
{
  long id = va_arg(vaList, long);

  mt.SelectById(id);
  mt.Next();
  mt.Close();

  gr.SelectById(mt.mtEvent.grID);
  gr.Next();

  cp.SelectById(gr.cpID);
  cp.Next();
  
  TmEntryStore tm;

  tm.SelectTeamById(mt.tmA, cp.cpType);
  tm.Next();
  tmA = tm;

  tm.SelectTeamById(mt.tmX, cp.cpType);
  tm.Next();
  tmX = tm;

  cpItem->SetListItem(new CpItem(cp));
  grItem->SetListItem(new GrItem(gr));
  tmAItem->SetListItem(new TmItem(tmA));
  tmXItem->SetListItem(new TmItem(tmX));

  if (mt.mtEvent.mtRound > gr.grQualRounds)
    XRCCTRL(*this, "Round", wxTextCtrl)->SetValue(wxString::Format("%d", mt.mtEvent.mtRound - gr.grQualRounds));
  else if (gr.grQualRounds == 1)
    XRCCTRL(*this, "Round", wxTextCtrl)->SetValue(wxString::Format(_("Qu.")));
  else
    XRCCTRL(*this, "Round", wxTextCtrl)->SetValue(wxString::Format(_("Qu.") + " %d", mt.mtEvent.mtRound));

  FindWindow("lblToss")->Show(cp.cpType == CP_TEAM);
  FindWindow("Toss")->Show(cp.cpType == CP_TEAM);

  TransferDataToWindow();

  return true;
}


// -----------------------------------------------------------------------
// Message handling

void CMtDetails::OnInitialUpdate() 
{
	CFormViewEx::OnInitialUpdate();
	
	cpItem = XRCCTRL(*this, "Event", CItemCtrl);
	grItem = XRCCTRL(*this, "Group", CItemCtrl);
		
  tmAItem = XRCCTRL(*this, "TeamA", CItemCtrl);
  tmXItem = XRCCTRL(*this, "TeamX", CItemCtrl);

  cpItem->SetItemHeight(1);
  grItem->SetItemHeight(1);
  tmAItem->SetItemHeight(2);
  tmXItem->SetItemHeight(2);

  FindWindow("Round")->SetValidator(CShortValidator(&mt.mtEvent.mtRound));
  FindWindow("Match")->SetValidator(CShortValidator(&mt.mtEvent.mtMatch));
  FindWindow("MatchNo")->SetValidator(CLongValidator(&mt.mtNr));
  FindWindow("Scheduled")->SetValidator(CDateTimeValidator(&mt.mtPlace.mtDateTime));

  FindWindow("Score")->SetValidator(CDateTimeValidator(&mt.mtDetails.mtPrintScoreTime));
  FindWindow("Checked")->SetValidator(CDateTimeValidator(&mt.mtDetails.mtCheckMatchTime));
  FindWindow("Started")->SetValidator(CDateTimeValidator(&mt.mtDetails.mtStartMatchTime));
  FindWindow("Ended")->SetValidator(CDateTimeValidator(&mt.mtDetails.mtEndMatchTime));
  FindWindow("Toss")->SetValidator(CDateTimeValidator(&mt.mtDetails.mtPrintTossTime));

  Layout();
}
