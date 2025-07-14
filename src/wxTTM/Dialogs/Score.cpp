/* Copyright (C) 2020 Christoph Theis */

// Score.cpp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "Score.h"

#include "CpListStore.h"
#include "GrListStore.h"
#include "MtListStore.h"
#include "MtEntryStore.h"

#include "Printer.h"

#include "RasterSC.h"
#include "RasterRR.h"

#include "Profile.h"
#include "Res.h"

#include <set>
#include <vector>


IMPLEMENT_DYNAMIC_CLASS(CScore, CFormViewEx)

BEGIN_EVENT_TABLE(CScore, CFormViewEx)
  EVT_CHECKBOX(XRCID("CombinedScoresheet"), CScore::OnCombined)
  EVT_RADIOBUTTON(XRCID("IndividualMatch"), CScore::OnSelectMatch)
  EVT_RADIOBUTTON(XRCID("ThisMatch"), CScore::OnSelectMatch)
  EVT_RADIOBUTTON(XRCID("ThisRound"), CScore::OnSelectMatch)
  EVT_RADIOBUTTON(XRCID("ThisGroup"), CScore::OnSelectMatch)
  EVT_RADIOBUTTON(XRCID("ScheduledMatches"), CScore::OnSelectMatch)
  EVT_BUTTON(XRCID("ResetPrinted"), CScore::OnResetPrinted)
  EVT_BUTTON(XRCID("MarkPrinted"), CScore::OnMarkPrinted)
  EVT_BUTTON(XRCID("Print"), CFormViewEx::OnCommand)
END_EVENT_TABLE()


// -----------------------------------------------------------------------
// CScore

enum
{
  Individual = 1,
  Match = 2,
  Round = 3,
  Group = 4,
  Selected = 5
};


CScore::CScore() : CFormViewEx()
{

  m_matchOption = Match;
  m_notPrinted = true;
  m_inclTeam = false;
  m_combined = false;
  m_consolation = false; 

  memset(&fromPlace, 0, sizeof(MtStore::MtPlace));
  memset(&toPlace, 0, sizeof(MtStore::MtPlace));

  time_t t = time(0);
  struct tm *tm = localtime(&t);

  fromPlace.mtDateTime.year  = tm->tm_year + 1900;
  fromPlace.mtDateTime.month = tm->tm_mon + 1;
  fromPlace.mtDateTime.day   = tm->tm_mday;

  fromPlace.mtDateTime.hour = 0;
  fromPlace.mtDateTime.minute = 0;

  fromPlace.mtTable = 0;

  toPlace = fromPlace;

  toPlace.mtDateTime.hour = 23;
  toPlace.mtDateTime.minute = 59;

  toPlace.mtTable = 999;
}


CScore::~CScore()
{
}


