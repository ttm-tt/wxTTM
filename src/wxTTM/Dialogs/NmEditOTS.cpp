/* Copyright (C) 2020 Christoph Theis */

// NmEditOTS.cpp : Edit Nomination fuer olympic Team System (3 Spieler)
//

#include "stdafx.h"
#include "TT32App.h"
#include "NmEditOTS.h"

#include  "CpItem.h"
#include  "GrItem.h"
#include  "TmItem.h"
#include  "NmItem.h"
#include  "LtItem.h"

#include  "LtEntryStore.h"


IMPLEMENT_DYNAMIC_CLASS(CNmEditOTS, CFormViewEx)

BEGIN_EVENT_TABLE(CNmEditOTS, CFormViewEx)
  EVT_LIST_ITEM_SELECTED(XRCID("NominatedPlayers"), CNmEditOTS::OnSelChanged)
END_EVENT_TABLE()


// -----------------------------------------------------------------------
// CNmEditOTS

CNmEditOTS::CNmEditOTS() : CFormViewEx()
{
}


CNmEditOTS::~CNmEditOTS()
{
}


bool  CNmEditOTS::Edit(va_list vaList)
{
  long mtID = va_arg(vaList, long);
  long tmID = va_arg(vaList, long);
  int  ax   = va_arg(vaList, int);

  mt.SelectById(mtID);
  mt.Next();
  mt.Close(); // XXX Hack

  gr.SelectById(mt.mtEvent.grID);
  gr.Next();

  sy.SelectById(gr.syID);
  sy.Next();

  cp.SelectById(gr.cpID);
  cp.Next();

  TmEntryStore  tmp;
  tmp.SelectTeamById(tmID, cp.cpType);
  tmp.Next();

  tm = tmp;

  nm.SelectByMtTm(mt, tm);
  nm.Next();
    
  cpItem->SetListItem(new CpItem(cp));
  grItem->SetListItem(new GrItem(gr));
  tmItem->SetListItem(new TmItem(tm));  

  TransferDataToWindow();

  // Flag, ob schon eine Aufstellung existiert.
  bool hasNomination = false;
  
  int sySingles = sy.sySingles;
  int syDoubles = sy.syDoubles;
  
  // Versteckter Spieler im olympischen System
  sySingles = 3;

  if (syDoubles)
    nmList->SetItemHeight(1.5);

  wxChar *alpha[] = {wxT("A"), wxT("B"), wxT("C"), wxT("X"), wxT("Y"), wxT("Z")};
  wxChar str[6];

  for (int nmSingle = 1; nmSingle <= sySingles; nmSingle++)
  {
    wxSprintf(str, "%s", ax < 0 ? alpha[nmSingle - 1] : alpha[nmSingle + 2]);

    NmItem *itemPtr = new NmItem(str, false);
    itemPtr->nm.team.cpType = CP_SINGLE;
    itemPtr->nm.nmNr = nmSingle;
    itemPtr->nm.mtID = mt.mtID;
    itemPtr->nm.tmID = tm.tmID;
    itemPtr->SetType(IDC_DELETE);
    
    nmList->AddListItem(itemPtr);
  }

  for (int nmDouble = 1; nmDouble <= syDoubles; nmDouble++)
  {
    wxSprintf(str, "%s", ax < 0 ? "C A/B" : "Z X/Y");

    NmItem *itemPtr = new NmItem(str, false);
    itemPtr->nm.team.cpType = CP_DOUBLE;
    itemPtr->nm.nmNr = nmDouble;
    itemPtr->nm.mtID = mt.mtID;
    itemPtr->nm.tmID = tm.tmID;
    itemPtr->SetType(IDC_DELETE);
    
    nmList->AddListItem(itemPtr);
  }

  NmEntryStore  nm;
  nm.SelectByMtTm(mt, tm);
  while (nm.Next())
  {
    if (nm.team.cpType == CP_SINGLE && nm.nmNr > sySingles ||
        nm.team.cpType == CP_DOUBLE && nm.nmNr > syDoubles)
      continue;
      
    long idx = (nm.team.cpType == CP_SINGLE ? nm.nmNr : sySingles + nm.nmNr);
    NmItem *itemPtr = (NmItem *) nmList->GetListItem(idx-1);
    wxASSERT(itemPtr);
    itemPtr->SetValue(nm);
    
    // Wenn eine existiert, ist es eine
    hasNomination = true;
  }    

  nmList->SetSelected(0);
  
  for (int i = 0; i < nmList->GetItemCount(); i++)
  {
    const NmEntry &nm = ((NmItem *) nmList->GetListItem(i))->nm;
    if ( nm.ltA == 0 || nm.team.cpType == CP_DOUBLE && nm.ltB == 0 )
    {
      nmList->SetSelected(i);
      break;
    }
  }
  
  OnSelChanged(wxListEvent());

  // OK-Button sperren, wenn schon Ergebnisse und Aufstellung existieren
  hasNomination &= (mt.mtResA + mt.mtResX) != 0;
  // GetDlgItem(IDOK)->EnableWindow(hasNomination ? FALSE : TRUE);
  return true;
}


