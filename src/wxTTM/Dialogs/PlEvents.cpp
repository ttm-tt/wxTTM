/* Copyright (C) 2020 Christoph Theis */

// PlEvents.cpp : implementation file
// TODO: Vergroesserbares Fenster
// TODO: Entered Events ausfuellen

#include "stdafx.h"

#include "TT32App.h"
#include "PlEvents.h"

#include "CpItem.h"
#include "GrItem.h"

#include "CpStore.h"
#include "LtStore.h"

#include "CpListStore.h"
#include "GrListStore.h"
#include "MtListStore.h"
#include "NtListStore.h"
#include "StListStore.h"
#include "TmListStore.h"

// #include "LtDouble.h"
// #include "TmEdit.h"
// #include "StListView.h"

#include "InfoSystem.h"
#include "Rec.h"


// -----------------------------------------------------------------------
namespace 
{
  // Eigenes Item fuer die Liste
  class PlEventsItem : public CpItem
  {
    public:
      PlEventsItem(const CpRec &cp, long id = 0) : CpItem(cp), ltID(id) {}

    public:
      long  ltID;
  };


  // And for the entered groups
  class GrItemEx : public GrItem
  {
    public:
      GrItemEx(const GrRec &gr, short pos) : GrItem(gr), stPos(pos) {};

      virtual void DrawItem(wxDC *pDC, wxRect &rect);

    public:
      short stPos;
  };

  void GrItemEx::DrawItem(wxDC *pDC, wxRect &rect)
  {
    unsigned  cW = pDC->GetTextExtent("M").GetWidth();

    wxRect  rcName = rect;
    wxRect  rcDesc = rect;
    wxRect  rcStage = rect;
    wxRect  rcPos = rect;

    rcName.SetWidth(4 * cW);
    rcDesc.SetLeft(rcName.GetRight());
    rcDesc.SetWidth(rect.GetWidth() / 2 - 4 * cW);
    rcStage.SetLeft(rcDesc.GetRight());
    rcStage.SetWidth(rect.GetWidth() / 2 - 4 * cW);
    rcPos.SetLeft(rcStage.GetRight());
    rcPos.SetWidth(4 * cW);

    rcName.Deflate(offset, 0);
    rcDesc.Deflate(offset, 0);
    rcStage.Deflate(offset, 0);
    rcPos.Deflate(offset, 0);

    DrawString(pDC, rcName, gr.grName);
    DrawString(pDC, rcDesc, gr.grDesc);
    DrawString(pDC, rcStage, gr.grStage);
    DrawString(pDC, rcPos, wxString::Format("P. %d", stPos));
  }

  // Und fuer die Spiele
  class ScheduleItem : public ListItem
  {
    public:
      ScheduleItem(const MtListRec &mt, bool reverse);

      virtual void DrawItem(wxDC *pDC, wxRect &rect);

    private:
      wxString date;
      wxString time;
      wxString table;
      wxString res;
  };



  ScheduleItem::ScheduleItem(const MtListRec &mt, bool reverse) : ListItem(mt.mtID)
  {
    struct tm  tmp;
    memset(&tmp, 0, sizeof(struct tm));

    tmp.tm_year = mt.mtPlace.mtDateTime.year - 1900;
    tmp.tm_mon  = mt.mtPlace.mtDateTime.month - 1;
    tmp.tm_mday = mt.mtPlace.mtDateTime.day;
    tmp.tm_hour = mt.mtPlace.mtDateTime.hour;
    tmp.tm_min  = mt.mtPlace.mtDateTime.minute;
    tmp.tm_sec  = mt.mtPlace.mtDateTime.second;
    tmp.tm_isdst = -1;

    date = wxDateTime(tmp).Format(CTT32App::instance()->GetDateFormat());
    if (mt.mtPlace.mtDateTime.hour)
      time = wxDateTime(tmp).Format(CTT32App::instance()->GetTimeFormatW());
    if (mt.mtPlace.mtTable)
      table = wxString::Format("T. %d", mt.mtPlace.mtTable);

    if (reverse && mt.mtWalkOverX || !reverse && mt.mtWalkOverA)
      res = _("w/o");
    else if (reverse && mt.mtDisqualifiedX || !reverse && mt.mtDisqualifiedA)
      res = _("Disqu.");
    else if (reverse && mt.mtInjuredX || !reverse && mt.mtInjuredA)
      res = _("Inj.");
    else if (mt.mtResA || mt.mtResX)
      res = wxString::Format("%d : %d", reverse ? mt.mtResX : mt.mtResA, reverse? mt.mtResA : mt.mtResX);
  }


