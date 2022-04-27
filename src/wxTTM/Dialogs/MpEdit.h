/* Copyright (C) 2022 Christoph Theis */
#pragma once


#include "FormViewEx.h"
#include "MpStore.h"

class CMpEdit : public CFormViewEx
{
  public:
    CMpEdit();
   ~CMpEdit();

    virtual bool Edit(va_list);

  protected:
    virtual void OnInitialUpdate();
    virtual void OnOK();

  private:
    void OnChangeMpEditCount(wxFocusEvent &);

  private:
    void WriteGridCtrl();
    void ReadGridCtrl();

  private:
    wxGrid * m_gridCtrl = nullptr;
    short    m_count = 0;
    MpStore  mp;

  DECLARE_DYNAMIC_CLASS(CMpEdit)
  DECLARE_EVENT_TABLE()
};
