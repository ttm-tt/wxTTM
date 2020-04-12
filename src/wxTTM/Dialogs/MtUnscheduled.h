/* Copyright (C) 2020 Christoph Theis */

#ifndef MTUNSCHEDULED_H_
#define MTUNSCHEDULED_H_

#include "FormViewEx.h"
#include "ComboBoxEx.h"
#include "ListCtrlEx.h"

class CMtUnscheduled : public CFormViewEx
{
  public:
	  CMtUnscheduled(wxMDIParentFrame * = NULL);           
	  ~CMtUnscheduled();

  private:
    virtual void OnInitialUpdate();
    virtual void OnEdit();
    virtual void OnUpdate(CRequest *);

  private:
    void OnSelChangeCp(wxCommandEvent &);
    void OnSelChangeGr(wxCommandEvent &);


  private:
    CComboBoxEx *  m_cbCp;
    CComboBoxEx *  m_cbGr;
    CListCtrlEx * m_listCtrl;

	DECLARE_DYNAMIC_CLASS(CMtUnscheduled)
	DECLARE_EVENT_TABLE()
};


#endif