bool CScore::Edit(va_list vaList)
{
  long  id = va_arg(vaList, long);
  short matchOption = va_arg(vaList, int); 
  short fromTable = (matchOption == Selected ? va_arg(vaList, short) : 0);
  short toTable = (matchOption == Selected ? va_arg(vaList, short) : 999);
  timestamp fromTime = (matchOption == Selected ? va_arg(vaList, timestamp) : timestamp());
  timestamp toTime = (matchOption == Selected ? va_arg(vaList, timestamp) : timestamp());

  MtListStore  tmpMt;
  tmpMt.SelectById(id);
  tmpMt.Next();
  tmpMt.Close();

  mt = tmpMt;

  if (tmpMt.mtID)
  {
    GrListStore tmpGr;
    tmpGr.SelectById(tmpMt.mtEvent.grID);
    tmpGr.Next();

    gr = tmpGr;
    
    CpListStore tmpCp;
    tmpCp.SelectById(tmpGr.cpID);
    tmpCp.Next();

    cp = tmpCp;
    
    if (tmpMt.mtPlace.mtDateTime.year > 0)
    {
      fromPlace = toPlace = tmpMt.mtPlace;
      
      fromPlace.mtTable = fromTable;
      toPlace.mtTable = toTable;
    }
    
    if (matchOption > 0)
    {
      m_matchOption = matchOption;
    }

    if (matchOption == Group)
    {
      m_combined = true;
      m_consolation = CTT32App::instance()->GetType() == TT_SCI;
    }
  }
  else
  {
    m_matchOption = Selected;
    fromPlace.mtDateTime = fromTime;
    toPlace.mtDateTime = toTime;
  }

  if (matchOption == Selected)
  {
    // We are combined if there is at least one case where 2 matches of a group are on one table.
    bool combined = false;

    std::set<long> grIDs;

    MtListStore  mt, lastMt;
    mt.SelectByTime(fromPlace.mtDateTime, fromPlace.mtTable, toPlace.mtDateTime, toPlace.mtTable);


    while (mt.Next())
    {
      grIDs.insert(mt.mtEvent.grID);
      if (mt.mtEvent.grID != lastMt.mtEvent.grID)
      {
        // Group on different (not adjectant) tables / times: not combined
        if (grIDs.find(mt.mtEvent.grID) != grIDs.end())
          combined = false;
      }
      else if (mt.mtPlace.mtDateTime != lastMt.mtPlace.mtDateTime || mt.mtPlace.mtTable != lastMt.mtPlace.mtTable)
      {
        // Group on different tables / times: not combined 
        combined = false;
      }
      else
      {
        // 2 matches on a table: must be combined
        combined = true;
      }

      lastMt = mt;
    }

    if (combined)
    {
      m_combined = true;
      m_consolation = CTT32App::instance()->GetType() == TT_SCI;
    }
  }

  if (m_matchOption == Selected)
  {
    // Wurde "Print Scheduled" vorgegeben, alles andere disablen.
    // Diese Vorgabe kommt (eigentlich) nur von OvList und 
    // das Spiel hat keine besondere Bedeutung, als die Zeit
    // vorzugeben.
    FindWindow("ThisMatch")->Enable(false);
    FindWindow("IndividualMatch")->Enable(false);
    FindWindow("ThisRound")->Enable(false);
    FindWindow("ThisGroup")->Enable(false);
  }

  // TODO: Einzelspiele in Mannschaften drucken
  // GetDlgItem(IDC_SCORE_INCLTEAM)->EnableWindow(false);  

  TransferDataToWindow();
  
  OnSelectMatch(wxCommandEvent_);
  
  return true;
}


// -----------------------------------------------------------------------
// CScore message handlers

void CScore::OnInitialUpdate() 
{
	CFormViewEx::OnInitialUpdate();
	
	FindWindow("DateFrom")->SetValidator(CDateValidator(&fromPlace.mtDateTime));
	FindWindow("DateUntil")->SetValidator(CDateValidator(&toPlace.mtDateTime));
	FindWindow("TimeFrom")->SetValidator(CTimeValidator(&fromPlace.mtDateTime, false));
	FindWindow("TimeUntil")->SetValidator(CTimeValidator(&toPlace.mtDateTime, false));
	FindWindow("TableFrom")->SetValidator(CShortValidator(&fromPlace.mtTable));
	FindWindow("TableUntil")->SetValidator(CShortValidator(&toPlace.mtTable));
	
	FindWindow("IndividualMatch")->SetValidator(CEnumValidator(&m_matchOption, Individual));
	FindWindow("ThisMatch")->SetValidator(CEnumValidator(&m_matchOption, Match));
	FindWindow("ThisRound")->SetValidator(CEnumValidator(&m_matchOption, Round));
	FindWindow("ThisGroup")->SetValidator(CEnumValidator(&m_matchOption, Group));
	FindWindow("ScheduledMatches")->SetValidator(CEnumValidator(&m_matchOption, Selected));

  FindWindow("NotPrinted")->SetValidator(wxGenericValidator(&m_notPrinted));
  FindWindow("CombinedScoresheet")->SetValidator(wxGenericValidator(&m_combined));
  FindWindow("PrintParticipateConsolation")->SetValidator(wxGenericValidator(&m_consolation));
}

