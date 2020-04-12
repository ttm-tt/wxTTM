/* Copyright (C) 2020 Christoph Theis */

#ifndef _SYEDIT_H_
#define _SYEDIT_H_


// -----------------------------------------------------------------------
// CSyEdit form view

#include  "FormViewEx.h"
#include  "SyStore.h"

class CSyEdit : public CFormViewEx
{
  public:
	  CSyEdit();    
	 ~CSyEdit(); 

    virtual bool  Edit(va_list);

  protected:
	  virtual void OnInitialUpdate();
    virtual void OnOK();
    
  private:  
    void OnChangeSyEditMatches(wxFocusEvent &);
    void OnChangeSyEditSinglesDoubles(wxFocusEvent &);

  private:
    wxGrid *  m_gridCtrl;
    SyStore   sy;
    
  DECLARE_DYNAMIC_CLASS(CSyEdit)
  DECLARE_EVENT_TABLE()    
};


#endif 
