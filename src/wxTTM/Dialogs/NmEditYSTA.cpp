/* Copyright (C) 2023 Christoph Theis */

// NmEditYSTA.cpp: Edit Nomination fuer EC Mannschaft
// Mannschaft nominiert 4 Spieler. 
// Nach dem zweiten Spiel kann sie entscheiden, ob der 4-te Spieler den ersten oder zweiten oder keinen ersetzt
// Abgebildet wird es mit 6 Spielern: 1-4 aus der Nominierung, 5 und 6 sind entweder 1 und 2 oder der Ersatz
//

#include "stdafx.h"
#include "TT32App.h"
#include "NmEditYSTA.h"

#include  "CpItem.h"
#include  "GrItem.h"
#include  "TmItem.h"
#include  "NmItem.h"
#include  "LtItem.h"

#include  "LtEntryStore.h"


IMPLEMENT_DYNAMIC_CLASS(NmEditYSTA, CFormViewEx)

BEGIN_EVENT_TABLE(NmEditYSTA, CFormViewEx)
  EVT_LIST_ITEM_SELECTED(XRCID("NominatedPlayers"), NmEditYSTA::OnSelChanged)
END_EVENT_TABLE()


// -----------------------------------------------------------------------
// NmEditYSTA

NmEditYSTA::NmEditYSTA() : CFormViewEx()
{
}


NmEditYSTA::~NmEditYSTA()
{
}


