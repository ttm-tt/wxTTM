/* Copyright (C) 2020 Christoph Theis */

// StList.cpp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "StListView.h"

#include "CpListStore.h"
#include "GrListStore.h"
#include "StEntryStore.h"
#include "TbEntryStore.h"
#include "MtStore.h"

#include "CpItem.h"
#include "GrItem.h"
#include "StItem.h"
#include "TmItem.h"
#include "TbItem.h"

#include "Select.h"
#include "InfoSystem.h"

#include "Request.h"
#include "TbSort.h"


// -----------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS(CStListView, CFormViewEx)


BEGIN_EVENT_TABLE(CStListView, CFormViewEx)
  EVT_COMBOBOX(XRCID("Event"), CStListView::OnSelChangeCp)
  EVT_COMBOBOX(XRCID("Group"), CStListView::OnSelChangeGr)
  EVT_COMBOBOX(XRCID("Count"), CStListView::OnSelChangeCount)
END_EVENT_TABLE()


// -----------------------------------------------------------------------
// CStListView

CStListView::CStListView() : CFormViewEx(), m_cbCp(0), m_cbGr(0), m_listCtrl(0)
{
}


CStListView::~CStListView()
{
}


bool  CStListView::Edit(va_list vaList)
{
  id = va_arg(vaList, long);
  
  if (id)
  {
    StListStore st;
    st.SelectById(id);
    st.Next();
    st.Close();
    
    gr.grID = st.grID;
  }
  
  if (gr.grID || id && !gr.grID)
  {
    gr.SelectById(gr.grID ? gr.grID : id);
    gr.Next();
    gr.Close();
  }
  
  // Liste der WB
  cp.SelectAll();
  while (cp.Next())
    m_cbCp->AddListItem(new CpItem(cp));

  // Ein Hack, um zwischen normaler Darstellung und RR-Tabelle zu unterscheiden
  if (gr.grID)
  {
    // Sollte existieren
    m_cbCp->SetCurrentItem(gr.cpID);
    long grID = gr.grID;
    OnSelChangeCp(wxCommandEvent());
    m_cbGr->SetCurrentItem(grID);    
    OnSelChangeGr(wxCommandEvent());
    
    if (gr.grModus == MOD_RR)
      m_listCtrl->SortItems(5);
      
    if (id && m_listCtrl->FindListItem(id))
      m_listCtrl->SetCurrentItem(id);
  
    return true;
  }

  // Item auswaehlen und WB auslesen
  m_cbCp->SetCurrentItem(m_cbCp->GetListItem(0));     
  wxString cpName = CTT32App::instance()->GetDefaultCP();
  m_cbCp->SetCurrentItem(cpName);

  OnSelChangeCp(wxCommandEvent());

  return true;
}


