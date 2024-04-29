/* Copyright (C) 2020 Christoph Theis */

// NmEditXTS.cpp : implementation file
// XXX: Hier faellt auch OTS2 drunter: 
//      An sich ein einfaches Spielsystem, aber die Einzel sind zwangslaeufig auch im Doppel
// XXX: Kann man OTS2 auf OTS abbilden? Ansonsten muss man beim Spiel nochmal auswaehlen koennen
//

#include "stdafx.h"
#include "TT32App.h"
#include "NmEditXTSA.h"

#include  "CpItem.h"
#include  "GrItem.h"
#include  "TmItem.h"
#include  "NmItem.h"
#include  "LtItem.h"

#include  "LtEntryStore.h"


IMPLEMENT_DYNAMIC_CLASS(CNmEditXTSA, CFormViewEx)

BEGIN_EVENT_TABLE(CNmEditXTSA, CFormViewEx)
  EVT_LIST_ITEM_SELECTED(XRCID("NominatedPlayers"), CNmEditXTSA::OnSelChanged)
END_EVENT_TABLE()


// LtItem with sex
class LtItemXTSA : public LtItem
{
  public:
    LtItemXTSA(bool showNaName = true) : LtItem(showNaName) {};
    LtItemXTSA(const LtEntry &lt, bool showNaName = true) : LtItem(lt, showNaName) {};

  public:
    virtual void DrawItem(wxDC *pDC, wxRect &rect) override;
};

void LtItemXTSA::DrawItem(wxDC* pDC, wxRect& rect)
{
  unsigned cW = pDC->GetTextExtent("M").GetWidth();
  wxRect tmp(rect);
  tmp.width -= 2 * cW;
  LtItem::DrawItem(pDC, tmp);

  tmp.SetLeft(tmp.GetRight());
  tmp.SetRight(rect.GetRight());

  DrawStringCentered(pDC, tmp, pl.psSex == SEX_MALE ? "M" : "F");
}


// -----------------------------------------------------------------------
// CNmEditXTSA

CNmEditXTSA::CNmEditXTSA() : CFormViewEx()
{
}


CNmEditXTSA::~CNmEditXTSA()
{
}


