/* Copyright (C) 2023 Christoph Theis */
#pragma once

// TmGroups for view

#include "FormViewEx.h"
#include "ComboBoxEx.h"
#include "ListCtrlEx.h"
#include "MdItem.h"
#include "GrListStore.h"
#include "ItemCtrl.h"


class CMdGroups : public CFormViewEx
{
  public:
    CMdGroups();
    ~CMdGroups();

    virtual bool Edit(va_list);

  private:
    virtual void OnInitialUpdate();

  private:
    void OnSelChangedGroupSystem(wxCommandEvent &);
    void OnPositions(wxCommandEvent &);

  private:
    virtual void  OnEdit();

  private:
    CComboBoxEx * m_cbModus = nullptr;
    CListCtrlEx * m_grList = nullptr;

    DECLARE_DYNAMIC_CLASS(CMdGroups)
    DECLARE_EVENT_TABLE()
};