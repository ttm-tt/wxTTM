/* Copyright (C) 2020 Christoph Theis */

// TmEdit.cpp : implementation file

#include "stdafx.h"
#include "TT32App.h"
#include "TmEdit.h"

#include "NaItem.h"
#include "LtItem.h"

#include "NaListStore.h"
#include "LtEntryStore.h"
#include "NtEntryStore.h"

#include "RkStore.h"
#include "TmStore.h"

#include "InfoSystem.h"


// Eine Klasse, die sich auch ntNr merkt
class NtItem : public LtItem
{
  public:
    NtItem(const LtEntry &lt)
      : LtItem(lt) {ntNr = 0;}
      
    NtItem(const NtEntry &nt)
      : LtItem(nt) {ntNr = nt.nt.ntNr;}
      
    short ntNr;
};

// =======================================================================
// CTmEdit

IMPLEMENT_DYNAMIC_CLASS(CTmEdit, CFormViewEx)

BEGIN_EVENT_TABLE(CTmEdit, CFormViewEx)
  EVT_CHECKBOX(XRCID("ShowAll"), CTmEdit::OnShowAll)
  EVT_BUTTON(XRCID("Up"), CTmEdit::OnUp)
  EVT_BUTTON(XRCID("Down"), CTmEdit::OnDown)
  EVT_COMBOBOX(XRCID("Association"), CTmEdit::OnSelChangedNa)
END_EVENT_TABLE()


// -----------------------------------------------------------------------
CTmEdit::CTmEdit() : CFormViewEx(), m_cbBox(0), m_plAvail(0), m_plList(0)
{
}

CTmEdit::~CTmEdit()
{
}


// -----------------------------------------------------------------------
// CTmEdit message handlers

bool  CTmEdit::Edit(va_list vaList)
{
  long tmID = va_arg(vaList, long);
  long cpID = va_arg(vaList, long);
  
  if (!tm.SelectById(tmID) || !tm.Next())
  {
    cp.SelectById(cpID);
    cp.Next();
    
    wxString naName = CTT32App::instance()->GetDefaultNA();

    na.SelectByName(naName);
    if (!na.Next())
    {
      na.SelectAll();
      na.Next();
    }
    
    // Statement wieder freigeben
    na.Close();
  }
  else
  {
    RkStore rk;

    cp.SelectById(tm.cpID);
    cp.Next();

    rk.SelectByTm(tm);
    rk.Next();

    na.SelectById(rk.naID);
    na.Next();

    // m_cbBox->EnableWindow(FALSE);
  }

  // Fill ComboBox of nations
  NaListStore  naList;
  naList.SelectAll();
  int i = sizeof(NaItem);
  while (naList.Next())
    m_cbBox->AddListItem(new NaItem(naList));
  
  // Set current nation
  m_cbBox->SetCurrentItem(na.naID);
  
  TransferDataToWindow();

  OnSelChangedNa(wxCommandEvent());  

  // Und jetzt noch die Spieler
  NtEntryStore  ntpl;
  ntpl.SelectByTm(tm);
  while (ntpl.Next())
  {
    NtItem *itemPtr = new NtItem(ntpl);
    itemPtr->SetType(IDC_DELETE);
    m_plList->AddListItem(itemPtr);
  }

  return true;
}

// -----------------------------------------------------------------------
void CTmEdit::OnInitialUpdate() 
{
	CFormViewEx::OnInitialUpdate();	
		
  m_cbBox = XRCCTRL(*this, "Association", CComboBoxEx);
  m_plAvail = XRCCTRL(*this, "AvailablePlayers", CListCtrlEx);
  m_plList = XRCCTRL(*this, "EnteredPlayers", CListCtrlEx);
  
  XRCCTRL(*this, "Name", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS, wxFocusEventHandler(CTmEdit::OnKillFocusName), NULL, this);
 
  FindWindow("Name")->SetValidator(CCharArrayValidator(tm.tmName, sizeof(tm.tmName) / sizeof(wxChar)));
  FindWindow("Description")->SetValidator(CCharArrayValidator(tm.tmDesc, sizeof(tm.tmDesc) / sizeof(wxChar)));
}  


void  CTmEdit::OnOK() 
{
  TransferDataFromWindow();

  TTDbse::instance()->GetDefaultConnection()->StartTransaction();

  ListItem *itemPtr = m_cbBox->GetCurrentItem();
  if (itemPtr)
  {
    na.SelectById(itemPtr->GetID());
    na.Next();
  }
  else
    na.Init();
    
  // Test, ob tm eindeutig ist
  TmStore tm2;
  tm2.SelectByCpTmName(cp, tm.tmName);
  if (tm2.Next() && tm2.tmID != tm.tmID)
  {
    tm2.Close();
    infoSystem.Error(_("A team with this name already exists for this event"));
    return;
  }

  if (!tm.WasOK())
  {
    if (!cp.CreateTeam(tm, na, 0))
    {
      TTDbse::instance()->GetDefaultConnection()->Rollback();
      return;
    }
  }
  else
  {
    if (!tm.Update())
    {
      TTDbse::instance()->GetDefaultConnection()->Rollback();
      return;
    }
  }

  // Update der Spielerliste.
  // Die Nominierung verweist auf ltId, darum kann man 
  // hier die Eintraege komplett neu aufsetzen. 
  bool res = tm.RemoveAllEntries();

  for (int idx = 0; idx < m_plList->GetCount(); idx++)
  {
    LtItem *itemPtr = (LtItem *) m_plList->GetListItem(idx);

    if ( !(res = tm.AddEntry(itemPtr->lt, idx+1)) )
      break;
  }

  if (res)
    TTDbse::instance()->GetDefaultConnection()->Commit();
  else
    TTDbse::instance()->GetDefaultConnection()->Rollback();
    
  CTT32App::instance()->SetDefaultNA(na.naName);

  CFormViewEx::OnOK();
}