  void ScheduleItem::DrawItem(wxDC *pDC, wxRect &rect)
  {
    wxRect rectDate = rect, rectTime = rect, rectTable = rect, rectRes = rect;
    rectDate.SetRight(rectDate.GetLeft() + rect.GetWidth() / 3);
    rectTime.SetLeft(rectDate.GetRight());
    rectTime.SetRight(rectTime.GetLeft() + rect.GetWidth() / 3);
    rectTable.SetLeft(rectTime.GetRight());
    rectTable.SetRight(rectTable.GetLeft() + rect.GetWidth() / 6);
    rectRes.SetLeft(rectTable.GetRight());

    DrawString(pDC, rectDate, date);
    DrawString(pDC, rectTime, time);
    DrawString(pDC, rectTable, table);
    DrawString(pDC, rectRes, res);
  }
}

// -----------------------------------------------------------------------
// CPlEvents

IMPLEMENT_DYNAMIC_CLASS(CPlEvents, CFormViewEx)

BEGIN_EVENT_TABLE(CPlEvents, CFormViewEx)
  EVT_LIST_ITEM_SELECTED(XRCID("EnteredEvents"), CPlEvents::OnSelChangedEnteredEvents)
  EVT_LIST_ITEM_SELECTED(XRCID("EnteredGroups"), CPlEvents::OnSelChangedEnteredGroups)
END_EVENT_TABLE()


// -----------------------------------------------------------------------  
CPlEvents::CPlEvents(): CFormViewEx()
{
}


CPlEvents::~CPlEvents()
{
}


bool CPlEvents::Edit(va_list vaList)
{
  long id = va_arg(vaList, long);

  pl.SelectById(id);
  pl.Next();
  m_plItem->SetListItem(new PlItem(pl));

  // Listen fuellen
  CpStore  cp;
  cp.SelectAll();
  while (cp.Next())
  {
    if (cp.IsAllowed(pl))
    {
      PlEventsItem *itemPtr = new PlEventsItem(cp);
      itemPtr->SetType(IDC_ADD);
      m_cpAvail->InsertListItem(itemPtr);
    }
  }

  // Jetzt den Transfer
  LtStore  lt;
  lt.SelectByPl(pl.plID);
  while (lt.Next())
  {  
    if (m_cpEntered->FindListItem(lt.cpID))
      continue;
      
    ListItem *itemPtr = m_cpAvail->CutListItem(lt.cpID);
    if (itemPtr)
    {
      itemPtr->SetType(IDC_EDIT);
      m_cpEntered->InsertListItem(itemPtr);
    }
    else
    {
      // Spieler ist fuer einen WB gemeldet, darf aber
      // eigentlich nicht mehr spielen.
      // Ich muss aber mit lt wieder von vorne anfangen
      lt.Close();
      cp.SelectById(lt.cpID);
      if (cp.Next())
      {
        PlEventsItem *itemPtr = new PlEventsItem(cp);
        itemPtr->SetType(IDC_EDIT);
        m_cpEntered->InsertListItem(itemPtr);
      }
        
      cp.Close();
      
      lt.SelectByPl(pl.plID);
    }
  }  
  
  m_cpAvail->SortItems();
  m_cpEntered->SortItems();

  if (m_cpAvail->GetCount() > 0)
    m_cpAvail->SetCurrentIndex(0);

  if (m_cpEntered->GetCount() > 0)
    m_cpEntered->SetCurrentIndex(0);

  return true;
}


// -----------------------------------------------------------------------
// CPlEvents message handlers

void CPlEvents::OnSelChangedEnteredEvents(wxListEvent &)
{
  m_grEntered->RemoveAllListItems();
  m_mtSchedules->RemoveAllListItems();

  if (m_cpEntered->GetCount() == 0)
    return;
  
  ListItem *itemPtr = m_cpEntered->GetCurrentItem();
  if (itemPtr == 0)
    return;

  CpListStore cp;
  GrListStore gr;
  TmListStore tm;

  cp.SelectById(itemPtr->GetID());
  cp.Next();
  cp.Close();
  
  tm.SelectByCpPl(cp, pl);
  tm.Next();
  tm.Close();

  std::set<long> grSet;
  std::map<long, short> stPosList;

  StListStore  stList;
  stList.SelectByCpTm(cp, tm);
  while (stList.Next())
  {
    grSet.insert(stList.grID);
    stPosList.insert({stList.grID, stList.stNr});
  }
  stList.Close();

  GrListStore grList;
  grList.SelectById(grSet);
  while (grList.Next())
  {
    GrItemEx *itemPtr = new GrItemEx(grList, stPosList[grList.grID]);
    itemPtr->SetType(IDC_EDIT);
    m_grEntered->InsertListItem(itemPtr);
  }

  m_grEntered->SortItems();

  if (m_grEntered->GetCount() > 0)
    m_grEntered->SetCurrentIndex(0);

  OnSelChangedEnteredGroups(wxListEvent());
}


