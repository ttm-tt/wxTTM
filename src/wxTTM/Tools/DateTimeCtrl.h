/* Copyright (C) 2020 Christoph Theis */

#ifndef _DATETIMECTRL_H_
#define _DATETIMECTRL_H_

class CDateTimeCtrl : public wxTextCtrl
{
  public:
    CDateTimeCtrl() : wxTextCtrl() {}
    
  private:
    void OnKillFocus(wxFocusEvent &);
    
  DECLARE_DYNAMIC_CLASS(CDateTimeCtrl)
  DECLARE_EVENT_TABLE()
};

#endif