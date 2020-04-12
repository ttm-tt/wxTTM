/* Copyright (C) 2020 Christoph Theis */

// PlEvents.cpp : implementation file
// TODO: Vergroesserbares Fenster
// TODO: Entered Events ausfuellen

#include "stdafx.h"

#include "TT32App.h"
#include "PlHistEvents.h"

#include "CpItem.h"
#include "GrItem.h"

#include "CpStore.h"

#include "LtListStore.h"
#include "CpListStore.h"
#include "GrListStore.h"
#include "MtListStore.h"
#include "NtListStore.h"
#include "StListStore.h"
#include "TmListStore.h"

#include "TmEntryStore.h"

// #include "LtDouble.h"
// #include "TmEdit.h"
// #include "StListView.h"

#include "InfoSystem.h"
#include "Rec.h"


// -----------------------------------------------------------------------
namespace 
{
  // Eigenes Item fuer die Timestamps
  class TsItem : public ListItem
  {
    public:
      TsItem(const timestamp &ts_, long id = 0) : ListItem(id), ts(ts_) {}

      virtual int  Compare(const ListItem *itemPtr, int col) const;
      virtual void DrawItem(wxDC *pDC, wxRect &rect);

    public:
      timestamp  ts;
  };

  int TsItem::Compare(const ListItem *itemPtr, int col) const
  {
    if ( ((const TsItem *) itemPtr)->ts < ts )
      return -1;
    else if ( ((const TsItem *) itemPtr)->ts > ts )
      return +1;
    else
      return 0;
  }

  void TsItem::DrawItem(wxDC *pDC, wxRect &rect)
  {
    wxRect tmp = rect;
    tmp.Deflate(offset, 0);

    if (ts.year == 9999)
      DrawString(pDC, tmp, _("Current"));
    else
      DrawString(pDC, tmp, wxString::Format(
        "%04d-%02d-%02d %02d:%02d:%02d.%03dZ",
        ts.year, ts.month, ts.day, ts.hour, ts.minute, ts.second, ts.fraction / 1000000
      ));
  }


  // And to show entered events together with partner
  class RgItem : public ListItem
  {
    public:
      RgItem(const CpRec &cp, long tmID_, const wxString &add, const wxString naName_) : 
          ListItem(cp.cpID), cpName(cp.cpName), addName(add), tmID(tmID_), naName(naName_) {}

      virtual void DrawItem(wxDC *pDC, wxRect &rect);

    public:
      wxString cpName;
      wxString addName;
      wxString naName;
      long tmID;
  };


  void RgItem::DrawItem(wxDC *pDC, wxRect &rect)
  {
    unsigned  cW = pDC->GetTextExtent("M").GetWidth();

    wxRect rcCP = rect;
    wxRect rcAdd = rect;
    wxRect rcNA = rect;

    rcCP.SetWidth(4 * cW);
    rcNA.SetWidth(4 * cW);
    rcNA.SetLeft(rect.GetRight() - rcNA.GetWidth());
    rcAdd.SetLeft(rcCP.GetRight());
    rcAdd.SetWidth(rect.GetWidth() - rcCP.GetWidth() - rcNA.GetWidth());

    rcCP.Deflate(offset, 0);
    rcAdd.Deflate(offset, 0);
    rcNA.Deflate(offset, 0);

    DrawString(pDC, rcCP, cpName);
    DrawString(pDC, rcAdd, addName);
    DrawString(pDC, rcNA, naName);
  }


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
}

// -----------------------------------------------------------------------
// CPlHistEvents

IMPLEMENT_DYNAMIC_CLASS(CPlHistEvents, CFormViewEx)

BEGIN_EVENT_TABLE(CPlHistEvents, CFormViewEx)
  EVT_COMBOBOX(XRCID("Event"), CPlHistEvents::OnSelChangedCp)
  EVT_LIST_ITEM_SELECTED(XRCID("EventTimestamps"), CPlHistEvents::OnSelChangedEventTimestamps)
  EVT_LIST_ITEM_SELECTED(XRCID("EnteredEvents"), CPlHistEvents::OnSelChangedEnteredEvents)
  EVT_LIST_ITEM_SELECTED(XRCID("GroupTimestamps"), CPlHistEvents::OnSelChangedGroupTimestamps)
  EVT_BUTTON(IDC_REFRESH, CFormViewEx::OnCommand)
END_EVENT_TABLE()


// -----------------------------------------------------------------------  
CPlHistEvents::CPlHistEvents(): CFormViewEx()
{
}


CPlHistEvents::~CPlHistEvents()
{
}


bool CPlHistEvents::Edit(va_list vaList)
{
  long id = va_arg(vaList, long);

  pl.SelectById(id);
  pl.Next();
  m_plItem->SetListItem(new PlItem(pl));

  CpListStore  cp;

  // "All Events" entry
  cp.cpID = 0;
  wxStrncpy(cp.cpDesc, _("All Events").c_str(), sizeof(cp.cpDesc) / sizeof(wxChar) - 1);
  m_cbEvent->AddListItem(new CpItem(cp));

  cp.SelectAll();
  while (cp.Next())
  {
    if (cp.IsAllowed(pl))
      m_cbEvent->AddListItem(new CpItem(cp));
  }

  m_cbEvent->SetCurrentItem(0L);

  OnRefresh();

  return true;
}


