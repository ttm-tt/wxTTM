/* Copyright (C) 2020 Christoph Theis */

// Preview.cpp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "Preview.h"
#include "Printer.h"
#include "Profile.h"
#include "Res.h"
#include "wx/pdfdc.h"


IMPLEMENT_DYNAMIC_CLASS(CPreview, wxFrame)

BEGIN_EVENT_TABLE(CPreview, wxFrame)
  EVT_COMMAND(wxID_ANY, IDC_STARTDOC, CPreview::OnStartDoc)
  EVT_COMMAND(wxID_ANY, IDC_ENDDOC, CPreview::OnEndDoc)
  EVT_COMMAND(wxID_ANY, IDC_ABORTDOC, CPreview::OnEndDoc)

  EVT_BUTTON(IDC_FIRST, CPreview::OnPreviewFirst)
  EVT_BUTTON(IDC_PREV, CPreview::OnPreviewPrev)
  EVT_BUTTON(IDC_NEXT, CPreview::OnPreviewNext)
  EVT_BUTTON(IDC_LAST, CPreview::OnPreviewLast)

  EVT_BUTTON(XRCID("Close"), CPreview::OnClose)

  EVT_SLIDER(XRCID("Zoom"), CPreview::OnZoom)
  
  EVT_BUTTON(wxID_PRINT, CPreview::OnPrint)

END_EVENT_TABLE()


// -----------------------------------------------------------------------
// CPreview

CPreview::CPreview(): wxFrame(), previewWnd(0)
{
  pages = 0;
  lastPage = currentPage = 0;

  height = width = 1;
}

CPreview::~CPreview()
{
  for (int i = 0; i < lastPage; i++)
    if (pages && pages[i])
      delete pages[i];

  if (pages)
    free(pages);
}


bool CPreview::Show(bool show)
{
  if (show)
    RestoreSize();
  else
    SaveSize();

  return wxFrame::Show(show);
}


// -----------------------------------------------------------------------
void  CPreview::SetSize(long newWidth, long newHeight)
{
  if (previewWnd)
    previewWnd->SetSize(newWidth, newHeight);
}


// -----------------------------------------------------------------------
// CPreview message handlers

void CPreview::OnStartDoc(wxCommandEvent&)
{
  // 
}


void CPreview::OnEndDoc(wxCommandEvent &)
{
  if (GetPrinter())
    printData = GetPrinter()->GetPrintData();

  SetPrinter(NULL);

  FindWindow("Print")->SetId(wxID_PRINT);

  FindWindow("First")->SetId(IDC_FIRST);
  FindWindow("Prev")->SetId(IDC_PREV);
  FindWindow("Next")->SetId(IDC_NEXT);
  FindWindow("Last")->SetId(IDC_LAST);

  wxAcceleratorEntry entries[] = 
  {
    // Drucken
    wxAcceleratorEntry(wxACCEL_CTRL, 'P', wxID_PRINT),
    wxAcceleratorEntry(wxACCEL_CTRL | wxACCEL_SHIFT, 'P', wxID_PRINT),

    // Vorige / Naechste Seite
    wxAcceleratorEntry(wxACCEL_CTRL, WXK_PAGEUP, IDC_FIRST),
    wxAcceleratorEntry(wxACCEL_NORMAL, WXK_PAGEUP, IDC_PREV),
    wxAcceleratorEntry(wxACCEL_NORMAL, WXK_PAGEDOWN, IDC_NEXT),
    wxAcceleratorEntry(wxACCEL_CTRL, WXK_PAGEDOWN, IDC_LAST),

    // Und das gleiche wie in MtListView
    wxAcceleratorEntry(wxACCEL_CTRL | wxACCEL_ALT, WXK_LEFT, IDC_FIRST),
    wxAcceleratorEntry(wxACCEL_ALT, WXK_LEFT, IDC_PREV),
    wxAcceleratorEntry(wxACCEL_ALT, WXK_RIGHT, IDC_NEXT),
    wxAcceleratorEntry(wxACCEL_CTRL | wxACCEL_ALT, WXK_RIGHT, IDC_LAST),

  };
  
  SetAcceleratorTable(wxAcceleratorTable(sizeof(entries) / sizeof(entries[0]), entries));


  XRCCTRL(*this, "Zoom", wxSlider)->SetRange(0, 100);
  XRCCTRL(*this, "Zoom", wxSlider)->SetValue(0);
  
  OnPreviewFirst(wxCommandEvent());
}


void CPreview::OnPreviewFirst(wxCommandEvent &) 
{
  currentPage = 0;
  
  if (pages && previewWnd)
    previewWnd->SetMetafile(pages[currentPage]);

  XRCCTRL(*this, "Pages", wxTextCtrl)->SetValue(wxString::Format("%d / %d", currentPage + 1, lastPage));
}