void CScore::OnCombined(wxCommandEvent &)
{
  bool state = XRCCTRL(*this, "CombinedScoresheet", wxCheckBox)->GetValue();
  FindWindow("PrintParticipateConsolation")->Enable(state);
}


void  CScore::OnSelectMatch(wxCommandEvent &)
{
  TransferDataFromWindow();

  switch (m_matchOption)
  {
    case Individual : // Individual match
    case Match :      // This match
    case Round :      // This round
    case Group :      // This group
      break;
    case Selected :   // Selected matches
      break;

    default :
      break;
  }

  if (m_matchOption == Group) // This group
    FindWindow(XRCID("NotPrinted"))->Enable(gr.grModus != MOD_RR);
  else
    FindWindow(XRCID("NotPrinted"))->Enable(m_matchOption > 2);

  FindWindow(XRCID("DateFrom"))->Enable(m_matchOption == Selected);
  FindWindow(XRCID("DateUntil"))->Enable(m_matchOption == Selected);
  FindWindow(XRCID("TimeFrom"))->Enable(m_matchOption == Selected);
  FindWindow(XRCID("TimeUntil"))->Enable(m_matchOption == Selected);
  FindWindow(XRCID("TableFrom"))->Enable(m_matchOption == Selected);
  FindWindow(XRCID("TableUntil"))->Enable(m_matchOption == Selected);
  
  // Combined Scoresheet nur bei "This Group" oder "Scheduled"
  FindWindow(XRCID("CombinedScoresheet"))->Enable(m_matchOption >= Group);
  FindWindow(XRCID("PrintParticipateConsolation"))->Enable(m_matchOption >= Group && m_combined);
}


// -----------------------------------------------------------------------
void CScore::OnKillFocus(wxFocusEvent &evt)
{
  evt.Skip();

  wxValidator *val = FindWindow(evt.GetId())->GetValidator();
  if (val->IsKindOf(CLASSINFO(CDateValidator)) || val->IsKindOf(CLASSINFO(CTimeValidator)))
  {
    val->TransferFromWindow();
    val->TransferToWindow();
  }  
}

// -----------------------------------------------------------------------
void CScore::OnResetPrinted(wxCommandEvent &)
{
  DoSetPrinted(false);
}


void CScore::OnMarkPrinted(wxCommandEvent &)
{
  DoSetPrinted(true);
}


void CScore::DoSetPrinted(boolean f)
{
  TransferDataFromWindow();
  
  TTDbse::instance()->GetDefaultConnection()->StartTransaction();
  
  switch (m_matchOption)
  {
    case Individual : // Individual Match
      break;
      
    case Match : // This Match
    {
      MtStore().UpdateScorePrinted(mt.mtID, f);
      break;
    }
    
    case Round : // This Round
    {
      MtStore().UpdateScorePrintedForRound(gr.grID, mt.mtEvent.mtRound, f);
      break;
    }
    
    case Group : // This Group
    {
      MtStore().UpdateScorePrintedForGroup(gr.grID, f);
      break;
    }
    
    case Selected: // Scheduled
    {
      MtStore().UpdateScorePrintedScheduled(fromPlace, toPlace, f);
      break; 
    }
  }
  
  TTDbse::instance()->GetDefaultConnection()->Commit();
  
  CFormViewEx::Close();
}


// -----------------------------------------------------------------------
void  CScore::OnPrint()
{
  TransferDataFromWindow();

  DoPrint();

  CFormViewEx::Close();
}