void CPlEvents::OnSelChangedEnteredGroups(wxListEvent &)
{
  if (m_grEntered->GetCount() == 0)
  {
    m_mtSchedules->RemoveAllListItems();
    return;
  }
  
  ListItem *itemPtr = m_grEntered->GetCurrentItem();
  if (itemPtr == 0)
    return;

  m_mtSchedules->RemoveAllListItems();

  CpListStore cp;
  GrListStore gr;
  TmListStore tm;

  gr.SelectById(itemPtr->GetID());
  gr.Next();
  gr.Close();

  cp.SelectById(gr.cpID);
  cp.Next();
  cp.Close();
  
  tm.SelectByCpPl(cp, pl);
  tm.Next();
  tm.Close();

  MtListStore  mtList;
  mtList.SelectByGrTm(gr, tm);

  while (mtList.Next())
  {
    if (!mtList.mtPlace.mtDateTime.day)
      continue;

    ListItem *itemPtr = new ScheduleItem(mtList, tm.tmID == mtList.tmX);
    m_mtSchedules->InsertListItem(itemPtr);
  }
}


// -----------------------------------------------------------------------
void CPlEvents::OnInitialUpdate() 
{
	CFormViewEx::OnInitialUpdate();
	
	m_plItem      = XRCCTRL(*this, "Player", CItemCtrl);
	m_cpAvail     = XRCCTRL(*this, "AvailableEvents", CListCtrlEx);
	m_cpEntered   = XRCCTRL(*this, "EnteredEvents", CListCtrlEx);
	m_grEntered   = XRCCTRL(*this, "EnteredGroups", CListCtrlEx);
  m_mtSchedules = XRCCTRL(*this, "Schedules", CListCtrlEx);
	
	m_plItem->SetSize(wxSize(-1, wxClientDC(m_plItem).GetTextExtent(wxT("M")).GetHeight() * 2));
	m_plItem->SetMinSize(wxSize(-1, m_plItem->GetSize().GetHeight()));

	Layout();
}


void CPlEvents::OnDelete() 
{
  for (int idx = m_cpEntered->GetItemCount(); idx--; )
  {
    if (!m_cpEntered->IsSelected(idx))
      continue;
      
    ListItem *itemPtr = m_cpEntered->GetListItem(idx);

    if (itemPtr)
    {
      TmListStore tm;
      GrListStore gr;
      
      tm.SelectByCpPl(((PlEventsItem *) itemPtr)->cp, pl);
      tm.Next();
      tm.Close();

      gr.SelectByCpTm(((PlEventsItem *) itemPtr)->cp, tm);
      
      std::list<GrRec> grList;

      while (gr.Next())
        grList.push_back(gr);
        
      for (std::list<GrRec>::iterator it = grList.begin(); 
           it != grList.end(); it = grList.begin())
      {
        gr.SelectById((*it).grID);
        gr.Next();
        gr.Close();
        
        if (gr.QryStarted())
          break;
          
        grList.pop_front();
      }

      if (grList.size() > 0)
      {
        infoSystem.Error(_("Player is member of a group that has already started")); 
        // Weiter mit naechsten Spieler
        continue;
      }

      if ( ((PlEventsItem *) itemPtr)->cp.cpType == CP_TEAM && NtListStore().Count(tm) == 1 )
      {
        infoSystem.Information(_("Team %s will then be empty"), tm.tmDesc);
      }

      m_cpEntered->CutListItem(itemPtr->GetID());
      itemPtr->SetType(IDC_ADD);
      m_cpAvail->InsertListItem(itemPtr);
      
      if (m_cpEntered->GetCount() == 0)
        m_grEntered->RemoveAllListItems();
      else 
        m_cpEntered->SetCurrentIndex(idx);

      m_cpAvail->SortItems();
    }
      
#if 0      
    // Wird nicht automatisch aufgerufen?!
    OnSelChangedEntered();
#endif    
  }
}