// -----------------------------------------------------------------------
void CNmEditOTS::OnInitialUpdate() 
{
	CFormViewEx::OnInitialUpdate();
	
  cpItem = XRCCTRL(*this, "Event", CItemCtrl);
  grItem = XRCCTRL(*this, "Group", CItemCtrl);
  tmItem = XRCCTRL(*this, "Team", CItemCtrl);
  
  plList = XRCCTRL(*this, "AvailablePlayers", CListCtrlEx);
  nmList = XRCCTRL(*this, "NominatedPlayers", CListCtrlEx);
  
  FindWindow("MatchNo")->SetValidator(CLongValidator(&mt.mtNr));
}


// -----------------------------------------------------------------------
void  CNmEditOTS::OnSelChanged(wxListEvent &)
{
  NmItem *nmItemPtr = (NmItem *) nmList->GetCurrentItem();
  if (!nmItemPtr)
    return;
    
  plList->RemoveAllListItems();
    
  LtEntryStore lt;
  lt.SelectPlayerByTm(tm);
  while (lt.Next())
  {
    if (nmItemPtr->nm.team.cpType == CP_DOUBLE && nmItemPtr->nm.ltA != 0)
    {
      if (lt.ltID != ((NmItem *) nmList->GetListItem(0))->nm.ltA &&
          lt.ltID != ((NmItem *) nmList->GetListItem(1))->nm.ltA)
        continue;
    }
    
    LtItem *itemPtr = new LtItem(lt, false);
    itemPtr->SetType(IDC_ADD);
    plList->AddListItem(itemPtr);
  }
    
  for (int idx = 0; /* true */ ; idx++ )
  {
    NmItem *tmpPtr = (NmItem *) nmList->GetListItem(idx);
    if (!tmpPtr)
      break;
          
    if (tmpPtr->nm.team.cpType != nmItemPtr->nm.team.cpType)
      continue;

    plList->RemoveListItem(tmpPtr->nm.ltA);
    plList->RemoveListItem(tmpPtr->nm.ltB);
  }
  
  plList->SetSelected(0);
}


// -----------------------------------------------------------------------
void  CNmEditOTS::OnAdd()
{
  NmItem *nmItemPtr = (NmItem *) nmList->GetCurrentItem();
  if (!nmItemPtr)
    nmItemPtr = (NmItem *) nmList->GetListItem(0);
  if (!nmItemPtr)
    return;
    
  LtItem *ltItemPtr = (LtItem *) plList->GetCurrentItem();
  if (!ltItemPtr)
    return;
    
  LtRec lt = ltItemPtr->lt;
  PlRec pl = ltItemPtr->pl;

  if (nmItemPtr->AddPlayer(LtEntry(lt, pl)))
  {
    // Im olympic team system ist der dritte Spieler auch im Doppel.
    if (nmItemPtr->nm.team.cpType == CP_SINGLE && nmItemPtr->nm.nmNr == 3)
    {
      NmItem *nmDoublePtr = (NmItem *) nmList->GetListItem(3);
      if (nmDoublePtr && nmDoublePtr->nm.team.cpType == CP_DOUBLE)
        nmDoublePtr->AddPlayer(LtEntry(lt, pl));
    }

    delete plList->CutCurrentItem();
  }
  

  if (nmItemPtr->nm.ltB || nmItemPtr->nm.team.cpType == CP_SINGLE)
  {
    long idx = nmList->GetCurrentIndex();
    if (idx < nmList->GetCount() - 1)
      nmList->SetSelected(idx+1);
      
    // Wenn sich der Typ geaendert hat, die "available players"
    // Liste neu aufbauen. Ansonsten wurde sie oben schon korrigiert
    if ( nmList->GetCurrentItem() && 
         ((NmItem *) nmList->GetCurrentItem())->nm.team.cpType != nmItemPtr->nm.team.cpType
        )
    {
      OnSelChanged(wxListEvent());
    }
  }     
  else
    plList->SetSelected(0);

  nmList->Refresh();
}


