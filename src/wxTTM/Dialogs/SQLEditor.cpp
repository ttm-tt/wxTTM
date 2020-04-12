/* Copyright (C) 2020 Christoph Theis */

// SQLEditor.cpp : implementation file
//

#include "stdafx.h"
#include "SQLEditor.h"

#include "TT32App.h"
#include "SQLItem.h"
#include "TTDbse.h"
#include "Statement.h"
#include "ResultSet.h"
#include "SQLException.h"

#include "Printer.h"
#include "StrUtils.h"
#include "InfoSystem.h"

#include "Res.h"

#include <string>
#include <fstream>
#include <vector>


IMPLEMENT_DYNAMIC_CLASS(CSQLEditor, CFormViewEx)

BEGIN_EVENT_TABLE(CSQLEditor, CFormViewEx)
  EVT_BUTTON(IDC_EXECUTE, CSQLEditor::OnExecuteSQL)
  EVT_BUTTON(wxID_SAVEAS, CSQLEditor::OnSave)
  EVT_BUTTON(XRCID("SaveQuery"), CSQLEditor::OnSaveQuery)
  EVT_BUTTON(XRCID("OpenQuery"), CSQLEditor::OnOpenQuery)
END_EVENT_TABLE()

// -----------------------------------------------------------------------
// CSQLEditor

CSQLEditor::CSQLEditor(wxMDIParentFrame *parent) : CFormViewEx(parent), m_listCtrl(0)
{
}

CSQLEditor::~CSQLEditor()
{
}


// -----------------------------------------------------------------------
void CSQLEditor::OnInitialUpdate()
{
  m_listCtrl = XRCCTRL(*this, "SQLLIST", CListCtrlEx);
  
  XRCCTRL(*this, "SQLEDIT", wxTextCtrl)->SetValidator(wxTextValidator(wxFILTER_NONE, &m_sqlString));
  
  FindWindow("Execute")->SetId(IDC_EXECUTE);
  FindWindow("Save")->SetId(wxID_SAVEAS);

  // Close verstecken, zumindest solange es nicht funktioniert.
  FindWindow("Close")->Show(false);
}

// -----------------------------------------------------------------------
// CSQLEditor message handlers

void CSQLEditor::OnExecuteSQL(wxCommandEvent &evt) 
{
  // Ein Hack: Global shortcuts gehen an den ersten Tab
  // Wenn ich die statt hier mit dem parent verknuepfe kann ich den 
  // SQL Editor nicht mehr standalone verwenden. So ich das jemals will
  if (GetParent()->IsKindOf(wxCLASSINFO(wxAuiNotebook)))
  {
    wxAuiNotebook *nb = (wxAuiNotebook *) GetParent();
    if (nb->GetPage(nb->GetSelection()) != this)
    {
      nb->GetPage(nb->GetSelection())->GetEventHandler()->ProcessEvent(evt);
      return;
    }
  }

  TransferDataFromWindow();
  
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  Statement  *stmtPtr = connPtr->CreateStatement();
  ResultSet  *resPtr  = 0;
  try
  {
    if ( stmtPtr->Execute(m_sqlString) )
      resPtr = stmtPtr->GetResultSet(false);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(m_sqlString, e);
  }

  long idx;

  // Items loeschen
  for (idx = m_listCtrl->GetItemCount(); idx--; )
    delete ( (ListItem *) m_listCtrl->GetItemData(idx) );
  m_listCtrl->DeleteAllItems();

  // Spalten loeschen
  while ( m_listCtrl->DeleteColumn(0) )
    ;

  if (resPtr && resPtr->GetColumnCount())
  {
    wxClientDC   dc(this);
    wxString     label;
    unsigned    *colSize;
    int          cW = dc.GetTextExtent("M").GetWidth();

    int numOfRows = resPtr->GetColumnCount();

    colSize = new unsigned[numOfRows];

#ifdef TIM
    // Seltsamerweise konnte ich die Ergebnisse nicht anzeigen, 
    // der Speicher ging aus. Daher direkt in File schreiben.
    FILE *f = fopen("C:\\Programme\\TTM\\EYC2007\\tim.csv", "w");
#endif

    for (idx = 0; idx < numOfRows; idx++)
    {
      label = resPtr->GetColumnLabel(idx+1);
      colSize[idx] = label.length();

#ifdef TIM
      fprintf(f, "%s;", label.data());
#else      
      m_listCtrl->InsertColumn(idx, label.data(), wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE); 
#endif      
    }
    
#ifdef TIM    
    fprintf(f, "\n");
#endif

    try
    {
      while (resPtr->Next())
      {
#ifdef TIM      
        wxChar str[128];
        idx = 0;
        
        for (idx = 0; idx < numOfRows; idx++)        
        {
          resPtr->GetData(idx+1, str, 128);
          if (resPtr->WasNull())
            fprintf(f, "<null>;");
          else
            fprintf(f, "%s;", str);
        }
        
        fprintf(f, "\n");
        
        continue;
#endif        
          
        SQLItem *itemPtr = new SQLItem(resPtr);
        m_listCtrl->AddListItem(itemPtr);

        for (idx = 0; idx < numOfRows; idx++)
        {
          if (colSize[idx] < itemPtr->GetColumnSize(idx))
          {
            colSize[idx] = itemPtr->GetColumnSize(idx);
            m_listCtrl->SetColumnWidth(idx, (colSize[idx] + 1) * cW);
          }
        }
      }
    }
    catch (SQLException &e)
    {
      infoSystem.Exception(m_sqlString, e);
    }

    // fclose(f);

    delete[] colSize;
  }
  
  delete resPtr;
  delete stmtPtr;
}


