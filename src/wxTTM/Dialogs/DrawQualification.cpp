/* Copyright (C) 2020 Christoph Theis */

// DrawQualification.cpp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "DrawQualification.h"

#include "GrListStore.h"

#include "CpItem.h"

#include "DrawRR.h"



IMPLEMENT_DYNAMIC_CLASS(CDrawQualification, CFormViewEx)

BEGIN_EVENT_TABLE(CDrawQualification, CFormViewEx)
  EVT_COMBOBOX(XRCID("Events"), CDrawQualification::OnSelChangeCP)
END_EVENT_TABLE()


// -----------------------------------------------------------------------
// CDrawQualification
CDrawQualification::CDrawQualification() : CFormViewEx()
{
  stage = "Qualification";
  seed = 0;
}


CDrawQualification::~CDrawQualification()
{
}


// -----------------------------------------------------------------------
// CDrawQualification message handlers

void CDrawQualification::OnInitialUpdate() 
{
	CFormViewEx::OnInitialUpdate();
	
	cbCP = XRCCTRL(*this, "Events", CComboBoxEx);
  cbStage = XRCCTRL(*this, "Stage", wxComboBox);

	FindWindow("Stage")->SetValidator(wxGenericValidator(&stage));

  FindWindow("textSeed")->SetValidator(wxGenericValidator(&seed));

# ifndef _DEBUG
  FindWindow("labelSeed")->Hide();
  FindWindow("textSeed")->Hide();
# endif
	
  CpListStore  cp;
  cp.SelectAll();
  while (cp.Next())
    cbCP->AddListItem(new CpItem(cp));

  // Item auswaehlen und WB auslesen
  wxString cpName = CTT32App::instance()->GetDefaultCP();
  ListItem *cpItemPtr = (cpName.Length() ? cbCP->FindListItem(cpName) : 0);
  if (cpItemPtr == 0)
    cpItemPtr = cbCP->GetListItem(0);
  if (cpItemPtr)
    cbCP->SetCurrentItem(cpItemPtr->GetID());    
  else
    cbCP->SetCurrentItem(cbCP->GetListItem(0));

  OnSelChangeCP(wxCommandEvent());
}


void CDrawQualification::OnSelChangeCP(wxCommandEvent &)
{
  cbStage->Clear();

  if (!cbCP->GetCurrentItem())
    return;

  std::list<wxString> stages = GrListStore().ListStages( ((CpItem *) cbCP->GetCurrentItem())->cp );
  while (stages.size() > 0)
  {
    wxString s = *stages.begin();
    stages.pop_front();

    cbStage->Append(s);
  }

  int idx = cbStage->FindString(stage);
  if (idx == wxNOT_FOUND)
    idx = 0;

  cbStage->SetSelection(idx);

  seed = 0;

  TransferDataToWindow();
}

// -----------------------------------------------------------------------

unsigned int CDrawQualification::DrawThread(void *arg)
{
  CDrawQualification *wnd = (CDrawQualification *) arg;
  
  CpItem *item = (CpItem *) wnd->cbCP->GetCurrentItem();
  if (item)
  {
    // Debug Heap
    _CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF);  
  
    DrawRR  draw(TTDbse::instance()->GetNewConnection());
    draw.Draw(item->cp, wxEmptyString, wnd->stage, 0, 0, World, wnd->seed ? wnd->seed : (int) time(NULL));
  }

  CTT32App::instance()->ProgressBarExit(0);

  wxSleep(2);
  
  wnd->FindWindow("OK")->Enable(true);
  wnd->FindWindow("Cancel")->Enable(true);
  
  return 0;
}


void  CDrawQualification::OnOK()
{
  TransferDataFromWindow();

  CpItem *item = (CpItem *) cbCP->GetCurrentItem();
  if (!item)
    return;
    
  CTT32App::instance()->SetDefaultCP(item->cp.cpName);

  FindWindow("OK")->Enable(false);
  FindWindow("Cancel")->Enable(false);

#ifndef _DEBUG
  seed = 0;
#endif

  CTT32App::instance()->ProgressBarThread(DrawThread, this, "Draw", 100, true);
  
  // Warten, bis der andere Thread sicher laeuft.
  Sleep(250);  
}
