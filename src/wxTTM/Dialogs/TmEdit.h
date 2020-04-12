/* Copyright (C) 2020 Christoph Theis */

#ifndef _TMEDIT_H_
#define _TMEDIT_H_


// -----------------------------------------------------------------------
// CTmEdit form view

#include "FormViewEx.h"
#include "ComboBoxEx.h"
#include "ListCtrlEx.h"

#include "CpStore.h"
#include "NaStore.h"
#include "TmStore.h"


class CTmEdit : public CFormViewEx
{
  public:
	  CTmEdit();           
	 ~CTmEdit(); 

    virtual bool  Edit(va_list);
    
  public:
    void OnInitialUpdate();

  private:
	  void OnDown(wxCommandEvent &);
	  void OnUp(wxCommandEvent &);
	  void OnKillFocusName(wxFocusEvent &);
	  void OnSelChangedNa(wxCommandEvent &);
	  void OnShowAll(wxCommandEvent &);

  private:
    virtual void OnOK();
    virtual void OnAdd();
    virtual void OnDelete();

  private:
	  CComboBoxEx * m_cbBox;
    CListCtrlEx * m_plAvail;
    CListCtrlEx * m_plList;

    CpStore  cp;
    NaStore  na;
    TmStore  tm;
    
 DECLARE_DYNAMIC_CLASS(CTmEdit)
 DECLARE_EVENT_TABLE()   
};

#endif 
