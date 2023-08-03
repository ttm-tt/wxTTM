/* Copyright (C) 2020 Christoph Theis */

#ifndef _TTNEWDLG_H_
#define _TTNEWDLG_H_


#include  <string>

// -----------------------------------------------------------------------
// CTTNewDlg dialog

class CTTNewDlg : public wxDialog
{
  // Construction
  public:
    CTTNewDlg();
   ~CTTNewDlg();
    
  public:
    wxString GetTournament()const {return m_tournament;}
    wxString GetDatabase() const  {return m_database;}
    wxString GetServer() const    {return m_server;}
    wxString GetUser() const      {return m_user;}
    wxString GetPassword() const  {return m_passwd;}
    bool     GetWindowsAuthentication() const {return m_windowsAuth;}
    
    void   SetType(short t)          {m_type = t - 1;}
    short  GetType() const           {return (short) (m_type + 1);}
    
    void   SetTable(short t)         {m_table = t - 1;}
    short  GetTable() const          {return (short) (m_table + 1);}

  private:
	  void OnInitDialog(wxInitDialogEvent &);
	  void OnKillFocus(wxFocusEvent &);

  private:
	  wxString	m_tournament;
	  wxString  m_database;
	  wxString  m_server;
	  wxString	m_user;
	  wxString	m_passwd;
    bool      m_windowsAuth = true;
	  int       m_type = 1;   // "Regular"
	  int       m_table = 0;  // "ITTF"

  DECLARE_DYNAMIC_CLASS(CTTNewDlg)
  DECLARE_EVENT_TABLE()
};

#endif 
