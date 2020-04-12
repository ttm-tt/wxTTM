/* Copyright (C) 2020 Christoph Theis */

// TTOpenDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "TTOpenDlg.h"

#include "DriverManager.h"

#include "Profile.h"
#include "Res.h"



IMPLEMENT_DYNAMIC_CLASS(CTTOpenDlg, wxDialog)

BEGIN_EVENT_TABLE(CTTOpenDlg, wxDialog)
  EVT_INIT_DIALOG(CTTOpenDlg::OnInitDialog)
END_EVENT_TABLE()

// -----------------------------------------------------------------------
// CTTOpenDlg dialog

CTTOpenDlg::CTTOpenDlg() : wxDialog()
{
}


CTTOpenDlg::~CTTOpenDlg()
{
}


// -----------------------------------------------------------------------
// CTTOpenDlg message handlers

void CTTOpenDlg::OnInitDialog(wxInitDialogEvent &)
{
  wxStaticCast(FindWindow(wxID_OK), wxButton)->SetDefault();

  wxComboBox *comboBoxPtr = XRCCTRL(*this, "Tournament", wxComboBox);
  
  comboBoxPtr->SetValidator(wxGenericValidator(&m_tournament));

  for (wxString strDbse = ttProfile.GetFirstKey(PRF_TURNIER); !strDbse.IsEmpty(); strDbse = ttProfile.GetNextKey())
  {
    comboBoxPtr->AppendString( strDbse );
  }

  if ( !ttProfile.GetString(PRF_OPEN, PRF_LAST).IsEmpty() )
    comboBoxPtr->SetValue(ttProfile.GetString(PRF_OPEN, PRF_LAST));
  else
    comboBoxPtr->Select(0);

  comboBoxPtr->SetFocus();
}
