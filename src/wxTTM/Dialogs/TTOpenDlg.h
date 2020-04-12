/* Copyright (C) 2020 Christoph Theis */

#ifndef _TTOPENDLG_H_
#define _TTOPENDLG_H_

#include  <string>

// -----------------------------------------------------------------------
// CTTOpenDlg dialog

class CTTOpenDlg : public wxDialog
{
  // Construction
  public:
    CTTOpenDlg();
   ~CTTOpenDlg();

  public:
    wxString GetTournament()const {return m_tournament;}
    wxString GetUser() const      {return m_user;}
    wxString GetPassword() const  {return m_passwd;}
    
  private:
    void OnInitDialog(wxInitDialogEvent &);

  private:    
	  wxString m_passwd;
	  wxString m_tournament;
	  wxString m_user;

  DECLARE_DYNAMIC_CLASS(CTTOpenDlg)
  DECLARE_EVENT_TABLE()
};

#endif 
