/* Copyright (C) 2022 Christoph Theis */
#pragma once

#include  "FormViewEx.h"
#include  "ComboBoxEx.h"
#include  "ListCtrlEx.h"

#include  "CpListStore.h"
#include  "GrListStore.h"

class CMtLeg : public CFormViewEx
{
  public:
    CMtLeg();
   ~CMtLeg();

    virtual bool Edit(va_list);

  private:
    virtual void OnInitialUpdate();

  private:
    void OnSelChangeCp(wxCommandEvent&);
    void OnSelChangeGr(wxCommandEvent&);

private:
    CComboBoxEx* m_cbCp;
    CComboBoxEx* m_cbGr;
    CListCtrlEx* m_listCtrl;

    CpRec  cp;
    GrRec  gr;
    
    DECLARE_DYNAMIC_CLASS(CMtLeg)
    DECLARE_EVENT_TABLE()
};