// -----------------------------------------------------------------------
void  CStListView::OnAdd()
{
  class TmItemEx : public TmItem
  {
    public:
      TmItemEx(StEntryStore &st) : TmItem(st) {}
      TmItemEx(TmEntryStore &tm) : TmItem(tm) {}
    
    public:
      virtual int Compare(const ListItem *itemPtr, int col) const {return TmItem::Compare(itemPtr, 1);}
  };

  StItem *stItemPtr = (StItem *) m_listCtrl->GetCurrentItem();  
  if (!stItemPtr)
    return;
    
  // Nur wenn noch moeglich
  if (!gr.QryStarted())
    ;
  else if (gr.grModus == MOD_RR)
  {
    if (gr.QryFinished())
    {
      infoSystem.Information(_("Group is finished."));
      return;
    }
  }
  else
  {
    MtRec mt;
    mt.mtEvent.grID = gr.grID;
    mt.mtEvent.mtRound = 1;
    mt.mtEvent.mtMatch = (stItemPtr->st.stNr + 1) / 2;
    mt.mtEvent.mtMS = 0;
    
    MtStore mtWinner, mtLoser;
    short ax;

    if (gr.QryToWinner(mt, &mtWinner, ax))
    {
      mtWinner.SelectByEvent(mtWinner.mtEvent);
      mtWinner.Next();
      if (mtWinner.mtResA || mtWinner.mtResX)
      {
        infoSystem.Information(_("First match already played."));
        return;
      }
    }
    
    if (gr.QryToLoser(mt, &mtLoser, ax))
    {
      mtLoser.SelectByEvent(mtLoser.mtEvent);
      mtLoser.Next();
      if (mtLoser.mtResA || mtLoser.mtResX)
      {
        infoSystem.Information(_("First match already played."));
        return;
      }
    }    
  }

  CSelect  select(_("Select Participant"));
  
  bool fromQualification = false;
  
  if ( !wxStrcmp(gr.grStage, "Championship") || 
       !wxStrcmp(gr.grStage, "Consolation"))
  {
    GrListStore grTmp;
    grTmp.SelectByStage(cp, "Qualification");
    if (grTmp.Next() && grTmp.grModus == MOD_RR)
      fromQualification = true;
      
    grTmp.Close();
  }
  
  if ( !wxStrcmp(gr.grStage, "Championship") && fromQualification)
  {
    TmEntryStore  tm;
    StEntryStore  st;
  
    if (CTT32App::instance()->GetType() == TT_SCI)
    {
      // Bei Oldies nur die aufnehmen, die auch Quali gespielt haben.
      st.SelectAll(cp, "Qualification");
      while (st.Next())
      {
        if (st.tmID && st.st.stPos <= 2)
          select.InsertListItem(new TmItemEx(st));
      }
    }
    else
    {
      std::map<long, GrRec> grMap;
      GrStore grTmp;

      grTmp.SelectAll(cp, "Qualification");
      while (grTmp.Next())
        grMap[grTmp.grID] = grTmp;
      grTmp.Close();

      // Alle existierenden Teams
      tm.SelectTeamForSeeding(gr, cp.cpType);
      while (tm.Next())
        select.InsertListItem(new TmItemEx(tm));

      // Alle nicht qualifizierten wieder raus
      st.SelectAll(cp, "Qualification");
      while (st.Next())
      {
        if (st.st.stPos > (grMap[st.st.grID].grSize + 1) / 2)
          select.RemoveListItem(st.tmID);
      }    
    }

    // Alle aus dieser Stufe wieder entfernen. 
    st.SelectAll(cp, gr.grStage);
    while (st.Next())
      select.RemoveListItem(st.tmID);
  }
  else if (!wxStrcmp(gr.grStage, "Consolation") && fromQualification)
  {
      std::map<long, GrRec> grMap;
      GrStore grTmp;

      grTmp.SelectAll(cp, "Qualification");
      while (grTmp.Next())
        grMap[grTmp.grID] = grTmp;
      grTmp.Close();

    // Alle existierenden Teams
    TmEntryStore tm;
    tm.SelectTeamByCp(cp);
    while (tm.Next())
      select.InsertListItem(new TmItemEx(tm));
      
    StEntryStore  st;
    
    // Alle fuer Championship qualifizierten raus
    // Ebenso wer "Non" zur Consolation gesagt hat.
    st.SelectAll(cp, "Qualification");
    while (st.Next())
    {
      if (st.st.stNocons || st.st.stPos <= (grMap[st.st.grID].grSize + 1) / 2)
        select.RemoveListItem(st.tmID);
    }
    
    // Alle aus dieser Stufe wieder entfernen. 
    st.SelectAll(cp, gr.grStage);
    while (st.Next())
      select.RemoveListItem(st.tmID);
  }
  else
  {
    TmEntryStore  tm;
  
    tm.SelectTeamForSeeding(gr, cp.cpType);
    while (tm.Next())
      select.AddListItem(new TmItemEx(tm));
  }

  // Und an das Ende die Gruppen
  std::list<XxRec> xxList;
  XxStore xx;
  xx.SelectAll();
  while (xx.Next())
    xxList.push_back(XxRec(xx));

  GrStore  grQual;
  grQual.SelectAll(cp);
  while (grQual.Next())
  {
    if (!wxStrcoll(grQual.grStage, gr.grStage))
      continue;

    for (int i = 1; i <= grQual.grSize; i++)
    {

      bool toAdd = false;

      if (grQual.grModus == MOD_RR)
      {
        if (!wxStrcmp(grQual.grStage, "Qualification"))
        {
          if (!wxStrcmp(gr.grStage, "Championship"))
          {
            if (i > (grQual.grSize + 1) / 2)
              continue;

            toAdd = true;
          }
          else if (!wxStrcmp(gr.grStage, "Consolation"))
          {
            if (i <= (grQual.grSize + 1) / 2)
              continue;

            toAdd = true;
          }
          else
          {
            // if (i > 1)
            //   break;
            toAdd = true;
          }
        }
        else
        {
          //
          toAdd = true;
        }
      }
      else if (grQual.grModus == MOD_PLO ||
               grQual.grModus == MOD_SKO && grQual.grSize == 2)
      {
        //
        toAdd = true;
      }
      else
      {
        if (i > 1)
          break;

        toAdd = true;
      }    

      if (toAdd)
      {
        for (std::list<XxRec>::iterator it = xxList.begin(); it != xxList.end(); it++)
        {
          if ( (*it).grID == grQual.grID && (*it).grPos == i )
          {
            toAdd = (*it).stID == stItemPtr->st.stID;

            break;
          }
        }
      }

      if (toAdd)
      {
        TmEntryStore tmEntry;

        tmEntry.team.cpType = CP_GROUP;
        tmEntry.tmID = grQual.grID;
    
        tmEntry.SetGroup(grQual, i);
        select.AddListItem(new TmItemEx(tmEntry));
      }
    }
  }
            
  // Sortiert auch
  select.SetSortColumn(1);
  
  // "To be defined" an den Beginn
  {
    TmEntryStore tm;

    tm.SetGroup(GrRec(), 0);
    select.AddListItem(new TmItemEx(tm), 0);
  }

  TmItem * tmItemPtr = (TmItem *) select.Select();  
  if (!tmItemPtr)
    return;

  TTDbse::instance()->GetDefaultConnection()->StartTransaction();

  // Nur echte Spieler sind gesetzt
  if (gr.SetTeam(stItemPtr->st.stNr, tmItemPtr->entry, tmItemPtr->entry.tmID ? StStore::ST_SEEDED : 0))
    TTDbse::instance()->GetDefaultConnection()->Commit();
  else
    TTDbse::instance()->GetDefaultConnection()->Rollback();

  long idx = m_listCtrl->GetCurrentIndex();
  if (idx < m_listCtrl->GetItemCount() - 1)
    m_listCtrl->SetCurrentIndex(++idx);
}