bool  NmEditYSTA::Edit(va_list vaList)
{
  long mtID = va_arg(vaList, long);
  long tmID = va_arg(vaList, long);
  ax   = va_arg(vaList, int);

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
  
  // Versteckte Spieler in YSTA
  sySingles = 4;

  // We have a double, increase item height
  nmList->SetItemHeight(1.5);

  wxChar str[6];
    
  for (int nmSingle = 1; nmSingle <= sySingles; nmSingle++)
  {
    wxSprintf(str, "%c-%d", ax < 0 ? 'A' : 'X', nmSingle);

    NmItem *itemPtr = new NmItem(str, false);
    itemPtr->nm.team.cpType = CP_SINGLE;
    itemPtr->nm.nmNr = nmSingle;
    itemPtr->nm.mtID = mt.mtID;
    itemPtr->nm.tmID = tm.tmID;
    itemPtr->SetType(IDC_DELETE);
    
    nmList->AddListItem(itemPtr);
  }

  // And the double
  {
    wxSprintf(str, "D%c", ax < 0 ? 'A' : 'X');

    NmItem * itemPtr = new NmItem(str, false);
    itemPtr->nm.team.cpType = CP_DOUBLE;
    itemPtr->nm.nmNr = 1;
    itemPtr->nm.mtID = mt.mtID;
    itemPtr->nm.tmID = tm.tmID;
    itemPtr->SetType(IDC_DELETE);

    nmList->AddListItem(itemPtr);
  }

  NmEntryStore  nmEntry;
  nmEntry.SelectByMtTm(mt, tm);
  while (nmEntry.Next())
  {
    if (nmEntry.team.cpType == CP_SINGLE && nmEntry.nmNr > sySingles ||
        nmEntry.team.cpType == CP_DOUBLE && nmEntry.nmNr > syDoubles)
      continue;
      
    long idx = (nmEntry.team.cpType == CP_SINGLE ? nmEntry.nmNr : sySingles + nmEntry.nmNr);
    NmItem *itemPtr = (NmItem *) nmList->GetListItem(idx-1);
    wxASSERT(itemPtr);
    itemPtr->SetValue(nmEntry);
    
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

  if (ax < 0)
  {
    lblPlReplaces[0] = wxString(_("Not decided"));
    lblPlReplaces[1] = wxString(_("None"));
    lblPlReplaces[2] = wxString::Format(_("A-3 replaces A-1"));
    lblPlReplaces[3] = wxString::Format(_("A-3 replaces A-2"));
    lblPlReplaces[4] = wxString::Format(_("A-4 replaces A-1"));
    lblPlReplaces[5] = wxString::Format(_("A-4 replaces A-2"));
  }
  else
  {
    lblPlReplaces[0] = wxString(_("Not decided"));
    lblPlReplaces[1] = wxString(_("None"));
    lblPlReplaces[2] = wxString::Format(_("X-3 replaces X-1"));
    lblPlReplaces[3] = wxString::Format(_("X-3 replaces X-2"));
    lblPlReplaces[4] = wxString::Format(_("X-4 replaces X-1"));
    lblPlReplaces[5] = wxString::Format(_("X-4 replaces X-2"));
  }

  ResetPlReplaces();

  // Test, ob 5 und 6 gesetzt sind. NmRec ist 0-basiert
  if (nm.GetSingle(2) == 0)                              // No 3rd, we'll stay at None
    plReplace->Select(1);
  else if (nm.GetSingle(4) == 0 || nm.GetSingle(5) == 0) // Not decided (players for 5th or 6th not set
    plReplace->Select(0);                                
  else if (nm.GetSingle(4) == nm.GetSingle(2))           // 3 replaces 1
    plReplace->Select(2);
  else if (nm.GetSingle(5) == nm.GetSingle(2))           // 3 replaces 2
    plReplace->Select(3);
  else if (nm.GetSingle(4) == nm.GetSingle(3))           // 4 replaces 1
    plReplace->Select(4);
  else if (nm.GetSingle(5) == nm.GetSingle(3))           // 4 replaces 2
    plReplace->Select(5);
  else                                                   // Else 3 and 4 replaces none
    plReplace->Select(1);

  plReplace->Enable(nm.GetSingle(2) != 0);
  // plReplace->Enable(true);

  OnSelChanged(wxListEvent_);

  // OK-Button sperren, wenn schon Ergebnisse und Aufstellung existieren
  hasNomination &= (mt.mtResA + mt.mtResX) != 0;
  // GetDlgItem(IDOK)->EnableWindow(hasNomination ? FALSE : TRUE);
  return true;
}


// -----------------------------------------------------------------------

void NmEditYSTA::ResetPlReplaces()
{
  // If 3rd player is not set, only "None" is possible
  // If 4th player is not set, only "None" and "Player 3 replaces ..." are possible
  // Indices are 0-based
  plReplace->Clear();

  if (((NmItem *) nmList->GetListItem(2))->nm.ltA == 0)
  {
    // No 3rd, only "None" available
    plReplace->AppendString(lblPlReplaces[1]);
  }
  else if (((NmItem *) nmList->GetListItem(3))->nm.ltA == 0)
  {
    // Only 3rd, no 4th: only Not decided, None, and 3 replaces ... available
    plReplace->AppendString(lblPlReplaces[0]); // Not decided
    plReplace->AppendString(lblPlReplaces[1]); // None
    plReplace->AppendString(lblPlReplaces[2]); // 3 -> 1
    plReplace->AppendString(lblPlReplaces[3]); // 3 -> 2
  }
  else
  {
    // 3rd and 4th present, all options available
    plReplace->AppendString(lblPlReplaces[0]); // Not decided
    plReplace->AppendString(lblPlReplaces[1]); // None
    plReplace->AppendString(lblPlReplaces[2]); // 3 -> 1
    plReplace->AppendString(lblPlReplaces[3]); // 3 -> 2
    plReplace->AppendString(lblPlReplaces[4]); // 4 -> 1
    plReplace->AppendString(lblPlReplaces[5]); // 4 -> 2
  }

  plReplace->Enable(true);
  plReplace->Select(0);

  nmList->Refresh();
}

void NmEditYSTA::OnInitialUpdate() 
{
	CFormViewEx::OnInitialUpdate();
	
  cpItem = XRCCTRL(*this, "Event", CItemCtrl);
  grItem = XRCCTRL(*this, "Group", CItemCtrl);
  tmItem = XRCCTRL(*this, "Team", CItemCtrl);
  
  plList = XRCCTRL(*this, "AvailablePlayers", CListCtrlEx);
  nmList = XRCCTRL(*this, "NominatedPlayers", CListCtrlEx);

  plReplace = XRCCTRL(*this, "PlayerReplaces", wxChoice);

  FindWindow("MatchNo")->SetValidator(CLongValidator(&mt.mtNr));
}


// -----------------------------------------------------------------------
void  NmEditYSTA::OnSelChanged(wxListEvent &)
{
  NmItem *nmItemPtr = (NmItem *) nmList->GetCurrentItem();
  if (!nmItemPtr)
    return;
    
  plList->RemoveAllListItems();
    
  LtEntryStore lt;
  lt.SelectPlayerByTm(tm);
  while (lt.Next())
  {
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
void  NmEditYSTA::OnAdd()
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
    delete plList->CutCurrentItem();
  }
  
  // We completed a double or this is a single: 
  // continue to the next possible entry
  if (nmItemPtr->nm.ltB || nmItemPtr->nm.team.cpType == CP_SINGLE)
  {
    long idx = nmList->GetCurrentIndex();
    while (++idx < nmList->GetCount() - 1)
    {
      NmItem * nmItem = (NmItem *) nmList->GetListItem(idx);

      // Break on not-single
      if (nmItem->nm.team.cpType != CP_SINGLE)
        break;

      // Stop when player is already nominated
      if (nmItem->nm.ltA != 0)
        break;

      // Continue when no more players are available
      if (plList->GetCount() == 0)
        continue;

      break;
    }

    nmList->SetSelected(idx);
  }

  ResetPlReplaces();

  if (((NmItem *) nmList->GetListItem(2))->nm.ltA != 0)
    plReplace->Select(1);  // No change
  else
    plReplace->Select(0);  // Not decided

}


void  NmEditYSTA::OnDelete()
{
  NmItem *nmItemPtr = (NmItem *) nmList->GetCurrentItem();
  if (!nmItemPtr)
    return;

  LtEntry lt;
  lt.cpID = cp.cpID;

  if (nmItemPtr->nm.ltA)
  {
    lt.ltID = nmItemPtr->nm.ltA;
    lt.SetPlayer(nmItemPtr->nm.team.pl);
  }
  else
    return;
    
  if (!nmItemPtr->RemovePlayer())
    return;
    
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
      OnSelChanged(wxListEvent_);
    }
  }

  ResetPlReplaces();
}


