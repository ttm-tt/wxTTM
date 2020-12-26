/* Copyright (C) 2020 Christoph Theis */

#ifndef _PLLISTVIEW_H_
#define _PLLISTVIEW_H_

// -----------------------------------------------------------------------
// CPlListView form view

#include "ListCtrlEx.h"
#include "FormViewEx.h"


class CPlListView : public CFormViewEx
{
  public:
	  CPlListView();
	 ~CPlListView(); 
	 
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
	  void OnPlEditEvents(wxCommandEvent &);
    void OnNotes(wxCommandEvent &);
    void OnClone(wxCommandEvent &);
    void OnHistory(wxCommandEvent &);
    void OnEventHistory(wxCommandEvent &);
    void OnChildClose(wxCloseEvent &);

  private:
    static const char *  headers[];
	  CListCtrlEx *m_listCtrl;
	  
	DECLARE_DYNAMIC_CLASS(CPlListView)
	DECLARE_EVENT_TABLE()
};

// -----------------------------------------------------------------------

#endif 
