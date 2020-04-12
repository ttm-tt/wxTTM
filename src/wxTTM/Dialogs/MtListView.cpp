/* Copyright (C) 2020 Christoph Theis */

// MtListView.cpp : implementation file
// TODO: Group Tables --> StListView, sortiert nach Position

#include "stdafx.h"
#include "TT32App.h"
#include "MtListView.h"

#include "StListView.h"

#include "CpListStore.h"
#include "GrListStore.h"
#include "MtEntryStore.h"

#include "CpItem.h"
#include "GrItem.h"
#include "MtItem.h"

#include "Request.h"


// -----------------------------------------------------------------------
// CMtListView

IMPLEMENT_DYNAMIC_CLASS(CMtListView, CFormViewEx)


BEGIN_EVENT_TABLE(CMtListView, CFormViewEx)
	EVT_COMBOBOX(XRCID("Event"), CMtListView::OnSelChangeCp)
	EVT_COMBOBOX(XRCID("Group"), CMtListView::OnSelChangeGr)
	EVT_BUTTON(IDC_FIRST, CMtListView::OnRoundFirst)
	EVT_BUTTON(IDC_PREV, CMtListView::OnRoundPrev)
	EVT_BUTTON(IDC_NEXT, CMtListView::OnRoundNext)
	EVT_BUTTON(IDC_LAST, CMtListView::OnRoundLast)
	EVT_BUTTON(XRCID("MatchTime"), CMtListView::OnResultTime)
  EVT_BUTTON(XRCID("ScoreSheet"), CMtListView::OnScore)
  EVT_BUTTON(XRCID("GroupTable"), CMtListView::OnTable)
  EVT_BUTTON(XRCID("Consolation"), CMtListView::OnConsolation)
  EVT_CHECKBOX(XRCID("GroupView"), CMtListView::OnGroupView)
END_EVENT_TABLE()


// -----------------------------------------------------------------------
CMtListView::CMtListView() : CFormViewEx(), m_cbCp(0), m_cbGr(0), m_listCtrl(0)
{
  round = 1;
}


CMtListView::~CMtListView()
{
}


bool  CMtListView::Edit(va_list vaList)
{
  long id    = va_arg(vaList, long);
  showTimes = va_arg(vaList, bool);
    
  // Muss kommen, bevor die Spiele eingetragen werden
  // OnResultTime invertiert showTimes
  showTimes = !showTimes;  
  OnResultTime(wxCommandEvent());

  if (id)
  {
    MtListStore mtList;
  
    mtList.SelectById(id);
    mtList.Next();
    mtList.Close();
    
    mt = mtList;
  }
  
  m_chance = mt.mtEvent.mtChance;
  
  wxString cpName = CTT32App::instance()->GetDefaultCP();
  wxStrncpy(cp.cpName, cpName, sizeof(cp.cpName) / sizeof(wxChar) - 1);

  wxString grName = CTT32App::instance()->GetDefaultGR();
  wxStrncpy(gr.grName, grName, sizeof(gr.grName) / sizeof(wxChar) - 1);
	
  // Liste der WB
  CpListStore cpList;
  cpList.SelectAll();
  while (cpList.Next())
    m_cbCp->AddListItem(new CpItem(cpList));

  if (mt.mtID)
  {
    GrListStore grList;
    grList.SelectById(mt.mtEvent.grID);
    grList.Next();
    gr = grList;
    
    CpListStore cpList;
    cpList.SelectById(grList.cpID);
    cpList.Next();
    cp = cpList;
    
    m_cbCp->SetCurrentItem(cp.cpID);    

    grList.SelectAll(cp);
    while (grList.Next())
      m_cbGr->AddListItem(new GrItem(grList));
    
    m_cbGr->SetCurrentItem(gr.grID);
    OnSelChangeGr(wxCommandEvent());
  
    round = mt.mtEvent.mtRound;
    OnChangeRound();
        
    m_listCtrl->SetCurrentItem(mt.mtID);
  }
  else
  {
    // Erstes Item auswaehlen und WB auslesen
    ListItem *cpItemPtr = m_cbCp->FindListItem(cp.cpName);
    if (!cpItemPtr)
      cpItemPtr = m_cbCp->GetListItem(0);
    if (!cpItemPtr)
      return true;
      
    m_cbCp->SetCurrentItem(cpItemPtr->GetID());

    OnSelChangeCp(wxCommandEvent());
  }

  return true;
}