bool  CNmEditXTSA::Edit(va_list vaList)
{
  long mtID = va_arg(vaList, long);
  long tmID = va_arg(vaList, long);
  int  ax   = va_arg(vaList, int) > 0 ? 1 : 0;

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

  // Hidden players n XTSA
  sySingles = 5;
  
  if (syDoubles)
    nmList->SetItemHeight(1.5);

	const wxChar *xtsa[][5] = 
	{
		{wxT("A-G1"), wxT("A-B1"), wxT("A-G2"), wxT("A-B2"), wxT("A-Res")},
		{wxT("X-G1"), wxT("X-B1"), wxT("X-G2"), wxT("X-B2"), wxT("X-Res")}
	};

  wxString str;
    
  for (int nmDouble = 1; nmDouble <= syDoubles; nmDouble++)
  {
    str = ax ? "X-B\nA-G" : "A-B\nA-G";

    NmItem *itemPtr = new NmItem(str, false);
    itemPtr->nm.team.cpType = CP_DOUBLE;
    itemPtr->nm.nmNr = nmDouble;
    itemPtr->nm.mtID = mt.mtID;
    itemPtr->nm.tmID = tm.tmID;
    itemPtr->SetType(IDC_DELETE);
    
    nmList->AddListItem(itemPtr);
  }

  for (int nmSingle = 1; nmSingle <= sySingles; nmSingle++)
  {
    str = xtsa[ax ? 1 : 0][nmSingle - 1];

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

    long idx = (nmEntry.team.cpType == CP_DOUBLE ? nmEntry.nmNr : syDoubles + nmEntry.nmNr);
    NmItem *itemPtr = (NmItem *) nmList->GetListItem(idx-1);
    if (!itemPtr)
      continue;
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
  
  OnSelChanged(wxListEvent_);

  // OK-Button sperren, wenn schon Ergebnisse und Aufstellung existieren
  hasNomination &= (mt.mtResA + mt.mtResX) != 0;
  // GetDlgItem(IDOK)->EnableWindow(hasNomination ? FALSE : TRUE);
  return true;
}


// -----------------------------------------------------------------------
void CNmEditXTSA::ResetPlReplaces()
{
  // 0 is the double, 1-4 the player, 5 the reseerve
  NmItem* itemPtr = (NmItem*)nmList->GetListItem(5);
  bool isBoy = itemPtr && itemPtr->nm.team.pl.psSex == SEX_MALE;
  bool isGirl = itemPtr && itemPtr->nm.team.pl.psSex == SEX_FEMALE;
  char cax = ax == 0 ? 'A' : 'X';

  lblPlReplaces[0] = wxString(_("Not decided"));
  lblPlReplaces[1] = wxString(_("None"));

  if (isBoy)
  {
    lblPlReplaces[2] = wxString::Format(_("%c-Res replaces %c-B1"), cax, cax);
    lblPlReplaces[3] = wxString::Format(_("%c-Res replaces %c-B2"), cax, cax);
  }
  else if (isGirl)
  {
    lblPlReplaces[2] = wxString::Format(_("%c-Res replaces %c-G1"), cax, cax);
    lblPlReplaces[3] = wxString::Format(_("%c-Res replaces %c-G2"), cax, cax);
  }
  else
  {
    // No 5th player
    lblPlReplaces[2] = wxEmptyString;
    lblPlReplaces[3] = wxEmptyString;
  }

  plReplace->Clear();

  for (int idx = 0; idx < 4; ++idx)
  {
    if (lblPlReplaces[idx].IsEmpty())
      break;

    plReplace->AppendString(lblPlReplaces[idx]);
  }

  // Test, ob 6 - 9 gesetzt sind. NmRec ist 0-basiert
  if (nm.GetSingle(4) == 0)                              // No 5th, we'll stay at None
    plReplace->Select(1);
  else if (nm.GetSingle(5) == 0 || nm.GetSingle(6) == 0) // Not decided (players for 5th to 8th not set
    plReplace->Select(0);
  else if (nm.GetSingle(7) == 0 || nm.GetSingle(8) == 0) 
    plReplace->Select(0);
  else if (nm.GetSingle(4) == nm.GetSingle(5))           // 5 replaces Girl 1
    plReplace->Select(2);
  else if (nm.GetSingle(4) == nm.GetSingle(6))           // 5 replaces Girl 2
    plReplace->Select(3);
  else if (nm.GetSingle(4) == nm.GetSingle(7))           // 5 replaces Boy 1
    plReplace->Select(2);
  else if (nm.GetSingle(4) == nm.GetSingle(8))           // 5 replaces Boy 2
    plReplace->Select(3);
  else                                                   // Else not decided
    plReplace->Select(1);

  plReplace->Enable(nm.GetSingle(4) != 0);

}

// -----------------------------------------------------------------------
void CNmEditXTSA::OnInitialUpdate() 
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
void  CNmEditXTSA::OnSelChanged(wxListEvent &)
{
  NmItem *nmItemPtr = (NmItem *) nmList->GetCurrentItem();
  if (!nmItemPtr)
    return;
    
  plList->RemoveAllListItems();

  // 2nd, 4th entry is a girl, also the 2nd player in the double, and the reserve can be any
  bool wantGirl = 
    (nmList->GetCurrentIndex() > 0 && (nmList->GetCurrentIndex() & 0x1) == 1) || 
    (nmList->GetCurrentIndex() == 0 && nmItemPtr->nm.ltA != 0) ||
    (nmList->GetCurrentIndex() == 5)
  ;
  // 3rd, 5th entry is a boy, also the 1st player in the double, and the reserve can be any
  bool wantBoy  = 
    (nmList->GetCurrentIndex() > 0 && (nmList->GetCurrentIndex() & 0x1) == 0) || 
    (nmList->GetCurrentIndex() == 0 && nmItemPtr->nm.ltA == 0) ||
    (nmList->GetCurrentIndex() == 5)
  ;
    
  LtEntryStore lt;
  lt.SelectPlayerByTm(tm);
  while (lt.Next())
  {
    if (!wantGirl && lt.psSex == SEX_FEMALE)
      continue;
    if (!wantBoy && lt.psSex == SEX_MALE)
      continue;

    LtItem *itemPtr = new LtItemXTSA(lt, false);
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

  ResetPlReplaces();
}


// -----------------------------------------------------------------------
void  CNmEditXTSA::OnAdd()
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
      if (nmItem->GetType() != CP_SINGLE)
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
  else
    plList->SetSelected(0);
      
  // Always rebuild the list
  OnSelChanged(wxListEvent_);

  nmList->Refresh();
}


void  CNmEditXTSA::OnDelete()
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

  if (nmItemPtr->nm.team.cpType == CP_DOUBLE)
  {
    if (lt.psSex == SEX_FEMALE)
      ((NmItem *) nmList->GetListItem(3))->RemovePlayer();
    else
      ((NmItem *) nmList->GetListItem(4))->RemovePlayer();
  }
    
  nmList->Refresh();

  LtItem *ltItemPtr = new LtItemXTSA(lt, false);
  ltItemPtr->SetType(IDC_ADD);
  plList->AddListItem(ltItemPtr);
  plList->Refresh();
  
  if ( !nmItemPtr->nm.ltA && !nmItemPtr->nm.ltB &&
       nmList->GetCurrentIndex() < nmList->GetCount() - 1 )
  {
    nmList->SetSelected(nmList->GetCurrentIndex() + 1);
  }
      
  // Always rebuild the list
  OnSelChanged(wxListEvent_);
}


