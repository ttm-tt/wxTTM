/* Copyright (C) 2020 Christoph Theis */

#ifndef _GREDIT_H_
#define _GREDIT_H_



// -----------------------------------------------------------------------
// CGrEdit form view

#include  "FormViewEx.h"
#include  "ComboBoxEx.h"
#include  "ItemCtrl.h"

#include  "GrStore.h"
#include  "CpStore.h"


class CGrEdit : public CFormViewEx
{
  public:
	  CGrEdit();           
	 ~CGrEdit(); 

    virtual bool  Edit(va_list);

  protected:
	  virtual void OnInitialUpdate();
    virtual void OnOK();
    
  private:  
    virtual void OnTemplate(wxCommandEvent &);

  private:
    void OnSelChangeStage(wxCommandEvent &);
	  void OnSelChangeMd(wxCommandEvent &);
    void OnKillFocusSize(wxFocusEvent &);
    void OnKillFocusStage(wxFocusEvent &);

  private:
    void CalculateNextSortOrder();

  private:
    CItemCtrl *  m_cpItem;     // Spielerdaten Spieler
    wxComboBox *  m_cbStage;   // Qualifikation etc.
    CComboBoxEx * m_cbModus;   // Spielmodi
    CComboBoxEx * m_cbSystem;  // Spielsystem

    GrStore     gr;            // Gruppe
    CpStore     cp;            // Wettbewerb

    short nofEntries;
    
  DECLARE_DYNAMIC_CLASS(CGrEdit)
  DECLARE_EVENT_TABLE()  
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GREDIT_H__CE420989_DFCC_11D4_9D66_000000000000__INCLUDED_)