// -----------------------------------------------------------------------
// CMtListView message handlers
void CMtListView::OnInitialUpdate() 
{
  FindWindow("FirstRound")->SetId(IDC_FIRST);
  FindWindow("PrevRound")->SetId(IDC_PREV);
  FindWindow("NextRound")->SetId(IDC_NEXT);
  FindWindow("LastRound")->SetId(IDC_LAST);

  m_cbCp = XRCCTRL(*this, "Event", CComboBoxEx);
  m_cbGr = XRCCTRL(*this, "Group", CComboBoxEx);
  m_listCtrl = XRCCTRL(*this, "MtList", CListCtrlEx);
}


void CMtListView::OnSelChangeCp(wxCommandEvent &) 
{
  // Liste der Gruppen aktualisieren
  m_cbGr->RemoveAllListItems();
  m_listCtrl->RemoveAllListItems();

  CpItem *cpItemPtr = (CpItem *) m_cbCp->GetCurrentItem();
  if (!cpItemPtr)
    return;

  cp = cpItemPtr->cp;

  CTT32App::instance()->SetDefaultCP(cp.cpName);

  GrListStore grList;
  grList.SelectAll(cp);

  while (grList.Next())
    m_cbGr->AddListItem(new GrItem(grList));

  // Erste Gruppe auswaehlen
  ListItem *grItemPtr = m_cbGr->FindListItem(GrItem(gr).GetLabel());
  if (!grItemPtr)
    grItemPtr = m_cbGr->GetListItem(0);
  if (!grItemPtr)
    return;

  m_cbGr->SetCurrentItem(grItemPtr->GetID());

  OnSelChangeGr(wxCommandEvent());
}


void CMtListView::OnSelChangeGr(wxCommandEvent &) 
{
  // stMap loeschen
  stMap.clear();

  // Liste der Setzungen aktualisieren
  m_listCtrl->RemoveAllListItems();

  if ( (cp.cpType == CP_DOUBLE) || (cp.cpType == CP_MIXED) )
    m_listCtrl->SetItemHeight(2);
  else
    m_listCtrl->SetItemHeight(1);

  GrItem *grItemPtr = (GrItem *) m_cbGr->GetCurrentItem();
  if (!grItemPtr)
    return;
    
  int oldNofRounds = gr.NofRounds(m_chance ? true : false);

  gr = grItemPtr->gr;
  
  FindWindow("GroupTable")->Enable(gr.grModus == MOD_RR);
  FindWindow("Consolation")->Enable(gr.grModus == MOD_DKO || gr.grModus == MOD_MDK);
  
  m_chance = 0;

  FindWindow("GroupView")->Enable(gr.grModus == MOD_RR);

  GrListStore grList;
  grList.grID = gr.grID;

  if (gr.grModus != MOD_RR)
    XRCCTRL(*this, "GroupView", wxCheckBox)->SetValue(false);
  else if (grList.QryCombined())
    XRCCTRL(*this, "GroupView", wxCheckBox)->SetValue(true);

  TransferDataToWindow();

  CTT32App::instance()->SetDefaultGR(gr.grName);

#if 0
  // Titel anpassen
  if (GetParentFrame())
  {
    CString title;
    title.LoadString(IDS_MTLIST_TITLE);
    title += " - ";
    title += cp.cpName;
    title += " / ";
    title += gr.grName;
    GetParentFrame()->SetWindowText(title);
  }
#endif  

  // stMap fuellen
  StEntryStore  st;
  st.SelectByGr(gr, cp.cpType);
  while (st.Next())
    stMap.insert(StEntryMap::value_type(st.st.stID, st));

  if (oldNofRounds == 0)
  {
    round = 1;
  }	
	else if (gr.grModus == MOD_RR)
	{
    if (round > gr.NofRounds(m_chance ? true : false))
      round = gr.NofRounds(m_chance ? true : false);
  }
  else
  {
    round = gr.NofRounds(m_chance ? true : false) - (oldNofRounds - round);
    if (round <= 0)
      round = 1;
    else if (round > gr.NofRounds(m_chance ? true : false))
      round = gr.NofRounds(m_chance ? true : false);
  }

  if (gr.grNofRounds)
    round = std::min(round, gr.grNofRounds);

  OnChangeRound();

  m_listCtrl->SetSelected(0);
}


