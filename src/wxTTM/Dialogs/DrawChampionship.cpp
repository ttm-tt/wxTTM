/* Copyright (C) 2020 Christoph Theis */

// DialogsDrawChampionship.cpp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "DrawChampionship.h"

#include "CpListStore.h"
#include "CpItem.h"
#include  "GrItem.h"
#include "DrawCP.h"
#include "DrawLP.h"
#include "DrawCSD.h"
#include "DrawRR.h"

#include "GrListStore.h"

#include <list>


IMPLEMENT_DYNAMIC_CLASS(CDrawChampionship, CFormViewEx)

BEGIN_EVENT_TABLE(CDrawChampionship, CFormViewEx)
  EVT_COMBOBOX(XRCID("Events"), CDrawChampionship::OnSelChangeCP)
  EVT_COMBOBOX(XRCID("ToStage"), CDrawChampionship::OnSelChangeToStage)
END_EVENT_TABLE()

// -----------------------------------------------------------------------
// CDrawChampionship
CDrawChampionship::CDrawChampionship() : CFormViewEx()
{
  frStage = "Qualification";
  toStage = "Championship";
  what = Championship;
  fromPos = 0;
  toPos = 0;
  toGroup = 0;
  seed = 0;
  lpsolve = true;
}


CDrawChampionship::~CDrawChampionship()
{
}


// -----------------------------------------------------------------------
bool CDrawChampionship::Edit(va_list vaList)
{
  what = (DrawType) va_arg(vaList, int);
  
  switch (what)
  {
    case Championship : // Top half of groups into KO
      toStage = "Championship";

      FindWindow("FromPosLabel")->Show(false);
      FindWindow("FromPos")->Show(false);
      FindWindow("ToPosLabel")->Show(false);
      FindWindow("ToPos")->Show(false);
      FindWindow("ToGroupLabel")->Show(false);
      FindWindow("ToGroup")->Show(false);

      fromPos = 1;
      toPos = 2;

      break;
      
    case Consolation : // Bottom half of groups into KO
      toStage = "Consolation";

      FindWindow("FromPosLabel")->Show(false);
      FindWindow("FromPos")->Show(false);
      FindWindow("ToPosLabel")->Show(false);
      FindWindow("ToPos")->Show(false);
      FindWindow("ToGroupLabel")->Show(false);
      FindWindow("ToGroup")->Show(false);

      fromPos = 3;
      toPos = 4;

      break;
      
    case CSD : // Direct into KO and optional Q-Groups)
      frStage = "";
      toStage = "Championship";

      FindWindow("FromPosLabel")->Show(false);
      FindWindow("FromPos")->Show(false);
      FindWindow("ToPosLabel")->Show(false);
      FindWindow("ToPos")->Show(false);
      FindWindow("ToGroupLabel")->Show(false);
      FindWindow("ToGroup")->Show(false);
      FindWindow("RankingLabel")->Show(false);
      FindWindow("Ranking")->Show(false);

      fromPos = 1;
      toPos = 2;

      // Texte aendern
      FindWindow("From Stage")->SetLabel(_("Qualification"));
      FindWindow("To Stage")->SetLabel(_("Championship"));

      // Nicht benoetigte Eingaben verstecken
      FindWindow("cbLpsolve")->Hide();
      FindWindow("labelSeed")->Hide();
      FindWindow("textSeed")->Hide();

      // Re-Layout
      SendSizeEvent();

      break;

    case FromQualification : // Configurable which positions from Qualification into KO
      toStage = "Championship";
      break;

    case FromKO : // Losers of KO round into Consolation
      frStage = "Championship";
      toStage = "Consolation";

      fromPos = 1;
      toPos = 1;

      FindWindow("FromPosLabel")->SetLabel(_("From round"));
      FindWindow("ToPosLabel")->SetLabel(_("To round"));

      FindWindow("cbLpsolve")->Hide();

      // Re-Layout
      SendSizeEvent();

      break;

    case GroupsFromQualification: // Qualification into another stage with groups
      frStage = "Qualification";

      fromPos = 1;
      toPos = 2;

      FindWindow("cbLpsolve")->Hide();

      // Re-Layout
      SendSizeEvent();

      break;
  }

  if (what != 3 && what != 4)
  {
  }
  
  OnSelChangeCP(wxCommandEvent());
  
  TransferDataToWindow();

  return true;
}
  

