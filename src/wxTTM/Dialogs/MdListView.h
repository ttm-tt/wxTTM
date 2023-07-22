/* Copyright (C) 2020 Christoph Theis */

#ifndef _MDLISTVIEW_H_
#define _MDLISTVIEW_H_


// -----------------------------------------------------------------------
// CMdListView form view

#include "ListCtrlEx.h"
#include "FormViewEx.h"


class CMdListView : public CFormViewEx
{
  public:
	  CMdListView();
	 ~CMdListView(); 
	 
	  bool Edit(va_list);

  protected:
	  virtual void OnInitialUpdate();
    virtual void OnAdd();
    virtual void OnEdit();
    virtual void OnDelete();

		void OnGroups(wxCommandEvent&);

	  virtual void OnUpdate(CRequest *);

  private:
	  CListCtrlEx * m_listCtrl;

	  
	DECLARE_DYNAMIC_CLASS(CMdListView)  
	DECLARE_EVENT_TABLE()
};


#endif 