// -----------------------------------------------------------------------
// CPlHistEvents message handlers

void CPlHistEvents::OnSelChangedCp(wxCommandEvent &)
{
  OnRefresh();
}

void CPlHistEvents::OnSelChangedEventTimestamps(wxListEvent &)
{
  m_cpEntered->RemoveAllListItems();
  m_stHistory->RemoveAllListItems();
  m_grEntered->RemoveAllListItems();

  CpItem *cpItem = (CpItem *) m_cbEvent->GetCurrentItem();

  TsItem *tsItem = (TsItem *) m_ltHistory->GetCurrentItem();
  if (!tsItem)
    return;

  timestamp ts = tsItem->ts;

  std::map<long, CpRec> cpMap;
  std::set<long> cpSet;
  std::vector<LtRec> ltList;
  std::map<long, long> ltMap;
  std::map<long, short> tmMap;

  LtListStore lt;
  lt.SelectByPl(pl.plID, &ts);

  while (lt.Next())
  {
    if (cpItem && cpItem->cp.cpID && lt.cpID && cpItem->cp.cpID != lt.cpID)
      continue;

    ltList.push_back(lt);
    ltMap.insert({lt.ltID, lt.cpID});
    cpSet.insert(lt.cpID);
  }

  lt.Close();

  CpListStore cp;
  cp.SelectById(cpSet);
  while (cp.Next())
    cpMap[cp.cpID] = cp;
  cp.Close();

  // Now get the teams if they exist
  // Lt -> Nt -> Tm
  for (const LtRec &lt : ltList)
  {
    NtListStore nt;
    nt.SelectByLt(lt, &ts);
    if (nt.Next())
      tmMap[nt.tmID] = cpMap[lt.cpID].cpType;
    nt.Close();
  }

  for (auto nt : tmMap)
  {
    TmEntryStore tm;
    tm.SelectTeamById(nt.first, nt.second, &ts);
    if (tm.Next())
    {
      wxString add;
      wxString na;

      if (nt.second == CP_TEAM)
      {
        add = tm.team.tm.tmDesc;
        na = tm.team.tm.naName;
      }
      else if (nt.second == CP_DOUBLE || nt.second == CP_MIXED)
      {
        if ( (tm.team.pl.plNr % 10000) == pl.plNr )
          add = wxString(tm.team.bd.psName.psLast) + ", " + tm.team.bd.psName.psFirst + " (" + tm.team.bd.naName + ")";
        else if ( (tm.team.bd.plNr % 10000) == pl.plNr )
          add = wxString(tm.team.pl.psName.psLast) + ", " + tm.team.pl.psName.psFirst + " (" + tm.team.pl.naName + ")";

        if (tm.naID == tm.team.pl.naID)
          na = tm.team.pl.naName;
        else if (tm.naID == tm.team.bd.naID)
          na = tm.team.bd.naName;
      }

      RgItem *itemPtr = new RgItem(cpMap[tm.cpID], tm.tmID, add, na);
      m_cpEntered->AddListItem(itemPtr);
      cpSet.erase(tm.cpID);
    }
  }

  // And now the reamining ones, which don't belong to a team
  for (long cpID : cpSet)
    m_cpEntered->AddListItem(new RgItem(cpMap[cpID], 0, wxEmptyString, wxEmptyString));

  m_cpEntered->SortItems();

  if (ts.year != 9999 && m_cpEntered->FindListItem(ltMap[tsItem->GetID()]))
    m_cpEntered->SetCurrentItem(ltMap[tsItem->GetID()]);
  else if (m_cpEntered->GetCount())
    m_cpEntered->SetCurrentIndex(0);
}

void CPlHistEvents::OnSelChangedEnteredEvents(wxListEvent &)
{
  m_stHistory->RemoveAllListItems();
  m_grEntered->RemoveAllListItems();

  ListItem *itemPtr = m_cpEntered->GetCurrentItem();
  if (itemPtr == 0)
    return;

  long tmID = ((RgItem *) itemPtr)->tmID;

  if (!tmID)
    return;

  StListStore st;
  
  std::set<long> stSet;
  std::map<long, long> stMap;
  std::vector<std::pair<timestamp, long>> tsList = st.GetTimestamps(tmID);

  for (auto it : tsList)
    stSet.insert(it.second);

  st.SelectById(stSet);
  while (st.Next())
    stMap.insert({st.stID, st.grID});
  st.Close();

  for (auto it : tsList)
    m_stHistory->AddListItem(new TsItem(it.first, it.second));

  m_stHistory->SortItems();
  m_stHistory->SetCurrentIndex(0);
}


