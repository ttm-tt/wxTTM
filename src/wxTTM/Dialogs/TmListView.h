/* Copyright (C) 2020 Christoph Theis */

#ifndef _TMLISTVIEW_H_
#define _TMLISTVIEW_H_


// -----------------------------------------------------------------------
// CTmListView form view

#include  "FormViewEx.h"
#include  "ListCtrlEx.h"
#include  "ComboBoxEx.h"

#include  "CpListStore.h"

class CTmListView : public CFormViewEx
{
  public:
	  CTmListView();           
	 ~CTmListView();
	 
	 virtual bool Edit(va_list vaList);

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

  private:
    CpListStore   cp;
    CComboBoxEx * m_cbCp;
	  CListCtrlEx * m_listCtrl;
	  
	DECLARE_DYNAMIC_CLASS(CTmListView)
	DECLARE_EVENT_TABLE()
};


#endif 
