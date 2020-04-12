/* Copyright (C) 2020 Christoph Theis */

// CpEdit.cpp : implementation file
//

#include  "stdafx.h"
#include  "TT32App.h"
#include  "CpEdit.h"
#include  "SyListStore.h"
#include  "SyItem.h"

#include  "Request.h"
#include  "Rec.h"


IMPLEMENT_DYNAMIC_CLASS(CCpEdit, CFormViewEx)

BEGIN_EVENT_TABLE(CCpEdit, CFormViewEx)
  EVT_RADIOBUTTON(XRCID("Single"), CCpEdit::OnSelectType)
  EVT_RADIOBUTTON(XRCID("Double"), CCpEdit::OnSelectType)
  EVT_RADIOBUTTON(XRCID("Mixed"), CCpEdit::OnSelectType)
  EVT_RADIOBUTTON(XRCID("Team"), CCpEdit::OnSelectType)
END_EVENT_TABLE()


// -----------------------------------------------------------------------
// CCpEdit
CCpEdit::CCpEdit() : CFormViewEx()
{
}


CCpEdit::~CCpEdit()
{
}


bool  CCpEdit::Edit(va_list vaList)
{
  long id = va_arg(vaList, long);

  if (id)
  {
    cp.SelectById(id);
    cp.Next();
  }
  else
  {
    cp.cpType = CP_SINGLE;
    cp.cpSex  = SEX_MALE;
  }
    
  m_cbSystem->AddListItem(new SyItem(SyListRec()));
  SyListStore  sy;
  sy.SelectAll();
  while (sy.Next())
    m_cbSystem->AddListItem(new SyItem(sy));

  m_cbSystem->SetCurrentItem(cp.syID);

  m_cbSystem->Enable(cp.cpType == CP_TEAM);

  FindWindow("Male")->Enable(cp.cpType != CP_MIXED);
  FindWindow("Female")->Enable(cp.cpType != CP_MIXED);
  
  if (cp.cpID)
  {
    FindWindow("Male")->Enable(false);
    FindWindow("Female")->Enable(false);
    
    FindWindow("Single")->Enable(false);
    FindWindow("Double")->Enable(false);
    FindWindow("Mixed")->Enable(false);
    FindWindow("Team")->Enable(false);
  }

  TransferDataToWindow();

  return true;
}


// -----------------------------------------------------------------------
// CCpEdit message handlers

void CCpEdit::OnSelectType(wxCommandEvent &)
{
  TransferDataFromWindow();
  
  if (cp.cpType == CP_MIXED)
    cp.cpSex = SEX_MIXED;
  else if (cp.cpSex != SEX_MALE && cp.cpSex != SEX_FEMALE)
    cp.cpSex = SEX_MALE;

  if (cp.cpType != CP_TEAM)
    m_cbSystem->SetCurrentItem((long) 0);
  else
    m_cbSystem->SetCurrentItem(cp.syID);

  m_cbSystem->Enable(cp.cpType == CP_TEAM);
  
  FindWindow("Male")->Enable(cp.cpType != CP_MIXED);
  FindWindow("Female")->Enable(cp.cpType != CP_MIXED);
  
  TransferDataToWindow();
}


void CCpEdit::OnInitialUpdate() 
{
	CFormViewEx::OnInitialUpdate();	
	
	FindWindow("Name")->SetValidator(CCharArrayValidator(cp.cpName, sizeof(cp.cpName) / sizeof(wxChar)));
	FindWindow("Description")->SetValidator(CCharArrayValidator(cp.cpDesc, sizeof(cp.cpDesc) / sizeof(wxChar)));

  FindWindow("BestOf")->SetValidator(CShortValidator(&cp.cpBestOf));
	
  m_cbSystem = XRCCTRL(*this, "TeamSystem", CComboBoxEx);	  

  FindWindow("Single")->SetValidator(CEnumValidator(&cp.cpType, CP_SINGLE));
	FindWindow("Double")->SetValidator(CEnumValidator(&cp.cpType, CP_DOUBLE));
	FindWindow("Mixed")->SetValidator(CEnumValidator(&cp.cpType, CP_MIXED));
	FindWindow("Team")->SetValidator(CEnumValidator(&cp.cpType, CP_TEAM));
	
	FindWindow("Male")->SetValidator(CEnumValidator(&cp.cpSex, SEX_MALE));
	FindWindow("Female")->SetValidator(CEnumValidator(&cp.cpSex, SEX_FEMALE));

  FindWindow("Year")->SetValidator(CLongValidator(&cp.cpYear, true));
	
	if (CTT32App::instance()->GetType() == TT_SCI)
	  XRCCTRL(*this, "OrAfter", wxStaticText)->SetLabel(_("or before"));
	else
	  XRCCTRL(*this, "OrAfter", wxStaticText)->SetLabel(_("or after"));

}


void  CCpEdit::OnOK()
{
  TransferDataFromWindow();

  if (cp.cpType == CP_MIXED)
    cp.cpSex = SEX_MIXED;
  else if (cp.cpSex != SEX_MALE && cp.cpSex != SEX_FEMALE)
    cp.cpSex = SEX_MALE;
  
  if (cp.cpType != CP_TEAM)
    cp.syID = 0;
  else
    cp.syID = m_cbSystem->GetCurrentItem()->GetID();

  bool res = false;

  TTDbse::instance()->GetDefaultConnection()->StartTransaction();

  if (!cp.WasOK())
    res = cp.Insert();
  else
    res = cp.Update();

  if (res)
    TTDbse::instance()->GetDefaultConnection()->Commit();
  else
    TTDbse::instance()->GetDefaultConnection()->Rollback();

  CFormViewEx::OnOK();
}
