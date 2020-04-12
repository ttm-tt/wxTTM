/* Copyright (C) 2020 Christoph Theis */

#include "stdafx.h"

#include "ChildFrm.h"
#include "FormViewEx.h"

IMPLEMENT_DYNAMIC_CLASS(CChildFrame, wxMDIChildFrame)

BEGIN_EVENT_TABLE(CChildFrame, wxMDIChildFrame)
  EVT_NAVIGATION_KEY(CChildFrame::OnNavigationKey)
END_EVENT_TABLE()  


CChildFrame::CChildFrame() : wxMDIChildFrame()
{
  wxAcceleratorEntry entries[] = 
  {
    // Allgemein
    wxAcceleratorEntry(wxACCEL_CTRL, 'N', wxID_NEW),
    wxAcceleratorEntry(wxACCEL_CTRL, 'O', wxID_OPEN),
    wxAcceleratorEntry(wxACCEL_CTRL, 'S', wxID_SAVEAS),
    // Siehe IDC_REFRESH
    // wxAcceleratorEntry(wxACCEL_CTRL, 'P', wxID_PRINT),
    
    // SQL Ausfuehren (SQLEditor)
    wxAcceleratorEntry(wxACCEL_CTRL, 'E', IDC_EXECUTE),

    // ADD / DELETE kann kein Accelerator sein, sonst funktionieren sie nicht in Textfeldern
    // wxAcceleratorEntry(wxACCEL_NORMAL, WXK_INSERT, IDC_ADD),
    // wxAcceleratorEntry(wxACCEL_NORMAL, WXK_DELETE, IDC_DELETE),
    wxAcceleratorEntry(wxACCEL_ALT, WXK_RETURN, IDC_EDIT),
    // Und auch nciht F5: es funktioniert sonst nicht in Notebook-Ctrls
    // wxAcceleratorEntry(wxACCEL_NORMAL, WXK_F5, IDC_REFRESH),

    // Erste / Letzte / Vorige / Naechste (Runde) (MtListView)
    wxAcceleratorEntry(wxACCEL_CTRL | wxACCEL_ALT, WXK_LEFT, IDC_FIRST),
    wxAcceleratorEntry(wxACCEL_CTRL | wxACCEL_ALT, WXK_RIGHT, IDC_LAST),
    wxAcceleratorEntry(wxACCEL_ALT, WXK_LEFT, IDC_PREV),
    wxAcceleratorEntry(wxACCEL_ALT, WXK_RIGHT, IDC_NEXT),

    // Hoch- / Runterschieden (RkEdit)
    wxAcceleratorEntry(wxACCEL_ALT, WXK_UP, IDC_UP),
    wxAcceleratorEntry(wxACCEL_ALT, WXK_DOWN, IDC_DOWN),
  };
  
  SetAcceleratorTable(wxAcceleratorTable(sizeof(entries) / sizeof(entries[0]), entries));

  Connect(IDC_SET_TITLE, wxCommandEventHandler(CChildFrame::OnSetTitle));
}    


CFormViewEx * CChildFrame::GetFormView()
{
  wxWindow * formView = GetChildren().GetFirst()->GetData();

  return wxDynamicCast(formView, CFormViewEx);
}


void CChildFrame::OnNavigationKey(wxNavigationKeyEvent &evt)
{
  // Wenn wir hier ankommen, hat ein <TAB> das letzte Fenster erreicht.
  // Normalerweise wuerde es weiter zum Parent  gehen und dort ginge der 
  // Fokus an das nächste Fenster. Abbrechen kann man das mit einem Trick:
  // Wenn das Event vom Parent kommt wird ein Event handler nicht wieder 
  // zum parent propagieren.
  if (evt.IsWindowChange() || GetChildren().GetCount() == 0)
    evt.Skip();
  else
  {
    wxNavigationKeyEvent newEvent(evt);
    newEvent.SetEventObject(this);
    if (evt.GetDirection() == wxNavigationKeyEvent::IsForward)
      GetChildren().GetLast()->GetData()->GetEventHandler()->ProcessEvent(newEvent);
    else
      GetChildren().GetFirst()->GetData()->GetEventHandler()->ProcessEvent(newEvent);    
  }
    
  return;
}   


void CChildFrame::OnSetTitle(wxCommandEvent &evt)
{
  if (m_title.IsEmpty())
    m_title = GetTitle();

  if ( evt.GetString().IsEmpty() )
    SetTitle(m_title);
  else
    SetTitle(evt.GetString() + " - " + m_title);
}

