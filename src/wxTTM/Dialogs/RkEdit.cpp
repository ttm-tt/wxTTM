/* Copyright (C) 2020 Christoph Theis */

// RkEdit.cpp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "RkEdit.h"

#include "CpItem.h"
#include "NaItem.h"
#include "RkItem.h"

#include "RkStore.h"

#include "Rec.h"
#include "Request.h"

#include "InfoSystem.h"


IMPLEMENT_DYNAMIC_CLASS(CRkEdit, CFormViewEx)

BEGIN_EVENT_TABLE(CRkEdit, CFormViewEx)
  EVT_BUTTON(IDC_UP, CRkEdit::OnUp)
  EVT_BUTTON(IDC_DOWN, CRkEdit::OnDown)
END_EVENT_TABLE()


// -----------------------------------------------------------------------
// CRkEdit
CRkEdit::CRkEdit() : CFormViewEx()
{
}


CRkEdit::~CRkEdit()
{
}


bool CRkEdit::Edit(va_list vaList)
{
  long cpID = va_arg(vaList, long);
  long naID = va_arg(vaList, long);

  cp.SelectById(cpID);
  cp.Next();
  cp.Close();
  m_cpItem->SetListItem(new CpItem(cp));

  if (cp.cpType == CP_TEAM && cp.syID == 0)
    infoSystem.Warning(_("Team ranking requires a default team system set for the event"));

  if (naID)
  {
    na.SelectById(naID);
    na.Next();
    na.Close();
    m_naItem->SetListItem(new NaItem(na));
  }
  else
  {
    FindWindow("labelAssociation")->Show(false);
    FindWindow("Association")->Show(false);
  }

  if ( (cp.cpType == CP_DOUBLE) || (cp.cpType == CP_MIXED) )
    m_listCtrl->SetItemHeight(2);
  else
    m_listCtrl->SetItemHeight(1);

  RkEntryStore  rk;

  m_de = m_qu = 0;
  
  int tmp = 1;

  if (na.naID)
    rk.SelectByCpNa(cp, na);
  else
    rk.SelectByCp(cp);

  while (rk.Next())
  {
    if (rk.rk.rkDirectEntry)
      m_de++;
    else
      m_qu++;
      
    // rk.rk.rkNatlRank = tmp++;

    m_listCtrl->AddListItem(new RkItem(rk));
  }

  if (na.naID)
    m_listCtrl->SortItems(2);
  else
  {
    m_listCtrl->SortItems(4);

    for (int idx = m_listCtrl->GetCount(); idx--; )
    {
      ((RkItem *) m_listCtrl->GetListItem(idx))->rk.rkNatlRank = idx + 1;
    }
  }

  m_listCtrl->ResizeColumn();

  TransferDataToWindow();

  return true;
}


// -----------------------------------------------------------------------
void CRkEdit::OnInitialUpdate() 
{
	CFormViewEx::OnInitialUpdate();
	
	m_cpItem = XRCCTRL(*this, "Event", CItemCtrl);
	m_naItem = XRCCTRL(*this, "Association", CItemCtrl);
	m_listCtrl = XRCCTRL(*this, "Participants", CListCtrlEx);
	
  // Players, Type, Ranking
  m_listCtrl->InsertColumn(0, _("Players / Team"));
  m_listCtrl->InsertColumn(1, _("Type"), wxALIGN_LEFT);
  m_listCtrl->InsertColumn(2, _("Rank."), wxALIGN_LEFT);
  m_listCtrl->InsertColumn(3, _("Int'l"), wxALIGN_RIGHT);
  m_listCtrl->InsertColumn(4, _("Rank. Pts."), wxALIGN_RIGHT);

  m_listCtrl->SetResizeColumn(0);
  
  FindWindow("DirectEntries")->Connect(wxEVT_KILL_FOCUS, wxFocusEventHandler(CRkEdit::OnKillFocusDe), NULL, this);
  FindWindow("Qualifiers")->Connect(wxEVT_KILL_FOCUS, wxFocusEventHandler(CRkEdit::OnKillFocusQu), NULL, this);
  
  FindWindow("DirectEntries")->SetValidator(CShortValidator(&m_de));
  FindWindow("Qualifiers")->SetValidator(CShortValidator(&m_qu));
  
  FindWindow("Up")->SetId(IDC_UP);
  FindWindow("Down")->SetId(IDC_DOWN);
}