void  CNmEditOTS::OnDelete()
{
  NmItem *nmItemPtr = (NmItem *) nmList->GetCurrentItem();
  if (!nmItemPtr)
    return;

  LtEntry lt;
  lt.cpID = cp.cpID;

  if (nmItemPtr->nm.team.cpType != CP_SINGLE && nmItemPtr->nm.ltB)
  {
    lt.ltID = nmItemPtr->nm.ltB;
    lt.SetPlayer(nmItemPtr->nm.team.bd);
  }
  else if (nmItemPtr->nm.ltA)
  {
    lt.ltID = nmItemPtr->nm.ltA;
    lt.SetPlayer(nmItemPtr->nm.team.pl);
  }
  else
    return;
        
  if (!nmItemPtr->RemovePlayer())
    return;
    
  // Im olympic team system ist der dritte Spieler auch im Doppel.
  // Wird dieser Spieler abgemeldet, wird auch das Doppel aufgeloest.
  if (nmItemPtr->nm.team.cpType == CP_SINGLE && nmItemPtr->nm.nmNr == 3)
  {
    NmItem *nmDoublePtr = (NmItem *) nmList->GetListItem(3);
    if (nmDoublePtr && nmDoublePtr->nm.team.cpType == CP_DOUBLE)
    {
      // Remove both players
      nmDoublePtr->RemovePlayer();
      nmDoublePtr->RemovePlayer();
    }
  }

  nmList->Refresh();

  LtItem *ltItemPtr = new LtItem(lt, false);
  ltItemPtr->SetType(IDC_ADD);
  plList->AddListItem(ltItemPtr);
  plList->Refresh();
  
  if ( !nmItemPtr->nm.ltA && !nmItemPtr->nm.ltB &&
       nmList->GetCurrentIndex() < nmList->GetCount() - 1 )
  {
    nmList->SetSelected(nmList->GetCurrentIndex() + 1);
      
    // Wenn sich der Typ geaendert hat, die "available players"
    // Liste neu aufbauen. Ansonsten wurde sie oben schon korrigiert
    if ( nmList->GetCurrentItem() && 
         ((NmItem *) nmList->GetCurrentItem())->nm.team.cpType != nmItemPtr->nm.team.cpType
        )
    {
      OnSelChanged(wxListEvent());
    }
  }
}


void  CNmEditOTS::OnOK()
{
  TTDbse::instance()->GetDefaultConnection()->StartTransaction();
  
  bool hasNomination = false;

  for (long idx = 0; idx < nmList->GetCount(); idx++)
  {
    NmItem *nmItemPtr = (NmItem *) nmList->GetListItem(idx);
    wxASSERT(nmItemPtr);

    if (nmItemPtr->nm.team.cpType == CP_SINGLE)
    {
      hasNomination |= nmItemPtr->nm.ltA != 0;
      nm.SetSingle(nmItemPtr->nm.nmNr-1, nmItemPtr->nm.ltA);
    }
    else if (nmItemPtr->nm.team.cpType == CP_DOUBLE)
    {
      hasNomination |= nmItemPtr->nm.ltA != 0;
      nm.SetDoubles(nmItemPtr->nm.nmNr-1, nmItemPtr->nm.ltA, nmItemPtr->nm.ltB);
    }
  }

  // Im olympischen System den vierten Spieler mitsetzen
  NmItem *nmAPtr = (NmItem *) nmList->GetListItem(0);
  NmItem *nmBPtr = (NmItem *) nmList->GetListItem(1);
  NmItem *nmDoublePtr = (NmItem *) nmList->GetListItem(3);
    
  if (!nmAPtr || nmAPtr->nm.ltA == 0 || !nmBPtr || nmBPtr->nm.ltA == 0)  
  {
    // Weniger als 2 Spieler
    nm.SetSingle(3, 0);
  }
  else if (!nmDoublePtr || nmDoublePtr->nm.ltA == 0)  // Kein Spieler fuer Doppel gemeldet
  {
    // Kein Spieler fuer Doppel gemeldet
    nm.SetSingle(3, 0);
  }    
  else if (nmAPtr->nm.ltA == nmDoublePtr->nm.ltA || nmAPtr->nm.ltA == nmDoublePtr->nm.ltB)
  {
    // Spieler A im Doppel: B spielt Einzel
    nm.SetSingle(3, nmBPtr->nm.ltA);
  }
  else if (nmBPtr->nm.ltA == nmDoublePtr->nm.ltA || nmBPtr->nm.ltA == nmDoublePtr->nm.ltB)
  {
    // Spieler B im Doppel: A spielt Einzel
    nm.SetSingle(3, nmAPtr->nm.ltA);
  }
  else
  {
    // Nicht moeglich
    nm.SetSingle(3, 0);
  }
  
  nm.Remove();
  
  // Einfuegen nur, wenn auch Nominierung existiert
  if (!hasNomination)
    TTDbse::instance()->GetDefaultConnection()->Commit();
  else if (nm.Insert(mt, tm))
    TTDbse::instance()->GetDefaultConnection()->Commit();
  else
    TTDbse::instance()->GetDefaultConnection()->Rollback();

  CFormViewEx::OnOK();
}
