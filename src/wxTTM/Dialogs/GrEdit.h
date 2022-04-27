/* Copyright (C) 2020 Christoph Theis */

#pragma once


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
    CItemCtrl *  m_cpItem;       // Player
    wxComboBox *  m_cbStage;     // Qualification etc.
    CComboBoxEx * m_cbModus;     // Group modus
    CComboBoxEx * m_cbSystem;    // Team system

    GrStore     gr;              // Group
    CpStore     cp;              // Event

    short nofEntries;
    
  DECLARE_DYNAMIC_CLASS(CGrEdit)
  DECLARE_EVENT_TABLE()  
};

