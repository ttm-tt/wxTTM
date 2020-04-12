/* Copyright (C) 2020 Christoph Theis */

// NmEdit.cpp : implementation file
// XXX: Hier faellt auch OTS2 drunter: 
//      An sich ein einfaches Spielsystem, aber die Einzel sind zwangslaeufig auch im Doppel
// XXX: Kann man OTS2 auf OTS abbilden? Ansonsten muss man beim Spiel nochmal auswaehlen koennen
//

#include "stdafx.h"
#include "TT32App.h"
#include "NmEdit.h"

#include  "CpItem.h"
#include  "GrItem.h"
#include  "TmItem.h"
#include  "NmItem.h"
#include  "LtItem.h"

#include  "LtEntryStore.h"


IMPLEMENT_DYNAMIC_CLASS(CNmEdit, CFormViewEx)

BEGIN_EVENT_TABLE(CNmEdit, CFormViewEx)
  EVT_LIST_ITEM_SELECTED(XRCID("NominatedPlayers"), CNmEdit::OnSelChanged)
END_EVENT_TABLE()


// -----------------------------------------------------------------------
// CNmEdit

CNmEdit::CNmEdit() : CFormViewEx()
{
}


CNmEdit::~CNmEdit()
{
}


bool  CNmEdit::Edit(va_list vaList)
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
  
  if (syDoubles)
    nmList->SetItemHeight(1.5);

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

  for (int nmDouble = 1; nmDouble <= syDoubles; nmDouble++)
  {
    if (syDoubles > 1)
      wxSprintf(str, "D%c%d", ax < 0 ? 'A' : 'X', nmDouble);
    else
      wxSprintf(str, "D%c", ax < 0 ? 'A' : 'X');

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
void CNmEdit::OnInitialUpdate() 
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
void  CNmEdit::OnSelChanged(wxListEvent &)
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
void  CNmEdit::OnAdd()
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


void  CNmEdit::OnDelete()
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


void  CNmEdit::OnOK()
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
