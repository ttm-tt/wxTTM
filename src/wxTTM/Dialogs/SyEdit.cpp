/* Copyright (C) 2020 Christoph Theis */

// SyEdit.syp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "SyEdit.h"

#include "GrStore.h"

#include "Request.h"


IMPLEMENT_DYNAMIC_CLASS(CSyEdit, CFormViewEx)

BEGIN_EVENT_TABLE(CSyEdit, CFormViewEx)
END_EVENT_TABLE()


// -----------------------------------------------------------------------
// CSyEdit
CSyEdit::CSyEdit() : CFormViewEx()
{
}


CSyEdit::~CSyEdit()
{
}


bool  CSyEdit::Edit(va_list vaList)
{
  long id = va_arg(vaList, long);

  if (id)
  {
    sy.SelectById(id);
    sy.Next();
    sy.Close();
    
    short count = GrStore().CountGroups(sy);

    FindWindow("Matches")->Enable(count == 0);
    FindWindow("Singles")->Enable(count == 0);
    FindWindow("Doubles")->Enable(count == 0);
            
    m_gridCtrl->EnableEditing(count == 0);
  }
  else
  {
    sy.ChangeSize(2);
    sy.sySingles = 1;
    sy.syDoubles = 1;
  }

  m_gridCtrl->CreateGrid(sy.syMatches, 2);
  
  TransferDataToWindow();

  m_gridCtrl->ClearGrid();
  m_gridCtrl->DeleteRows(0, m_gridCtrl->GetNumberRows());
  m_gridCtrl->DeleteCols(0, m_gridCtrl->GetNumberCols());
  
  m_gridCtrl->AppendRows(sy.syMatches);
  m_gridCtrl->AppendCols(2);

  wxString einzel = _("Single %d");
  wxString doppel = _("Double %d");
  wxString match = _("Match %d");
  wxString team  = _("Team %c");  
          
  for (int row = 0; row <= m_gridCtrl->GetNumberRows(); row++)
  {
    for (int col = 0; col <= m_gridCtrl->GetNumberCols(); col++)
    {
      if (row == 0 && col == 0)
        ; 
      else if (row == 0)
        m_gridCtrl->SetColLabelValue(col - 1, wxString::Format(team, col == 1 ? wxChar('A') : wxChar('X')));
      else if (col == 0)
        m_gridCtrl->SetRowLabelValue(row - 1, wxString::Format(match, row));
      else
      {
        SyStore::SyList syList = sy.syList[row-1];
        m_gridCtrl->SetCellValue(row - 1, col - 1, 
            wxString::Format(
                syList.syType == CP_SINGLE ? einzel : doppel, 
                col == 1 ? syList.syPlayerA : syList.syPlayerX) );
                
        m_gridCtrl->SetCellBackgroundColour(row - 1, col - 1, wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));        
		  }
    }
  }

  OnChangeSyEditSinglesDoubles(wxFocusEvent());
  
  // m_gridCtrl->ExpandColumnsToFit();
  return true;
}



// -----------------------------------------------------------------------
// CSyEdit message handlers

void CSyEdit::OnInitialUpdate() 
{
	CFormViewEx::OnInitialUpdate();	
	
	m_gridCtrl = XRCCTRL(*this, "System", wxGrid);
	
  FindWindow("Name")->SetValidator(CCharArrayValidator(sy.syName, sizeof(sy.syName) / sizeof(wxChar)));	
  FindWindow("Description")->SetValidator(CCharArrayValidator(sy.syDesc, sizeof(sy.syDesc) / sizeof(wxChar)));	
  FindWindow("Matches")->SetValidator(CShortValidator(&sy.syMatches));
  FindWindow("Singles")->SetValidator(CShortValidator(&sy.sySingles));
  FindWindow("Doubles")->SetValidator(CShortValidator(&sy.syDoubles));
  FindWindow("PlayAllMatches")->SetValidator(CEnumValidator(&sy.syComplete, 1));

  FindWindow("Matches")->Connect(wxEVT_KILL_FOCUS, wxFocusEventHandler(CSyEdit::OnChangeSyEditMatches), NULL, this);
  FindWindow("Singles")->Connect(wxEVT_KILL_FOCUS, wxFocusEventHandler(CSyEdit::OnChangeSyEditSinglesDoubles), NULL, this);
  FindWindow("Doubles")->Connect(wxEVT_KILL_FOCUS, wxFocusEventHandler(CSyEdit::OnChangeSyEditSinglesDoubles), NULL, this);

  m_gridCtrl->SetDefaultEditor(new wxGridCellChoiceEditor());
  m_gridCtrl->SetDefaultCellBackgroundColour(GetBackgroundColour());
  // m_gridCtrl->SetDefaultRowSize(40);
}