void CPreview::OnPreviewLast(wxCommandEvent &) 
{
  currentPage = (lastPage > 0 ? lastPage-1 : 0);

  OnPreview();
}


void CPreview::OnPreviewNext(wxCommandEvent &) 
{
  if (currentPage < lastPage-1)
    currentPage++;

  OnPreview();
}


void CPreview::OnPreviewPrev(wxCommandEvent &) 
{
  if (currentPage > 0)
    currentPage--;

  OnPreview();
}


void CPreview::OnPreview()
{
  if (pages && previewWnd)
    previewWnd->SetMetafile(pages[currentPage]);

  XRCCTRL(*this, "Pages", wxTextCtrl)->SetValue(wxString::Format("%d / %d", currentPage + 1, lastPage));
}


void CPreview::OnZoom(wxCommandEvent &)
{
  double zoom = 1. + XRCCTRL(*this, "Zoom", wxSlider)->GetValue() / 10.;
  if (previewWnd)
    previewWnd->SetZoom(zoom);
}


void CPreview::OnClose(wxCommandEvent &)
{
  SaveSize();
  Close();
}


void CPreview::OnPrint(wxCommandEvent &)
{
  wxPrintDialogData printDlgData;
  printDlgData.EnablePageNumbers(true);
  printDlgData.SetMinPage(1);
  printDlgData.SetMaxPage(lastPage);
  printDlgData.SetAllPages(true);

  int fromPage = 1;
  int toPage = lastPage;

  wxDC *dcOutput = NULL;

  // TODO: Nach PDF drucken. 
  // Bedeutet wohl, aus WMF ein Bitmap machen und das in PDF einfuegen.
  // Oder Preview erzeugt PDF, die wiederum nach PDF kopiert werden.
  if (false && CTT32App::instance()->GetPrintPdf())
  {
    wxFileDialog fileDlg(
      this, wxFileSelectorPromptStr, CTT32App::instance()->GetPath(), "Preview.pdf", 
      wxT("PDF Files (*.pdf)|*.pdf|All Files (*.*)|*.*||"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (fileDlg.ShowModal() != wxID_OK)
      return;
    printData.SetFilename(fileDlg.GetPath());

    dcOutput = new wxPdfDC(printData);
  }
  else 
  {  
    printDlgData.SetPrintData(printData);

    wxPrintDialog printDlg(this, &printDlgData);
    if (!wxGetKeyState(WXK_SHIFT))
    {
      if (printDlg.ShowModal() != wxID_OK)
        return;
    }
    
    dcOutput = printDlg.GetPrintDC();
    printDlgData = printDlg.GetPrintDialogData();
  
    fromPage = std::min((int) lastPage, printDlgData.GetFromPage());
    toPage = std::min((int) lastPage, printDlgData.GetToPage());
  
    if (printDlgData.GetAllPages())
    {
      fromPage = 1;
      toPage = lastPage;
    }
  }
  
  dcOutput->StartDoc("Print Preview");
  
  for (int page = fromPage; page <= toPage; page++)
  {
    wxRect rect(0, 0, dcOutput->GetSize().GetWidth(), dcOutput->GetSize().GetHeight());
    
    dcOutput->StartPage();
    pages[page - 1]->Play(dcOutput, &rect);
    dcOutput->EndPage();    
  }
  
  dcOutput->EndDoc();
}

// -----------------------------------------------------------------------
void CPreview::SaveSize()
{
  wxSize size = GetSize();
  wxPoint pos = GetPosition();

  if (wxPoint(origX, origY) != pos || wxSize(origW, origH) != size)
  {
    wxString tmp = wxString::Format("%d,%dx%d,%d", std::max(0, pos.x), std::max(0, pos.y), size.x, size.y);
    ttProfile.AddString(PRF_GLOBAL_SETTINGS, GetClassInfo()->GetClassName(), tmp);
  }
}


void  CPreview::RestoreSize()
{
  wxString tmp = ttProfile.GetString(PRF_GLOBAL_SETTINGS, GetClassInfo()->GetClassName());
  if (!tmp.IsEmpty())
  {
    int x, y, w, h;
    sscanf(tmp.c_str(), "%d,%dx%d,%d", &x, &y, &w, &h);

    wxFrame::SetSize(w, h);

    SetPosition(wxPoint(std::max(0, x), std::max(0, y)));
  }
  else
    wxFrame::SetSize(400, 600);

  wxSize size = GetSize();
  wxPoint pos = GetPosition();

  origX = pos.x;
  origY = pos.y;
  origW = size.x;
  origH = size.y;
}


