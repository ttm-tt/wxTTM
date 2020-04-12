/* Copyright (C) 2020 Christoph Theis */

#ifndef _SYLISTVIEW_H_
#define _SYLISTVIEW_H_


// -----------------------------------------------------------------------
// CSyListView form view

#include "ListCtrlEx.h"
#include "FormViewEx.h"

class CSyListView : public CFormViewEx
{
  public:
	  CSyListView();
	 ~CSyListView(); 
	 
	  virtual bool Edit(va_list);

  protected:
	  virtual void OnInitialUpdate();
    virtual void OnAdd();
    virtual void OnEdit();
    virtual void OnDelete();

	  virtual void OnUpdate(CRequest *);

  private:
	  CListCtrlEx * m_listCtrl;
	  
	DECLARE_DYNAMIC_CLASS(CSyListView)
};

#endif 
