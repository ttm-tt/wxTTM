/* Copyright (C) 2020 Christoph Theis */

#ifndef _NALISTVIEW_H_
#define _NALISTVIEW_H_


// -----------------------------------------------------------------------
// CNaListView form view

#include "ListCtrlEx.h"
#include "FormViewEx.h"


class CNaListView : public CFormViewEx
{
  public:
	  CNaListView();           
	 ~CNaListView();
	 
	  virtual bool Edit(va_list vaList); 

  public:
    virtual void SaveSettings();
    virtual void RestoreSettings();

  protected:
	  virtual void OnInitialUpdate();
	  virtual void OnUpdate(CRequest *);

  // Implementation
  protected:
    virtual void OnAdd();
    virtual void OnEdit();
    virtual void OnDelete();

    void OnPlayers(wxCommandEvent &);
    void OnTeams(wxCommandEvent &);

  private:
	  CListCtrlEx * m_listCtrl;
	  
	DECLARE_DYNAMIC_CLASS(CNaListView)
	DECLARE_EVENT_TABLE()
};

#endif 
