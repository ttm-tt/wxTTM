/* Copyright (C) 2020 Christoph Theis */

// CGrPrint.cpp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "GrPrint.h"

#include "CpItem.h"
#include "GrItem.h"

#include "Printer.h"
#include "RasterKO.h"
#include "RasterRR.h"

#include "GrOptions.h"

#include "InfoSystem.h"
#include "Printer.h"
#include "Profile.h"
#include "Res.h"

#include "SQLException.h"


IMPLEMENT_DYNAMIC_CLASS(CGrPrint, CGrPrintBase)

BEGIN_EVENT_TABLE(CGrPrint, CGrPrintBase)
  EVT_COMBOBOX(XRCID("Events"), CGrPrint::OnSelChangeCp)
END_EVENT_TABLE()


// -----------------------------------------------------------------------
// Use namespace to make class declaration local to this file
namespace
{
  class GrItemEx : public GrItem
  {
    public:
      GrItemEx(const GrRec &gr) : GrItem(gr) { SetLabel(""); }  // Sonst hat man einen Tooltip

    public:
      void DrawColumn(wxDC *, int col, wxRect &rect);

      int  Compare(const ListItem *, int col) const;
  };
}

void GrItemEx::DrawColumn(wxDC *pDC, int col, wxRect &rect)
{
  switch (col)
  {
    case 3:
      if (gr.grPrinted.year)
        DrawString(pDC, rect, wxString::Format("%04d-%02d-%02d %02d:%02d", gr.grPrinted.year, gr.grPrinted.month, gr.grPrinted.day, gr.grPrinted.hour, gr.grPrinted.minute));

      break;

    default:
      GrItem::DrawColumn(pDC, col, rect);
      break;
  }
}


int GrItemEx::Compare(const ListItem *itemPtr, int col) const
{
  if (col == 3)
  {
    if (gr.grPrinted > ((GrItemEx *)itemPtr)->gr.grPrinted)
      return 1;
    else if (gr.grPrinted < ((GrItemEx *)itemPtr)->gr.grPrinted)
      return -1;
    else
      return 0;
  }

  return GrItem::Compare(itemPtr, col);
}




// -----------------------------------------------------------------------
// CGrPrint dialog
CGrPrint::CGrPrint() : CGrPrintBase()
{
}


CGrPrint::~CGrPrint()
{
}


// -----------------------------------------------------------------------
// CGrPrint message handlers

void CGrPrint::OnInitialUpdate() 
{
  CGrPrintBase::OnInitialUpdate();

  m_cpCombo = XRCCTRL(*this, "Events", CComboBoxEx);

  m_grList = XRCCTRL(*this, "Groups", CListCtrlEx);

  // Name
  m_grList->InsertColumn(0, _("Name"), wxALIGN_LEFT, 6 * cW);

  // Description
  m_grList->InsertColumn(1, _("Description"), wxALIGN_LEFT);
  
  // Stufe
  m_grList->InsertColumn(2, _("Stage"), wxALIGN_LEFT, 10 * cW);

  // Stufe
  m_grList->InsertColumn(3, _("Printed"), wxALIGN_LEFT, 10 * cW);
  m_grList->HideColumn(3);
  m_grList->AllowHideColumn(3);

  m_grList->SetResizeColumn(1);

  m_grList->SetSortColumn(2);

  CpListStore  cp;

  // "All Events" entry
  cp.cpID = 0;
  wxStrncpy(cp.cpDesc, _("All Groups").c_str(), sizeof(cp.cpDesc) / sizeof(wxChar) - 1);
  m_cpCombo->AddListItem(new CpItem(cp));

  cp.SelectAll();
  while (cp.Next())
    m_cpCombo->AddListItem(new CpItem(cp));

  wxString cpName = CTT32App::instance()->GetDefaultCP();
  ListItem *cpItemPtr = (cpName.Length() ? m_cpCombo->FindListItem(cpName) : 0);
  if (cpItemPtr)
    m_cpCombo->SetCurrentItem(cpItemPtr->GetID());
  else
    m_cpCombo->SetCurrentItem(m_cpCombo->GetListItem(0));

  OnSelChangeCp(wxCommandEvent_);
}


