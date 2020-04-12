/* Copyright (C) 2020 Christoph Theis */

// UpEdit.cpp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "UpEdit.h"
#include "NaItem.h"
#include "NaListStore.h"
#include "Request.h"
#include "Rec.h"

#include "LtStore.h"


IMPLEMENT_DYNAMIC_CLASS(CUpEdit, CFormViewEx)

// -----------------------------------------------------------------------
// CUpEdit

CUpEdit::CUpEdit() : CFormViewEx()
{
}

CUpEdit::~CUpEdit()
{
}

// -----------------------------------------------------------------------
bool  CUpEdit::Edit(va_list vaList)
{
  long id = va_arg(vaList, long);

  if (id)
  {
    up.SelectById(id);
    up.Next();
  }
  else
  {
    up.psSex = SEX_MALE;
  }

  TransferDataToWindow();

  // Fill ComboBox of nations
  NaListStore  na;
  na.SelectAll();
  int i = sizeof(NaItem);
  while (na.Next())
    m_cbBox->AddListItem(new NaItem(na));
  
  // Set current nation
  m_cbBox->SetCurrentItem(up.naID);
  
  return true;
}



void CUpEdit::OnInitialUpdate() 
{
	CFormViewEx::OnInitialUpdate();	
	
	m_cbBox = XRCCTRL(*this, "Association", CComboBoxEx);
	
	FindWindow("UpNr")->SetValidator(CLongValidator(&up.upNr));
	FindWindow("LastName")->SetValidator(CCharArrayValidator(up.psName.psLast, sizeof(up.psName.psLast) / sizeof(wxChar)));
	FindWindow("FirstName")->SetValidator(CCharArrayValidator(up.psName.psFirst, sizeof(up.psName.psFirst) / sizeof(wxChar)));
	FindWindow("Male")->SetValidator(CEnumValidator(&up.psSex, SEX_MALE));
	FindWindow("Female")->SetValidator(CEnumValidator(&up.psSex, SEX_FEMALE));
  FindWindow("Email")->SetValidator(CCharArrayValidator(up.psEmail, sizeof(up.psEmail) / sizeof(wxChar)));
  FindWindow("Phone")->SetValidator(CCharArrayValidator(up.psPhone, sizeof(up.psPhone) / sizeof(wxChar)));
}


void  CUpEdit::OnOK() 
{
  TransferDataFromWindow();

  ListItem *itemPtr = m_cbBox->GetCurrentItem();
  if (itemPtr)
  {
    wxStrcpy(up.naName, ((NaItem *) itemPtr)->na.naName);
    up.naID = ((NaItem *) itemPtr)->na.naID;
  }
  else
  {
    wxStrcpy(up.naName, "");
    up.naID = 0;
  }

  bool res = false;

  TTDbse::instance()->GetDefaultConnection()->StartTransaction();

  if (!up.WasOK())
    res = up.Insert();
  else
    res = up.Update();

  if (res)
    TTDbse::instance()->GetDefaultConnection()->Commit();
  else
    TTDbse::instance()->GetDefaultConnection()->Rollback();

  CFormViewEx::OnOK();
}


