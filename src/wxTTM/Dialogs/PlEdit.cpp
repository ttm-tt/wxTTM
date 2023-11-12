/* Copyright (C) 2020 Christoph Theis */

// PlEdit.cpp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "PlEdit.h"
#include "NaItem.h"
#include "NaListStore.h"
#include "Request.h"
#include "Rec.h"

#include "LtStore.h"
#include "CpStore.h"


IMPLEMENT_DYNAMIC_CLASS(CPlEdit, CFormViewEx)

BEGIN_EVENT_TABLE(CPlEdit, CFormViewEx)
  EVT_BUTTON(XRCID("EditRankingPoints"), CPlEdit::OnEditRankingPoints)
END_EVENT_TABLE()

// -----------------------------------------------------------------------
// CPlEdit

CPlEdit::CPlEdit() : CFormViewEx()
{
}

CPlEdit::~CPlEdit()
{
}

// -----------------------------------------------------------------------
bool  CPlEdit::Edit(va_list vaList)
{
  long id = va_arg(vaList, long);

  if (id)
  {
    pl.SelectById(id);
    pl.Next();
  }
  else
  {
    pl.psSex = SEX_MALE;
  }

  TransferDataToWindow();

  // Fill ComboBox of nations
  NaListStore  na;
  na.SelectAll();
  int i = sizeof(NaItem);
  while (na.Next())
    m_cbBox->AddListItem(new NaItem(na));
  
  // Set current nation
  m_cbBox->SetCurrentItem(pl.naID);
  
  // Wenn eine Meldung existiert, kann man Nation und Geschlecht nicht mehr editieren.
  LtStore lt;
  lt.SelectByPl(pl.plID);
  if (lt.Next())
  {
    FindWindow("Male")->Enable(false);
    FindWindow("Female")->Enable(false);
  }
  // Allow to change the nation if not yet set
  if (pl.naID)
  {
    FindWindow("Association")->Enable(false);
  }
  lt.Close();

  bool hasYear = false;
  CpStore cp;
  cp.SelectAll();
  do
  {
    
  } while (cp.Next() && !(hasYear = cp.cpYear > 0));

  cp.Close();

  FindWindow("EditRankingPoints")->Enable(pl.plID && hasYear);

  return true;
}



void CPlEdit::OnInitialUpdate() 
{
	CFormViewEx::OnInitialUpdate();	
	
	m_cbBox = XRCCTRL(*this, "Association", CComboBoxEx);
	m_cbBox = XRCCTRL(*this, "Association", CComboBoxEx);
	
	FindWindow("PlNr")->SetValidator(CLongValidator(&pl.plNr));
	FindWindow("ExtID")->SetValidator(CCharArrayValidator(pl.plExtID, sizeof(pl.plExtID) / sizeof(wxChar)));
	FindWindow("RankPts")->SetValidator(CDoubleValidator(&pl.plRankPts, "%.2lf"));
	FindWindow("LastName")->SetValidator(CCharArrayValidator(pl.psName.psLast, sizeof(pl.psName.psLast) / sizeof(wxChar)));
	FindWindow("FirstName")->SetValidator(CCharArrayValidator(pl.psName.psFirst, sizeof(pl.psName.psFirst) / sizeof(wxChar)));
	FindWindow("Year")->SetValidator(CLongValidator(&pl.psBirthday));
	FindWindow("Male")->SetValidator(CEnumValidator(&pl.psSex, SEX_MALE));
	FindWindow("Female")->SetValidator(CEnumValidator(&pl.psSex, SEX_FEMALE));
}


void  CPlEdit::OnOK() 
{
  TransferDataFromWindow();

  ListItem *itemPtr = m_cbBox->GetCurrentItem();
  if (itemPtr)
  {
    wxStrcpy(pl.naName, ((NaItem *) itemPtr)->na.naName);
    pl.naID = ((NaItem *) itemPtr)->na.naID;
  }
  else
  {
    wxStrcpy(pl.naName, "");
    pl.naID = 0;
  }

  // Saving a deleted player will restore him
  pl.plDeleted = false;

  bool res = false;

  TTDbse::instance()->GetDefaultConnection()->StartTransaction();

  if (!pl.WasOK())
    res = pl.Insert();
  else
    res = pl.Update();

  if (res)
    TTDbse::instance()->GetDefaultConnection()->Commit();
  else
    TTDbse::instance()->GetDefaultConnection()->Rollback();

  CFormViewEx::OnOK();
}


// -----------------------------------------------------------------------
void CPlEdit::OnEditRankingPoints(wxCommandEvent &)
{
  CTT32App::instance()->OpenView(_("Edit Ranking Points"), wxT("RpEdit"), pl.plID);
}