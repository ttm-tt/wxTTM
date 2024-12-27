/* Copyright (C) 2020 Christoph Theis */

// MtTime.cpp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "MtTime.h"

#include "CpItem.h"
#include "GrItem.h"
#include "TmItem.h"

#include "MtListStore.h"

#include "InfoSystem.h"


// -----------------------------------------------------------------------
// CMtTime

IMPLEMENT_DYNAMIC_CLASS(CMtTime, CFormViewEx)

BEGIN_EVENT_TABLE(CMtTime, CFormViewEx)
  EVT_RADIOBUTTON(XRCID("ApplyToMatch"), CMtTime::OnBnClickedApplyTo)
  EVT_RADIOBUTTON(XRCID("ApplyToRound"), CMtTime::OnBnClickedApplyTo)
  EVT_RADIOBUTTON(XRCID("ApplyToRoundExByes"), CMtTime::OnBnClickedApplyTo)
  EVT_RADIOBUTTON(XRCID("ApplyToRoundInGroups"), CMtTime::OnBnClickedApplyTo)
  EVT_RADIOBUTTON(XRCID("ApplyToRoundInGroupsExByes"), CMtTime::OnBnClickedApplyTo)
  EVT_RADIOBUTTON(XRCID("ApplyToMatchInGroups"), CMtTime::OnBnClickedApplyTo)
  EVT_RADIOBUTTON(XRCID("ApplyToGroup"), CMtTime::OnBnClickedApplyTo)
END_EVENT_TABLE()


enum
{
  APPLY_TO_MATCH = 1,
  APPLY_TO_ROUND = 2,
  APPLY_TO_ROUND_EX_BYES = 3,
  APPLY_TO_ROUND_IN_GROUP = 4,
  APPLY_TO_ROUND_IN_GROUP_EX_BYES = 5,
  APPLY_TO_MATCH_IN_GROUP = 6,
  APPLY_TO_GROUP = 7
};

CMtTime::CMtTime() : CFormViewEx()
{
  m_applyTo = 1;
	m_assignPlayer = false;
	m_availTables = 1;
	m_decTables = false;
}

CMtTime::~CMtTime()
{
}


bool  CMtTime::Edit(va_list vaList)
{
  long id = va_arg(vaList, long);
  m_applyTo = va_arg(vaList, short);

  mt.SelectById(id);
  mt.Next();
  mt.Close();

  gr.SelectById(mt.mtEvent.grID);
  gr.Next();

  cp.SelectById(gr.cpID);
  cp.Next();
  
  m_assignPlayer = (mt.mtUmpire == -1);
  
  m_sequence = mt.mtPlace.mtDateTime.second;

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

  FindWindow("AvailTables")->Enable(m_applyTo != 1);
  FindWindow("DecrementNo")->Enable(m_applyTo != 1);
  FindWindow("Umpire")->Enable( !m_assignPlayer );
  FindWindow("Umpire2")->Enable( !m_assignPlayer );
  FindWindow("AssignPlayer")->Enable(gr.grModus == MOD_RR);

  TransferDataToWindow();

  // Fokus auf wahrscheinliche Eingabe setzen
  // TransferDataToWindow setzt einen "Pending Focus" fuer spaeter, das wuerde SetFocus ueberschreiben.
  // Als workaround selbst WXSetPendingFocus aufrufen
  if (mt.mtPlace.mtDateTime.day)
  {
    if (mt.mtPlace.mtDateTime.hour)
      FindWindow("Table")->SetFocus();
    else
      FindWindow("Time")->SetFocus();
  }
  else
    FindWindow("Date")->SetFocus();

  if (FindFocus()->IsKindOf(CLASSINFO(wxTextCtrl)))
    ((wxTextCtrl *)FindFocus())->SetSelection(-1, -1);

  m_noSchedule = mt.mtPlace.mtDateTime.hour == 0;

  if (m_noSchedule)
    FindWindow(XRCID("Table"))->Connect(wxEVT_SET_FOCUS, wxFocusEventHandler(CMtTime::OnSetFocusTable), NULL, this);
  else if (mt.mtPlace.mtTable == 0)
    FindWindow(XRCID("Table"))->Connect(wxEVT_KEY_DOWN, wxKeyEventHandler(CMtTime::OnKeyDownTable), NULL, this);

  return true;
}


void  CMtTime::UpdateMatch()
{
  TTDbse::instance()->GetDefaultConnection()->StartTransaction();

  if ( mt.UpdateSchedule())
    TTDbse::instance()->GetDefaultConnection()->Commit();
  else
    TTDbse::instance()->GetDefaultConnection()->Rollback();
}


