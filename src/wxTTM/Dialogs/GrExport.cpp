/* Copyright (C) 2020 Christoph Theis */

// ExpGR.cpp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "GrExport.h"

#include "CpListStore.h"
#include "GrListStore.h"

#include "StStore.h"
#include "MtStore.h"

#include "CpItem.h"
#include "GrItem.h"

#include <wx/progdlg.h>

#include <fstream>

// CGrExport

IMPLEMENT_DYNAMIC_CLASS(CGrExport, CFormViewEx)

BEGIN_EVENT_TABLE(CGrExport, CFormViewEx)
  EVT_COMBOBOX(XRCID("Events"), CGrExport::OnSelChangeCp)
  EVT_BUTTON(XRCID("Browse"), CGrExport::OnBrowse)
  EVT_BUTTON(XRCID("Export"), CGrExport::OnExport)
END_EVENT_TABLE()


// -----------------------------------------------------------------------
CGrExport::CGrExport() : CFormViewEx()
{
  func = 0;
}

CGrExport::~CGrExport()
{
}


// -----------------------------------------------------------------------
bool CGrExport::Edit(va_list vaList)
{
  void *ptr = va_arg(vaList, void *);

  func = ( unsigned (*) (wxTextBuffer &, short, std::vector<long> &, bool, long) ) (ptr);
  maxSupportedVersion = va_arg(vaList, long);

  return true;
}



// -----------------------------------------------------------------------
// CExpGR message handlers
void CGrExport::OnBrowse(wxCommandEvent &evt)
{
  evt.Skip();

  wxFileDialog  fileDlg(this, _("Select Export File"), CTT32App::instance()->GetPath(), fileName,
    _("CSV Files (*.csv)|*.csv|All Files (*.*)|*.*"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

  if (fileDlg.ShowModal() != wxID_OK)
    return;

  fileName = fileDlg.GetPath();

  XRCCTRL(*this, "File", wxTextCtrl)->SetValue(fileName);
}


void CGrExport::OnExport(wxCommandEvent &evt)
{
  evt.Skip();

  fileName = XRCCTRL(*this, "File", wxTextCtrl)->GetValue();

  if (wxFileName(fileName).GetPath().IsEmpty())
    fileName = CTT32App::instance()->GetPath() + wxFileName::GetPathSeparator() + fileName;
  
  wxMemoryText buf;

  if (cp.cpID == 0)
  {
    std::vector<CpRec> cpList;

    for (int idx = 0; idx < lbGR->GetCount(); idx++)
    {
      if (lbGR->IsSelected(idx))
        cpList.push_back( ((CpItem *) lbGR->GetListItem(idx))->cp);
    }

    // Progress-Dialog anzeigen, um den Benutzer den Fortschritt zu vermitteln (Oldies dauern lange) 
    // und Abbruch zu ermoeglichen
    wxProgressDialog dlg(((wxMDIChildFrame *) GetParent())->GetTitle(), "", cpList.size(), NULL, wxPD_AUTO_HIDE | wxPD_APP_MODAL | wxPD_CAN_ABORT);
    int i = 0;

    for (auto it = cpList.begin(); it != cpList.end(); it++)
    {
      std::vector<long> idList;
      GrListStore grList;
      grList.SelectAll( (*it) );
      while (grList.Next())
        idList.push_back(grList.grID);

      dlg.Update(++i, (*it).cpDesc);
      if (!(*func)(buf, (*it).cpType, idList, it != cpList.begin(), maxSupportedVersion))
        return;

      if (dlg.WasCancelled())
        return;
    }
  }
  else 
  {
    std::vector<long> idList;

    for (int idx = 0; idx < lbGR->GetCount(); idx++)
    {
      if (lbGR->IsSelected(idx))
        idList.push_back(lbGR->GetListItem(idx)->GetID());
    }

    if (!(*func)(buf, cp.cpType, idList, false, maxSupportedVersion))
      return;
  }

  std::ofstream  ofs(fileName.mb_str(), std::ios::out);

  const wxString bom(wxChar(0xFEFF));
  ofs << bom.ToUTF8();

  for (wxString str = buf.GetFirstLine(); !buf.Eof(); str = buf.GetNextLine())
    ofs << str.ToUTF8() << std::endl;

  ofs.close();
}


void CGrExport::OnSelChangeCp(wxCommandEvent &)
{
  lbGR->RemoveAllListItems();

  CpItem *cpItem = (CpItem *) cbCP->GetCurrentItem();
  if (!cpItem)
    return;
    
  cp = cpItem->cp;

  lbGR->ShowColumn(2, cp.cpID != 0);

  if (cp.cpID == 0)
  {
    CpListStore cpList;
    cpList.SelectAll();

    while (cpList.Next())
      lbGR->AddListItem(new CpItem(cpList));

    return;
  }
    
  // GR neu fuellen
  GrListStore  grList;
  grList.SelectAll(cp);
  
  while (grList.Next())
    lbGR->AddListItem(new GrItem(grList));
}


// -----------------------------------------------------------------------
void CGrExport::OnInitialUpdate()
{
  CFormViewEx::OnInitialUpdate();
  
  XRCCTRL(*this, "File", wxTextCtrl)->SetValue(fileName);

  cbCP = XRCCTRL(*this, "Events", CComboBoxEx);
  lbGR = XRCCTRL(*this, "Groups", CListCtrlEx);

  // CP fuellen
  CpListStore  cpList;

  // "All Events" entry
  cpList.cpID = 0;
  wxStrncpy(cpList.cpDesc, _("All Groups").c_str(), sizeof(cpList.cpDesc) / sizeof(wxChar) - 1);
  cbCP->AddListItem(new CpItem(cpList));

  cpList.SelectAll();
  
  while (cpList.Next())
    cbCP->AddListItem(new CpItem(cpList));
    
  cbCP->SetCurrentItem(cbCP->GetListItem(0));
  cbCP->SetCurrentItem(CTT32App::instance()->GetDefaultCP());    
  
  OnSelChangeCp(wxCommandEvent());  
}

