/* Copyright (C) 2020 Christoph Theis */

// NmEditETS.cpp: Edit Nomination fuer EC Mannschaft
// Mannschaft nominiert 4 Spieler. 
// Nach dem zweiten Spiel kann sie entscheiden, ob der 4-te Spieler den ersten oder zweiten oder keinen ersetzt
// Abgebildet wird es mit 6 Spielern: 1-4 aus der Nominierung, 5 und 6 sind entweder 1 und 2 oder der Ersatz
//

#include "stdafx.h"
#include "TT32App.h"
#include "NmEditETS.h"

#include  "CpItem.h"
#include  "GrItem.h"
#include  "TmItem.h"
#include  "NmItem.h"
#include  "LtItem.h"

#include  "LtEntryStore.h"


IMPLEMENT_DYNAMIC_CLASS(CNmEditETS, CFormViewEx)

BEGIN_EVENT_TABLE(CNmEditETS, CFormViewEx)
  EVT_LIST_ITEM_SELECTED(XRCID("NominatedPlayers"), CNmEditETS::OnSelChanged)
END_EVENT_TABLE()


// -----------------------------------------------------------------------
// CNmEditETS

CNmEditETS::CNmEditETS() : CFormViewEx()
{
}


CNmEditETS::~CNmEditETS()
{
}


bool  CNmEditETS::Edit(va_list vaList)
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
  
  // Versteckte Spieler in ETS
  sySingles = 4;

  wxChar *alpha[] = {wxT("A"), wxT("B"), wxT("C"), wxT("X"), wxT("Y"), wxT("Z")};
  wxChar str[6];
    
  for (int nmSingle = 1; nmSingle <= sySingles; nmSingle++)
  {
    if (sySingles < 4)
      wxSprintf(str, "%s", ax < 0 ? alpha[nmSingle - 1] : alpha[nmSingle + 2]);
    else
      wxSprintf(str, "%c%d", ax < 0 ? 'A' : 'X', nmSingle);

    NmItem *itemPtr = new NmItem(str, false);
    itemPtr->nm.team.cpType = CP_SINGLE;
    itemPtr->nm.nmNr = nmSingle;
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

  // Test, ob 5 und 6 gesetzt sind. NmRec ist 0-basiert
  if (nm.GetSingle(3) == 0)
    plFourReplace->Select(1);
  else if (nm.GetSingle(4) == 0 || nm.GetSingle(5) == 0)
    plFourReplace->Select(0); // Not decided
  else if (nm.GetSingle(3) == nm.GetSingle(4))
    plFourReplace->Select(2);
  else if (nm.GetSingle(3) == nm.GetSingle(5))
    plFourReplace->Select(3);
  else
    plFourReplace->Select(0);

  plFourReplace->Enable(nm.GetSingle(3) != 0);
  
  OnSelChanged(wxListEvent());

  // OK-Button sperren, wenn schon Ergebnisse und Aufstellung existieren
  hasNomination &= (mt.mtResA + mt.mtResX) != 0;
  // GetDlgItem(IDOK)->EnableWindow(hasNomination ? FALSE : TRUE);
  return true;
}


// -----------------------------------------------------------------------
void CNmEditETS::OnInitialUpdate() 
{
	CFormViewEx::OnInitialUpdate();
	
  cpItem = XRCCTRL(*this, "Event", CItemCtrl);
  grItem = XRCCTRL(*this, "Group", CItemCtrl);
  tmItem = XRCCTRL(*this, "Team", CItemCtrl);
  
  plList = XRCCTRL(*this, "AvailablePlayers", CListCtrlEx);
  nmList = XRCCTRL(*this, "NominatedPlayers", CListCtrlEx);

  plFourReplace = XRCCTRL(*this, "PlayerFourReplace", wxChoice);

  // wxFormbuilder kann die Strings nicht speicher, darum muss ich das hier machen.
  // Vorerst.
  plFourReplace->AppendString(_("Not decided"));
  plFourReplace->AppendString(_("None"));
  plFourReplace->AppendString(_("Player 1"));
  plFourReplace->AppendString(_("Player 2"));

  plFourReplace->Select(0);
  
  FindWindow("MatchNo")->SetValidator(CLongValidator(&mt.mtNr));
}


// -----------------------------------------------------------------------
void  CNmEditETS::OnSelChanged(wxListEvent &)
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
void  CNmEditETS::OnAdd()
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
  
  plList->SetSelected(0);

  long idx = nmList->GetCurrentIndex();
  if (idx < nmList->GetCount() - 1)
    nmList->SetSelected(idx+1);
      
  // With 4 players enable "Player four replaces" and select "Not decided"
  if ( ((NmItem *) nmList->GetListItem(3))->nm.ltA != 0)
  {
    plFourReplace->Enable(true);
    plFourReplace->Select(0);
  }
  else
  {
    plFourReplace->Select(1);
    plFourReplace->Enable(false);
  }

  nmList->Refresh();
}


void  CNmEditETS::OnDelete()
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
      OnSelChanged(wxListEvent());
    }
  }

  if ( ((NmItem *) nmList->GetListItem(3))->nm.ltA == 0)
  {
    plFourReplace->Select(1);
    plFourReplace->Enable(false);
  }
  else
  {
    plFourReplace->Enable(true);
    plFourReplace->Select(0);
  }
}


void  CNmEditETS::OnOK()
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

  // NmRec beginnt bei 0 zu zaehlen
  int choice = (nm.GetSingle(3) == 0 ? 0 : plFourReplace->GetSelection());
  switch (choice)
  {
    case 0 :
    {
      nm.SetSingle(4, 0);
      nm.SetSingle(5, 0);
      break;
    }

    case 1 :
    {
      nm.SetSingle(4, nm.GetSingle(0));
      nm.SetSingle(5, nm.GetSingle(1));
      break;
    }

    case 2 :
    {
      nm.SetSingle(4, nm.GetSingle(3));
      nm.SetSingle(5, nm.GetSingle(1));
      break;
    }

    case 3 :
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