void CMtListView::OnRoundFirst(wxCommandEvent &) 
{
  long idx = m_listCtrl->GetCurrentIndex();
  
  round = 1;
  OnChangeRound();
  
  if (idx >= 0)
    m_listCtrl->SetCurrentIndex(0);
}

void CMtListView::OnRoundNext(wxCommandEvent &) 
{
  if (round == gr.NofRounds(m_chance ? true : false))
    return;

  if (gr.grNofRounds && round == gr.grNofRounds)
    return;
    
  long  idx = m_listCtrl->GetCurrentIndex();

  round++;
  OnChangeRound();

  if (idx >= 0)
  {
    switch (gr.grModus)
    {
      case MOD_SKO :
      case MOD_DKO :  // TODO: Trostrunde
        m_listCtrl->SetCurrentIndex(idx / 2);
        break;
        
      case MOD_RR :
        m_listCtrl->SetCurrentIndex(idx);
        break;
        
      case MOD_PLO :  // TODO: Play off
        m_listCtrl->SetCurrentIndex(idx);
        break;
    }
  }
}


void CMtListView::OnRoundPrev(wxCommandEvent &) 
{
  if (round == 1)
    return;
    
  long  idx = m_listCtrl->GetCurrentIndex();

  round--;
  OnChangeRound();

  if (idx >= 0)
  {
    switch (gr.grModus)
    {
      case MOD_SKO :
      case MOD_DKO :  // TODO: Trostrunde
        m_listCtrl->SetCurrentIndex(idx * 2);
        break;
        
      case MOD_RR :
        m_listCtrl->SetCurrentIndex(idx);
        break;
        
      case MOD_PLO :  // TODO: Play off
        m_listCtrl->SetCurrentIndex(idx);
        break;
    }
  }  
}


void CMtListView::OnRoundLast(wxCommandEvent &) 
{
  long idx = m_listCtrl->GetCurrentIndex();
  
  round = gr.NofRounds(m_chance ? true : false);
  if (gr.grNofRounds)
    round = std::min(round, gr.grNofRounds);

  OnChangeRound();
  
  if (idx >= 0)
    m_listCtrl->SetCurrentIndex(0);
}


void  CMtListView::OnChangeRound()
{
  bool groupView = XRCCTRL(*this, "GroupView", wxCheckBox)->GetValue();

  FindWindow("FirstRound")->Enable(!groupView && round > 1);
  FindWindow("PrevRound")->Enable(!groupView && round > 1);

  short lastRound = gr.NofRounds(m_chance ? true : false);
  if (gr.grNofRounds)
    lastRound = std::min(lastRound, gr.grNofRounds);
  
  FindWindow("NextRound")->Enable(!groupView && round < lastRound);
  FindWindow("LastRound")->Enable(!groupView && round < lastRound);

  m_listCtrl->Freeze();

  m_listCtrl->RemoveAllListItems();

  wxString strColumn;

  if (!groupView)
  {
    if (round <= gr.grQualRounds)
      strColumn = wxString::Format(_("Qualification Round"));
    else
      strColumn = wxString::Format(_("Round %d"), round - gr.grQualRounds);
  }
  
  wxListItem item;
  item.SetMask(wxLIST_MASK_TEXT);
  item.SetText(strColumn);
  
  m_listCtrl->SetColumn(1, item);

  MtListStore  mt;
  mt.SelectByGr(gr);

  short lastMatch = gr.grNofMatches;
  if (lastMatch)
    lastMatch /= (1 << (round - 1));
  else
    lastMatch = gr.NofMatches(round, m_chance != 0);
  
  while (mt.Next())
  {
    if (mt.mtEvent.mtChance != m_chance)
      continue;
      
    if (!groupView && mt.mtEvent.mtRound != round)
      continue;

    if (mt.mtEvent.mtMatch > lastMatch)
      continue;

    MtEntry mtEntry(mt, GetTeamA(mt), GetTeamX(mt));

    m_listCtrl->AddListItem(new MtItem(mtEntry, showTimes));
  }

  m_listCtrl->SortItems();

  m_listCtrl->Thaw();
}


