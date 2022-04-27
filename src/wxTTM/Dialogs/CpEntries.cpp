/* Copyright (C) 2020 Christoph Theis */

// EvEntries.cpp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "CpEntries.h"

#include "TmStore.h"
#include "LtEntryStore.h"
#include "NtListStore.h"
#include "TmEntryStore.h"

#include "LtDouble.h"

#include "CpItem.h"
#include "LtItem.h"
#include "TmItem.h"

#include "Select.h"

#include "Rec.h"

#include "InfoSystem.h"
#include "Request.h"
#include "Res.h"


IMPLEMENT_DYNAMIC_CLASS(CCpEntries, CFormViewEx)

BEGIN_EVENT_TABLE(CCpEntries, CFormViewEx)
  EVT_COMBOBOX(XRCID("Event"), CCpEntries::OnSelChangeCp)
  EVT_BUTTON(XRCID("AddComplete"), CCpEntries::OnAddComplete)
  EVT_BUTTON(XRCID("EditComplete"), CCpEntries::OnEditComplete)
  EVT_BUTTON(XRCID("DeleteComplete"), CCpEntries::OnDeleteComplete)
  EVT_BUTTON(XRCID("AddOpen"), CCpEntries::OnAddOpen)
  EVT_BUTTON(XRCID("EditOpen"), CCpEntries::OnEditOpen)
  EVT_BUTTON(XRCID("DeleteOpen"), CCpEntries::OnDeleteOpen)
END_EVENT_TABLE()


const char * CCpEntries::openHeaders[] = 
{
  wxTRANSLATE("Pl.Nr."),
  wxTRANSLATE("Name"),
  wxTRANSLATE("Assoc."),
  wxTRANSLATE("Sex"),
  wxTRANSLATE("Born in"),
  wxTRANSLATE("Extern ID"),
  wxTRANSLATE("Rk.Pts."),
  wxTRANSLATE("Region"),
  NULL
};


// -----------------------------------------------------------------------
// CCpEntries dialog
CCpEntries::CCpEntries() : CFormViewEx()
{
	m_allEntries = 0;
	m_openEntries = 0;
}


CCpEntries::~CCpEntries()
{
  if (notebook->GetPageCount() == 1)
    notebook->AddPage(openPage, openTitle);
}


bool  CCpEntries::Edit(va_list vaList)
{
  cp.cpID = va_arg(vaList, long);

  // Insert Items
  CpListStore  cpList;
  cpList.SelectAll();

  while (cpList.Next())
  {
    cbCp->AddListItem(new CpItem(cpList));
  }

  ListItem *itemPtr = cbCp->GetListItem(0);
  if (!itemPtr)
    return true;

  cbCp->SetCurrentItem(cp.cpID ? cp.cpID : itemPtr->GetID());

  TransferDataToWindow();
  
  OnSelChangeCp(wxCommandEvent_);

  return true;
}


// -----------------------------------------------------------------------
// CCpEntries message handlers
void  CCpEntries::OnInitialUpdate()
{
	CFormViewEx::OnInitialUpdate();
	
	cbCp = XRCCTRL(*this, "Event", CComboBoxEx);
	completeList = XRCCTRL(*this, "CompleteEntries", CListCtrlEx);	
  openList = XRCCTRL(*this, "OpenEntries", CListCtrlEx);

  notebook = XRCCTRL(*this, "Notebook", wxNotebook);
  openPage = notebook->GetPage(1);
  openTitle = notebook->GetPageText(1);

	FindWindow("NofCompletePairs")->SetValidator(CLongValidator(&m_allEntries));
	FindWindow("NofOpenPairs")->SetValidator(CLongValidator(&m_openEntries));

  CListCtrlEx *openEntries = XRCCTRL(*this, "OpenEntries", CListCtrlEx);

  for (size_t i = 0; openHeaders[i]; i++)
  {
    openEntries->InsertColumn(i, wxGetTranslation(openHeaders[i]), wxLIST_FORMAT_LEFT, i == 1 ? GetClientSize().GetWidth() : wxLIST_AUTOSIZE_USEHEADER);
  }

  for (size_t i = 0; openHeaders[i]; i++)
  {
    if (i != 1)
      openEntries->SetColumnWidth(i, wxLIST_AUTOSIZE_USEHEADER);
  }

  openEntries->ShowColumn(4, CTT32App::instance()->GetType() == TT_YOUTH || CTT32App::instance()->GetType() == TT_SCI);
  openEntries->HideColumn(5);
  openEntries->HideColumn(6);

  openEntries->AllowHideColumn(0);
  openEntries->AllowHideColumn(2);
  openEntries->AllowHideColumn(3);
  openEntries->AllowHideColumn(4);
  openEntries->AllowHideColumn(5);
  openEntries->AllowHideColumn(6);
  openEntries->AllowHideColumn(7);

  openEntries->SetResizeColumn(1);


}


