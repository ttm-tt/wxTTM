/* Copyright (C) 2020 Christoph Theis */

#ifndef _STEDIT_H_
#define _STEDIT_H_


// -----------------------------------------------------------------------
// CStEdit form view

#include  "FormViewEx.h"
#include  "ComboBoxEx.h"
#include  "ItemCtrl.h"

#include  "CpStore.h"
#include  "GrStore.h"
#include  "StStore.h"
#include  "XxStore.h"
#include  "TmEntryStore.h"
#include  "ComboBoxEx.h"


class CStEdit : public CFormViewEx
{
  public:
	  CStEdit();    
	 ~CStEdit(); 

    virtual bool  Edit(va_list);

  public:
	  virtual void OnInitialUpdate();

  protected:
    virtual void OnOK();

  private:
    CpStore  cp;
    GrStore  gr;
    StStore  st;
    TmEntryStore  tm;

    CComboBoxEx * m_cbTeam;
    
  DECLARE_DYNAMIC_CLASS(CStEdit)
  DECLARE_EVENT_TABLE()
};

#endif 
