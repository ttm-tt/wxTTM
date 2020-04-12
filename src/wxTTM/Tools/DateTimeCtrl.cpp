/* Copyright (C) 2020 Christoph Theis */

#include "stdafx.h"

#include "DateTimeCtrl.h"


IMPLEMENT_DYNAMIC_CLASS(CDateTimeCtrl, wxTextCtrl)

BEGIN_EVENT_TABLE(CDateTimeCtrl, wxTextCtrl)
  EVT_KILL_FOCUS(CDateTimeCtrl::OnKillFocus)
END_EVENT_TABLE()


void CDateTimeCtrl::OnKillFocus(wxFocusEvent &evt)
{
  evt.Skip();
  
  if ( GetValidator() )
  {
    GetValidator()->TransferFromWindow();
    GetValidator()->TransferToWindow();
  }
}
