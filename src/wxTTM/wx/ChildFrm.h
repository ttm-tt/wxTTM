/* Copyright (C) 2020 Christoph Theis */

#ifndef _CHILDFRM_H_
#define _CHILDFRM_H_

#include "stdafx.h"

class CFormViewEx;

class CChildFrame : public wxMDIChildFrame
{
  public:
    CChildFrame();

  public:
    CFormViewEx * GetFormView();

  private:
    void OnNavigationKey(wxNavigationKeyEvent &);
    void OnUpdateView(wxCommandEvent &);
    void OnSetTitle(wxCommandEvent &);

    wxString m_title;
    
  DECLARE_DYNAMIC_CLASS(CChildFrame)
  DECLARE_EVENT_TABLE()  
};


#endif