// -----------------------------------------------------------------------
// CDrawChampionship message handlers
void CDrawChampionship::OnInitialUpdate() 
{
	CFormViewEx::OnInitialUpdate();

  cbCP = XRCCTRL(*this, "Events", CComboBoxEx);
  cbFromStage = XRCCTRL(*this, "FromStage", wxComboBox);
  cbToStage = XRCCTRL(*this, "ToStage", wxComboBox);
  cbToGroup = XRCCTRL(*this, "ToGroup", CComboBoxEx);
  
  FindWindow("FromStage")->SetValidator(wxGenericValidator(&frStage));
  FindWindow("ToStage")->SetValidator(wxGenericValidator(&toStage));

  FindWindow("FromPos")->SetValidator(wxGenericValidator(&fromPos));
  FindWindow("ToPos")->SetValidator(wxGenericValidator(&toPos));

  FindWindow("textSeed")->SetValidator(wxGenericValidator(&seed));

  FindWindow("cbLpsolve")->SetValidator(wxGenericValidator(&lpsolve));
	
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

#ifndef _DEBUG
  FindWindow("labelSeed")->Hide();
  FindWindow("textSeed")->Hide();
  FindWindow("cbLpsolve")->Hide();
#endif

  // Erst auf Edit hin die Stufen auslesen
}


// -----------------------------------------------------------------------
unsigned int CDrawChampionship::DrawThread(void *arg)
{
  CDrawChampionship *wnd = (CDrawChampionship *) arg;
  
  CpItem *item = (CpItem *) wnd->cbCP->GetCurrentItem();
  if (item)
  {  
    Connection *connPtr = TTDbse::instance()->GetNewConnection();
    connPtr->SetAutoCommit(false);

    if (wnd->seed == 0)
      wnd->seed = (int) time(NULL);

    if (wnd->what == 2)  // CSD
    {
      DrawCSD  draw(connPtr);
      draw.Draw(item->cp, wnd->toStage, wnd->frStage, wnd->seed);
                // frStage != "" ? (const char *) wnd->frStage : (const char *) 0);
    }
    else if (wnd->what == 1) // Consolation
    {
      if (wnd->lpsolve)
      {
        DrawLP  draw(false, false, connPtr);
        draw.Draw(item->cp, wnd->frStage, wnd->toStage, 0, 0, 0, wnd->rkChoice, wnd->seed);
      }
      else
      {
        DrawCP  draw(false, connPtr);
        draw.Draw(item->cp, wnd->frStage, wnd->toStage, 0, 0, wnd->seed);
      }
    }
    else if (wnd->what == 0) // Championship
    {  
      if (wnd->lpsolve)
      {
        DrawLP  draw(true, false, connPtr);
        draw.Draw(item->cp, wnd->frStage, wnd->toStage, 0, 0, 0, wnd->rkChoice, wnd->seed);
      }
      else
      {
        DrawCP  draw(true, connPtr);
        draw.Draw(item->cp, wnd->frStage, wnd->toStage, 0, 0, wnd->seed);
      }
    }
    else if (wnd->what == 3) // From Qualification groups
    {
      if (wnd->lpsolve)
      {
        DrawLP draw(true, false, connPtr);
        draw.Draw(item->cp, wnd->frStage, wnd->toStage, wnd->fromPos ? wnd->fromPos : 1, wnd->toPos, wnd->toGroup, wnd->rkChoice, wnd->seed);
      }
      else
      {
        DrawCP draw(true, connPtr);
        draw.Draw(item->cp, wnd->frStage, wnd->toStage, wnd->fromPos ? wnd->fromPos : 1, wnd->toPos, wnd->seed);
      }
    }
    else if (wnd->what == 4) // 'Consolation from KO
    {
      DrawLP draw(false, true, connPtr);
      draw.Draw(item->cp, wnd->frStage, wnd->toStage, wnd->fromPos ? wnd->fromPos : 1, wnd->toPos, 0, wnd->rkChoice, wnd->seed);
    }
    else if (wnd->what == 5) // From groups to groups
    {
      DrawRR draw(connPtr);
      draw.Draw(item->cp, wnd->frStage, wnd->toStage, wnd->fromPos ? wnd->fromPos : 1, wnd->toPos, wnd->rkChoice, wnd->seed);
    }
  }
  
  CTT32App::instance()->ProgressBarExit(0);

  wxSleep(2);
    
  wnd->FindWindow("OK")->Enable(true);
  wnd->FindWindow("Cancel")->Enable(true);

  return 0;
}