void  CMtListView::OnEdit()
{
  ListItem *itemPtr = m_listCtrl->GetCurrentItem();
  if (!itemPtr)
    return;

  bool combined = XRCCTRL(*this, "GroupView", wxCheckBox)->GetValue();

  if (showTimes)
    CTT32App::instance()->OpenView(_("Edit Schedule"), wxT("MtTime"), itemPtr->GetID(), combined ? 5 : 1);
  else
  {
    if (cp.cpType == CP_TEAM)
      CTT32App::instance()->OpenView(_("Edit Team Result"), wxT("MtTeam"), itemPtr->GetID());
    else
      CTT32App::instance()->OpenView(_("Edit Result"), wxT("MtRes"), itemPtr->GetID(), (short) 0);  

    // Current item wird in OnUpdate weitergeschoben
    if (false && combined) 
    {
      int idx = m_listCtrl->GetCurrentIndex();
      if (idx < m_listCtrl->GetCount() - 1)
        m_listCtrl->SetCurrentIndex(idx + 1);
    }
  }
}


void CMtListView::OnResultTime(wxCommandEvent &) 
{
  showTimes = !showTimes;
  
  m_listCtrl->Freeze();

  while (m_listCtrl->DeleteColumn(0))
    ;
    
  m_listCtrl->SetResizeColumn(-1);

  wxString strColumn = wxString::Format(_("Round %d"), round);

  if (showTimes)
  {
    // Match No
    m_listCtrl->InsertColumn(0, _("Mt.Nr"), wxALIGN_LEFT);

    // Spieler / Teams
    m_listCtrl->InsertColumn(1, strColumn, wxALIGN_CENTER);

    // Datum
    m_listCtrl->InsertColumn(2, _("Date"), wxALIGN_LEFT, GetTextExtent("00.00.00").GetWidth() + 2 * cW);

    // Zeit
    m_listCtrl->InsertColumn(3, _("Time"), wxALIGN_LEFT,  GetTextExtent("00:00").GetWidth() + 2 * cW);

    // Tisch
    m_listCtrl->InsertColumn(4, _("Table"), wxALIGN_LEFT);

    // SchiRi
    m_listCtrl->InsertColumn(5, _("Umpire"), wxALIGN_LEFT);
  }
  else
  {
    // Match No
    m_listCtrl->InsertColumn(0, _("Mt.Nr"), wxALIGN_LEFT);

    // Spieler / Teams
    m_listCtrl->InsertColumn(1, strColumn, wxALIGN_CENTER);

    // Ergebnis
    m_listCtrl->InsertColumn(2, _("Result"), wxALIGN_LEFT);
  }

  for (int idx = m_listCtrl->GetItemCount(); idx--; )
  {
    MtItem *itemPtr = (MtItem *) m_listCtrl->GetListItem(idx);
    itemPtr->SetToTime(showTimes);
  }
  
  m_listCtrl->Thaw();
  
  m_listCtrl->SetResizeColumn(1);

  m_listCtrl->Refresh();
}


void CMtListView::OnScore(wxCommandEvent &)
{
  ListItem *itemPtr = m_listCtrl->GetCurrentItem();

  bool selected = XRCCTRL(*this, "GroupView", wxCheckBox)->GetValue();

  CTT32App::instance()->OpenDialog(true, _("Print Scoresheets"), wxT("Score"), itemPtr ? itemPtr->GetID() : 0, selected ? 4 : 0);
}


void CMtListView::OnTable(wxCommandEvent &)
{
  ListItem *itemPtr = m_cbGr->GetCurrentItem();
  if (itemPtr)
  {
    CTT32App::instance()->OpenView(_("Group Positioning"), wxT("StListView"), itemPtr->GetID());
  }  
}