void  CMtTime::UpdateRound()
{
  TTDbse::instance()->GetDefaultConnection()->StartTransaction();

  if ( mt.UpdateScheduleRound(
            mt.mtEvent, mt.mtPlace, m_availTables, m_decTables, mt.mtUmpire, mt.mtUmpire2) )
    TTDbse::instance()->GetDefaultConnection()->Commit();
  else
    TTDbse::instance()->GetDefaultConnection()->Rollback();
}


void  CMtTime::UpdateRoundExclude()
{
  TTDbse::instance()->GetDefaultConnection()->StartTransaction();

  if ( mt.UpdateScheduleRoundExcludeByes(
            mt.mtEvent, mt.mtPlace, m_availTables, m_decTables, mt.mtUmpire, mt.mtUmpire2) )
    TTDbse::instance()->GetDefaultConnection()->Commit();
  else
    TTDbse::instance()->GetDefaultConnection()->Rollback();
}


void  CMtTime::UpdateRoundsGroup()
{
  TTDbse::instance()->GetDefaultConnection()->StartTransaction();

  if ( mt.UpdateScheduleRoundsGroup(
            mt.mtEvent, mt.mtPlace, m_availTables, m_decTables, mt.mtUmpire, mt.mtUmpire2) )
    TTDbse::instance()->GetDefaultConnection()->Commit();
  else
    TTDbse::instance()->GetDefaultConnection()->Rollback();
}


void  CMtTime::UpdateRoundsGroupExclude()
{
  TTDbse::instance()->GetDefaultConnection()->StartTransaction();

  if (mt.UpdateScheduleRoundsGroupExcludeByes(
    mt.mtEvent, mt.mtPlace, m_availTables, m_decTables, mt.mtUmpire, mt.mtUmpire2))
    TTDbse::instance()->GetDefaultConnection()->Commit();
  else
    TTDbse::instance()->GetDefaultConnection()->Rollback();
}


void  CMtTime::UpdateMatchesGroup()
{
  TTDbse::instance()->GetDefaultConnection()->StartTransaction();

  if ( mt.UpdateScheduleMatchesGroup(
            mt.mtEvent, mt.mtPlace, m_availTables, m_decTables, mt.mtUmpire, mt.mtUmpire2) )
    TTDbse::instance()->GetDefaultConnection()->Commit();
  else
    TTDbse::instance()->GetDefaultConnection()->Rollback();
}


void  CMtTime::UpdateGroup()
{
  TTDbse::instance()->GetDefaultConnection()->StartTransaction();

  if ( mt.UpdateScheduleGroup(
            mt.mtEvent, mt.mtPlace, m_availTables, m_decTables, mt.mtUmpire, mt.mtUmpire2) )
    TTDbse::instance()->GetDefaultConnection()->Commit();
  else
    TTDbse::instance()->GetDefaultConnection()->Rollback();
}