void  CNmEditXTSA::OnOK()
{
  TTDbse::instance()->GetDefaultConnection()->StartTransaction();
  
  bool hasNomination = false;

  for (long idx = 0; idx < nmList->GetCount(); idx++)
  {
    NmItem *nmItemPtr = (NmItem *) nmList->GetListItem(idx);
    wxASSERT(nmItemPtr);

    if (nmItemPtr->nm.team.cpType == CP_SINGLE)
    {
      // 5th single is optional
      bool optional = (nmItemPtr->nm.nmNr == 5);

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
  int choice = plReplace->GetSelection();
  NmItem *itemPtr = (NmItem *) nmList->GetListItem(5); // Index 0 is the mixed double
  bool isBoy = (itemPtr && itemPtr->nm.team.pl.psSex == SEX_MALE);
  bool isGirl = (itemPtr && itemPtr->nm.team.pl.psSex == SEX_FEMALE);

  if (itemPtr && choice == 0)
  {
    nm.SetSingle(5, 0);
    nm.SetSingle(6, 0);
    nm.SetSingle(7, 0);
    nm.SetSingle(8, 0);
  }
  else
  {
    nm.SetSingle(5, nm.GetSingle(0));  // A-G1 / X-G1
    nm.SetSingle(6, nm.GetSingle(1));  // A-B1 / X-B1
    nm.SetSingle(7, nm.GetSingle(2));  // A-G2 / X-G2
    nm.SetSingle(8, nm.GetSingle(3));  // A-B2 / X-B2
  }

  switch (choice)
  {
    case 0:  // Not decided
    {
      nm.SetSingle(5, 0);
      nm.SetSingle(6, 0);
      nm.SetSingle(7, 0);
      nm.SetSingle(8, 0);

      break;
    }

    case 1:  // None
    {
      nm.SetSingle(5, nm.GetSingle(0));  // A-G1 / X-G1
      nm.SetSingle(6, nm.GetSingle(1));  // A-B1 / X-B1
      nm.SetSingle(7, nm.GetSingle(2));  // A-G2 / X-G2
      nm.SetSingle(8, nm.GetSingle(3));  // A-B2 / X-B2

      break;
    }

    case 2:  // Res replaces G1 / B1
    {
      if (isBoy)
        nm.SetSingle(6, nm.GetSingle(4));
      else if (isGirl)
        nm.SetSingle(5, nm.GetSingle(4));

      break;
    }

    case 3:  // Res replaces G2 / B2
    {
      if (isBoy)
        nm.SetSingle(8, nm.GetSingle(4));
      else if (isGirl)
        nm.SetSingle(7, nm.GetSingle(4));

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