void  CMtListView::OnConsolation(wxCommandEvent &)
{
  TransferDataFromWindow();
  
  if (m_chance)
  {
    round = (round == 1 ? 1 : (round - 1) * 2);
  }
  else
  {
    round = (round == 1 ? 1 : round / 2 + 1);
  }
  
  OnChangeRound();
}

void CMtListView::OnGroupView(wxCommandEvent &)
{
  OnChangeRound();
}


// -----------------------------------------------------------------------
void CMtListView::OnUpdate(CRequest *reqPtr) 
{
	if (!reqPtr)
  {
    CFormViewEx::OnUpdate(reqPtr);
    return;
  }

  if (reqPtr->rec != CRequest::MTREC)
    return;

  bool groupView = gr.grModus == MOD_RR && XRCCTRL(*this, "GroupView", wxCheckBox)->GetValue();

  switch (reqPtr->type)
  {
    case CRequest::UPDATE :
    {
      if (!reqPtr->id)
      {
        MtEntryStore  mtList;
        mtList.SelectByGr(gr, groupView ? 0 : round, cp.cpType);
        while (mtList.Next())
        {
          MtItem *itemPtr = (MtItem *) m_listCtrl->FindListItem(mtList.mt.mtID);
          if (itemPtr)
            itemPtr->SetValue(mtList);
        }
      }
      else
      {
        MtItem *itemPtr = (MtItem *) m_listCtrl->FindListItem(reqPtr->id);
        if (!itemPtr)
          return;

        MtEntryStore  mtList;
        if (!mtList.SelectById(reqPtr->id, cp.cpType))
          return;
        if (!mtList.Next())
          return;

        itemPtr->SetValue(mtList);
      }

      break;
    }

    case CRequest::UPDATE_SCHEDULE :
    case CRequest::UPDATE_RESULT :
    {
      if (!reqPtr->id)
      {
        MtEntryStore  mtList;
        mtList.SelectByGr(gr, groupView ? 0 : round, cp.cpType);
        while (mtList.Next())
        {
          MtItem *itemPtr = (MtItem *) m_listCtrl->FindListItem(mtList.mt.mtID);
          if (itemPtr)
            itemPtr->SetValue(mtList);
        }
      }
      else
      {
        MtItem *itemPtr = (MtItem *) m_listCtrl->FindListItem(reqPtr->id);
        if (!itemPtr)
          return;

        MtListStore  mtList;
        if (!mtList.SelectById(reqPtr->id))
          return;
        if (!mtList.Next())
          return;

        if (reqPtr->type == CRequest::UPDATE_SCHEDULE)
          itemPtr->SetSchedule(mtList.mtPlace);
        else
          itemPtr->SetResult(mtList.mtResA, mtList.mtResX);

        // Wenn das aktuelle Spiel geaendert wurde, current item weiterschieben.
        // Denn wahrscheinlich wurde das Edit von hier angestossen.
        int idx = m_listCtrl->GetCurrentIndex();
        int nextIdx = m_listCtrl->GetListIndex(itemPtr);

        for (; idx < nextIdx; ++idx)
        {
          if (!m_listCtrl->GetListItem(idx) || !((MtItem *) m_listCtrl->GetListItem(idx))->mt.IsAByeOrXBye())
            break;
        }

        if (idx == nextIdx)
          m_listCtrl->SetCurrentIndex(idx < m_listCtrl->GetCount() - 1 ? idx + 1 : idx);
      }

      break;
    }

    default :
      break;
  }

  m_listCtrl->Refresh();
}


StEntry CMtListView::GetTeamA(const MtRec &mt)
{
  return GetTeam(mt.stA);
}


StEntry CMtListView::GetTeamX(const MtRec &mt)
{
  return GetTeam(mt.stX);
}


StEntry CMtListView::GetTeam(long id)
{
  if (!id)
    return StEntry();

  StEntryMap::iterator it = stMap.find(id);
  if (it == stMap.end())
    return StEntry();

  return (*it).second;
}