/* Copyright (C) 2020 Christoph Theis */

#ifndef _CPEDIT_H_
#define _CPEDIT_H_


// -----------------------------------------------------------------------
// CCpEdit form view

#include  "FormViewEx.h"
#include  "ComboBoxEx.h"
#include  "CpStore.h"

class CCpEdit : public CFormViewEx
{
  public:
	  CCpEdit();   
	 ~CCpEdit();  

    virtual bool  Edit(va_list);

  protected:
	  virtual void OnInitialUpdate();
    virtual void OnOK();

  private:
    void OnSelectType(wxCommandEvent &);

  private:
    CComboBoxEx * m_cbSystem;  // Spielsystem
    CpStore  cp;
    
  DECLARE_DYNAMIC_CLASS(CCpEdit)
  DECLARE_EVENT_TABLE()
};


#endif 