// -----------------------------------------------------------------------
void  CScore::DoPrint()
{
  if (m_matchOption == Selected)
  {
    DoPrintScheduled();
    return;
  }

  bool showDlg = !wxGetKeyState(WXK_SHIFT);

  if (CTT32App::instance()->GetPrintPreview())
    m_printer = new PrinterPreview(_("Print Scoresheet"));
  else if (CTT32App::instance()->GetPrintPdf())
  {
    wxFileDialog fileDlg(
      this, wxFileSelectorPromptStr, CTT32App::instance()->GetPath(), wxString::Format("Score.pdf"), 
      wxT("PDF Files (*.pdf)|*.pdf|All Files (*.*)|*.*||"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (fileDlg.ShowModal() != wxID_OK)
      return;

    m_printer = new PrinterPdf(fileDlg.GetPath());
  }
  else
    m_printer = new Printer(showDlg);

  if (m_printer->PrinterAborted())
    return;

  if (!m_printer->StartDoc(_("Print Scoresheet")))
    return;

  switch (m_matchOption)
  {
    case Individual : // Individual match
      DoPrintMatch();
      break;

    case Match : // This match 
      DoPrintMatch();
      break;

    case Round : // This round
      DoPrintRound();
      break;

    case Group : // This group
      DoPrintGroup();
      break;

    case Selected: // Secheduled matches
     DoPrintScheduled();
     break;

    default :
      break;
  }

  m_printer->EndPage();
  m_printer->EndDoc();

  delete m_printer;
  m_printer = 0;
}


void  CScore::DoPrintMatch()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  MtEntryStore tmp(connPtr);

  tmp.SelectById(mt.mtID, cp.cpType);
  if (!tmp.Next())
    return;

  tmp.Close();

  m_printer->StartPage();

  RasterScore  raster(m_printer, connPtr);
  raster.Print(cp, gr, tmp);
  
  m_printer->EndPage();

  if (!CTT32App::instance()->GetPrintPreview())
    MtStore(connPtr).UpdateScorePrinted(tmp.mt.mtID, true);
}


void  CScore::DoPrintRound()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  std::vector<MtEntry *> mtList;

  MtEntryStore  tmp(connPtr);
  tmp.SelectByGr(gr, mt.mtEvent.mtRound, cp.cpType);
  while (tmp.Next())
    mtList.push_back(new MtEntry(tmp));
    
  connPtr->StartTransaction();

  for (std::vector<MtEntry *>::iterator it = mtList.begin();
       it != mtList.end(); it++)
  {
    MtEntry &mt = *(*it);
    
    if (m_notPrinted && mt.mt.mtScorePrinted)
      continue;

    if (!mt.mt.stA || !mt.mt.stX)
      continue;

    if (mt.mt.IsABye() || mt.mt.IsXBye())
      continue;

    m_printer->StartPage();
    
    RasterScore raster(m_printer, connPtr);
    raster.Print(cp, gr, mt);
    
    m_printer->EndPage();

    if (!CTT32App::instance()->GetPrintPreview())
      MtStore(connPtr).UpdateScorePrinted(mt.mt.mtID, true);
  }
  
  connPtr->Commit();
}


void  CScore::DoPrintGroup()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  if (m_combined && gr.grModus == MOD_RR)
  {
    PrintRasterOptions po;
    memset(&po, 0, sizeof(PrintRasterOptions));
    po.rrResults = 1;
    po.rrCombined = m_combined;
    po.rrConsolation = m_consolation;

    RasterRR raster(m_printer, connPtr);
    int offsetX = 0, offsetY = 0;
    int page = 0;
    
    m_printer->StartPage();
    
    raster.Print(cp, gr, po, &offsetX, &offsetY, &page);
    raster.PrintMatches(cp, gr, po, &offsetX, &offsetY, &page);
    
    m_printer->EndPage();
    
    if (!CTT32App::instance()->GetPrintPreview())
      MtStore(connPtr).UpdateScorePrintedForGroup(gr.grID, true);
    
    return;
  }
  
  std::vector<MtEntry *> mtList;

  MtEntryStore  tmp(connPtr);
  tmp.SelectByGr(gr, 0, cp.cpType);
  while (tmp.Next())
    mtList.push_back(new MtEntry(tmp));
    
  connPtr->StartTransaction();

  for (std::vector<MtEntry *>::iterator it = mtList.begin();
      it != mtList.end(); it++)
  {
    MtEntry &mt = *(*it);
    
    if (m_notPrinted && mt.mt.mtScorePrinted)
      continue;

    if (!mt.mt.stA || !mt.mt.stX)
      continue;

    if (mt.mt.IsABye() || mt.mt.IsXBye())
      continue;

    m_printer->StartPage();
    
    RasterScore raster(m_printer, connPtr);
    raster.Print(cp, gr, mt);
    
    m_printer->EndPage();

    if (!CTT32App::instance()->GetPrintPreview())
      MtStore(connPtr).UpdateScorePrinted(mt.mt.mtID, true);
  }

  connPtr->Commit();
}


