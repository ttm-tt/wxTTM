/* Copyright (C) 2020 Christoph Theis */

#ifndef _STLISTVIEW_H_
#define _STLISTVIEW_H_

/////////////////////////////////////////////////////////////////////////////
// CStListView form view

#include  "FormViewEx.h"
#include  "ComboBoxEx.h"
#include  "ListCtrlEx.h"

#include  "CpListStore.h"
#include  "GrListStore.h"

class StItem;

class CStListView : public CFormViewEx
{
  public:
	  CStListView();    
	 ~CStListView(); 

    virtual bool Edit(va_list);	

  public:
	  virtual void OnInitialUpdate();

	  virtual void OnUpdate(CRequest *);

  // Implementation
  protected:
    virtual void OnAdd();
    virtual void OnEdit();
    virtual void OnDelete();

	private:
	  void OnSelChangeCp(wxCommandEvent &);
	  void OnSelChangeGr(wxCommandEvent &);
    void OnSelChangeCount(wxCommandEvent &);

    void OnMouseLeftDown(wxMouseEvent &);

    void OnChildClose(wxCloseEvent &);

    void OnSetFocus(wxFocusEvent &);

    void OnCalculate(wxCommandEvent &);

  private:
    void DoDelete(StItem *);

  private:
    CComboBoxEx * m_cbCp;
    CComboBoxEx * m_cbGr;
	  CListCtrlEx * m_listCtrl;

    // Da man aendern darf, kann man auch gleich die Tabellen verwenden
    CpStore  cp;
    GrStore  gr;
    
    long id;
    
  DECLARE_DYNAMIC_CLASS(CStListView)
  DECLARE_EVENT_TABLE()  
};


#endif 