// -----------------------------------------------------------------------
void  CTmEdit::OnAdd()
{
  for (int idx = m_plAvail->GetItemCount(); idx--; )
  {
    if (!m_plAvail->IsSelected(idx))
      continue;
      
    ListItem * itemPtr = m_plAvail->CutListItem(idx);
    if (itemPtr)
    {
      itemPtr->SetType(IDC_DELETE);
      m_plList->AddListItem(itemPtr);
    }
  }
}


// -----------------------------------------------------------------------
void  CTmEdit::OnDelete()
{
  for (int idx = m_plList->GetItemCount(); idx--; )
  {
    if (!m_plList->IsSelected(idx))
      continue;
      
    ListItem *itemPtr = m_plList->CutListItem(idx);
    if (itemPtr)
    {
      itemPtr->SetType(IDC_ADD);
      m_plAvail->AddListItem(itemPtr);
    }
  }
}


void CTmEdit::OnDown(wxCommandEvent &) 
{
  long idx = m_plList->GetCurrentIndex();
  if (idx == -1 || idx >= m_plList->GetCount() - 1)
    return;
    
  NtItem *itemPtr = (NtItem *) m_plList->CutCurrentItem();
  if (itemPtr == 0)
    return;
    
  itemPtr->ntNr++;
  m_plList->AddListItem(itemPtr, idx + 1);
  itemPtr = (NtItem *) m_plList->GetListItem(idx);
  if (itemPtr != 0)
    itemPtr->ntNr--;
    
  m_plList->SetCurrentIndex(idx+1);

  m_plList->Refresh();
}


void CTmEdit::OnUp(wxCommandEvent &) 
{
  long idx = m_plList->GetCurrentIndex();
  if (idx < 1)
    return;
    
  NtItem *itemPtr = (NtItem *) m_plList->CutCurrentItem();
  if (itemPtr == 0)
    return;
    
  itemPtr->ntNr--;
  m_plList->AddListItem(itemPtr, idx-1);
  itemPtr = (NtItem *) m_plList->GetListItem(idx);
  if (itemPtr != 0)
    itemPtr->ntNr++;
    
  m_plList->SetCurrentIndex(idx-1);
}

void CTmEdit::OnKillFocusName(wxFocusEvent &evt)
{
  TransferDataFromWindow();
  
  if (!tm.WasOK() && *tm.tmName)
  {
    NaItem *naItemPtr = (NaItem *) m_cbBox->FindListItem(tm.tmName);
    if (naItemPtr != NULL)
    {
      wxStrcpy(tm.tmDesc, naItemPtr->na.naDesc);
      m_cbBox->SetCurrentItem(naItemPtr);
      
      OnSelChangedNa(wxCommandEvent());
      
      TransferDataToWindow();
    }
  }

  evt.Skip();
}


void CTmEdit::OnSelChangedNa(wxCommandEvent &)
{
  m_plList->RemoveAllListItems();
  m_plAvail->RemoveAllListItems();
  
  if (m_cbBox->GetCurrentItem() != NULL)
    na = ((NaItem *) m_cbBox->GetCurrentItem())->na;
  
  LtEntryStore  lt;
  lt.SelectOpenEntriesByCpNa(cp, na);

  while (lt.Next())
  {
    if (!m_plList->FindListItem(lt.ltID))
    {
      NtItem *itemPtr = new NtItem(lt);
      itemPtr->SetType(IDC_ADD);
      m_plAvail->InsertListItem(itemPtr);
    }
  }
  
  if ( !tm.WasOK() && wxStrncmp(tm.tmName, na.naName, wxStrlen(na.naName)) )
  {
    wxStrcpy(tm.tmName, na.naName);
    wxStrcpy(tm.tmDesc, na.naDesc);
    
    TransferDataToWindow();
  }
  
  OnShowAll(wxCommandEvent());
}


void CTmEdit::OnShowAll(wxCommandEvent &)
{
  if ( XRCCTRL(*this, "ShowAll", wxCheckBox)->GetValue() )
  {
    LtEntryStore lt;
    lt.SelectOpenEntriesByCpNa(cp, NaRec());
    while (lt.Next())
    {
      if (lt.naID != na.naID)
      {
        NtItem *itemPtr = new NtItem(lt);
        itemPtr->SetType(IDC_ADD);
        m_plAvail->InsertListItem(itemPtr);
      }
    }
  }
  else 
  {
    for (int idx = m_plAvail->GetCount(); idx--; )
    {
      if ( ((NtItem *) m_plAvail->GetListItem(idx))->pl.naID != na.naID)
        delete m_plAvail->CutListItem(idx);
    }
  }
}