// -----------------------------------------------------------------------
// CRkEdit message handlers
void CRkEdit::OnKillFocusDe(wxFocusEvent &evt) 
{
  evt.Skip();
  
  short tmp = m_de;  // Alter Wert

  // Daten holen und verifizieren
  TransferDataFromWindow();

  if (m_de > tmp + m_qu)
    m_de = tmp + m_qu;
  else if (m_de < 0)
    m_de = 0;

  // m_qu berechnen
  m_qu = (tmp + m_qu) - m_de;

  // Neue Daten setzen
  TransferDataToWindow();

  // Liste aktualisieren
  for (int idx = m_listCtrl->GetItemCount(); idx--; )
  {
    RkItem *itemPtr = (RkItem *) m_listCtrl->GetListItem(idx);
    itemPtr->SetDirectEntry(idx < m_de ? true : false);
  }

  m_listCtrl->Refresh();
}


void CRkEdit::OnKillFocusQu(wxFocusEvent &evt) 
{
  evt.Skip();
  
  int  tmp = m_qu;  // Alter Wert

  // Daten holen und verifizieren
  TransferDataFromWindow();

  if (m_qu > tmp + m_de)
    m_qu = tmp + m_de;
  else if (m_qu < 0)
    m_qu = 0;

  // m_de berechnen
  m_de = (tmp + m_de) - m_qu;

  // Neue Daten setzen
  TransferDataToWindow();

  // Liste aktualisieren
  for (int idx = m_listCtrl->GetItemCount(); idx--; )
  {
    RkItem *itemPtr = (RkItem *) m_listCtrl->GetListItem(idx);
    itemPtr->SetDirectEntry(idx < m_de ? true : false);
  }

  m_listCtrl->Refresh();
}


void CRkEdit::OnDown(wxCommandEvent &) 
{
  long idx = m_listCtrl->GetCurrentIndex();
  if (idx == -1 || idx == m_de + m_qu - 1)
    return;

  RkItem *curItemPtr = (RkItem *) m_listCtrl->GetListItem(idx);
  RkItem *nxtItemPtr = (RkItem *) m_listCtrl->GetListItem(idx+1);

  if (!curItemPtr || !nxtItemPtr)
    return;
    
  // Es kann passieren, dass 2 aufeinanderfolgende Eintraege
  // das gleiche Ranking haben.
  if (nxtItemPtr->GetNatlRank() > curItemPtr->GetNatlRank() + 1)
  {
    curItemPtr->SetNatlRank(curItemPtr->GetNatlRank() + 1);
    m_listCtrl->Refresh();
    
    return;
  }

  // Swap ranking
  short rank = curItemPtr->GetNatlRank();
  curItemPtr->SetNatlRank(nxtItemPtr->GetNatlRank());
  nxtItemPtr->SetNatlRank(rank);

  // swap type
  bool type = curItemPtr->GetDirectEntry();
  curItemPtr->SetDirectEntry(nxtItemPtr->GetDirectEntry());
  nxtItemPtr->SetDirectEntry(type);

  // Wirklich nach Spalte 2? Oder in Abhaengigkeit von naID nach 2 oder 4?
  m_listCtrl->SortItems(2);
  m_listCtrl->Refresh();
}


void CRkEdit::OnUp(wxCommandEvent &) 
{
  long idx = m_listCtrl->GetCurrentIndex();
  if (idx <= 0)
    return;

  RkItem *curItemPtr = (RkItem *) m_listCtrl->GetListItem(idx);
  RkItem *prvItemPtr = (RkItem *) m_listCtrl->GetListItem(idx-1);

  if (!curItemPtr || !prvItemPtr)
    return;

  // Swap ranking
  short rank = curItemPtr->GetNatlRank();
  curItemPtr->SetNatlRank(prvItemPtr->GetNatlRank());
  prvItemPtr->SetNatlRank(rank);

  // swap type
  bool type = curItemPtr->GetDirectEntry();
  curItemPtr->SetDirectEntry(prvItemPtr->GetDirectEntry());
  prvItemPtr->SetDirectEntry(type);

  // Wirklich nach Spalte 2? Oder in Abhaengigkeit von naID nach 2 oder 4?
  m_listCtrl->SortItems(2);
  m_listCtrl->Refresh();
}


