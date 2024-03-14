/* Copyright (C) 2023 Christoph Theis */
#pragma once

// TmGroups for view

#include "FormViewEx.h"
#include "ComboBoxEx.h"
#include "ListCtrlEx.h"
#include "SyItem.h"
#include "GrListStore.h"
#include "ItemCtrl.h"


class CSyGroups : public CFormViewEx
{
  public:
    CSyGroups();
    ~CSyGroups();

    virtual bool Edit(va_list);

  private:
    virtual void OnInitialUpdate();

  private:
    void OnSelChangedGroupSystem(wxCommandEvent &);

  private:
    virtual void  OnEdit();

  private:
    CComboBoxEx * m_cbSystem = nullptr;
    CListCtrlEx * m_grList = nullptr;

    DECLARE_DYNAMIC_CLASS(CSyGroups)
    DECLARE_EVENT_TABLE()
};