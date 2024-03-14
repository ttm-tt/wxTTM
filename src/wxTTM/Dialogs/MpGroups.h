/* Copyright (C) 2023 Christoph Theis */
#pragma once

// TmGroups for view

#include "FormViewEx.h"
#include "ComboBoxEx.h"
#include "ListCtrlEx.h"
#include "SyItem.h"
#include "GrListStore.h"
#include "ItemCtrl.h"


class CMpGroups : public CFormViewEx
{
  public:
    CMpGroups();
    ~CMpGroups();

    virtual bool Edit(va_list);

  private:
    virtual void OnInitialUpdate();

  private:
    void OnSelChangedMatchPoints(wxCommandEvent &);

  private:
    virtual void  OnEdit();

  private:
    CComboBoxEx * m_cbPoints = nullptr;
    CListCtrlEx * m_grList = nullptr;

    DECLARE_DYNAMIC_CLASS(CMpGroups)
    DECLARE_EVENT_TABLE()
};