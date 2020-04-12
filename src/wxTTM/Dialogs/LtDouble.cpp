/* Copyright (C) 2020 Christoph Theis */

// LtDouble.cpp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "LtDouble.h"

#include "NaItem.h"
#include "LtItem.h"

#include "NaListStore.h"
#include "PlStore.h"
#include "LtEntryStore.h"
#include "TmListStore.h"
#include "NtStore.h"
#include "RkStore.h"

#include "Rec.h"


IMPLEMENT_DYNAMIC_CLASS(CLtDouble, CFormViewEx)
BEGIN_EVENT_TABLE(CLtDouble, CFormViewEx)
  EVT_COMBOBOX(XRCID("Partner"), CLtDouble::OnSelchangeLtDoublePartner)
END_EVENT_TABLE()

// -----------------------------------------------------------------------
// CLtDouble

CLtDouble::CLtDouble() : CFormViewEx(), m_plItem(0), m_bdCbBox(0), m_naCbBox(0)
{
}

CLtDouble::~CLtDouble()
{
}



// -----------------------------------------------------------------------
bool  CLtDouble::Edit(va_list vaList)
{
  m_cpID = va_arg(vaList, long);
  m_plID = va_arg(vaList, long);
  uintptr_t ptr = va_arg(vaList, uintptr_t);

  timestamp ts = {};
  if (ptr)
    ts = * ((timestamp *) ptr);

  CpStore      cp;
  LtEntryStore ltpl;
  LtEntryStore ltbd;
  TmListStore  tm;
  NaListStore  na;
  
  ltpl.SelectByCpPl(m_cpID, m_plID, ptr ? &ts : NULL);
  ltpl.Next();
  ltpl.Close();
  
  cp.SelectById(m_cpID);
  cp.Next();
  
  if (!ltpl.ltID || !cp.cpID)
    return true;

  m_plItem->SetListItem(new LtItem(ltpl));

  // "Partner Wanted" Eintrag
  m_bdCbBox->AddListItem(new LtItem());

  // Freie Spieler
  ltbd.SelectForDouble(ltpl, cp);

  while (ltbd.Next())
  {
    if (ltbd.ltID != ltpl.ltID)
      m_bdCbBox->AddListItem(new LtItem(ltbd));
  }

  na.SelectById(ltpl.naID);
  na.Next();
  m_naCbBox->AddListItem(new NaItem(na));
  m_naCbBox->SetCurrentItem(na.naID);
  
  // Partner suchen
  tm.SelectByLt(ltpl, ptr ? &ts : NULL);
  if (tm.Next())
  {
    ltbd.SelectBuddy(ltpl, ptr ? &ts : NULL);
    if (ltbd.Next())
    {
      ltbd.Close();

      // Aktueller Spieler kommt an zweiter Stelle, nach dem "Partner Wanted"
      m_bdCbBox->AddListItem(new LtItem(ltbd), 1);

      if (ltbd.naID != ltpl.naID)
      {
        na.SelectById(ltbd.naID);
        na.Next();
        m_naCbBox->AddListItem(new NaItem(na));
      }

      m_naCbBox->SetCurrentItem(tm.naID);
    }
  } // Partner suchen

  // Wenn er oben gefunden ist, wird er ausgewaehlt,
  // ansonsten ist es der leere Eintrag
  m_bdCbBox->SetCurrentItem(ltbd.ltID);  

#if 0
  // Check / mark falsche Nation
  if (plbd.plID && plbd.naID != plpl.naID)
  {
    long rkID = (m_naCbBox->GetCurrentItem() ? ((NaItem *) m_naCbBox->GetCurrentItem())->na.naID : 0);
    if (plpl.naID == rkID && ltpl.ltRankPts < ltbd.ltRankPts ||
        plbd.naID == rkID && ltbd.ltRankPts < ltpl.ltRankPts )
    {
      m_naCbBox->GetCurrentItem()->SetForeground(wxColor(255, 0, 0));
    }
  }
#endif
      
  // Disable when only viewing history
  if (ptr)
  {
    m_bdCbBox->Enable(false);
    m_naCbBox->Enable(false);

    FindWindow("OK")->Enable(false);
  }

  return true;
}

