/* Copyright (C) 2020 Christoph Theis */

#ifndef _RPEDIT_H_
#define _RPEDIT_H_

// -----------------------------------------------------------------------
// CRpEdit form view

#include  "FormViewEx.h"
#include  "ListCtrlEx.h"
#include  "ItemCtrl.h"

#include  "NaListStore.h"
#include  "CpListStore.h"

class CRpEdit : public CFormViewEx
{
  public:
	  CRpEdit();
	 ~CRpEdit(); 

    virtual bool  Edit(va_list);

  protected:
	  virtual void OnInitialUpdate();
    virtual void OnEdit();
    virtual void OnOK();

  private:
    CListCtrlEx * m_listCtrl;

    long plID;

  DECLARE_DYNAMIC_CLASS(CRpEdit)
  DECLARE_EVENT_TABLE()
};


#endif 
