/* Copyright (C) 2020 Christoph Theis */

#include "stdafx.h"

#include "DatabaseLogin.h"


IMPLEMENT_DYNAMIC_CLASS(CDatabaseLogin, wxDialog)

BEGIN_EVENT_TABLE(CDatabaseLogin, wxDialog)
  EVT_INIT_DIALOG(CDatabaseLogin::OnInitDialog)
  EVT_CHECKBOX(XRCID("WindowsAuthentication"), CDatabaseLogin::OnCheckbox)
END_EVENT_TABLE()


// -----------------------------------------------------------------------
CDatabaseLogin::CDatabaseLogin() : wxDialog()
{
  windowsAuth = true;
}


CDatabaseLogin::~CDatabaseLogin()
{

}


// -----------------------------------------------------------------------
void CDatabaseLogin::OnInitDialog(wxInitDialogEvent &)
{
  XRCCTRL(*this, "Server", wxTextCtrl)->SetValue(server);
  XRCCTRL(*this, "Database", wxTextCtrl)->SetValue(database);

  FindWindow("WindowsAuthentication")->SetValidator(wxGenericValidator(&windowsAuth));
  FindWindow("Username")->SetValidator(wxGenericValidator(&user));
  FindWindow("Password")->SetValidator(wxGenericValidator(&pwd));

  FindWindow("Username")->Enable(false);
  FindWindow("Password")->Enable(false);

  TransferDataToWindow();
}


void CDatabaseLogin::OnCheckbox(wxCommandEvent &)
{
  TransferDataFromWindow();
  FindWindow("Username")->Enable(!windowsAuth);
  FindWindow("Password")->Enable(!windowsAuth);
}


// -----------------------------------------------------------------------
void CDatabaseLogin::SetServer(const wxString &s) {
  server = s;
}


void CDatabaseLogin::SetDatabase(const wxString &d)
{
  database = d;
}


wxString CDatabaseLogin::GetConnectionString() const
{
  wxString connStr;

  if (windowsAuth)
  {
    connStr = wxString::Format(
        "DRIVER=SQL Server;SERVER=%s;Trusted_Connection=Yes;", 
        server.wx_str()
    );
  }
  else if (!user.IsEmpty() && !pwd.IsEmpty())
  {
    connStr = wxString::Format(
        "DRIVER=SQL Server;SERVER=%s;Trusted_Connection=No;UID=%s;PWD=%s;", 
        server.wx_str(), user.wx_str(), pwd.wx_str()
    );
  }
  else
  {
    return wxEmptyString;
  }

  if (!database.IsEmpty())
    connStr += "DATABASE=" + database;

  return connStr;
}