void  CMtTime::OnOK()
{
  TransferDataFromWindow();
  
  // Pruefen, ob die Tische bereits belegt sind
  if (mt.mtPlace.mtDateTime.hour && mt.mtPlace.mtTable)
  {
    MtListStore mtList;
    mtList.SelectByTime(mt.mtPlace.mtDateTime, mt.mtPlace.mtTable, mt.mtPlace.mtDateTime, mt.mtPlace.mtTable + (m_decTables ? -m_availTables + 1 : +m_availTables - 1), false);
    if (mtList.Next())
    {
      if (mtList.mtID && mtList.mtID != mt.mtID)
      {
        if (!infoSystem.Confirmation(_("Matches are already scheduled on selected tables")))
          return;
      }
    }
  }

  if (mt.mtPlace.mtDateTime.hour)
    mt.mtPlace.mtDateTime.second = m_sequence;
  else
    mt.mtPlace.mtDateTime.second = 0;

  if (m_applyTo != APPLY_TO_MATCH_IN_GROUP)
  {
    // Wenn nicht fuer eine Gruppe angegeben wurde, Zeiten der Runden pruefen
    timestamp latestMatchTime, earliestMatchTime;
  
    memset(&latestMatchTime, 0, sizeof(timestamp));
    memset(&earliestMatchTime, 0, sizeof(timestamp));
  
    if (mt.mtEvent.mtRound > 1)
    {
      MtStore::MtEvent mtEvent = mt.mtEvent;
      mtEvent.mtRound--;    
      latestMatchTime = mt.GetLatestMatchTime(mtEvent);
    
      // Eine Stunde zurueck faelschen, wenn keine Zeit angegeben war
      if (latestMatchTime.day > 0 && latestMatchTime.hour == 0)
        latestMatchTime.hour = -1;
    }

    if (mt.mtEvent.mtRound < gr.NofRounds(mt.mtEvent.mtChance ? true : false))
    {
      MtStore::MtEvent mtEvent = mt.mtEvent;
      mtEvent.mtRound++;
      earliestMatchTime = mt.GetEarliestMatchTime(mtEvent);
    
      // Auf Mitternacht faelschen, wenn keine Zeit angegeben war
      if (earliestMatchTime.day > 0 && earliestMatchTime.hour == 0)
        earliestMatchTime.hour = 24;
    }
  
    if ( mt.mtPlace.mtDateTime.day > 0 && earliestMatchTime.day > 0 && 
         mt.mtPlace.mtDateTime >= earliestMatchTime 
       )
    {
      if (!infoSystem.Confirmation(_("Match is played after next round")))
        return;
    }
  
    if ( mt.mtPlace.mtDateTime.day > 0 &&
         mt.mtEvent.mtRound > 1
       )
    {
      if (latestMatchTime.day == 0)
      {
        if (!infoSystem.Confirmation(_("Previous round has no time")))
          return;
      }
      else if (mt.mtPlace.mtDateTime <= latestMatchTime)
      {
        if (!infoSystem.Confirmation(_("Match is played before previous round")))
          return;
      }
    }
  }
  
  if (m_assignPlayer)
  {
    mt.mtUmpire = -1;
    mt.mtUmpire2 = 0;
  }
  else if (mt.mtUmpire == -1)
  {
    mt.mtUmpire = 0;
    mt.mtUmpire2 = 0;
  }

  // matchDate.GetDateTime(mt.mtPlace.mtDateTime);
  // matchTime.GetDateTime(mt.mtPlace.mtDateTime);

  switch (m_applyTo)
  {
    case APPLY_TO_MATCH : // This match
      UpdateMatch();
      break;

    case APPLY_TO_ROUND : // This round
      UpdateRound();
      break;

    case APPLY_TO_ROUND_EX_BYES : // This round exclude byes
      UpdateRoundExclude();
      break;

    case APPLY_TO_ROUND_IN_GROUP : // This round in all groups
      UpdateRoundsGroup();
      break;

    case APPLY_TO_ROUND_IN_GROUP_EX_BYES : // This round in all groups exclude byes
      UpdateRoundsGroupExclude();
      break;

    case APPLY_TO_MATCH_IN_GROUP :
      UpdateMatchesGroup();
      break;

    case APPLY_TO_GROUP :
      UpdateGroup();
      break;

    default :
      return;
  }

  CFormViewEx::OnOK();
}


// -----------------------------------------------------------------------
// Message handling

void CMtTime::OnInitialUpdate() 
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

  FindWindow("MatchNo")->SetValidator(CLongValidator(&mt.mtNr));
  // FindWindow("Round")->SetValidator(CShortValidator(&mt.mtEvent.mtRound));
  FindWindow("Match")->SetValidator(CShortValidator(&mt.mtEvent.mtMatch));
  
  FindWindow("Table")->SetValidator(CShortValidator(&mt.mtPlace.mtTable, true));
  FindWindow("Umpire")->SetValidator(CLongValidator(&mt.mtUmpire));  
  FindWindow("Umpire2")->SetValidator(CLongValidator(&mt.mtUmpire2));  
  
  FindWindow("Date")->SetValidator(CDateValidator(&mt.mtPlace.mtDateTime));
  FindWindow("Time")->SetValidator(CTimeValidator(&mt.mtPlace.mtDateTime));
  
  tmAItem = XRCCTRL(*this, "TeamA", CItemCtrl);
  tmXItem = XRCCTRL(*this, "TeamX", CItemCtrl);

  FindWindow("Sequence")->SetValidator(CShortValidator(&m_sequence));
  
  FindWindow("ApplyToMatch")->SetValidator(CEnumValidator(&m_applyTo, APPLY_TO_MATCH));
  FindWindow("ApplyToRound")->SetValidator(CEnumValidator(&m_applyTo, APPLY_TO_ROUND));
  FindWindow("ApplyToRoundExByes")->SetValidator(CEnumValidator(&m_applyTo, APPLY_TO_ROUND_EX_BYES));
  FindWindow("ApplyToRoundInGroups")->SetValidator(CEnumValidator(&m_applyTo, APPLY_TO_ROUND_IN_GROUP));
  FindWindow("ApplyToRoundInGroupsExByes")->SetValidator(CEnumValidator(&m_applyTo, APPLY_TO_ROUND_IN_GROUP_EX_BYES));
  FindWindow("ApplyToMatchInGroups")->SetValidator(CEnumValidator(&m_applyTo, APPLY_TO_MATCH_IN_GROUP));
  FindWindow("ApplyToGroup")->SetValidator(CEnumValidator(&m_applyTo, APPLY_TO_GROUP));

  FindWindow("AvailTables")->SetValidator(CShortValidator(&m_availTables));

  FindWindow("DecrementNo")->SetValidator(wxGenericValidator(&m_decTables));

  FindWindow("AssignPlayer")->SetValidator(wxGenericValidator(&m_assignPlayer));

  // XXX Solange wxFormBuilder nicht erlaubt, die Buttons in einem wxStdDialogButtonSizer zu editieren.
  XRCCTRL(*this, "wxID_OK", wxButton)->SetDefault();

  Layout();
}


