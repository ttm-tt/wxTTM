/* Copyright (C) 2020 Christoph Theis */

#pragma once

// -----------------------------------------------------------------------
// CPlEvents form view

#include "FormViewEx.h"
#include "ComboBoxEx.h"
#include "ListCtrlEx.h"
#include "PlItem.h"
#include "ItemCtrl.h"
#include "PlStore.h"

class CPlHistEvents : public CFormViewEx
{
  public:
	  CPlHistEvents();           // protected constructor used by dynamic creation
	 ~CPlHistEvents();
	  
    virtual  bool  Edit(va_list);

  private:
	  virtual void OnInitialUpdate();
    virtual void OnEdit();
    virtual void OnRefresh();

  private:
    void OnSelChangedCp(wxCommandEvent &);
	  void OnSelChangedEventTimestamps(wxListEvent &);
	  void OnSelChangedEnteredEvents(wxListEvent &);
	  void OnSelChangedGroupTimestamps(wxListEvent &);

  private:
    CItemCtrl    *m_plItem;       // Spielerdaten
    CComboBoxEx  *m_cbEvent;      // Selected event
    CListCtrlEx  *m_ltHistory;    // Event history
    CListCtrlEx  *m_cpEntered;    // Entered Events
    CListCtrlEx  *m_stHistory;    // Group history
    CListCtrlEx  *m_grEntered;    // Entered Groups

    PlListStore pl;
    
  DECLARE_DYNAMIC_CLASS(CPlHistEvents)
  DECLARE_EVENT_TABLE()
};

