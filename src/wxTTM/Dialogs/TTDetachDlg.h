/* Copyright (C) 2020 Christoph Theis */

#ifndef _TTDETACHDLG_H_
#define _TTDETACHDLG_H_


#include  <string>

// -----------------------------------------------------------------------
// CTTDetachDlg dialog

class CTTDetachDlg : public wxDialog
{
  // Construction
  public:
    CTTDetachDlg();
   ~CTTDetachDlg(); 

  public:
    wxString GetTournament()const {return m_tournament;}
    wxString GetUser() const      {return m_user;}
    wxString GetPassword() const  {return m_passwd;}
    bool     GetDeleteDirectory() const {return m_deleteDir;}

  private:
	  void OnInitDialog(wxInitDialogEvent &);

  private:
	  wxString	m_passwd;
	  wxString	m_tournament;
	  wxString	m_user;
    bool      m_deleteDir;

  DECLARE_DYNAMIC_CLASS(CTTDetachDlg)
  DECLARE_EVENT_TABLE()
};

#endif 