void CMtTime::OnBnClickedApplyTo(wxCommandEvent &)
{
  TransferDataFromWindow();
  
  switch (m_applyTo)
  {
    case APPLY_TO_MATCH : // This match
      m_availTables = 1;
      break;

    case APPLY_TO_ROUND :         // This round
    case APPLY_TO_ROUND_EX_BYES : // This round excl. byes
    {
      short availTables = gr.grNofMatches ? (gr.grNofMatches >> (mt.mtEvent.mtRound - 1)) : gr.NofMatches(mt.mtEvent.mtRound);
      // Minus der Spiele davor
      availTables -= (mt.mtEvent.mtMatch - 1);
      m_availTables = availTables;

      break;
    }

    case APPLY_TO_ROUND_IN_GROUP : // This round group
    case APPLY_TO_ROUND_IN_GROUP_EX_BYES: // This round group exclude byes
    {
      m_availTables = MtStore().CountMatchesInRoundGroup(mt.mtEvent);
      // Minus der Spieler dieser Runde davor
      m_availTables -= (mt.mtEvent.mtMatch - 1);
      break;
    }

    case APPLY_TO_MATCH_IN_GROUP : // This match group (combined)
    {
      m_availTables = MtStore().CountMatchesInMatchGroup(mt.mtEvent);
      break;
    }

    case APPLY_TO_GROUP : // This group
    {
      m_availTables = MtStore().CountMatchesInGroupGroup(mt.mtEvent);
      break;
    }

    default :
      break;
  } 
    
  FindWindow("AvailTables")->Enable(m_applyTo != 1);
  FindWindow("DecrementNo")->Enable(m_applyTo != 1);

  TransferDataToWindow();
}


void CMtTime::OnBnClickedAssign(wxCommandEvent &)
{
  TransferDataFromWindow();
  
  if (m_assignPlayer)
  {
    mt.mtUmpire = -1;
    mt.mtUmpire2 = 0;
  }
  else
  {
    mt.mtUmpire = 0;
    mt.mtUmpire2 = 0;
  }
  
  FindWindow("Umpire")->Enable(!m_assignPlayer);
  
  TransferDataToWindow();
}


void CMtTime::OnSetFocusTable(wxFocusEvent &evt)
{
  evt.Skip();

  CalculateNextTable();
}


void CMtTime::OnKeyDownTable(wxKeyEvent &evt)
{
  if (evt.GetKeyCode() == '#')
  {
    CalculateNextTable();
  }
  else
  {
    evt.Skip();
  }
}


void CMtTime::CalculateNextTable()
{
  TransferDataFromWindow();

  // Wenn es jetzt eine Zeit gibt aber noch keinen Tisch, den hoechsten vergebenen Tisch ermitteln.
  // Wenn das nicht 0 ist, also schon Tische vergeben wurden, dann +1. Ansonsten auf 0 lassen.
  // Somit kann man weiterhin Spiele ansetzen ohne Tische zu vergeben. 
  if (mt.mtPlace.mtDateTime.hour != 0 && mt.mtPlace.mtTable == 0)
  {
    mt.mtPlace.mtTable = mt.GetHighestTableNumber(mt.mtPlace);

    if (mt.mtPlace.mtTable)
      mt.mtPlace.mtTable += 1;
  }

  TransferDataToWindow();

  ((wxTextCtrl *) FindWindow("Table"))->SetSelection(-1, -1);
}