// -----------------------------------------------------------------------
void  CLtDouble::OnOK()
{
  CpStore  cp;       // WB, um den es hier geht
  NaStore  na;       // Ausgewaehlte Nation fuer Doppel

  LtItem *plItemPtr = (LtItem *) m_plItem->GetListItem();
  LtItem *bdItemPtr = (LtItem *) m_bdCbBox->GetCurrentItem();
  NaItem *naItemPtr = (NaItem *) m_naCbBox->GetCurrentItem();

  if (!plItemPtr || !bdItemPtr)
  {
    // Kein Spieler oder Partner zu ermitteln
    CFormViewEx::OnOK();
    return;
  }

  // WB ermitteln
  cp.SelectById(m_cpID);
  cp.Next();

  // Nation ermitteln. Gehoeren beide nicht einer Nation an, 
  // die des Spielers (also <null> verwenden.
  na.SelectById(naItemPtr ? naItemPtr->na.naID : plItemPtr->pl.naID);
  na.Next();

  TTDbse::instance()->GetDefaultConnection()->StartTransaction();

  // Testen ob die Spieler getauscht werden muessen
  bool check = 
    ( plItemPtr->pl.naID == bdItemPtr->pl.naID ? 
        cp.CheckDoubleOrder(plItemPtr->GetEntry(), bdItemPtr->GetEntry(), NaRec()) : 
        cp.CheckDoubleOrder(plItemPtr->GetEntry(), bdItemPtr->GetEntry(), na) );

  bool commit = true;

  if (check)
    commit &= cp.CreateDouble(plItemPtr->lt, bdItemPtr->lt, na);
  else
    commit &= cp.CreateDouble(bdItemPtr->lt, plItemPtr->lt, na);

  if (commit)
    cp.Commit();
  else
    cp.Rollback();

  CFormViewEx::OnOK();
}

// -----------------------------------------------------------------------
// CLtDouble message handlers

void CLtDouble::OnInitialUpdate() 
{
	CFormViewEx::OnInitialUpdate();	
	
	m_plItem  = XRCCTRL(*this, "Player", CItemCtrl);
	m_bdCbBox = XRCCTRL(*this, "Partner", CComboBoxEx);
	m_naCbBox = XRCCTRL(*this, "Association", CComboBoxEx);

	m_plItem->SetSize(wxSize(-1, wxClientDC(m_plItem).GetTextExtent(wxT("M")).GetHeight() * 2));
	m_plItem->SetMinSize(wxSize(-1, m_plItem->GetSize().GetHeight()));

	Layout();
}


void CLtDouble::OnSelchangeLtDoublePartner(wxCommandEvent &) 
{
  const LtItem *plItemPtr, *bdItemPtr;
  NaListStore  napl, nabd;
  PsStore  pspl, psbd;

  m_naCbBox->RemoveAllListItems();

  // Zuerst Partner, wenn er existiert
  bdItemPtr = (const LtItem *) m_bdCbBox->GetCurrentItem();
  psbd.SelectById(bdItemPtr->pl.psID);
  psbd.Next();

  nabd.SelectById(bdItemPtr->pl.naID);
  if (nabd.Next())
    m_naCbBox->AddListItem(new NaItem(nabd));

  // Dann der Spieler
  plItemPtr = (const LtItem *) m_plItem->GetListItem();
  if (plItemPtr->pl.psID)
  {
    pspl.SelectById(plItemPtr->pl.psID);
    pspl.Next();
  }

  if (plItemPtr->pl.naID != bdItemPtr->pl.naID)
  {
    napl.SelectById(plItemPtr->pl.naID);
    if (napl.Next())
      m_naCbBox->AddListItem(new NaItem(napl));
  }
  else
    napl.naID = nabd.naID;

  // Und jetzt die Defaultauswahl
  if (!bdItemPtr->pl.psID ||!nabd.naID)
    m_naCbBox->SetCurrentItem(napl.naID);
  else if (pspl.psSex == psbd.psSex)
  {
    if (plItemPtr->lt.ltRankPts < bdItemPtr->lt.ltRankPts)
      m_naCbBox->SetCurrentItem(nabd.naID);
    else if (plItemPtr->lt.ltRankPts > bdItemPtr->lt.ltRankPts)
      m_naCbBox->SetCurrentItem(napl.naID);  
    else if (plItemPtr->pl.plNr < bdItemPtr->pl.plNr)      
      m_naCbBox->SetCurrentItem(napl.naID);
    else
      m_naCbBox->SetCurrentItem(nabd.naID);
  }
  else
  {
    if (pspl.psSex == SEX_MALE)
      m_naCbBox->SetCurrentItem(napl.naID);
    else
      m_naCbBox->SetCurrentItem(nabd.naID);
  }
}
