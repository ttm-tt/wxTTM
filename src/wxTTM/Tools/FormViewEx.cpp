/* Copyright (C) 2020 Christoph Theis */

// FormViewEx.cpp : implementation file
//

#include "stdafx.h"
#include "FormViewEx.h"
#include "ChildFrm.h"
#include "TT32App.h"
#include "Profile.h"
#include "Res.h"
#include "Rec.h"


IMPLEMENT_DYNAMIC_CLASS(CFormViewEx, wxPanel)


BEGIN_EVENT_TABLE(CFormViewEx, wxPanel)
  EVT_INIT_DIALOG(CFormViewEx::OnInitDialog)

  EVT_CHAR_HOOK(CFormViewEx::OnCharHook)
END_EVENT_TABLE()  

// -----------------------------------------------------------------------
// CFormViewEx

CFormViewEx::CFormViewEx(wxWindow * WXUNUSED(parent)) : wxPanel()
{
  cW = GetTextExtent("M").GetWidth();
  
  wxTheApp->Connect(IDC_UPDATEVIEW, wxCommandEventHandler(CFormViewEx::OnCommand), NULL, this);
}


CFormViewEx::~CFormViewEx()
{
  wxTheApp->Disconnect(IDC_UPDATEVIEW, wxCommandEventHandler(CFormViewEx::OnCommand), NULL, this);
}


// -----------------------------------------------------------------------
// Edit Record. Overload
bool  CFormViewEx::Edit(va_list vaList)
{
  return true;
}

// -----------------------------------------------------------------------
// CFormViewEx message handlers

void CFormViewEx::OnCommand(wxCommandEvent &evt) 
{
  // Kein switch, da IDC_ADD etc. keine Compiletime-Konstanten sind
  if (evt.GetId() == wxID_OK)
    OnOK();
  else if (evt.GetId() == wxID_CANCEL)
    OnCancel();
  else if (evt.GetId() == wxID_CLOSE)
    OnCancel();
  else if (evt.GetId() == IDC_ADD)
    OnAdd();
  else if (evt.GetId() == IDC_EDIT)
    OnEdit();
  else if (evt.GetId() == IDC_DELETE)
    OnDelete();
  else if (evt.GetId() == IDC_REFRESH)
    OnRefresh();
  else if (evt.GetId() == wxID_PRINT)
    OnPrint();
  else if (evt.GetEventType() == IDC_UPDATEVIEW)
  {
    OnUpdate( (CRequest *) evt.GetClientData() );
    evt.Skip();
  }
  else
    evt.Skip();
}


// -----------------------------------------------------------------------
void CFormViewEx::OnUpdate(CRequest *)
{
}



// -----------------------------------------------------------------------
void CFormViewEx::OnInitDialog(wxInitDialogEvent &)
{
  wxWindow *button;
  
  if ( (button = FindWindow("Add")) )
  {
    button->SetId(IDC_ADD);
    Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( CFormViewEx::OnCommand ), NULL, this );
  }
    
  if ( (button = FindWindow("Edit")) )
  {
    button->SetId(IDC_EDIT);
    Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( CFormViewEx::OnCommand ), NULL, this );
  }
    
  if ( (button = FindWindow("Delete")) )
  {
    button->SetId(IDC_DELETE);
    Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( CFormViewEx::OnCommand ), NULL, this);
  }
    
  if ( (button = FindWindow("OK")) || (button = FindWindow(wxID_OK)) )
  {
    button->SetId(wxID_OK);
    Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( CFormViewEx::OnCommand ), NULL, this);
  }
    
  if ( (button = FindWindow("Close")) || (button = FindWindow(wxID_CLOSE)) )
  {
    button->SetId(wxID_CLOSE);
    Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( CFormViewEx::OnCommand ), NULL, this);
  }
    
  if ( (button = FindWindow("Cancel")) || (button = FindWindow(wxID_CANCEL)) )
  {
    button->SetId(wxID_CANCEL);
    Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( CFormViewEx::OnCommand ), NULL, this);
  }

  if ( (button = FindWindow("Print")) )
  {
    button->SetId(wxID_PRINT);
    Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( CFormViewEx::OnCommand ), NULL, this);
  }

  GetParent()->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler( CFormViewEx::OnClose ), NULL, this);

  RestoreSize();

  OnInitialUpdate();

  RestoreSettings();
}