void  CSyEdit::OnOK()
{
  TransferDataFromWindow();

  wxString doppel = _("Double %d");  

  for (int row = 0; row < m_gridCtrl->GetNumberRows(); row++)
  {
    short plA = 0, plX = 0;
    wxChar  typeA = 0, typeX = 0;
    
    wxSscanf(m_gridCtrl->GetCellValue(row, 0), "%c%*[^ ] %hd", &typeA, &plA);
    
    wxSscanf(m_gridCtrl->GetCellValue(row, 1), "%c%*[^ ] %hd", &typeX, &plX);

    // Die Liste muss konsistent sein, also gleicher Typ, 
    // Spieler im Rahmen des erlaubten. Das wird schon durch das Grid
    // sichergestellt.    
    sy.syList[row].syType = (typeA == doppel.GetChar(0) ? CP_DOUBLE : CP_SINGLE);
    sy.syList[row].syPlayerA = plA;
    sy.syList[row].syPlayerX = plX;    
  }
    
  bool  res = false;

  TTDbse::instance()->GetDefaultConnection()->StartTransaction();

  if (!sy.WasOK())
    res = sy.Insert();
  else
    res = sy.Update();

  if (res)
    TTDbse::instance()->GetDefaultConnection()->Commit();
  else
    TTDbse::instance()->GetDefaultConnection()->Rollback();

  CFormViewEx::OnOK();
}


void CSyEdit::OnChangeSyEditMatches(wxFocusEvent &) 
{
  FindWindow("Matches")->GetValidator()->TransferFromWindow();
  
  wxString match = _("Match %d");
    
  sy.ChangeSize(sy.syMatches);

  if (m_gridCtrl->GetNumberRows() > sy.syMatches)
    m_gridCtrl->DeleteRows(sy.syMatches, m_gridCtrl->GetNumberRows() - sy.syMatches);
  if (m_gridCtrl->GetNumberRows() < sy.syMatches)
    m_gridCtrl->AppendRows(sy.syMatches - m_gridCtrl->GetNumberRows());  
  
  for (int row = 0; row < m_gridCtrl->GetNumberRows(); row++)
    m_gridCtrl->SetRowLabelValue(row, wxString::Format(match, row + 1));
    
  for (int row = 0; row < m_gridCtrl->GetNumberRows(); row++)
  {
    for (int col = 0; col < m_gridCtrl->GetNumberCols(); col++)
    {
      m_gridCtrl->SetCellBackgroundColour(row, col, wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));            
    }
  }
}


void CSyEdit::OnChangeSyEditSinglesDoubles(wxFocusEvent &) 
{
  FindWindow("Singles")->GetValidator()->TransferFromWindow();
  FindWindow("Doubles")->GetValidator()->TransferFromWindow();
  
  wxString einzel = _("Single %d");
  wxString doppel = _("Double %d");

  wxString params;

  for (int si = 0; si < sy.sySingles; si++)    
    params += wxString::Format(einzel, si + 1) + wxString(",");
    
  for (int db = 0; db < sy.syDoubles; db++)
    params += wxString::Format(doppel, db + 1) + wxString(",");
  
  params.RemoveLast();

  // Ich muss den Editor fuer alle Spalten (neu) setzen.
  // Nur einen default editor geht nicht, weil zum einen die Zellen
  // eine Kopie bekommen (?, jedenfalls kann ich die Werte drin 
  // nicht mehr aendern), und zum andere bekomme ich im Destruktor
  // ein assert, weil noch ein EventHandler ueber bleibt.
  wxGridCellChoiceEditor *editorA = new wxGridCellChoiceEditor();
  wxGridCellChoiceEditor *editorX = new wxGridCellChoiceEditor();
  
  editorA->SetParameters(params);
  editorX->SetParameters(params);

  wxGridCellAttr *attrA = new wxGridCellAttr(), *attrX = new wxGridCellAttr();
  attrA->SetEditor(editorA);
  attrX->SetEditor(editorX);

  m_gridCtrl->SetColAttr(0, attrA);
  m_gridCtrl->SetColAttr(1, attrX);  
}
