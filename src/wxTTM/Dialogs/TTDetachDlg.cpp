/* Copyright (C) 2020 Christoph Theis */

// TTOpenDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "TTDetachDlg.h"

#include "DriverManager.h"

#include "Profile.h"
#include "Res.h"


IMPLEMENT_DYNAMIC_CLASS(CTTDetachDlg, wxDialog)

BEGIN_EVENT_TABLE(CTTDetachDlg, wxDialog)
  EVT_INIT_DIALOG(CTTDetachDlg::OnInitDialog)
END_EVENT_TABLE()

// -----------------------------------------------------------------------
// CTTDetachDlg dialog
CTTDetachDlg::CTTDetachDlg() : wxDialog()
{
  m_deleteDir = false;
}


CTTDetachDlg::~CTTDetachDlg()
{
}


// -----------------------------------------------------------------------
// CTTDetachDlg message handlers

void CTTDetachDlg::OnInitDialog(wxInitDialogEvent &)
{
  wxComboBox  *comboBoxPtr = XRCCTRL(*this, "Tournament", wxComboBox);
  
  comboBoxPtr->SetValidator(wxGenericValidator(&m_tournament));
  FindWindow("DeleteDirectory")->SetValidator(wxGenericValidator(&m_deleteDir));

  for (wxString strDbse = ttProfile.GetFirstKey(PRF_TURNIER); !strDbse.IsEmpty(); strDbse = ttProfile.GetNextKey())
  {
    comboBoxPtr->AppendString( strDbse );
  }

  comboBoxPtr->Select(0);
}
