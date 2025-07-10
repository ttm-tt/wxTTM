/* Copyright (C) 2020 Christoph Theis */

// Score.cpp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "TossSheet.h"

#include "CpListStore.h"
#include "GrListStore.h"
#include "MtListStore.h"
#include "MtEntryStore.h"

#include "Printer.h"

#include "RasterTS.h"
#include "RasterRR.h"

#include "Profile.h"
#include "Res.h"

#include <set>
#include <vector>


IMPLEMENT_DYNAMIC_CLASS(CTossSheet, CFormViewEx)

BEGIN_EVENT_TABLE(CTossSheet, CFormViewEx)
  EVT_RADIOBUTTON(XRCID("ThisMatch"), CTossSheet::OnSelectMatch)
  EVT_RADIOBUTTON(XRCID("Scheduled"), CTossSheet::OnSelectMatch)
  EVT_BUTTON(XRCID("Print"), CFormViewEx::OnCommand)
END_EVENT_TABLE()


// -----------------------------------------------------------------------
// CTossSheet

enum
{
  Match = 1,
  Selected = 2
};


CTossSheet::CTossSheet() : CFormViewEx()
{
  m_matchOption = Match;

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


CTossSheet::~CTossSheet()
{
}


bool CTossSheet::Edit(va_list vaList)
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
  }
  else
  {
    m_matchOption = Selected;
    fromPlace.mtDateTime = fromTime;
    toPlace.mtDateTime = toTime;
  }

  if (m_matchOption == Selected)
  {
    // Wurde "Print Scheduled" vorgegeben, alles andere disablen.
    // Diese Vorgabe kommt (eigentlich) nur von OvList und 
    // das Spiel hat keine besondere Bedeutung, als die Zeit
    // vorzugeben.
    FindWindow("ThisMatch")->Enable(false);
  }

  // TODO: Einzelspiele in Mannschaften drucken
  // GetDlgItem(IDC_SCORE_INCLTEAM)->EnableWindow(false);  

  TransferDataToWindow();
  
  OnSelectMatch(wxCommandEvent_);
  
  return true;
}


// -----------------------------------------------------------------------
// CTossSheet message handlers

void CTossSheet::OnInitialUpdate() 
{
	CFormViewEx::OnInitialUpdate();
	
	FindWindow("DateFrom")->SetValidator(CDateValidator(&fromPlace.mtDateTime));
	FindWindow("DateUntil")->SetValidator(CDateValidator(&toPlace.mtDateTime));
	FindWindow("TimeFrom")->SetValidator(CTimeValidator(&fromPlace.mtDateTime, false));
	FindWindow("TimeUntil")->SetValidator(CTimeValidator(&toPlace.mtDateTime, false));
	FindWindow("TableFrom")->SetValidator(CShortValidator(&fromPlace.mtTable));
	FindWindow("TableUntil")->SetValidator(CShortValidator(&toPlace.mtTable));
	
	FindWindow("ThisMatch")->SetValidator(CEnumValidator(&m_matchOption, Match));
	FindWindow("Scheduled")->SetValidator(CEnumValidator(&m_matchOption, Selected));
}


void  CTossSheet::OnSelectMatch(wxCommandEvent &)
{
  TransferDataFromWindow();

  FindWindow(XRCID("DateFrom"))->Enable(m_matchOption == Selected);
  FindWindow(XRCID("DateUntil"))->Enable(m_matchOption == Selected);
  FindWindow(XRCID("TimeFrom"))->Enable(m_matchOption == Selected);
  FindWindow(XRCID("TimeUntil"))->Enable(m_matchOption == Selected);
  FindWindow(XRCID("TableFrom"))->Enable(m_matchOption == Selected);
  FindWindow(XRCID("TableUntil"))->Enable(m_matchOption == Selected);
}


// -----------------------------------------------------------------------
void CTossSheet::OnKillFocus(wxFocusEvent &evt)
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
void  CTossSheet::OnPrint()
{
  TransferDataFromWindow();

  DoPrint();

  CFormViewEx::Close();
}