void  CDrawChampionship::OnOK()
{
  TransferDataFromWindow();

  CpItem *item = (CpItem *) cbCP->GetCurrentItem();
  if (!item)
    return;
    
  CTT32App::instance()->SetDefaultCP(item->cp.cpName);

  if (what == 3)
    toGroup = ((GrItem *) cbToGroup->GetCurrentItem())->gr.grID;
  else
    toGroup = 0;

  rkChoice = (RankingChoice) XRCCTRL(*this, "Ranking", wxRadioBox)->GetSelection();

  FindWindow("OK")->Enable(false);
  FindWindow("Cancel")->Enable(false);

#ifndef _DEBUG
  seed = 0;
#endif

  CTT32App::instance()->ProgressBarThread(DrawThread, this, "Draw", 100, true);
  
  // Warte, bis der andere Thread sicher laeuft.
  wxSleep(1);
}


void CDrawChampionship::OnSelChangeCP(wxCommandEvent &)
{
  cbFromStage->Clear();
  cbToStage->Clear();

  if (!cbCP->GetCurrentItem())
    return;

  // CSD: Quali kann leer sein
  if (what == 2)
    cbFromStage->Append("");

  std::list<wxString> stages = GrListStore().ListStages( ((CpItem *) cbCP->GetCurrentItem())->cp );
  while (stages.size() > 0)
  {
    wxString s = *stages.begin();
    stages.pop_front();

    cbFromStage->Append(s);
    cbToStage->Append(s);
  }

  int frIdx = cbFromStage->FindString(frStage);
  int toIdx = cbToStage->FindString(toStage);

  if (frIdx == wxNOT_FOUND)
    frIdx = 0;
  if (toIdx = wxNOT_FOUND)
    toIdx = cbToStage->GetCount() - 1;

  cbFromStage->SetSelection(frIdx);
  cbToStage->SetSelection(toIdx);

  seed = 0;

  TransferDataToWindow();

  OnSelChangeToStage(wxCommandEvent());
}


void CDrawChampionship::OnSelChangeToStage(wxCommandEvent &)
{
  if (what != 3)
    return;

  FindWindow("ToGroupLabel")->Show(false);
  FindWindow("ToGroup")->Show(false);

  cbToGroup->Clear();

  GrListStore gr;
  cbToGroup->AddListItem(new GrItem(gr));

  toStage = cbToStage->GetString(cbToStage->GetCurrentSelection());
  toGroup = 0;

  if (toStage.IsEmpty())
    return;

  CpRec cp = ((CpItem *) cbCP->GetCurrentItem())->cp;

  if (GrStore().CountGroups(cp, toStage) < 2)
    return;

  gr.SelectByStage(cp, toStage);
  while (gr.Next())
    cbToGroup->AddListItem(new GrItem(gr));

  cbToGroup->SetSelection(0);

  FindWindow("ToGroupLabel")->Show(true);
  FindWindow("ToGroup")->Show(true);
}



