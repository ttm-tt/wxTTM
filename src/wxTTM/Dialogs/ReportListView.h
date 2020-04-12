/* Copyright (C) 2020 Christoph Theis */

#ifndef _REPORTLISTVIEW_H_
#define _REPORTLISTVIEW_H_



// -----------------------------------------------------------------------
// CReportList form view

#include "FormViewEx.h"
#include "ListCtrlEx.h"

class CReportListView : public CFormViewEx
{
  public:
	  CReportListView();
	 ~CReportListView();
	 
	  bool Edit(va_list);

  protected:
	  virtual void OnInitialUpdate();
    virtual void OnOK();
    virtual void OnEdit();
    virtual void OnPrint();

  private:
    CListCtrlEx * m_reportList;
    
  DECLARE_DYNAMIC_CLASS(CReportListView)  
  DECLARE_EVENT_TABLE()
};


#endif 