void  CScore::DoPrintScheduled()
{
  int total = 0;
  int printed = 0;

  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  std::vector<MtListRec *> *mtList = new std::vector<MtListRec *>;

  MtListStore  mt(connPtr);
  mt.SelectByTime(fromPlace.mtDateTime, fromPlace.mtTable, 
                  toPlace.mtDateTime, toPlace.mtTable);
  while (mt.Next())
  {
    total++;
    
    if (m_notPrinted && mt.mtScorePrinted)
    {
      printed++;
      continue;
    }

    if (!mt.stA || !mt.stX)
      continue;

    if (mt.IsABye() || mt.IsXBye())
    {
      total--;
      continue;
    }

    mtList->push_back(new MtListRec(mt));
  }

  if (mtList->size() == 0)
  {
    // TODO: Message dialog
    delete mtList;
    return;
  }

  char str[128];
  sprintf(str, "Print %d of %d score sheets", (int) (mtList->size() + printed), total);

  PrintScheduledScoreStruct *tmp = new PrintScheduledScoreStruct;

  if (CTT32App::instance()->GetPrintPreview())
    tmp->printer = new PrinterPreview(_("Print Scoresheets"));
  else if (CTT32App::instance()->GetPrintPdf())
  {
    wxFileDialog fileDlg(
      this, wxFileSelectorPromptStr, CTT32App::instance()->GetPath(), wxString::Format("Score.pdf"), 
      wxT("PDF Files (*.pdf)|*.pdf|All Files (*.*)|*.*||"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (fileDlg.ShowModal() != wxID_OK)
      return;

    tmp->printer = new PrinterPdf(fileDlg.GetPath());
  }
  else
    tmp->printer = new Printer;

  if (tmp->printer->PrinterAborted())
  {
    delete tmp->printer;
    delete tmp;

    return;
  }

  tmp->isPreview = CTT32App::instance()->GetPrintPreview();

  tmp->isCombined = m_combined;  

  tmp->rrConsolation = m_consolation;

  tmp->mtList = mtList;

  CTT32App::instance()->ProgressBarThread(PrintScheduledThread, tmp, str, mtList->size());
}


unsigned CScore::PrintScheduledThread(void *arg)
{
  Connection *connPtr = TTDbse::instance()->GetNewConnection();

  connPtr->StartTransaction();

  std::vector<MtListRec *> *mtList = ((PrintScheduledScoreStruct *) arg)->mtList;
  Printer *printer = ((PrintScheduledScoreStruct *) arg)->printer;
  bool isPreview   = ((PrintScheduledScoreStruct *) arg)->isPreview;
  bool isCombined  = ((PrintScheduledScoreStruct *) arg)->isCombined;
  bool rrConsolation = ((PrintScheduledScoreStruct *)arg)->rrConsolation;

  int count = mtList->size();

  if (!printer->StartDoc(_("Print Scoresheets")))
    return 0;

  // Cache Teams, Groups, Events
  std::map<long, TmEntry> tmMap;
  std::map<long, GrRec>   grMap;
  std::map<long, CpRec>   cpMap;

  // Variablen kapseln
  {
    std::map<short, std::set<long>> tmSets;
    std::set<long> grSet;
    std::set<long> cpSet;

    for (MtListRec *it : *mtList)
    {
      tmSets[it->cpType].insert(it->tmA);
      tmSets[it->cpType].insert(it->tmX);

      grSet.insert(it->mtEvent.grID);
    }

    TmEntryStore tm(connPtr);

    if (tmSets[CP_SINGLE].size())
    {
      tm.SelectTeamById(tmSets[CP_SINGLE], CP_SINGLE);
      while (tm.Next())
        tmMap[tm.tmID] = tm;
      tm.Close();
    }

    if (tmSets[CP_DOUBLE].size())
    {
      tm.SelectTeamById(tmSets[CP_DOUBLE], CP_DOUBLE);
      while (tm.Next())
        tmMap[tm.tmID] = tm;
      tm.Close();
    }

    if (tmSets[CP_MIXED].size())
    {
      tm.SelectTeamById(tmSets[CP_MIXED], CP_MIXED);
      while (tm.Next())
        tmMap[tm.tmID] = tm;
      tm.Close();
    }

    if (tmSets[CP_TEAM].size())
    {
      tm.SelectTeamById(tmSets[CP_TEAM], CP_TEAM);
      while (tm.Next())
        tmMap[tm.tmID] = tm;
      tm.Close();
    }

    GrListStore tmpGr(connPtr);
    tmpGr.SelectById(grSet);
    while (tmpGr.Next())
    {
      grMap[tmpGr.grID] = tmpGr;
      cpSet.insert(tmpGr.cpID);
    }
    tmpGr.Close();

    CpListStore tmpCp(connPtr);
    tmpCp.SelectById(cpSet);
    while (tmpCp.Next())
      cpMap[tmpCp.cpID] = tmpCp;
  }

  // [ Scope von cp / gr muss begrenzt bleiben, damit sie nicht
  //   nach connPtr geloescht werden
  {   
  CpRec cp;
  GrRec gr;
  
  for (std::vector<MtListRec *>::iterator it = mtList->begin(); 
       it != mtList->end(); CTT32App::ProgressBarStep(), delete (*it), it++)
  {
    count--;
    
    // Spiel wurde bereits als Combined Score Sheet gedruckt.
    if (gr.grID == (*it)->mtEvent.grID && gr.syID == 0 &&
        gr.grModus == MOD_RR && isCombined)
      continue;
    
    TmEntry tmA, tmX;

    if (!isCombined || gr.grModus != MOD_RR || gr.syID != 0)
    {
      tmA = tmMap[(*it)->tmA];
      if (!tmA.tmID)
        continue;

      tmX = tmMap[(*it)->tmX];
      if (!tmX.tmID)
        continue;
    }
    
    gr = grMap[(*it)->mtEvent.grID];
    if (!gr.grID)
      continue;

    cp = cpMap[gr.cpID];
    if (!cp.cpID)
      continue;

    printer->StartPage();
    
    if (isCombined && gr.grModus == MOD_RR && gr.syID == 0)
    {
      PrintRasterOptions po;
      memset(&po, 0, sizeof(PrintRasterOptions));
      po.rrCombined = 1;
      po.rrResults = 1;
      po.rrConsolation = rrConsolation;
      
      int offsetX = 0, offsetY = 0, page = 0;
      RasterRR raster(printer, connPtr);
      
      raster.Print(cp, gr, po, &offsetX, &offsetY, &page);
      raster.PrintMatches(cp, gr, po, &offsetX, &offsetY, &page);
      
      if (!isPreview)
        MtStore(connPtr).UpdateScorePrintedForGroup(gr.grID, true);
    }
    else
    {    
      MtEntry tmp(*(*it), tmA, tmX);
    
      RasterScore raster(printer, connPtr);
      raster.Print(cp, gr, tmp);

      if (!isPreview)
        MtStore(connPtr).UpdateScorePrinted((*it)->mtID, true);
    }
    
    printer->EndPage();
  }
  
  } // ]

  connPtr->Commit();

  printer->EndDoc();

  delete printer;
  delete mtList;
  delete connPtr;

  delete (PrintScheduledScoreStruct *) arg;

  return 0;
}