void CCpEntries::OnSelChangeCp(wxCommandEvent &) 
{
  completeList->RemoveAllListItems();
  openList->RemoveAllListItems();

  CpItem *itemPtr = (CpItem *) cbCp->GetCurrentItem();
  
  cp = itemPtr->cp;

  openList->SetItemHeight(1);
  completeList->SetItemHeight(cp.cpType == CP_DOUBLE || cp.cpType == CP_MIXED ? 2 : 1);

  if ( (cp.cpType == CP_DOUBLE) || (cp.cpType == CP_MIXED || cp.cpType == CP_TEAM) )
  {
    if (notebook->GetPageCount() == 1)
      notebook->AddPage(openPage, openTitle);
  }
  else
  {
    if (notebook->GetPageCount() == 2)
      notebook->RemovePage(1);
  }
  
  UpdateCounts();
    
  TransferDataToWindow();
  
  TmEntryStore tmEntry;
  LtEntryStore  ltEntry;
  
  tmEntry.SelectTeamByCp(cp);
  while (tmEntry.Next())
    completeList->AddListItem(new TmItem(tmEntry));

  if (cp.cpType == CP_MIXED)
  {
    ltEntry.SelectOpenEntriesByCp(cp);
    while (ltEntry.Next())
      openList->AddListItem(new LtItem(ltEntry));
  }
  else
  {
    ltEntry.SelectOpenEntriesByCp(cp);
    while (ltEntry.Next())
      openList->AddListItem(new LtItem(ltEntry));
  }

  TransferDataFromWindow();
  
  // Buttons einstellen
  FindWindow("AddComplete")->Enable(cp.cpType == CP_SINGLE || cp.cpType == CP_TEAM);
  FindWindow("EditComplete")->Enable(cp.cpType != CP_SINGLE);
  FindWindow("DeleteComplete")->Enable(cp.cpType == CP_SINGLE || cp.cpType == CP_TEAM);

  if (notebook->GetPageCount() == 2)
  {
    FindWindow("AddOpen")->Enable(cp.cpType != CP_SINGLE);
    FindWindow("EditOpen")->Enable(cp.cpType == CP_DOUBLE || cp.cpType == CP_MIXED);
    FindWindow("DeleteOpen")->Enable(cp.cpType != CP_SINGLE);
  }
}


void CCpEntries::UpdateCounts()
{
  TmEntryStore  tmEntry;
  LtEntryStore  ltEntry;

  int  totalFemale = ltEntry.CountFemaleEntries(cp);
  int  totalMale   = ltEntry.CountMaleEntries(cp);
  int  totalTeams  = tmEntry.CountTeams(cp);

  switch (cp.cpType)
  {
    case CP_SINGLE :
      m_allEntries = totalTeams;
      m_openEntries = 0;
      break;

    case CP_DOUBLE :
      m_allEntries = totalTeams;
      if (cp.cpSex == SEX_MALE)
        m_openEntries = (totalMale / 2) - totalTeams;
      else
        m_openEntries = (totalFemale / 2) - totalTeams;
      break;
        
    case CP_MIXED :
      m_allEntries = totalTeams;
      if (totalFemale < totalMale)
        m_openEntries = totalFemale - totalTeams;
      else
        m_openEntries = totalMale - totalTeams;
      break;

    case CP_TEAM :
      m_allEntries = totalTeams;
      m_openEntries = 0;
      break;

    default :
      m_allEntries = 0;
      m_openEntries = 0;
      break;
  }
}


