/* Copyright (C) 2020 Christoph Theis */

// GrOptions.cpp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "GrOptions.h"

#include "Validators.h"


IMPLEMENT_DYNAMIC_CLASS(CGrOptions, wxDialog)

BEGIN_EVENT_TABLE(CGrOptions, wxDialog)
  EVT_INIT_DIALOG(CGrOptions::OnInitDialog)

  EVT_CHECKBOX(XRCID("RRIncludeMatchDetails"), CGrOptions::OnCheckBoxSelected)
  EVT_CHECKBOX(XRCID("RRPrintSelectedRounds"), CGrOptions::OnCheckBoxSelected)
  EVT_CHECKBOX(XRCID("RRCombinedScoresheet"), CGrOptions::OnCheckBoxSelected)
  EVT_CHECKBOX(XRCID("KOPrintIndividualMatches"), CGrOptions::OnCheckBoxSelected)
  EVT_CHECKBOX(XRCID("KOPrintSelectedRounds"), CGrOptions::OnCheckBoxSelected)
  EVT_CHECKBOX(XRCID("KOPrintSelectedMatches"), CGrOptions::OnCheckBoxSelected)

  EVT_BUTTON(XRCID("SaveAs"), CGrOptions::OnSaveAs)
END_EVENT_TABLE()


// -----------------------------------------------------------------------
// CGrOptions dialog
CGrOptions::CGrOptions() : wxDialog()
{
  m_poPtr = NULL;
}


CGrOptions::~CGrOptions()
{
}


// -----------------------------------------------------------------------
void CGrOptions::OnInitDialog(wxInitDialogEvent &evt)
{
  wxDialog::OnInitDialog(evt);
  
  FindWindow("RRTreatByesLikePlayers")->SetValidator(wxGenericValidator(&m_poPtr->rrIgnoreByes));
  FindWindow("RRIncludeMatchDetails")->SetValidator(wxGenericValidator(&m_poPtr->rrResults));
  FindWindow("RRPrintScheduleInRaster")->SetValidator(wxGenericValidator(&m_poPtr->rrSchedule));
  FindWindow("RRPrintindividualMatches")->SetValidator(wxGenericValidator(&m_poPtr->rrTeamDetails));
  FindWindow("RRStartNewPage")->SetValidator(wxGenericValidator(&m_poPtr->rrNewPage));
  FindWindow("RRPrintSelectedRounds")->SetValidator(wxGenericValidator(&m_poPtr->rrSlctRound));
  FindWindow("RRFromRound")->SetValidator(CShortValidator(&m_poPtr->rrFromRound));
  FindWindow("RRToRound")->SetValidator(CShortValidator(&m_poPtr->rrToRound));
  FindWindow("RRCombinedScoresheet")->SetValidator(wxGenericValidator(&m_poPtr->rrCombined));
  FindWindow("RRPrintParticipateConsolation")->SetValidator(wxGenericValidator(&m_poPtr->rrConsolation));
  FindWindow("RRResultFromLastPlayedRound")->SetValidator(wxGenericValidator(&m_poPtr->rrLastResults));
  FindWindow("RRPrintNotes")->SetValidator(wxGenericValidator(&m_poPtr->rrPrintNotes));

  FindWindow("KOTreatByesLikePlayers")->SetValidator(wxGenericValidator(&m_poPtr->koIgnoreByes));
  FindWindow("KOPrintIndividualMatches")->SetValidator(wxGenericValidator(&m_poPtr->koTeamDetails));
  FindWindow("KOStartNewPage")->SetValidator(wxGenericValidator(&m_poPtr->koNewPage));
  FindWindow("KOPrintSelectedRounds")->SetValidator(wxGenericValidator(&m_poPtr->koSlctRound));
  FindWindow("KONoQualification")->SetValidator(wxGenericValidator(&m_poPtr->koNoQuRounds));
  FindWindow("KORounds")->SetValidator(wxGenericValidator(&koRounds));
  FindWindow("KOLastRounds")->SetValidator(wxGenericValidator(&koLastRounds));
  FindWindow("KOPrintSelectedMatches")->SetValidator(wxGenericValidator(&m_poPtr->koSlctMatch));
  FindWindow("KOMatches")->SetValidator(wxGenericValidator(&koMatches));
  FindWindow("KOLastMatches")->SetValidator(wxGenericValidator(&koLastMatches));
  FindWindow("KOResultFromLastPlayedRound")->SetValidator(wxGenericValidator(&m_poPtr->koLastResults));
  FindWindow("KOPrintPosNumbers")->SetValidator(wxGenericValidator(&m_poPtr->koPrintPosNrs));
  FindWindow("KOPrintNotes")->SetValidator(wxGenericValidator(&m_poPtr->koPrintNotes));
    
  FindWindow("RRResultFromLastPlayedRound")->Enable(m_poPtr->rrResults);
  FindWindow("RRPrintindividualMatches")->Enable(m_poPtr->rrResults);
  
  FindWindow("RRFromRound")->Enable(m_poPtr->rrSlctRound);
  FindWindow("RRToRound")->Enable(m_poPtr->rrSlctRound);
  
  FindWindow("RRPrintParticipateConsolation")->Enable(m_poPtr->rrCombined);
  
  FindWindow("KOResultFromLastPlayedRound")->Enable(m_poPtr->koTeamDetails);
  
  FindWindow("KONoQualification")->Enable(m_poPtr->koSlctRound);
  FindWindow("KORounds")->Enable(m_poPtr->koSlctRound);
  FindWindow("KOLastRounds")->Enable(m_poPtr->koSlctRound);
  
  FindWindow("KOMatches")->Enable(m_poPtr->koSlctMatch);
  FindWindow("KOLastMatches")->Enable(m_poPtr->koSlctMatch);
  
  TransferDataToWindow();
}


