/* Copyright (C) 2020 Christoph Theis */

#ifndef _UPLISTVIEW_H_
#define _UPLISTVIEW_H_

// -----------------------------------------------------------------------
// CUpListView form view

#include "ListCtrlEx.h"
#include "FormViewEx.h"


class CUpListView : public CFormViewEx
{
  public:
	  CUpListView();
	 ~CUpListView(); 
	 
	  bool Edit(va_list vaList);

  public:
    virtual void SaveSettings();
    virtual void RestoreSettings();

  // Operations
  public:
	  virtual void OnInitialUpdate();

  protected:
	  virtual void OnUpdate(CRequest *);

  // Implementation
  protected:
    virtual void OnAdd();
    virtual void OnEdit();
    virtual void OnDelete();  

  // Event handler
  private:
    void OnListColRightClick(wxListEvent &);
    void OnRestore(wxCommandEvent &);

  private:
    static const char *  headers[];
	  CListCtrlEx *m_listCtrl;
	  
	DECLARE_DYNAMIC_CLASS(CUpListView)
	DECLARE_EVENT_TABLE()
};

// -----------------------------------------------------------------------

#endif 
