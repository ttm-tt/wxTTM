/* Copyright (C) 2020 Christoph Theis */

// CExpGR form view
#ifndef _GREXPORT_H_
#define _GREXPORT_H_

#include "FormViewEx.h"
#include "ComboBoxEx.h"
#include "ListCtrlEx.h"

#include "CpStore.h"

class CGrExport : public CFormViewEx
{
  public:
	  CGrExport();           // protected constructor used by dynamic creation
	 ~CGrExport();
    
    virtual bool Edit(va_list vaList);

  protected:
    virtual void OnInitialUpdate();

  private:
    void OnBrowse(wxCommandEvent &);
    void OnExport(wxCommandEvent &);
    void OnSelChangeCp(wxCommandEvent &);
  
  protected:
    CpRec     cp;
    wxString  fileName;
    wxString  titleSuffix;
  
    CComboBoxEx  *cbCP;
    CListCtrlEx  *lbGR;

    unsigned (*func)(const wxString &fileName, short cpType, std::vector<long> &, bool); 

  DECLARE_DYNAMIC_CLASS(CGrExport)
  DECLARE_EVENT_TABLE()
};


#endif