void CGrOptions::OnCheckBoxSelected(wxCommandEvent &evt)
{
  if (evt.GetId() == XRCID("RRIncludeMatchDetails"))
  {
    bool val = XRCCTRL(*this, "RRIncludeMatchDetails", wxCheckBox)->GetValue();

    FindWindow("RRResultFromLastPlayedRound")->Enable(val);
    FindWindow("RRPrintindividualMatches")->Enable(val);  
  }
  else if (evt.GetId() == XRCID("RRPrintSelectedRounds"))
  {
    bool val = XRCCTRL(*this, "RRPrintSelectedRounds", wxCheckBox)->GetValue();

    FindWindow("RRFromRound")->Enable(val);
    FindWindow("RRToRound")->Enable(val);
  }
  else if (evt.GetId() == XRCID("RRCombinedScoresheet"))
  {
    bool val = XRCCTRL(*this, "RRCombinedScoresheet", wxCheckBox)->GetValue();
  
    FindWindow("RRPrintParticipateConsolation")->Enable(val);
  }
  else if (evt.GetId() == XRCID("KOPrintIndividualMatches"))
  {
    bool val = XRCCTRL(*this, "KOPrintIndividualMatches", wxCheckBox)->GetValue();

    FindWindow("KOResultFromLastPlayedRound")->Enable(val);
  }
  else if (evt.GetId() == XRCID("KOPrintSelectedRounds"))
  {
    bool val = XRCCTRL(*this, "KOPrintSelectedRounds", wxCheckBox)->GetValue();

    FindWindow("KONoQualification")->Enable(val);
    FindWindow("KORounds")->Enable(val);
    FindWindow("KOLastRounds")->Enable(val);
  }
  else if (evt.GetId() == XRCID("KOPrintSelectedMatches"))
  {
    bool val = XRCCTRL(*this, "KOPrintSelectedMatches", wxCheckBox)->GetValue();

    FindWindow("KOMatches")->Enable(val);
    FindWindow("KOLastMatches")->Enable(val);
  }
}


void CGrOptions::OnSaveAs(wxCommandEvent &)
{
  if (!Validate() || !TransferDataFromWindow())
    return;

  EndDialog(XRCID("SaveAs"));
}

void CGrOptions::SetPrintRasterOptions(PrintRasterOptions *ptr)
{
  // PrintRasterOptions merken und Daten in eigene Variablen kopieren
  m_poPtr = ptr;

  if (m_poPtr->koLastRounds)
    koRounds = m_poPtr->koFromRound;
  else
    koRounds = m_poPtr->koToRound;

  koLastRounds = m_poPtr->koLastRounds;

  if (m_poPtr->koLastMatches)
    koMatches = m_poPtr->koFromMatch;
  else
    koMatches = m_poPtr->koToMatch;

  koLastMatches = m_poPtr->koLastMatches;
}


bool CGrOptions::TransferDataFromWindow()
{
  // Basisklasse aufrufen und Daten von eigenen Variablen nach PrintRasterOptions kopieren
  if (!wxDialog::TransferDataFromWindow())
    return false;

  if (koLastRounds)
  {
    m_poPtr->koFromRound = koRounds;
    m_poPtr->koToRound = 0;
  }
  else
  {
    m_poPtr->koFromRound = 0;
    m_poPtr->koToRound = koRounds;
  }

  m_poPtr->koLastRounds = koLastRounds;

  if (koLastMatches)
  {
    m_poPtr->koFromMatch = koMatches;
    m_poPtr->koToMatch = 0;
  }
  else
  {
    m_poPtr->koFromMatch = 0;
    m_poPtr->koToMatch = koMatches;
  }

  m_poPtr->koLastMatches = koLastMatches;

  // koInbox auf 0 setzen
  m_poPtr->koInbox = 0;

  return true;
}