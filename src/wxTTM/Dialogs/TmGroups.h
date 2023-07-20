/* Copyright (C) 2023 Christoph Theis */
#pragma once

// TmGroups for view

#include "FormViewEx.h"
#include "ListCtrlEx.h"
#include "TmItem.h"
#include "TmListStore.h"
#include "ItemCtrl.h"


class CTmGroups : public CFormViewEx
{
  public:
    CTmGroups();
    ~CTmGroups();

    virtual bool Edit(va_list);
  private:
    virtual void OnInitialUpdate();

  private:
    void OnSelChangedEnteredGroups(wxListEvent&);

  private:
    CItemCtrl   * m_tmItem;
    CListCtrlEx * m_tmGroups;
    CListCtrlEx * m_tmSchedules;

    TmEntryStore tm;

    DECLARE_DYNAMIC_CLASS(CTmGroups)
    DECLARE_EVENT_TABLE()
};