// -----------------------------------------------------------------------
void CCpEntries::OnUpdate(CRequest *reqPtr) 
{
	if (!reqPtr)
  {
    CFormViewEx::OnUpdate(reqPtr);
    return;
  }
  
  switch (reqPtr->rec)
  {
    case CRequest::TMREC :
    {
      // Teams nur in Paare interessant
      if (reqPtr->type == CRequest::REMOVE)
      {
        completeList->RemoveListItem(reqPtr->id);

        break;
      }

      long id = completeList->GetCurrentItem() ? completeList->GetCurrentItem()->GetID() : 0;

      TmEntryStore tm;
      tm.SelectTeamById(reqPtr->id, cp.cpType);
      if (!tm.Next())
        return;       // Kann eigentlich nicht passieren
        
      tm.Close();
        
      // Richtiger Wettbewerb
      if (tm.cpID != cp.cpID)
        return;
      
      TmItem *itemPtr = (TmItem *) completeList->FindListItem(reqPtr->id);
      if (!itemPtr)
        completeList->InsertListItem(new TmItem(tm));
      else
        itemPtr->entry = tm;

      completeList->SortItems();

      if (id)
        completeList->SetCurrentItem(id);

      break;
    }
    
    case CRequest::LTREC :
    {
      // Nur in open pairs interessant
      if (notebook->GetPageCount() == 1)
        return;
        
      if (reqPtr->type == CRequest::REMOVE)
      {
        openList->RemoveListItem(reqPtr->id);

        break;
      }
      
      // NtEntryStore lesen, weil eigentlich nur interessant ist,
      // ob jetzt eine Meldung erzeugt wurde oder nicht.
      // LtRec bleibt unveraendert, NtRec faelscht eine Meldung
      // fuer LtRec mit UPDATE
      NtListStore  nt;
      LtEntryStore lt;
      
      // Meldungen einlesen
      lt.SelectById(reqPtr->id);
      if (!lt.Next())
        return;
        
      lt.Close();
      
      // Ist es ueberhaupt unser WB?
      if (lt.cpID != cp.cpID)
        return;
        
      // nt einlesen
      nt.SelectByLt(lt);
      nt.Next();  // Fehler ignorieren
      nt.Close();
      
      if (nt.tmID)
        openList->RemoveListItem(reqPtr->id);
      else
      {       
        long id = openList->GetCurrentItem() ? openList->GetCurrentItem()->GetID() : 0;

        LtItem *itemPtr = (LtItem *) openList->FindListItem(reqPtr->id);
        if (itemPtr)
          itemPtr->lt = lt;
        else
          openList->InsertListItem(new LtItem(lt));

        openList->SortItems();

        if (id)
          openList->SetCurrentItem(id);
      }
        
      break;
    }
  } 
  
  UpdateCounts();
  
  TransferDataToWindow();
}


// -----------------------------------------------------------------------
void CCpEntries::OnEdit()
{
  if (notebook->GetSelection() == 1)
    OnEditOpen(wxCommandEvent_);
  else
    OnEditComplete(wxCommandEvent_);
}


void CCpEntries::OnEditComplete(wxCommandEvent &)
{
  CpItem *cpItemPtr = (CpItem *) cbCp->GetCurrentItem();
  if (!cpItemPtr)
    return;

  if (cpItemPtr->cp.cpType == CP_SINGLE)
    return;
    
  if (cpItemPtr->cp.cpType == CP_TEAM)
  {
    ListItem *itemPtr = completeList->GetCurrentItem();
    if (!itemPtr)
      return;
      
    TmStore tm;
    tm.SelectById(itemPtr->GetID());
    tm.Next();
    tm.Close();
    
    if (tm.tmID)
      CTT32App::instance()->OpenView(_("Edit Team"), wxT("TmEdit"), tm.tmID, NULL);
      
    return;
  }

  LtStore  lt;

  ListItem *itemPtr = completeList->GetCurrentItem();
  if (!itemPtr)
    return;

  // Ich brauch hier die ltID
  lt.SelectByTm(itemPtr->GetID());
  lt.Next();
  lt.Close();

  if (lt.ltID)
    CTT32App::instance()->OpenView(_("Edit Double"), wxT("LtDouble"), lt.cpID, lt.plID, NULL);
}


void CCpEntries::OnEditOpen(wxCommandEvent &)
{
  CpItem *cpItemPtr = (CpItem *) cbCp->GetCurrentItem();
  if (!cpItemPtr)
    return;

  if (cpItemPtr->cp.cpType == CP_SINGLE)
    return;
    
  if (cpItemPtr->cp.cpType == CP_TEAM)
    return;

  LtStore  lt;

  ListItem *itemPtr = openList->GetCurrentItem();
  if (!itemPtr)
    return;

  lt.ltID = ((LtItem *) itemPtr)->lt.ltID;
  lt.cpID = ((LtItem *) itemPtr)->lt.cpID;
  lt.plID = ((LtItem *) itemPtr)->lt.plID;

  if (lt.ltID)
    CTT32App::instance()->OpenView(_("Edit Double"), wxT("LtDouble"), lt.cpID, lt.plID, NULL);
}


// -----------------------------------------------------------------------
void CCpEntries::OnAdd()
{
  if (notebook->GetSelection() == 1)
    OnAddOpen(wxCommandEvent_);
  else
    OnAddComplete(wxCommandEvent_);
}