void  CStListView::OnEdit()
{
  ListItem *itemPtr = m_listCtrl->GetCurrentItem();
  if (!itemPtr)
    return;

  wxPanel *panel = CTT32App::instance()->OpenView(_("Edit Seeding"), wxT("StEdit"), itemPtr->GetID());

  if (panel)
    panel->GetParent()->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(CStListView::OnChildClose), NULL, this);
}


void  CStListView::OnDelete()
{
  if (m_listCtrl->GetSelectedCount() == 0)
    return;
    
  if (infoSystem.Confirmation(_("Are you sure to delete selected entries?")) == false)
    return;

  int lastIdx = -1;
  for (int idx = 0; idx < m_listCtrl->GetItemCount(); idx++)
  {
    if (!m_listCtrl->IsSelected(idx))
      continue;
      
    StItem *stItemPtr = (StItem *) m_listCtrl->GetListItem(idx);

    if (!stItemPtr)
      continue;

    lastIdx = idx;

    DoDelete(stItemPtr);
  }

  if (lastIdx >= 0 && lastIdx < m_listCtrl->GetItemCount() - 1)
    m_listCtrl->SetCurrentIndex(++lastIdx);
}


void CStListView::DoDelete(StItem *stItemPtr)
{
  // Nur wenn noch moeglich
  if (!gr.QryStarted())
    ;
  else if (gr.grModus == MOD_RR)
  {
    MtStore mt(gr.GetConnectionPtr());
    mt.SelectByGr(gr);
    while (mt.Next())
    {
      if (mt.mtResA == 0 && mt.mtResX == 0)
        continue;
        
      if (mt.stA == stItemPtr->st.stID || mt.stX == stItemPtr->st.stID)
      {
        mt.Close();
        return;
      }
    }
  }
  else
  {
    MtStore mt;
    mt.mtEvent.grID = gr.grID;
    mt.mtEvent.mtRound = 1;
    mt.mtEvent.mtMatch = (stItemPtr->st.stNr + 1) / 2;
    mt.mtEvent.mtMS = 0;
    
    mt.SelectByEvent(mt.mtEvent);
    mt.Next();
    if (mt.mtResA || mt.mtResX)
    {
      infoSystem.Information(_("First match already played."));
      return;
    }
    
    MtStore mtWinner, mtLoser;
    short ax;

    if (gr.QryToWinner(mt, &mtWinner, ax))
    {
      mtWinner.SelectByEvent(mtWinner.mtEvent);
      mtWinner.Next();
      if (mtWinner.mtResA || mtWinner.mtResX)
      {
        infoSystem.Information(_("First match already played."));
        return;
      }
    }
    
    if (gr.QryToLoser(mt, &mtLoser, ax))
    {
      mtLoser.SelectByEvent(mtLoser.mtEvent);
      mtLoser.Next();
      if (mtLoser.mtResA || mtLoser.mtResX)
      {
        infoSystem.Information(_("First match already played."));
        return;
      }
    }    
  }

  TTDbse::instance()->GetDefaultConnection()->StartTransaction();

  if (!stItemPtr->st.tmID && gr.SetTeam(stItemPtr->st.stNr, TmEntry()))
    TTDbse::instance()->GetDefaultConnection()->Commit();
  else if (stItemPtr->st.tmID && gr.SetTeam(stItemPtr->st.stNr, TmRec()))
    TTDbse::instance()->GetDefaultConnection()->Commit();
  else
    TTDbse::instance()->GetDefaultConnection()->Rollback();
}