// -----------------------------------------------------------------------
void  CTossSheet::DoPrint()
{
  if (m_matchOption == Selected)
  {
    DoPrintScheduled();
    return;
  }

  if (CTT32App::instance()->GetPrintPreview())
    m_printer = new PrinterPreview(_("Print Toss sheet"));
  else if (CTT32App::instance()->GetPrintPdf())
  {
    wxFileDialog fileDlg(
      this, wxFileSelectorPromptStr, CTT32App::instance()->GetPath(), wxString::Format("Tosssheet.pdf"), 
      wxT("PDF Files (*.pdf)|*.pdf|All Files (*.*)|*.*||"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (fileDlg.ShowModal() != wxID_OK)
      return;

    m_printer = new PrinterPdf(fileDlg.GetPath());
  }
  else
    m_printer = new Printer;

  if (m_printer->PrinterAborted())
    return;

  if (!m_printer->StartDoc(_("Print Toss sheet")))
    return;

  DoPrintMatch();

  m_printer->EndPage();
  m_printer->EndDoc();

  delete m_printer;
  m_printer = 0;
}


void  CTossSheet::DoPrintMatch()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  MtEntryStore tmp(connPtr);

  tmp.SelectById(mt.mtID, cp.cpType);
  if (!tmp.Next())
    return;

  tmp.Close();

  m_printer->StartPage();

  RasterToss  raster(m_printer, connPtr);
  raster.Print(cp, gr, tmp, connPtr);
  
  m_printer->EndPage();

  if (!CTT32App::instance()->GetPrintPreview())
    MtStore(connPtr).UpdateTossPrinted(mt.mtID, true);
}


void  CTossSheet::DoPrintScheduled()
{
  int total = 0;

  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  std::vector<MtListRec *> *mtList = new std::vector<MtListRec *>;

  MtListStore  mt(connPtr);
  mt.SelectByTime(fromPlace.mtDateTime, fromPlace.mtTable, 
                  toPlace.mtDateTime, toPlace.mtTable);
  while (mt.Next())
  {
    total++;
    
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
  sprintf(str, "Print %d toss sheets", total);

  PrintScheduledTossStruct *tmp = new PrintScheduledTossStruct;

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

  tmp->mtList = mtList;

  CTT32App::instance()->ProgressBarThread(PrintScheduledThread, tmp, str, mtList->size());
}


unsigned CTossSheet::PrintScheduledThread(void *arg)
{
  Connection *connPtr = TTDbse::instance()->GetNewConnection();

  connPtr->StartTransaction();

  std::vector<MtListRec *> *mtList = ((PrintScheduledTossStruct *) arg)->mtList;
  Printer *printer = ((PrintScheduledTossStruct *) arg)->printer;
  bool isPreview   = ((PrintScheduledTossStruct *) arg)->isPreview;

  int count = mtList->size();

  if (!printer->StartDoc(_("Print toss sheets")))
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
      if (it->cpType != CP_TEAM)
        continue;

      tmSets[it->cpType].insert(it->tmA);
      tmSets[it->cpType].insert(it->tmX);

      grSet.insert(it->mtEvent.grID);
    }

    TmEntryStore tm(connPtr);

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
    
    TmEntry tmA, tmX;

    tmA = tmMap[(*it)->tmA];
    if (!tmA.tmID)
      continue;

    tmX = tmMap[(*it)->tmX];
    if (!tmX.tmID)
      continue;
    
    gr = grMap[(*it)->mtEvent.grID];
    if (!gr.grID)
      continue;

    cp = cpMap[gr.cpID];
    if (!cp.cpID || cp.cpType != CP_TEAM)
      continue;

    printer->StartPage();
    
    MtEntry tmp(*(*it), tmA, tmX);
    
    RasterToss raster(printer, connPtr);
    raster.Print(cp, gr, tmp, connPtr);

    if (!isPreview)
      MtStore(connPtr).UpdateTossPrinted((*it)->mtID, true);

    printer->EndPage();
  }
  
  } // ]

  connPtr->Commit();

  printer->EndDoc();

  delete printer;
  delete mtList;
  delete connPtr;

  delete (PrintScheduledTossStruct *) arg;

  return 0;
}