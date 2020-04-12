/* Copyright (C) 2020 Christoph Theis */

#pragma once

#include "FormViewEx.h"
#include "ListCtrlEx.h"

#include "PlListStore.h"


class CPlHistView : public CFormViewEx
{
  public:
    CPlHistView();
   ~CPlHistView();

    virtual bool Edit(va_list);

  public:
    virtual void SaveSettings();
    virtual void RestoreSettings();
	  virtual void OnInitialUpdate();

  private:
    void OnPlHistEvents(wxCommandEvent &);

  private:
    static const char *  headers[];
	  CListCtrlEx *m_listCtrl;

    long plID;
	  
	DECLARE_DYNAMIC_CLASS(CPlListView)
	DECLARE_EVENT_TABLE()

};