// -----------------------------------------------------------------------

void CStListView::OnInitialUpdate() 
{
	CFormViewEx::OnInitialUpdate();
	
	m_cbCp = XRCCTRL(*this, "Event", CComboBoxEx);
	m_cbGr = XRCCTRL(*this, "Group", CComboBoxEx);
	m_listCtrl = XRCCTRL(*this, "StList", CListCtrlEx);
	  
  unsigned fmt = wxALIGN_LEFT;                         
                         
  // Flags, Pos, Players
  // m_listCtrl->InsertColumn(0, _("Flags"), fmt);
  m_listCtrl->InsertColumn(0, _("Pos."), wxALIGN_RIGHT);
  m_listCtrl->InsertColumn(1, "S", wxALIGN_CENTER);
  m_listCtrl->InsertColumn(2, "G", wxALIGN_CENTER);
  m_listCtrl->InsertColumn(3, "D", wxALIGN_CENTER);
  m_listCtrl->InsertColumn(4, "N", wxALIGN_CENTER);

  m_listCtrl->InsertColumn(5, _("Player(s) / Team"), fmt, GetClientSize().GetWidth());

  // Eventuell Ergebnisse fuer RR-Tabelle
  m_listCtrl->InsertColumn(6, _("Mt.Pts."), fmt);
  m_listCtrl->InsertColumn(7, _("Result"), fmt);
  m_listCtrl->InsertColumn(8, _("Games"), fmt);
  m_listCtrl->InsertColumn(9, _("Points"), fmt);
  m_listCtrl->InsertColumn(10, _("Stndg."), fmt);

  m_listCtrl->AllowHideColumn(8);
  m_listCtrl->AllowHideColumn(9);

  m_listCtrl->HideColumn(8);
  m_listCtrl->HideColumn(9);
  
  m_listCtrl->SetResizeColumn(5);

  m_listCtrl->Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(CStListView::OnMouseLeftDown), NULL, this);
}


void CStListView::OnSelChangeCp(wxCommandEvent &) 
{
  // Liste der Gruppen aktualisieren
  m_cbGr->RemoveAllListItems();
  m_listCtrl->RemoveAllListItems();

  ListItem *cpItemPtr = m_cbCp->GetCurrentItem();
  if (!cpItemPtr)
    return;

  cp.SelectById(cpItemPtr->GetID());
  cp.Next();
  
  CTT32App::instance()->SetDefaultCP(cp.cpName);

  wxString grName = gr.grID ? gr.grName : CTT32App::instance()->GetDefaultGR();

  gr.SelectAll(cp);

  while (gr.Next())
    m_cbGr->AddListItem(new GrItem(gr));

  // Gruppe auswaehlen
  m_cbGr->SetCurrentItem(m_cbGr->GetListItem(0));
  m_cbGr->SetCurrentItem(grName);

  OnSelChangeGr(wxCommandEvent());
}


void CStListView::OnSelChangeGr(wxCommandEvent &) 
{
  if ( (cp.cpType == CP_DOUBLE) || (cp.cpType == CP_MIXED) )
    m_listCtrl->SetItemHeight(2);
  else
    m_listCtrl->SetItemHeight(1);

  ListItem *grItemPtr = m_cbGr->GetCurrentItem();
  if (!grItemPtr)
    return;

  gr.SelectById(grItemPtr->GetID());
  gr.Next();

  CTT32App::instance()->SetDefaultGR(gr.grName);

  // Angabe ueber RR-Ergebnises nur in RR-Gruppen
  m_listCtrl->ShowColumn(6, gr.grModus == MOD_RR);   // Mt. Pts.
  m_listCtrl->ShowColumn(7, gr.grModus == MOD_RR);   // Result (Matches / Games)
  m_listCtrl->ShowColumn(8, false && gr.grModus == MOD_RR && cp.cpType == CP_TEAM);   // Games
  m_listCtrl->ShowColumn(9, false && gr.grModus == MOD_RR);   // Points
  m_listCtrl->ShowColumn(10, gr.grModus == MOD_RR);  // Standing

  m_listCtrl->ResizeColumn();

  bool showCount = gr.grModus == MOD_SKO && (gr.grNofRounds == 0 || gr.grNofRounds > 1);
  
  wxComboBox *cbCount = XRCCTRL(*this, "Count", wxComboBox);

  FindWindow("CountLabel")->Show(showCount);
  cbCount->Show(showCount);

  cbCount->Clear();

  if (gr.grModus == MOD_SKO)
  {
    int val = gr.grSize;
    while (val > 1)
    {
      cbCount->AppendString(wxString::Format("%d", val));
      val /= 2;
    }

    cbCount->Select(0);
  }

  OnSelChangeCount(wxCommandEvent());
}


