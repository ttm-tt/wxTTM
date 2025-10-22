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
  wxString name, desc;
  int maxNameLen = sizeof(cgs->gr.grName) / sizeof(cgs->gr.grName[0]) - 1;
  int maxDescLen = sizeof(cgs->gr.grDesc) / sizeof(cgs->gr.grDesc[0]) - 1;

  if (wxStrchr(cgs->gr.grName, wxT('%')))
  {
    FindWindow("Numeric")->Enable(false);
    FindWindow("Alpha")->Enable(false);

    if (wxStrstr(cgs->gr.grName, wxT("%c")))
    {
      name = wxString::Format(cgs->gr.grName, 'A' + cgs->start).RemoveLast(maxNameLen);
      desc = wxString::Format(cgs->gr.grDesc, 'A' + cgs->start).RemoveLast(maxDescLen);
    
      XRCCTRL(*this, "Numeric", wxRadioButton)->SetLabel("");
      XRCCTRL(*this, "Alpha", wxRadioButton)->SetLabel(name + " / " + desc);

      FindWindow("Numeric")->Enable(false);
    }
    else
    {
      name = wxString::Format(cgs->gr.grName, cgs->start).RemoveLast(maxNameLen);   
      desc = wxString::Format(cgs->gr.grDesc, cgs->start).RemoveLast(maxDescLen);
    
      XRCCTRL(*this, "Numeric", wxRadioButton)->SetLabel(name + " / " + desc);
      XRCCTRL(*this, "Alpha", wxRadioButton)->SetLabel("");
      FindWindow("Alpha")->Enable(false);
    }
  }
  else
  {
    if (cgs->start + cgs->count > 100)
      name = wxString::Format("%s%03d", cgs->gr.grName, cgs->start);
    else if (cgs->start + cgs->count > 10)
      name = wxString::Format("%s%02d", cgs->gr.grName, cgs->start);
    else
      name = wxString::Format("%s%d", cgs->gr.grName, cgs->start);

    desc = wxString::Format("%s %d", cgs->gr.grDesc, cgs->start);

    name.RemoveLast(maxNameLen);
    desc.RemoveLast(maxDescLen);
    
    XRCCTRL(*this, "Numeric", wxRadioButton)->SetLabel(name + " / " + desc);

    // Nicht auswaehlbar, wenn Alphabet nicht mehr ausreicht
    if (cgs->start + cgs->count <= 26)
    {
      name = wxString::Format("%s%c", cgs->gr.grName, 'A' + cgs->start).RemoveLast(maxNameLen);
      desc = wxString::Format("%s %c", cgs->gr.grDesc, 'A' + cgs->start).RemoveLast(maxDescLen);

      XRCCTRL(*this, "Alpha", wxRadioButton)->SetLabel(name + " / " + desc);
      FindWindow("Alpha")->Enable(true);
    }
    else
    {
      XRCCTRL(*this, "Alpha", wxRadioButton)->SetLabel("");
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
