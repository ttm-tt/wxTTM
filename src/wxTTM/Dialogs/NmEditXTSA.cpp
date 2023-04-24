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
  
  if (syDoubles)
    nmList->SetItemHeight(1.5);

	const wxChar *xtsa[][4] = 
	{
		{wxT("A-G1"), wxT("A-B1"), wxT("A-G2"), wxT("A-B2")},
		{wxT("X-G1"), wxT("X-B1"), wxT("X-G2"), wxT("X-B2")}
	};

  wxString str;
    
  str = ax ? "X-B\nA-G" : "A-B\nA-G";

  NmItem *itemPtr = new NmItem(str, false);
  itemPtr->nm.team.cpType = CP_DOUBLE;
  itemPtr->nm.nmNr = 1;
  itemPtr->nm.mtID = mt.mtID;
  itemPtr->nm.tmID = tm.tmID;
  itemPtr->SetType(IDC_DELETE);
    
  nmList->AddListItem(itemPtr);

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

  NmEntryStore  nm;
  nm.SelectByMtTm(mt, tm);
  while (nm.Next())
  {
    long idx = (nm.team.cpType == CP_DOUBLE ? nm.nmNr : syDoubles + nm.nmNr);
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
  
  OnSelChanged(wxListEvent_);

  // OK-Button sperren, wenn schon Ergebnisse und Aufstellung existieren
  hasNomination &= (mt.mtResA + mt.mtResX) != 0;
  // GetDlgItem(IDOK)->EnableWindow(hasNomination ? FALSE : TRUE);
  return true;
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
  
  FindWindow("MatchNo")->SetValidator(CLongValidator(&mt.mtNr));
}


// -----------------------------------------------------------------------
void  CNmEditXTSA::OnSelChanged(wxListEvent &)
{
  NmItem *nmItemPtr = (NmItem *) nmList->GetCurrentItem();
  if (!nmItemPtr)
    return;
    
  plList->RemoveAllListItems();

  // 2nd, 4th, 6th and 8th match is a girl, also the 2nd player in the double
  bool wantGirl = 
    (nmList->GetCurrentIndex() > 0 && (nmList->GetCurrentIndex() & 0x1) == 1) || nmList->GetCurrentIndex() == 0 && nmItemPtr->nm.ltA != 0;
  // 3rd, 5th. 7th and 9th match is a boy, also the 1st player in the double
  bool wantBoy  = 
    (nmList->GetCurrentIndex() > 0 && (nmList->GetCurrentIndex() & 0x1) == 0) || nmList->GetCurrentIndex() == 0 && nmItemPtr->nm.ltA == 0;
    
  LtEntryStore lt;
  lt.SelectPlayerByTm(tm);
  while (lt.Next())
  {
    if (wantGirl && lt.psSex != SEX_FEMALE)
      continue;
    if (wantBoy && lt.psSex != SEX_MALE)
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
  
  if (nmItemPtr->nm.ltB || nmItemPtr->nm.team.cpType == CP_SINGLE)
  {
    long idx = nmList->GetCurrentIndex();
    if (idx < nmList->GetCount() - 1)
      nmList->SetSelected(idx+1);
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
