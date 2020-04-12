/* Copyright (C) 2020 Christoph Theis */

#ifndef _NAEDIT_H_
#define _NAEDIT_H_


// -----------------------------------------------------------------------
// CNaEdit form view

#include  "FormViewEx.h"

#include  "NaStore.h"


class CNaEdit : public CFormViewEx
{
  public:
	  CNaEdit();           
	 ~CNaEdit();
	  virtual bool Edit(va_list vaList); 

  protected:
	  virtual void OnInitialUpdate();

  protected:
    virtual void OnOK();

  private:
    NaStore  na;
    
  DECLARE_DYNAMIC_CLASS(CNaEdit)
  DECLARE_EVENT_TABLE()
};

#endif 
