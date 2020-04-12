/* Copyright (C) 2020 Christoph Theis */

#ifndef _RKEDIT_H_
#define _RKEDIT_H_



// -----------------------------------------------------------------------
// CRkEdit form view

#include  "FormViewEx.h"
#include  "ListCtrlEx.h"
#include  "ItemCtrl.h"

#include  "NaListStore.h"
#include  "CpListStore.h"

class CRkEdit : public CFormViewEx
{
  public:
	  CRkEdit();
	 ~CRkEdit(); 

    virtual bool  Edit(va_list);

  protected:
	  virtual void OnInitialUpdate();
    virtual void OnEdit();
    virtual void OnOK();

  private:
	  void OnKillFocusDe(wxFocusEvent &);
	  void OnKillFocusQu(wxFocusEvent &);
	  void OnDown(wxCommandEvent &);
	  void OnUp(wxCommandEvent &);
	  // void OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);

    void OnOKAssoc();
    void OnOKAll();

  private:
    CItemCtrl *   m_cpItem;     // Wettbewerb
    CItemCtrl *   m_naItem;     // Nation
    CListCtrlEx * m_listCtrl;

    short  m_de;  // Direct entries
    short  m_qu;  // Qualifiers

    NaListStore  na;
    CpListStore  cp;
    
  DECLARE_DYNAMIC_CLASS(CRkEdit)
  DECLARE_EVENT_TABLE()
};


#endif 
