/* Copyright (C) 2020 Christoph Theis */

#pragma once

class CDatabaseLogin : public wxDialog
{
  public:
    CDatabaseLogin();
   ~CDatabaseLogin();

  public:
    void SetServer(const wxString &s);
    void SetDatabase(const wxString &d);

    bool GetWindowsAuthentication() const {return windowsAuth;}
    const wxString & GetUser() const {return user;}
    const wxString & GetPassword() const {return pwd;}

    wxString GetConnectionString() const;

  private:
    void OnInitDialog(wxInitDialogEvent &);
    void OnCheckbox(wxCommandEvent &);

  private:
    wxString server;
    wxString database;
    bool     windowsAuth;
    wxString user;
    wxString pwd;

    DECLARE_DYNAMIC_CLASS(CDatabaseLogin)
    DECLARE_EVENT_TABLE()
};
