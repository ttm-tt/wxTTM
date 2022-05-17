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
  EVT_RADIOBUTTON(XRCID("TypeSingle"), CCpEdit::OnSelectType)
  EVT_RADIOBUTTON(XRCID("TypeDouble"), CCpEdit::OnSelectType)
  EVT_RADIOBUTTON(XRCID("TypeMixed"), CCpEdit::OnSelectType)
  EVT_RADIOBUTTON(XRCID("TypeTeam"), CCpEdit::OnSelectType)
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

  if (cp.cpID)
  {
    m_cbSystem->SetCurrentItem(cp.syID);

    FindWindow("SexMale")->Enable(false);
    FindWindow("SexFemale")->Enable(false);
    FindWindow("SexMixed")->Enable(false);
    
    FindWindow("TypeSingle")->Enable(false);
    FindWindow("TypeDouble")->Enable(false);
    FindWindow("TypeMixed")->Enable(false);
    FindWindow("TypeTeam")->Enable(false);
  }
  else
  {
    OnSelectType(wxCommandEvent_);
  }

  TransferDataToWindow();

  return true;
}


// -----------------------------------------------------------------------
// CCpEdit message handlers

void CCpEdit::OnSelectType(wxCommandEvent &)
{
  TransferDataFromWindow();

  switch (cp.cpType)
  {
    case CP_SINGLE :
      FindWindow("SexMale")->Enable(true);
      FindWindow("SexFemale")->Enable(true);
      FindWindow("SexMixed")->Enable(true);

      break;

    case CP_DOUBLE :
      if (cp.cpSex != SEX_MALE && cp.cpSex != SEX_FEMALE)
        cp.cpSex = SEX_MALE;

      m_cbSystem->SetCurrentItem((long) 0);

      FindWindow("SexMale")->Enable(true);
      FindWindow("SexFemale")->Enable(true);
      FindWindow("SexMixed")->Enable(false);

      break;

    case CP_MIXED :
      cp.cpSex = SEX_MIXED;

      m_cbSystem->SetCurrentItem((long) 0);

      FindWindow("SexMale")->Enable(false);
      FindWindow("SexFemale")->Enable(false);
      FindWindow("SexMixed")->Enable(true);

      break;

    case CP_TEAM :
      m_cbSystem->SetCurrentItem(cp.syID);

      FindWindow("SexMale")->Enable(true);
      FindWindow("SexFemale")->Enable(true);
      FindWindow("SexMixed")->Enable(true);

      break;
  }

  TransferDataToWindow();
}


void CCpEdit::OnInitialUpdate() 
{
	CFormViewEx::OnInitialUpdate();	
	
	FindWindow("Name")->SetValidator(CCharArrayValidator(cp.cpName, sizeof(cp.cpName) / sizeof(wxChar)));
	FindWindow("Description")->SetValidator(CCharArrayValidator(cp.cpDesc, sizeof(cp.cpDesc) / sizeof(wxChar)));
	FindWindow("Category")->SetValidator(CCharArrayValidator(cp.cpCategory, sizeof(cp.cpCategory) / sizeof(wxChar)));

  FindWindow("BestOf")->SetValidator(CShortValidator(&cp.cpBestOf));
	
  m_cbSystem = XRCCTRL(*this, "TeamSystem", CComboBoxEx);	  

  FindWindow("TypeSingle")->SetValidator(CEnumValidator(&cp.cpType, CP_SINGLE));
	FindWindow("TypeDouble")->SetValidator(CEnumValidator(&cp.cpType, CP_DOUBLE));
	FindWindow("TypeMixed")->SetValidator(CEnumValidator(&cp.cpType, CP_MIXED));
	FindWindow("TypeTeam")->SetValidator(CEnumValidator(&cp.cpType, CP_TEAM));
	
	FindWindow("SexMale")->SetValidator(CEnumValidator(&cp.cpSex, SEX_MALE));
	FindWindow("SexFemale")->SetValidator(CEnumValidator(&cp.cpSex, SEX_FEMALE));
	FindWindow("SexMixed")->SetValidator(CEnumValidator(&cp.cpSex, SEX_MIXED));

  FindWindow("Year")->SetValidator(CLongValidator(&cp.cpYear, true));
	
	if (CTT32App::instance()->GetType() == TT_SCI)
	  XRCCTRL(*this, "OrAfter", wxStaticText)->SetLabel(_("or before"));
	else
	  XRCCTRL(*this, "OrAfter", wxStaticText)->SetLabel(_("or after"));

  FindWindow("PtsToWin")->SetValidator(CShortValidator(&cp.cpPtsToWin));
  FindWindow("PtsToWinLast")->SetValidator(CShortValidator(&cp.cpPtsToWinLast));
  FindWindow("PtsAhead")->SetValidator(CShortValidator(&cp.cpPtsAhead));
  FindWindow("PtsAheadLast")->SetValidator(CShortValidator(&cp.cpPtsAheadLast));
}


void  CCpEdit::OnOK()
{
  TransferDataFromWindow();

  if (cp.cpType == CP_TEAM)
  {
    ListItem *itemPtr = m_cbSystem->GetCurrentItem();
    if (itemPtr)
      cp.syID = itemPtr->GetID();
  }

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
