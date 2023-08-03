/* Copyright (C) 2020 Christoph Theis */

// TTNewDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "TTNewDlg.h"
#include "TTDbse.h"

#include "DatabaseLogin.h"

#include "DriverManager.h"

#include "FormViewEx.h"

#include <list>
#include <string>



IMPLEMENT_DYNAMIC_CLASS(CTTNewDlg, wxDialog)

BEGIN_EVENT_TABLE(CTTNewDlg, wxDialog)
  EVT_INIT_DIALOG(CTTNewDlg::OnInitDialog)
END_EVENT_TABLE()

// -----------------------------------------------------------------------
// CTTNewDlg dialog
CTTNewDlg::CTTNewDlg() : wxDialog()
{
	m_type = CTT32App::instance()->GetDefaultType() - 1;
	m_table = CTT32App::instance()->GetDefaultTable() - 1;
  m_server = "localhost";
  m_windowsAuth = true;
}


CTTNewDlg::~CTTNewDlg()
{
}


// -----------------------------------------------------------------------
// CTTNewDlg message handlers

void CTTNewDlg::OnInitDialog(wxInitDialogEvent &)
{ 
  XRCCTRL(*this, "Server", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS, wxFocusEventHandler(CTTNewDlg::OnKillFocus), NULL, this);

  FindWindow("Tournament")->SetValidator(wxTextValidator(wxFILTER_NONE, &m_tournament));  
  FindWindow("Server")->SetValidator(wxGenericValidator(&m_server));
  FindWindow("Database")->SetValidator(wxGenericValidator(&m_database));

  FindWindow("Type")->SetValidator(wxGenericValidator(&m_type));
  FindWindow("GroupModus")->SetValidator(wxGenericValidator(&m_table));
  
  // FindWindow("wxID_OK")->Enable(false);
  
  TransferDataToWindow();
}


void CTTNewDlg::OnKillFocus(wxFocusEvent &evt)
{
  evt.Skip();
  
  if (m_server == "(local)")
    m_server = "localhost";

  wxString tmp = m_server;
  
  TransferDataFromWindow();

  if (m_server == "(local)")
    m_server = "localhost";
  
  if (tmp != m_server)
  {
    CDatabaseLogin *dlg = (CDatabaseLogin *)wxXmlResource::Get()->LoadDialog(CTT32App::instance()->GetTopWindow(), "DatabaseLogin");
    dlg->SetServer(m_server);
    dlg->SetDatabase("");

    if (dlg->ShowModal() != wxID_OK)
    {
      delete dlg;
      return;
    }

    wxString connStr = dlg->GetConnectionString();

    m_windowsAuth = dlg->GetWindowsAuthentication();
    if (m_windowsAuth)
    {
      m_user = "";
      m_passwd = "";
    }
    else
    {
      m_user = dlg->GetUser();
      m_passwd = dlg->GetPassword();
    }

    delete dlg;

    XRCCTRL(*this, "Database", wxComboBox)->Clear();
    
    if (!TTDbse::IsLocalAddress(m_server))
    {
      SetCursor(*wxHOURGLASS_CURSOR);

      wxString currentServer = m_server;
      std::list<wxString> dbList = TTDbse::instance()->ListDatabases( currentServer, connStr );
      for (std::list<wxString>::iterator it = dbList.begin();
           it != dbList.end(); it++)
      {
        XRCCTRL(*this, "Database", wxComboBox)->AppendString( (*it) );
      }

      SetCursor(wxNullCursor);
    }
    
    // "Create" Button erstmal wieder freigeben
    XRCCTRL(*this, "wxID_OK", wxButton)->Enable(true);

    if (TTDbse::IsLocalAddress(m_server))
    {
      // Turniereinstellungen zeigen
      FindWindow("Type")->Show(true);
      FindWindow("GroupModus")->Show(true);
      
      XRCCTRL(*this, "Database", wxComboBox)->SetValue("");  
    }
    else 
    {
      // Turniereinstellungen verstecken
      FindWindow("Type")->Show(false);
      FindWindow("GroupModus")->Show(false);

      if (XRCCTRL(*this, "Database", wxComboBox)->GetCount() > 0)
        XRCCTRL(*this, "Database", wxComboBox)->Select(0);
      else
      {
        // "Create" Button sperren
        XRCCTRL(*this, "wxID_OK", wxButton)->Enable(false);
      }
    }
  }
}
