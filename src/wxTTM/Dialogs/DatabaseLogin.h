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
    void SetServerEditable(bool e) {serverEditable = e;}
    void SetWindowsAuthentication(bool a) {windowsAuth = a; authPreset = true;}
    void SetUser(const wxString &u) {user = u;}

    const wxString & GetServer() const {return server;}

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
    bool     authPreset = false;
    bool     serverEditable = false;
    wxString user;
    wxString pwd;

    DECLARE_DYNAMIC_CLASS(CDatabaseLogin)
    DECLARE_EVENT_TABLE()
};