void CStListView::OnSelChangeCount(wxCommandEvent &)
{
  // Liste der Setzungen aktualisieren
  m_listCtrl->RemoveAllListItems();

  wxComboBox *cbCount = XRCCTRL(*this, "Count", wxComboBox);

  if (gr.grModus == MOD_RR)
  {
    TbEntryStore  st;
    st.SelectByGr(gr, cp.cpType);

    while (st.Next())
    {
      m_listCtrl->AddListItem(new TbItem(st));
    }

    // Trotzdem erstmal nach Position sortieren?
    m_listCtrl->SortItems();
  }
  else
  {
    StEntryStore  st;
    st.SelectByGr(gr, cp.cpType);

    int count = (cbCount->IsShown() ? atoi(cbCount->GetValue().c_str()) : gr.grSize);
    int mod;
    int fact = (gr.grSize / count) * 2;
    
    if (count <= 4)
      mod = count;          // Oben / Unten
    else if (count <= 16)
      mod = count / 2;      // Oben / Mitte / Unten
    else if (count <= 32)
      mod = count / 4;      // Viertel
    else if (gr.grSize <= 64)
      mod = count / 8;      // Viertel
    else if (count <= 128)
      mod = count / 16;
    else
      mod = count / 32;

    mod *= (fact / 2);

    while (st.Next())
    {
      if (gr.grNofMatches && st.st.stNr > 2 * gr.grNofMatches)
        continue;

      StItem *itemPtr = new StItem(st);
      int stNr = st.st.stNr;

      if (count != gr.grSize)
      {
        if ( (stNr % fact) != 0 && (stNr % fact) != 1)
          continue;
      }
      
      bool bold = ( (stNr % mod) == 0 ) || ( (stNr % mod) == 1 );
      
      itemPtr->boldStNr = bold;
      
      m_listCtrl->AddListItem(itemPtr);
    }

    m_listCtrl->SortItems();
  }
}


void CStListView::OnMouseLeftDown(wxMouseEvent &evt)
{
  if (evt.LeftDown())
  {
    int flags;
    long col;
    long item = m_listCtrl->HitTest(evt.GetPosition(), flags, &col);
    if (item >= 0 && col >= 1 && col <= 4)
    {
      StItem *itemPtr = (StItem *) m_listCtrl->GetListItem( (int) item );

      StStore st;
      st.stID = itemPtr->st.stID;

      switch (col)
      {
      case 1 :
        st.SetSeeded(!itemPtr->st.stSeeded);
        break;

      case 2 :
        st.SetGaveup(!itemPtr->st.stGaveup);
        break;

      case 3 :
        st.SetDisqualified(!itemPtr->st.stDisqu);
        break;

      case 4 :
        st.SetNoConsolation(!itemPtr->st.stNocons);
        break;
      }
    }
    else
      evt.Skip();
  }
  else
    evt.Skip();
}


// -----------------------------------------------------------------------
void CStListView::OnUpdate(CRequest *reqPtr) 
{
	if (!reqPtr)
  {
    CFormViewEx::OnUpdate(reqPtr);
    return;
  }

  if (reqPtr->rec != CRequest::STREC)
    return;

  switch (reqPtr->type)
  {
    case CRequest::UPDATE :
    {
      StItem *itemPtr = (StItem *) m_listCtrl->FindListItem(reqPtr->id);
      if (!itemPtr)
        return;

      StEntryStore  stList;
      if (!stList.SelectById(reqPtr->id, cp.cpType))
        return;
      if (!stList.Next())
        return;

      itemPtr->SetValue(stList);

      wxRect rect;
      m_listCtrl->GetItemRect(m_listCtrl->GetListIndex(itemPtr), rect);
      m_listCtrl->Refresh(TRUE, &rect);
      break;
    }

    default :
      break;
  }
}


// -----------------------------------------------------------------------
void CStListView::OnChildClose(wxCloseEvent& evt)
{
  evt.Skip();

  long idx = m_listCtrl->GetCurrentIndex();
  if (idx < m_listCtrl->GetItemCount() - 1)
    m_listCtrl->SetCurrentIndex(++idx);
}

