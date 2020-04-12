/* Copyright (C) 2020 Christoph Theis */

#ifndef _CPLISTVIEW_H_
#define _CPLISTVIEW_H_


// -----------------------------------------------------------------------
// CCpListView form view

#include "ListCtrlEx.h"
#include "FormViewEx.h"

class CCpListView : public CFormViewEx
{
  public:
	  CCpListView();           
	 ~CCpListView(); 
	 
	  virtual bool Edit(va_list);

  public:
    virtual void SaveSettings();
    virtual void RestoreSettings();

  protected:
	  virtual void OnInitialUpdate();
    virtual void OnAdd();
    virtual void OnEdit();
    virtual void OnDelete();

	  virtual void OnUpdate(CRequest *);

  private:
	  void OnEntries(wxCommandEvent &);

  private:
	  CListCtrlEx * m_listCtrl;
	  
	DECLARE_DYNAMIC_CLASS(CCpListView)
	DECLARE_EVENT_TABLE()
};


#endif 