void CRkEdit::OnEdit()
{
  long  idx = m_listCtrl->GetCurrentIndex();
  RkItem *itemPtr = (RkItem *) m_listCtrl->GetCurrentItem();
  
  if (itemPtr == NULL)
    return;

  long rank = ::wxGetNumberFromUser(
      _("Enter new international ranking"), _("Int'l rank."), _("Edit International Ranking"),
      itemPtr->GetIntlRank(), 0, 0x7FFFFFFF, this);

  if (rank >= 0)
  {
    itemPtr->SetIntlRank(rank);

    while (idx > 0)
    {
      // Rank 0 geht niemals hoch
      if (rank == 0)
        break;

      // So lange hoch, bis ein Ranking "0 < Ranking <= rank" gefunden wird
      int nextRank = ((RkItem *) m_listCtrl->GetListItem(idx - 1))->GetIntlRank();
      if (nextRank > 0 && nextRank <= rank)
        break;

      m_listCtrl->SetCurrentIndex(idx);
      OnUp(wxCommandEvent());
      m_listCtrl->SetCurrentIndex(--idx);
    }

    while (idx < m_listCtrl->GetCount() - 1)
    {
      int nextRank = ((RkItem *) m_listCtrl->GetListItem(idx + 1))->GetIntlRank();

      // Solange runter bis ein Ranking > rank > 0 gefunden wird
      if (rank > 0 && nextRank >= rank)
        break;

      // Oder Ranking == 0
      if (nextRank == 0)
        break;

      m_listCtrl->SetCurrentIndex(idx);
      OnDown(wxCommandEvent());
      m_listCtrl->SetCurrentIndex(++idx);
    }
  }

  m_listCtrl->Refresh();
}


void  CRkEdit::OnOK() 
{
  if (na.naID)
    OnOKAssoc();
  else
    OnOKAll();

  CFormViewEx::OnOK();
}


void CRkEdit::OnOKAll()
{
  TTDbse::instance()->GetDefaultConnection()->StartTransaction();

  RkStore rk;

  rk.Remove(cp);

  std::map<int, std::list<RkRec>> list;

  for (int idx = 0; idx < m_listCtrl->GetItemCount(); idx++)
  {
    const RkItem *itemPtr = (const RkItem *) m_listCtrl->GetListItem(idx);

    // Daten kopieren
    rk.tmID = itemPtr->entry.tmID;
    rk.naID = itemPtr->GetValue().naID;
    rk.rkNatlRank = itemPtr->GetValue().rkNatlRank;
    rk.rkIntlRank = itemPtr->GetValue().rkIntlRank;
    rk.rkDirectEntry = itemPtr->GetValue().rkDirectEntry;

    list[itemPtr->GetValue().naID].push_back(rk);
  }

  for (std::map<int, std::list<RkRec>>::iterator it = list.begin(); it != list.end(); it++)
  {
    int natlRank = 0;
    while ((*it).second.size())
    {
      rk = *((*it).second.begin());
      (*it).second.pop_front();

      rk.rkNatlRank = ++natlRank;
      
      if (!rk.Insert())
      {
        TTDbse::instance()->GetDefaultConnection()->Rollback();
        return;
      }
    }
  }

  // Notify RkListView (Absichtlich NAREC)
  CRequest update;
  update.type = CRequest::UPDATE;
  update.rec  = CRequest::NAREC;

  for (std::map<int, std::list<RkRec>>::iterator it = list.begin(); it != list.end(); it++)
  {
    update.id   = (*it).first;
    CTT32App::NotifyChange(update);
  }

  TTDbse::instance()->GetDefaultConnection()->Commit();
}


void CRkEdit::OnOKAssoc()
{
  RkStore  rk;

  // Alle Eintraege loeschen und neu einfuegen
  TTDbse::instance()->GetDefaultConnection()->StartTransaction();

  rk.Remove(cp, na);

  for (int idx = 0; idx < m_listCtrl->GetItemCount(); idx++)
  {
    const RkItem *itemPtr  = (const RkItem *) m_listCtrl->GetListItem(idx);
    
    // Daten kopieren
    rk.tmID = itemPtr->entry.tmID;
    rk.naID = itemPtr->GetValue().naID;
    rk.rkNatlRank = itemPtr->GetValue().rkNatlRank;
    rk.rkIntlRank = itemPtr->GetValue().rkIntlRank;
    rk.rkDirectEntry = itemPtr->GetValue().rkDirectEntry;

    if (!rk.Insert())
    {
      TTDbse::instance()->GetDefaultConnection()->Rollback();
      return;
    }
  }
  
  // Notify RkListView (Absichtlich NAREC)
  CRequest update;
  update.type = CRequest::UPDATE;
  update.rec  = CRequest::NAREC;
  update.id   = na.naID;

  CTT32App::NotifyChange(update);

  TTDbse::instance()->GetDefaultConnection()->Commit();
}
