/* Copyright (C) 2020 Christoph Theis */

#ifndef _UPEDIT_H_
#define _UPEDIT_H_

// -----------------------------------------------------------------------
// cuPeDIT form view

#include  "FormViewEx.h"
#include  "ComboBoxEx.h"
#include  "UpStore.h"

class CUpEdit : public CFormViewEx
{
  public:
	  CUpEdit();           
	 ~CUpEdit();

    virtual bool  Edit(va_list);

  public:
	  void OnInitialUpdate();

  protected:
    virtual void OnOK();

  private:
	  CComboBoxEx *m_cbBox;
    UpStore      up;
    
  DECLARE_DYNAMIC_CLASS(CUpEdit)
};

#endif 