void CCpEntries::OnAddComplete(wxCommandEvent &)
{
  if (cp.cpType == CP_TEAM)
    OnAddTeam();
  else
    OnAddPlayer();
}


void CCpEntries::OnAddOpen(wxCommandEvent &)
{
  OnAddPlayer();
}


void CCpEntries::OnAddTeam()
{
  CTT32App::instance()->OpenView(_("Edit Team"), wxT("TmEdit"), 0, cp.cpID, NULL);
}


void CCpEntries::OnAddPlayer()
{
  CSelect select(_("Select Player"));
  
  PlListStore pl;
  
  pl.SelectForCp(cp);
  while (pl.Next())
  {
    // Es werden nur Spieler selektiert, die auch spielen duerfen
    if (cp.cpType == CP_MIXED)
      select.AddListItem(new PlItemEx(pl));
    else
      select.AddListItem(new PlItem(pl));
  }

  select.SetSortColumn(1);
  
  PlItem *itemPtr = (PlItem *) select.Select();
  
  if (!itemPtr)
    return;
    
  CpStore cpTmp;
  cpTmp.SelectById(cp.cpID);
  cpTmp.Next();
  cpTmp.Close();
  
  TTDbse::instance()->GetDefaultConnection()->StartTransaction();
  
  if (!cpTmp.EnlistPlayer(itemPtr->pl))
  {
    TTDbse::instance()->GetDefaultConnection()->Rollback();
    return;
  }
  
  if (cp.cpType == CP_SINGLE)
  {
    // Im Einzel auch das Team erzeugen
    if (!cpTmp.CreateSingle(itemPtr->pl))
    {
      TTDbse::instance()->GetDefaultConnection()->Rollback();
      return;
    }
  }
  
  // Commit
  TTDbse::instance()->GetDefaultConnection()->Commit();
  
  // Der Spieler wird im Update aufgenommen  
}


// -----------------------------------------------------------------------
void CCpEntries::OnDelete()
{
  if (notebook->GetSelection() == 1)
    OnDeleteOpen(wxCommandEvent_);
  else
    OnDeleteComplete(wxCommandEvent_);
}

void CCpEntries::OnDeleteComplete(wxCommandEvent &)
{
  if (cp.cpType == CP_TEAM)
    OnDeleteTeam();
  else
    OnDeletePlayer();
}


void CCpEntries::OnDeleteOpen(wxCommandEvent &)
{
  OnDeletePlayer();
}


void CCpEntries::OnDeleteTeam()
{
  if (infoSystem.Confirmation(_("Are you sure to delete selected team(s)?")) == false)
    return;
    
  TTDbse::instance()->GetDefaultConnection()->StartTransaction();
  
  bool success = true;

  for (int idx = completeList->GetItemCount(); success && idx--; )
  {
    if (completeList->IsSelected(idx))
    {
      TmItem *itemPtr = (TmItem *) completeList->GetListItem(idx);

      if (!itemPtr)
        continue;
        
      success = success && TmStore().Remove(itemPtr->GetID());
    }
  }
  
  if (success)
    TTDbse::instance()->GetDefaultConnection()->Commit();
  else
    TTDbse::instance()->GetDefaultConnection()->Rollback();  
}


void CCpEntries::OnDeletePlayer()
{
  CListCtrlEx *lbList = notebook->GetSelection() == 0 ? completeList : openList;

  if (lbList->GetSelectedCount() > 0)
  {
    if (!infoSystem.Confirmation(_("Are you sure to delete selected entry?")))
      return;
      
    for (int idx = lbList->GetItemCount(); idx--; )
    {
      if (lbList->IsSelected(idx))
      {
        ListItem *itemPtr = lbList->GetListItem(idx);

        if (!itemPtr)
          continue;
      
        CpStore cpTmp;
        cpTmp.SelectById(cp.cpID);
        cpTmp.Next();
        cpTmp.Close();
        
        PlListStore pl;
        
        if (cp.cpType == CP_SINGLE)
        {
          pl.SelectByNr( ((TmItem *) itemPtr)->entry.team.pl.plNr );
          
          if (!pl.Next())
            return;
            
          pl.Close();
        }
        else
        {
          pl.SelectByNr( ((LtItem *) itemPtr)->pl.plNr );
          if (!pl.Next())
            return;
            
          pl.Close();
        }
          
        TTDbse::instance()->GetDefaultConnection()->StartTransaction();
          
        if (!cpTmp.RemovePlayer(pl))
        {
          TTDbse::instance()->GetDefaultConnection()->Rollback();
          return;
        }
          
        TTDbse::instance()->GetDefaultConnection()->Commit();
        
        // Update der Liste in Update
      }
    }
  }
}

