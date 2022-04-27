/* Copyright (C) Christoph Theis 2022 */
#pragma once

#include "ListCtrlEx.h"
#include "FormViewEx.h"

class CMpListView : public CFormViewEx
{
  public:
	  CMpListView();
	 ~CMpListView(); 
	 
	  virtual bool Edit(va_list);

  protected:
	  virtual void OnInitialUpdate();
    virtual void OnAdd();
    virtual void OnEdit();
    virtual void OnDelete();

	  virtual void OnUpdate(CRequest *);

  private:
	  CListCtrlEx * m_listCtrl;
	  
	DECLARE_DYNAMIC_CLASS(CMpListView)
};


