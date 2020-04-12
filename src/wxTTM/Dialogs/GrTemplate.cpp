/* Copyright (C) 2020 Christoph Theis */

// GrTemplate.cpp : implementation file
//

#include "stdafx.h"
#include "GrTemplate.h"
#include "GrStore.h"
#include "FormViewEx.h"



IMPLEMENT_DYNAMIC_CLASS(CGrTemplate, wxDialog)

BEGIN_EVENT_TABLE(CGrTemplate, wxDialog)
  EVT_INIT_DIALOG(CGrTemplate::OnInitDialog)
END_EVENT_TABLE()


// -----------------------------------------------------------------------
// CGrTemplate dialog
CGrTemplate::CGrTemplate(GrStore::CreateGroupStruct *ptr, wxWindow *parent) : wxDialog()
{
  cgs = ptr;
  
  wxASSERT(cgs);

  wxXmlResource::Get()->LoadDialog(this, parent, "GrTemplate");
  
  SetTitle(_("Group Template"));
}


CGrTemplate::~CGrTemplate()
{
}


void CGrTemplate::OnInitDialog(wxInitDialogEvent &evt)
{
  FindWindow("Start")->SetValidator(CShortValidator(&cgs->start));
  FindWindow("Count")->SetValidator(CShortValidator(&cgs->count));
  FindWindow("Numeric")->SetValidator(CEnumValidator(&cgs->numeric, 1));
  FindWindow("Alpha")->SetValidator(CEnumValidator(&cgs->numeric, 2));
  
  FindWindow("Start")->Connect(wxEVT_KILL_FOCUS, wxFocusEventHandler(CGrTemplate::OnKillFocus), NULL, this);
  FindWindow("Count")->Connect(wxEVT_KILL_FOCUS, wxFocusEventHandler(CGrTemplate::OnKillFocus), NULL, this);
  
  UpdateDialog();
  
  TransferDataToWindow();
}


void CGrTemplate::UpdateDialog()
{
  if (wxStrchr(cgs->gr.grName, wxT('%')))
  {
    FindWindow("Numeric")->Enable(false);
    FindWindow("Alpha")->Enable(false);
    
    wxString str = wxString::Format(cgs->gr.grName, cgs->start);
    
    if (wxStrstr(cgs->gr.grName, wxT("%c")))
    {
      XRCCTRL(*this, "Numeric", wxRadioButton)->SetLabel("");
      XRCCTRL(*this, "Alpha", wxRadioButton)->SetLabel(str);
      FindWindow("Numeric")->Enable(false);
    }
    else
    {
      XRCCTRL(*this, "Numeric", wxRadioButton)->SetLabel(str);
      XRCCTRL(*this, "Alpha", wxRadioButton)->SetLabel("");
      FindWindow("Alpha")->Enable(false);
    }
  }
  else
  {
    wxString str;
    if (wxStrlen(cgs->gr.grDesc) == 0)
      str = wxString::Format("%d", cgs->start);
    else if (cgs->start + cgs->count > 100)
      str = wxString::Format("%s %03d", cgs->gr.grDesc, cgs->start);
    else if (cgs->start + cgs->count > 10)
      str = wxString::Format("%s %02d", cgs->gr.grDesc, cgs->start);
    else
      str = wxString::Format("%s %d", cgs->gr.grDesc, cgs->start);

    str.RemoveLast(63);
    
    XRCCTRL(*this, "Numeric", wxRadioButton)->SetLabel(str);      
    
    str = wxString::Format("%s %c", cgs->gr.grDesc, 'A' - 1 + cgs->start);
    
    XRCCTRL(*this, "Alpha", wxRadioButton)->SetLabel(str);
    
    // Nicht auswaehlbar, wenn Alphabet nicht mehr ausreicht
    if (cgs->start + cgs->count <= 26)
    {
      FindWindow("Alpha")->Enable(true);
    }
    else
    {
      XRCCTRL(*this, "Alpha", wxRadioButton)->SetLabel(cgs->gr.grDesc);
      FindWindow("Alpha")->Enable(false);
    }
  }

  Fit();
}


// -----------------------------------------------------------------------
// CGrTemplate message handlers

void CGrTemplate::OnKillFocus(wxFocusEvent &evt)
{
  evt.Skip();

  TransferDataFromWindow();
  
  UpdateDialog();
}
