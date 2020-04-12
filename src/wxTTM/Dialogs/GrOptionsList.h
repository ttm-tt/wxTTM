/* Copyright (C) 2020 Christoph Theis */

#pragma once

#include "ListCtrlEx.h"
#include "FormViewEx.h"


class CGrOptionsList : public CFormViewEx
{
public:
  CGrOptionsList();
  ~CGrOptionsList();

  bool Edit(va_list vaList);

  // Operations
public:
  virtual void OnInitialUpdate();

public:
  void OnPublish(wxCommandEvent &);

  // Implementation
protected:
  virtual void OnAdd();
  virtual void OnEdit();
  virtual void OnDelete();

  // Event handler
private:

private:
  CListCtrlEx *m_listCtrl;

  DECLARE_DYNAMIC_CLASS(CGrOptionsList)
  DECLARE_EVENT_TABLE()
};


