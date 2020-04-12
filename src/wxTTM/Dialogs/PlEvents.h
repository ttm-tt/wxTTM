/* Copyright (C) 2020 Christoph Theis */

#ifndef _PLEVENTS_H_
#define _PLEVENTS_H_

// -----------------------------------------------------------------------
// CPlEvents form view

#include "FormViewEx.h"
#include "ListCtrlEx.h"
#include "PlItem.h"
#include "ItemCtrl.h"
#include "PlStore.h"

class CPlEvents : public CFormViewEx
{
  public:
	  CPlEvents();           // protected constructor used by dynamic creation
	 ~CPlEvents();
	  
    virtual  bool  Edit(va_list);

  private:
	  virtual void OnInitialUpdate();

  private:
	  void OnSelChangedEnteredEvents(wxListEvent &);
	  void OnSelChangedEnteredGroups(wxListEvent &);

  // Implementation
  protected:
    virtual  void  OnEdit();
    virtual  void  OnAdd();
    virtual  void  OnDelete();

    virtual void OnOK();

  private:
    CItemCtrl    *m_plItem;       // Spielerdaten
    CListCtrlEx  *m_cpAvail;      // Available Events
    CListCtrlEx  *m_cpEntered;    // Entered Events
    CListCtrlEx  *m_grEntered;    // Entered Groups
    CListCtrlEx  *m_mtSchedules;  // Match Schedules

    PlListStore pl;
    
  DECLARE_DYNAMIC_CLASS(CPlEvents)
  DECLARE_EVENT_TABLE()
};

#endif 