void  NmEditYSTA::OnOK()
{
  TTDbse::instance()->GetDefaultConnection()->StartTransaction();
  
  bool hasNomination = false;

  for (long idx = 0; idx < nmList->GetCount(); idx++)
  {
    NmItem *nmItemPtr = (NmItem *) nmList->GetListItem(idx);
    wxASSERT(nmItemPtr);

    // Players 3 and 4 are optional (0-based) 
    bool optional = (idx == 2) || (idx == 3);

    if (nmItemPtr->nm.team.cpType == CP_SINGLE)
    {
      hasNomination |= nmItemPtr->nm.ltA != 0;
      nm.SetSingle(nmItemPtr->nm.nmNr-1, nmItemPtr->nm.ltA, optional);
    }
    else if (nmItemPtr->nm.team.cpType == CP_DOUBLE)
    {
      hasNomination |= nmItemPtr->nm.ltA != 0;
      nm.SetDoubles(nmItemPtr->nm.nmNr-1, nmItemPtr->nm.ltA, nmItemPtr->nm.ltB);
    }
  }

  // NmRec beginnt bei 0 zu zaehlen
  // Match 4: player 5 (A2/A3/X4)
  // Match 5: player 6 (A1/A3/A4)
  int choice = plReplace->GetSelection();

  // Quick Fix: If no 3rd player it will be "none"
  if (nm.GetSingle(2) == 0)
    choice = 1;

  switch (choice)
  {
    case 0 :  // Not decided
    {
      nm.SetSingle(4, 0);
      nm.SetSingle(5, 0);
      break;
    }

    case 1 :  // None
    {
      nm.SetSingle(4, nm.GetSingle(0));  // A1 / X1 (6)
      nm.SetSingle(5, nm.GetSingle(1));  // A2 / X2 (5)
      break;
    }

    case 2 :  // 3 replaces A1 / X1 (5); A2 / X2 (6) stay
    {
      nm.SetSingle(4, nm.GetSingle(2));
      nm.SetSingle(5, nm.GetSingle(1));
      break;
    }

    case 3:  // 3 replaces A2 / X2 (6); A1 / X1 (5) stay
    {
      nm.SetSingle(4, nm.GetSingle(0));
      nm.SetSingle(5, nm.GetSingle(2));
      break;
    }

    case 4 : // 4 replaces A1 / X1 (5); A2 / X2 (6) stay
    {
      nm.SetSingle(4, nm.GetSingle(3));
      nm.SetSingle(5, nm.GetSingle(1));
      break;
    }

    case 5:  // 4 replaces A2 / X2 (6); A1 / X1 (5) stay
    {
      nm.SetSingle(4, nm.GetSingle(0));
      nm.SetSingle(5, nm.GetSingle(3));
      break;
    }
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
