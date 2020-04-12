/* Copyright (C) 2020 Christoph Theis */

#ifndef _PLEDIT_H_
#define _PLEDIT_H_

// -----------------------------------------------------------------------
// CPlEdit form view

#include  "FormViewEx.h"
#include  "ComboBoxEx.h"
#include  "PlStore.h"

class CPlEdit : public CFormViewEx
{
  public:
	  CPlEdit();           
	 ~CPlEdit();

    virtual bool  Edit(va_list);

  public:
	  void OnInitialUpdate();

  protected:
    virtual void OnOK();

  private:
    void OnEditRankingPoints(wxCommandEvent &);

  private:
	  CComboBoxEx *m_cbBox;
    PlStore      pl;
    
  DECLARE_DYNAMIC_CLASS(CPlEdit)
  DECLARE_EVENT_TABLE()
};

#endif 