void CGrPrint::OnSelChangeCp(wxCommandEvent &) 
{
  m_grList->RemoveAllListItems();

  CpItem *itemPtr = (CpItem *) m_cpCombo->GetCurrentItem();
  if (!itemPtr)
    return;

  m_cp = itemPtr->cp;

  m_grList->ShowColumn(2, m_cp.cpID != 0);

  if (m_cp.cpID == 0)
  {
    CpListStore cpList;
    cpList.SelectAll();

    while (cpList.Next())
      m_grList->AddListItem(new CpItem(cpList));

    return;
  }

  CTT32App::instance()->SetDefaultCP(m_cp.cpName);

  GrListStore  grList;
  grList.SelectAll(m_cp);

  while (grList.Next())
    m_grList->AddListItem(new GrItemEx(grList));

  m_grList->SortItems();

  OnOptionChange(wxCommandEvent_);
}


void CGrPrint::OnEdit()
{
  OnPrint();
}


void  CGrPrint::OnPrint()
{
  // Aktuelle Option sichern
  ttProfile.AddString(PRF_LOCAL_SETTINGS, PRF_PROPTIONS_LASTUSED, m_po.poName);

  if (!m_grList->GetSelectedCount())
    return;

  int nofItems = 0;
  long *idItems = NULL;
  
  if (m_cp.cpID == 0)
  {
    std::vector<long> ids;

    for (int idx = 0; idx < m_grList->GetItemCount(); idx++)
    {
      if (!m_grList->IsSelected(idx))
        continue;

      CpRec cp = ((CpItem *) m_grList->GetListItem(idx))->cp;

      GrListStore gr;
      gr.SelectAll(cp);
      while (gr.Next())
        ids.push_back(gr.grID);
    }

    idItems = new long[ids.size()];
    for (auto it : ids)
      idItems[nofItems++] = it;
  }
  else
  {
    idItems = new long[m_grList->GetItemCount()];

    for (int idx = 0; idx < m_grList->GetItemCount(); idx++)
    {
      if (!m_grList->IsSelected(idx))
        continue;

      GrItem *itemPtr = (GrItem *)m_grList->GetListItem(idx);
      if (itemPtr)
        idItems[nofItems++] = itemPtr->GetID();
    }
  }
  bool useThread = true;
  Printer *printer = NULL;
  bool showDlg = !wxGetKeyState(WXK_SHIFT);

  if (CTT32App::instance()->GetPrintPreview())
  {
    printer = new PrinterPreview(wxString::Format("%s %s", _("Result Forms"), m_cp.cpDesc), showDlg);
  }
  else if (CTT32App::instance()->GetPrintPdf())
  {
    wxString fname;

    if (m_cp.cpID == 0)
    {
      wxDirDialog dirDlg(this, wxDirSelectorPromptStr, CTT32App::instance()->GetPath());
      if (dirDlg.ShowModal() != wxID_OK)
        return;

      fname = dirDlg.GetPath();
    }
    else
    {
      wxFileDialog fileDlg(
        this, wxFileSelectorPromptStr, CTT32App::instance()->GetPath(), wxString::Format("%s.pdf", m_cp.cpName), 
        wxT("PDF Files (*.pdf)|*.pdf|All Files (*.*)|*.*||"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
      if (fileDlg.ShowModal() != wxID_OK)
        return;

      fname = fileDlg.GetPath();
    }

    printer = new PrinterPdf(fname, showDlg);
  }
  else
  {
    printer = new Printer(showDlg);
  }

  if (printer->PrinterAborted())
  {
    delete printer;
    return;
  }

  PrintThreadStruct *tmp = new PrintThreadStruct();
  tmp->nofItems = nofItems;
  tmp->idItems = idItems;
  tmp->m_cp = m_cp;
  tmp->m_po = m_po;
  tmp->printer = printer;
  tmp->isPreview = CTT32App::instance()->GetPrintPreview();
  tmp->isPdf = CTT32App::instance()->GetPrintPdf();

  if (useThread)
    tmp->connPtr = TTDbse::instance()->GetNewConnection();
  else
    tmp->connPtr = TTDbse::instance()->GetDefaultConnection();

  tmp->isThread = useThread;
  tmp->isMultiple = (m_cp.cpID == 0);

  wxString str;
  str = wxString::Format(_("Print %d Result Form(s)"), nofItems);

  if (useThread)
    CTT32App::instance()->ProgressBarThread(CGrPrintBase::PrintThread, tmp, str, nofItems);
  else
    CGrPrintBase::PrintThread(tmp);
}  