void CPlEvents::OnAdd() 
{
  for (int idx = m_cpAvail->GetItemCount(); idx--; )
  {
    if (!m_cpAvail->IsSelected(idx))
      continue;
      
    ListItem *itemPtr = m_cpAvail->GetListItem(idx);

    if (itemPtr)
    {
      m_cpAvail->CutListItem(itemPtr->GetID());
      itemPtr->SetType(IDC_EDIT);
      m_cpEntered->InsertListItem(itemPtr);

      m_cpEntered->SortItems();
    }
  }
}


void CPlEvents::OnEdit()
{
  if (FindWindow("Schedules")->HasFocus())
  {
    MtRec mt;
    ListItem *itemPtr = m_mtSchedules->GetCurrentItem();
    if (!itemPtr)
      return;

    long mtID = itemPtr->GetID();

    CTT32App::instance()->OpenView(_T("MatchResult"), wxT("MtListView"), mtID);

    return;
  }
  else if (FindWindow("EnteredGroups")->HasFocus())
  {
    GrRec gr;
    CpRec cp;
    
    ListItem *itemPtr = m_cpEntered->GetCurrentItem();
    if (!itemPtr)
      return;
      
    cp = ((PlEventsItem *) itemPtr)->cp;
      
    itemPtr = m_grEntered->GetCurrentItem();
    if (!itemPtr)
      return;
      
    gr = ((GrItem *) itemPtr)->gr;
      
    TmListStore tm;
    tm.SelectByCpPl(cp, pl);
    tm.Next();
    tm.Close();
    
    StListStore st;
    st.SelectByGrTm(gr, tm);
    st.Next();
    st.Close();

    CTT32App::instance()->OpenView(_T("Group Positioning"), wxT("StListView"), st.stID);
    
    return ;
  }  
  else
  {
  
    CpStore  cp;
    ListItem *itemPtr;    

    if ( !(itemPtr = m_cpEntered->GetCurrentItem()) )
      return;

    cp.SelectById(itemPtr->GetID());
    if (!cp.Next())
      return;
    
    cp.Close();

    if (cp.cpType == CP_SINGLE)
      return;

    // Sicherstellen (vorerst mal), dass Spieler fuer WB gemeldet ist
    cp.EnlistPlayer(pl);

    if (cp.cpType == CP_DOUBLE || cp.cpType == CP_MIXED)
    {
      CTT32App::instance()->OpenView(_T("Edit Double"), wxT("LtDouble"), cp.cpID, pl.plID, NULL);
    }
    else if (cp.cpType == CP_TEAM)
    {
      TmListStore tm;
      tm.SelectByCpPl(cp, pl);
      tm.Next();
      tm.Close();

      if (!tm.WasOK())
      {
        tm.SelectByName(pl.naName, cp);
        tm.Next();
        tm.Close();
      }
    
      if (tm.WasOK())
        CTT32App::instance()->OpenView(_T("Edit Team"), wxT("TmEdit"), tm.tmID, cp.cpID, NULL);
    } 
  }
}


void CPlEvents::OnOK()
{
  int  idx;
  ListItem *itemPtr = 0;
  CpStore   cp;
  LtStore   lt;

  TTDbse::instance()->GetDefaultConnection()->StartTransaction();

  // Abmeldungen
  for (idx = 0; (itemPtr = m_cpAvail->GetListItem(idx)); idx++)
  {
    if ( cp.SelectById( ((PlEventsItem *) itemPtr)->GetID()) && cp.Next() )
    {
      if ( lt.SelectByCpPl(cp.cpID, pl.plID) && lt.Next() )
      {
        if (!cp.RemovePlayer(pl))
        {
          TTDbse::instance()->GetDefaultConnection()->Rollback();
          return;
        }
      }
      // else ist kein Fehler
    }
    else
    {
      TTDbse::instance()->GetDefaultConnection()->Rollback();
      return;
    }
  }

  // Anmeldungen
  for (idx = 0; (itemPtr = m_cpEntered->GetListItem(idx)); idx++)
  {
    if ( cp.SelectById( ((PlEventsItem *) itemPtr)->GetID()) && cp.Next() )
    {
      if (!cp.EnlistPlayer(pl))
      {
        TTDbse::instance()->GetDefaultConnection()->Rollback();
        return;
      }

      // Bei Einzel auch gleich TM erzeugen
      if (cp.cpType == CP_SINGLE)
      {
        if (!cp.CreateSingle(pl))
        {
          TTDbse::instance()->GetDefaultConnection()->Rollback();
          return;
        }
      }
    }
  }
  
  TTDbse::instance()->GetDefaultConnection()->Commit();

  CFormViewEx::OnOK();
}

