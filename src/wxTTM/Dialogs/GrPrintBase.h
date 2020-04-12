/* Copyright (C) 2020 Christoph Theis */

#pragma once

#include  "FormViewEx.h"
#include  "ComboBoxEx.h"
#include  "ListCtrlEx.h"
#include  "CpListStore.h"
#include  "PoStore.h"

#include  "Raster.h"


// -----------------------------------------------------------------------
// CGrPrint dialog

class CGrPrintBase : public CFormViewEx
{
  // Construction
  public:
	  CGrPrintBase();   // standard constructor
	 ~CGrPrintBase(); 

	protected:
	  virtual void OnInitialUpdate();

  protected:
    void OnOptions(wxCommandEvent &);
    void OnOptionChange(wxCommandEvent &);

  protected:
    struct PrintThreadStruct
    {
      long  nofItems;
      long *idItems;
      CpRec m_cp;
      PrintRasterOptions m_po;
      Connection *connPtr;
      Printer *printer;
      bool  isThread;
      bool  isMultiple;
      bool  isPreview;
      bool  isPdf;
    };

    static unsigned  PrintThread(void *);
    CListCtrlEx * m_grList;
    wxComboBox  * m_options;

    PoStore  m_po;
    
  protected:
    DECLARE_DYNAMIC_CLASS(CGrPrintBase)
    DECLARE_EVENT_TABLE()
};

