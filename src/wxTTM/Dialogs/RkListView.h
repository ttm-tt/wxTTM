/* Copyright (C) 2020 Christoph Theis */

#ifndef _RKLISTVIEW_H_
#define _RKLISTVIEW_H_


// -----------------------------------------------------------------------
// CRkListView form view

#include  "FormViewEx.h"
#include  "ComboBoxEx.h"
#include  "ListCtrlEx.h"

#include  "CpListStore.h"


class CRkListView : public CFormViewEx
{
  public:
	  CRkListView(); 
	 ~CRkListView(); 
	  
	  virtual bool Edit(va_list);

  protected:
	  virtual void OnInitialUpdate();
    virtual void OnEdit();

	  virtual void OnUpdate(CRequest *);

  private:
	  void OnSelChangeCp(wxCommandEvent &);
    void OnChangeDECount(wxCommandEvent &);

  private:
    CpListStore  cp;
    CComboBoxEx * m_cbCp;
    CListCtrlEx * m_listCtrl;
    
  DECLARE_DYNAMIC_CLASS(CRkListView)
  DECLARE_EVENT_TABLE()
};


#endif 
