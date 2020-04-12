/* Copyright (C) 2020 Christoph Theis */

#ifndef _GRPRINT_H_
#define _GRPRINT_H_


#include  "GrPrintBase.h"
#include  "ComboBoxEx.h"
#include  "ListCtrlEx.h"
#include  "CpListStore.h"
#include  "PoStore.h"

#include  "Raster.h"


// -----------------------------------------------------------------------
// CGrPrint dialog

class CGrPrint : public CGrPrintBase
{
  // Construction
  public:
	  CGrPrint();   // standard constructor
	 ~CGrPrint(); 

	protected:
	  virtual void OnInitialUpdate();
    virtual void OnEdit();
    virtual void OnPrint();

  protected:
	  void OnSelChangeCp(wxCommandEvent &);

    CComboBoxEx * m_cpCombo;
    wxComboBox  * m_options;

    CpRec    m_cp;
    PoStore  m_po;
    
  protected:
    DECLARE_DYNAMIC_CLASS(CGrPrint)
    DECLARE_EVENT_TABLE()
};


#endif 
