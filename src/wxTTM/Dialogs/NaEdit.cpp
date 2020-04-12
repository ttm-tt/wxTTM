/* Copyright (C) 2020 Christoph Theis */

// NaEdit.cpp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "NaEdit.h"


// -----------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(CNaEdit, CFormViewEx)

BEGIN_EVENT_TABLE(CNaEdit, CFormViewEx)

END_EVENT_TABLE()

// -----------------------------------------------------------------------
// CNaEdit

CNaEdit::CNaEdit() : CFormViewEx()
{
}

CNaEdit::~CNaEdit()
{
}



// -----------------------------------------------------------------------
// 
bool  CNaEdit::Edit(va_list vaList)
{
  long id = va_arg(vaList, long);

  if (id)
  {
    na.SelectById(id);
    na.Next();
  }

  TransferDataToWindow();
  
  return true;
}


void CNaEdit::OnInitialUpdate() 
{
	CFormViewEx::OnInitialUpdate();	
	
	FindWindow("Name")->SetValidator(CCharArrayValidator(na.naName, sizeof(na.naName) / sizeof(wxChar)));
	FindWindow("Description")->SetValidator(CCharArrayValidator(na.naDesc, sizeof(na.naDesc) / sizeof(wxChar)));	
	FindWindow("Region")->SetValidator(CCharArrayValidator(na.naRegion, sizeof(na.naRegion) / sizeof(wxChar)));	

  NaStore na;
  na.SelectAll();
  while (na.Next())
  {
    if (XRCCTRL(*this, "Region", wxComboBox)->FindString(na.naRegion) == wxNOT_FOUND)
      XRCCTRL(*this, "Region", wxComboBox)->AppendString(na.naRegion);
  }
}


// -----------------------------------------------------------------------
// CNaEdit message handlers

void  CNaEdit::OnOK() 
{
  TransferDataFromWindow();

  bool res = false;

  TTDbse::instance()->GetDefaultConnection()->StartTransaction();

  if (!na.WasOK())
    res = na.Insert();
  else
    res = na.Update();

  if (res)
    TTDbse::instance()->GetDefaultConnection()->Commit();
  else
    TTDbse::instance()->GetDefaultConnection()->Rollback();

  CFormViewEx::OnOK();
}