void CPlHistEvents::OnSelChangedGroupTimestamps(wxListEvent &)
{
  m_grEntered->RemoveAllListItems();

  ListItem *itemPtr = m_cpEntered->GetCurrentItem();
  if (itemPtr == 0)
    return;

  TsItem *tsLtItem = (TsItem *) m_ltHistory->GetCurrentItem();
  TsItem *tsStItem = (TsItem *) m_stHistory->GetCurrentItem();

  CpListStore cp;
  GrListStore gr;
  TmListStore tm;

  cp.SelectById(itemPtr->GetID());
  cp.Next();
  cp.Close();
  
  tm.SelectByCpPl(cp, pl, &tsLtItem->ts);
  tm.Next();
  tm.Close();

  std::set<long> grSet;
  std::map<long, short> stPosList;

  StListStore  stList;
  stList.SelectByCpTm(cp, tm, &tsStItem->ts);
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
    m_grEntered->InsertListItem(itemPtr);
  }

  m_grEntered->SortItems();

  if (tsStItem->ts.year != 9999 && m_grEntered->FindListItem(tsStItem->GetID()))
    m_grEntered->SetCurrentItem(tsStItem->GetID());
  else if (m_grEntered->GetCount() > 0)
    m_grEntered->SetCurrentIndex(0);
}


// -----------------------------------------------------------------------
void CPlHistEvents::OnInitialUpdate() 
{
	CFormViewEx::OnInitialUpdate();
	
  FindWindow("Refresh")->SetId(IDC_REFRESH);

	m_plItem      = XRCCTRL(*this, "Player", CItemCtrl);
  m_cbEvent     = XRCCTRL(*this, "Event", CComboBoxEx);
	m_ltHistory   = XRCCTRL(*this, "EventTimestamps", CListCtrlEx);
	m_cpEntered   = XRCCTRL(*this, "EnteredEvents", CListCtrlEx);
	m_stHistory   = XRCCTRL(*this, "GroupTimestamps", CListCtrlEx);
	m_grEntered   = XRCCTRL(*this, "EnteredGroups", CListCtrlEx);
	
	m_plItem->SetSize(wxSize(-1, wxClientDC(m_plItem).GetTextExtent(wxT("M")).GetHeight() * 2));
	m_plItem->SetMinSize(wxSize(-1, m_plItem->GetSize().GetHeight()));

	Layout();
}


void CPlHistEvents::OnEdit()
{
  if (m_cpEntered->HasFocus())
  {
  
    CpStore  cp;
    ListItem *itemPtr;    

    if ( !(itemPtr = m_ltHistory->GetCurrentItem()) )
      return;

    timestamp ts = ((const TsItem *) itemPtr)->ts;

    if ( !(itemPtr = m_cpEntered->GetCurrentItem()) )
      return;

    cp.SelectById(itemPtr->GetID());
    if (!cp.Next())
      return;
    
    cp.Close();

    if (cp.cpType == CP_SINGLE)
      return;

    if (cp.cpType == CP_DOUBLE || cp.cpType == CP_MIXED)
    {
      CTT32App::instance()->OpenView(_T("Double Revision"), wxT("LtDouble"), cp.cpID, pl.plID, &ts);
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
        CTT32App::instance()->OpenView(_T("Team Revision"), wxT("TmEdit"), tm.tmID, cp.cpID, &ts);
    } 
  }
}


void CPlHistEvents::OnRefresh()
{
  m_ltHistory->RemoveAllListItems();
  m_cpEntered->RemoveAllListItems();
  m_stHistory->RemoveAllListItems();
  m_grEntered->RemoveAllListItems();

  CpItem *cpItem = (CpItem *) m_cbEvent->GetCurrentItem();

  std::map<timestamp, long> tsMap;

  // Listen fuellen
  for (auto lt : LtListStore().GetTimestamps(pl.plID))
  {
    tsMap.insert({lt.first, lt.second});

    for (auto nt : NtListStore().GetTimestamps(lt.second))
      tsMap.insert({nt.first, lt.second});    
  }

  std::set<long> cpSet;
  std::set<long> ltSet;

  CpStore cp;
  if (cpItem && cpItem->cp.cpID)
    cp.SelectById(cpItem->cp.cpID);
  else
    cp.SelectAll();
  while (cp.Next())
    cpSet.insert(cp.cpID);
  cp.Close();

  timestamp ts = {};
  LtListStore lt;
  lt.SelectByPl(pl.plID, &ts);
  while (lt.Next())
  {
    if (cpSet.find(lt.cpID) != cpSet.end())
      ltSet.insert(lt.ltID);
  }
  lt.Close();

  for (auto it : tsMap)
  {
    if (it.first.year == 9999 || ltSet.find(it.second) != ltSet.end())
      m_ltHistory->AddListItem(new TsItem(it.first, it.first.year == 9999 ? 0 :it.second));
  }

  m_ltHistory->SortItems();

  m_ltHistory->SetCurrentIndex(0);
}