void CSQLEditor::OnSave(wxCommandEvent &evt)
{
  if (GetParent()->IsKindOf(wxCLASSINFO(wxAuiNotebook)))
  {
    wxAuiNotebook *nb = (wxAuiNotebook *) GetParent();
    if (nb->GetPage(nb->GetSelection()) != this)
    {
      nb->GetPage(nb->GetSelection())->GetEventHandler()->ProcessEvent(evt);
      return;
    }
  }

  wxString name = "";

  wxFileDialog fileDlg(
      wxGetApp().GetTopWindow(), _("Save File"), CTT32App::instance()->GetPath(), wxEmptyString,
      wxT("Text CSV (*.csv)|*.csv|All Files (*.*)|*.*||"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
      
  if (fileDlg.ShowModal() != wxID_OK)
    return;

  name = fileDlg.GetPath(); 

  // Gespeichert wird in UTF-8 (siehe PlStore)
  std::ofstream os(name.t_str());

  const wxString bom(wxChar(0xFEFF));
  os << bom.ToUTF8();

  int col;
  
  wxString cmd = m_sqlString;
  cmd.Replace(wxT("\r\n"), wxT("\n"));

  wxStringTokenizer tokens(cmd, wxT("\n"));
  while (tokens.HasMoreTokens())
    os << "#" << tokens.GetNextToken() << std::endl;  

  for (col = 0; col < m_listCtrl->GetColumnCount(); col++)
  {
    wxListItem item;
    item.SetMask(wxLIST_MASK_TEXT);
    if (!m_listCtrl->GetColumn(col, item))
      break;
      
    os << item.GetText() << ";";
  }

  os << std::endl;

  for (int idx = 0; idx < m_listCtrl->GetItemCount(); idx++)
  {
    wxString str;

    SQLItem *itemPtr = (SQLItem *) m_listCtrl->GetListItem(idx);
    if (!itemPtr)
      continue;
    
    for (int i = 0; i < col; i++)
    {
      str << itemPtr->GetColumn(i) << ";";
    }

    os << str.ToUTF8() << std::endl;
  }
}


// -----------------------------------------------------------------------
void CSQLEditor::OnPrint()
{
  Printer *printer;
  
  if (CTT32App::instance()->GetPrintPreview())
    printer = new PrinterPreview("Result Forms");
  else if (CTT32App::instance()->GetPrintPdf())
  {
    wxFileDialog fileDlg(
      this, wxFileSelectorPromptStr, CTT32App::instance()->GetPath(), "ResultForm.pdf", 
      wxT("PDF Files (*.pdf)|*.pdf|All Files (*.*)|*.*||"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (fileDlg.ShowModal() != wxID_OK)
      return;

    printer = new PrinterPdf(fileDlg.GetPath());
  }
  else 
    printer = new Printer;

  if (printer->PrinterAborted())
    return;
    
  printer->StartDoc("Execute SQL");
  printer->StartPage();
  
  short textFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_NORMAL);
  short headFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_NORMALB);
  
  int col;
  long totalWidth = 0;
  
  std::vector<wxString> headerList;  // Start der Spalten
  std::vector<int>         widthList;   // Ueberschrift der Spalten
  
  // Initialisierung
  for (col = 0; ;col++ )
  {
    wxListItem column;
    column.m_mask = wxLIST_MASK_TEXT;
    if (!m_listCtrl->GetColumn(col, column))
      break;
      
    wxString str = column.GetText();
    headerList.push_back(str);
    widthList.push_back(totalWidth);
    totalWidth += column.GetWidth();
  }
  
  // col+1 ist das Ende der letzten Spalte
  widthList.push_back(totalWidth);  // right margin
  
  // Skalierung an die Seitenbreite
  double factor = (totalWidth < printer->width ? 1. : printer->width / totalWidth);

  // factor mit der Aufloesung Drucker / Schirm skalieren
  {
    wxScreenDC dc;
    factor *= 1440 / dc.GetPPI().GetWidth();
  }
  
  long offsetY = 0;
  bool firstPage = true;
  int  row = 0;
  
  for (int idx = 0; idx < m_listCtrl->GetItemCount(); idx++)
  {
    if (firstPage || offsetY + printer->cH >= printer->height)
    {
      if (!firstPage)
      {
        printer->EndPage();
        printer->StartPage();
      }
      
      firstPage = false;
      
      printer->SelectFont(headFont);
      offsetY = printer->cH;
      
      for (int c = 0; c < col; c++)
      {
        wxRect rect(
            widthList[c] * factor, offsetY - printer->cH, 
            (widthList[c+1] - widthList[c]) * factor, printer->cH);
        printer->Text(rect, headerList[c].data());
      }
      
      offsetY += printer->cH;
      
      printer->SelectFont(textFont);

      row = 0;
    }

    if ( (row++ % 2) == 0 )
    {
      wxRect rect(0, offsetY - printer->cH, widthList[col] * factor, printer->cH);

      printer->Rectangle(rect, 0, true, *wxLIGHT_GREY);
    }
    
    SQLItem *itemPtr = (SQLItem *) m_listCtrl->GetListItem(idx);
    for (int c = 0; c < col; c++)
    {
        wxRect rect(
            widthList[c] * factor, offsetY - printer->cH, 
            (widthList[c+1] - widthList[c]) * factor, printer->cH);
      printer->Text(rect,itemPtr->GetColumn(c).data());
    }
    
    offsetY += printer->cH;
  }
  
  printer->EndPage();
  printer->EndDoc();  

  delete printer;  
}


void CSQLEditor::OnSaveQuery(wxCommandEvent &evt)
{
  if (GetParent()->IsKindOf(wxCLASSINFO(wxAuiNotebook)))
  {
    wxAuiNotebook *nb = (wxAuiNotebook *) GetParent();
    if (nb->GetPage(nb->GetSelection()) != this)
    {
      nb->GetPage(nb->GetSelection())->GetEventHandler()->ProcessEvent(evt);
      return;
    }
  }

  TransferDataFromWindow();

  wxFileDialog fileDlg(
      wxGetApp().GetTopWindow(), _("Save SQL Query"), CTT32App::instance()->GetPath(), wxEmptyString,
      wxT("SQL Query (*.sql)|*.sql|All Files (*.*)|*.*||"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
      
  if (fileDlg.ShowModal() != wxID_OK)
    return;

  wxString name = fileDlg.GetPath(); 

  std::ofstream str(name.t_str());

  wxStringTokenizer tokens(m_sqlString, "\r\n");

  while (tokens.HasMoreTokens())
    str << tokens.GetNextToken().ToUTF8() << std::endl;

  str.close();
}


void CSQLEditor::OnOpenQuery(wxCommandEvent &evt)
{
  if (GetParent()->IsKindOf(wxCLASSINFO(wxAuiNotebook)))
  {
    wxAuiNotebook *nb = (wxAuiNotebook *) GetParent();
    if (nb->GetPage(nb->GetSelection()) != this)
    {
      nb->GetPage(nb->GetSelection())->GetEventHandler()->ProcessEvent(evt);
      return;
    }
  }

  wxFileDialog fileDlg(
      wxGetApp().GetTopWindow(), _("Open SQL Query File"), CTT32App::instance()->GetPath(), wxEmptyString,
      wxT("SQL Query (*.sql)|*.sql|All Files (*.*)|*.*||"), wxFD_OPEN);
      
  if (fileDlg.ShowModal() != wxID_OK)
    return;

  wxString name = fileDlg.GetPath(); 

  m_sqlString = "";

  wxTextFile file(name);
  file.Open();

  for (wxString line = file.GetFirstLine(); !file.Eof(); line = file.GetNextLine())
    m_sqlString += line + "\n";

  file.Close();

  TransferDataToWindow();  
}

