/* Copyright (C) 2020 Christoph Theis */

#pragma once

#include "GrPrintBase.h"

class CGrFinished : public CGrPrintBase
{
  // Construction
  public:
    CGrFinished();   // standard constructor
    ~CGrFinished();

  protected:
    virtual void OnInitialUpdate();
    virtual void OnEdit();
    virtual void OnPrint();
    virtual void OnRefresh();

  private:
    void OnTypeChange(wxCommandEvent &);

  private:
    DECLARE_DYNAMIC_CLASS(CGrFinished)
    DECLARE_EVENT_TABLE()
};