// -----------------------------------------------------------------------
void CFormViewEx::OnCharHook(wxKeyEvent &evt)
{
  if (wxDynamicCast(wxTheApp->GetTopWindow(), wxMDIParentFrame)->GetActiveChild() != GetParent())
  {
    evt.Skip();
    return;
  }

  wxWindow *focus = FindFocus();

  switch (evt.GetKeyCode())
  {
    case WXK_ESCAPE :
      if (evt.GetModifiers() == 0)
      {
        wxWindow *focus = FindFocus();

        if (focus)
        {
          bool ret = false;
          wxKeyEvent tmp(evt);

          tmp.SetEventType(wxEVT_KEY_DOWN);
          if (focus->HandleWindowEvent(tmp))
            ret = true;

          tmp.SetEventType(wxEVT_CHAR);
          if (focus->HandleWindowEvent(tmp))
            ret = true;

          if (ret)
            return;
        }

        wxWindow *button = FindWindow(wxID_CANCEL);
        if (button == NULL)
          button = FindWindow(wxID_CLOSE);

        if (button)
        {
          wxCommandEvent cmdEvt(wxEVT_COMMAND_BUTTON_CLICKED, button->GetId());
          cmdEvt.SetEventObject(button);
          button->GetEventHandler()->ProcessEvent(cmdEvt);
        }
      }

      break;

    // Ansonsten geht <Return> nicht mehr auf den Defaultbutton
    // und z.B. MTTime wird nicht geschlossen
    // XXX: Aber wenn man es so macht, funktioniert es anscheinend.
    // Ansonsten wieder auskommentieren.
#if 0
    // Aber wenn man es drin hat oeffnet ALT+RETURN den Dialog zweimal:
    // Einmal direkt, einmal ueber den Edit-Button
    case WXK_RETURN :
      if (evt.GetModifiers() == wxMOD_ALT)
      {
        wxCommandEvent cmdEvt(wxEVT_COMMAND_BUTTON_CLICKED, IDC_EDIT);
        cmdEvt.SetEventObject(NULL);
        GetEventHandler()->ProcessEvent(cmdEvt);
      }

      break;
#endif

    case WXK_F5 :
      if (evt.GetModifiers() == WXK_NONE)
      {
        wxCommandEvent cmdEvt(wxEVT_COMMAND_BUTTON_CLICKED, IDC_REFRESH);
        cmdEvt.SetEventObject(NULL);
        GetEventHandler()->ProcessEvent(cmdEvt);
      }

      break;

    case 'P' :
      if (evt.GetModifiers() & wxMOD_CONTROL)
      {
        wxCommandEvent cmdEvt(wxEVT_COMMAND_BUTTON_CLICKED, wxID_PRINT);
        cmdEvt.SetEventObject(NULL);
        GetEventHandler()->ProcessEvent(cmdEvt);
      }

      break;


    default :
      break;
  }

  evt.Skip();
}


void CFormViewEx::OnClose(wxCloseEvent &evt)
{
  SaveSettings();

  if (GetParent()->GetClassInfo()->IsKindOf(CLASSINFO(wxDialog)))
  {
    GetParent()->Destroy();
  }
  else
  {
    evt.Skip();
  }
}

// -----------------------------------------------------------------------
// Kommandos
void  CFormViewEx::OnOK()
{
  // Close window
  Close();
}


void  CFormViewEx::OnCancel() 
{
  // Close window
  Close();
}


void  CFormViewEx::OnAdd()
{
  return;
}


void  CFormViewEx::OnEdit()
{
  return;
}


void  CFormViewEx::OnDelete()
{
  return;
}


void  CFormViewEx::OnInitialUpdate()
{
}


void CFormViewEx::OnRefresh()
{
}


void CFormViewEx::OnPrint()
{
}


// -----------------------------------------------------------------------
void CFormViewEx::SaveSize()
{
  // Nicht speichern, wenn Fenster maximiert oder minimiert ist
  if (GetTopLevelWindow()->IsMaximized() || GetTopLevelWindow()->IsIconized())
    return;

  wxSize size = GetParent()->GetSize();
  wxPoint pos = GetParent()->GetPosition();

  if (wxPoint(origX, origY) != pos || wxSize(origW, origH) != size)
  {
    wxString tmp = wxString::Format("%d,%dx%d,%d", std::max(0, pos.x), std::max(0, pos.y), size.x, size.y);
    ttProfile.AddString(PRF_GLOBAL_SETTINGS, GetClassInfo()->GetClassName(), tmp);
  }
}


void  CFormViewEx::RestoreSize()
{
  wxString tmp = ttProfile.GetString(PRF_GLOBAL_SETTINGS, GetClassInfo()->GetClassName());
  if (!tmp.IsEmpty())
  {
    int x, y, w, h;
    sscanf(tmp.c_str(), "%d,%dx%d,%d", &x, &y, &w, &h);

    GetParent()->SetSize(w, h);

    GetParent()->SetPosition(wxPoint(std::max(0, x), std::max(0, y)));
  }
  else
    GetParent()->CenterOnParent();

  wxSize size = GetParent()->GetSize();
  wxPoint pos = GetParent()->GetPosition();

  origX = pos.x;
  origY = pos.y;
  origW = size.x;
  origH = size.y;
}


// -----------------------------------------------------------------------
void CFormViewEx::Close()
{
  GetParent()->Close();
}


void CFormViewEx::SaveSettings()
{
  SaveSize();
}


void CFormViewEx::RestoreSettings()
{
  // RestoreSize();
}


wxTopLevelWindow * CFormViewEx::GetTopLevelWindow() const 
{
  wxWindow *parent = GetParent();
  while (parent)
  {
    if (parent->IsKindOf(&wxTopLevelWindow::ms_classInfo))
      return (CChildFrame *) parent;

    parent = parent->GetParent();
  }

  return NULL;

}