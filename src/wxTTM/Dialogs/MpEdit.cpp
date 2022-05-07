/* Copyright (C) 2020 Christoph Theis */

#include "stdafx.h"
#include "TT32App.h"
#include "MpEdit.h"

#include "ComboBoxEx.h"

IMPLEMENT_DYNAMIC_CLASS(CMpEdit, CFormViewEx)

BEGIN_EVENT_TABLE(CMpEdit, CFormViewEx)

END_EVENT_TABLE()

// -----------------------------------------------------------------------
CMpEdit::CMpEdit() : CFormViewEx()
{

}


CMpEdit::~CMpEdit()
{

}


bool CMpEdit::Edit(va_list vaList)
{
  long id = va_arg(vaList, long);

  if (id)
  {
    mp.SelectById(id);
    mp.Next();
    mp.Close();
  }
  else
  {
    mp.ChangeCount(3);
  }

  m_count = mp.mpCount;

  m_gridCtrl->CreateGrid(mp.mpCount, 3);

  TransferDataToWindow();

  WriteGridCtrl();
  OnChangeMpEditCount(wxFocusEvent_);

  return true;
}


// -----------------------------------------------------------------------
void CMpEdit::OnInitialUpdate()
{
  CFormViewEx::OnInitialUpdate();

  m_gridCtrl = XRCCTRL(*this, "MatchPoints", wxGrid);

  FindWindow("Name")->SetValidator(CCharArrayValidator(mp.mpName, sizeof(mp.mpName) / sizeof(wxChar)));
	FindWindow("Description")->SetValidator(CCharArrayValidator(mp.mpDesc, sizeof(mp.mpDesc) / sizeof(wxChar)));
	FindWindow("Count")->SetValidator(CShortValidator(&m_count));

  FindWindow("Count")->Connect(wxEVT_KILL_FOCUS, wxFocusEventHandler(CMpEdit::OnChangeMpEditCount), NULL, this);
	
	m_gridCtrl->SetDefaultCellBackgroundColour(GetBackgroundColour());
}


// -----------------------------------------------------------------------
void CMpEdit::OnOK()
{
  TransferDataFromWindow();
  ReadGridCtrl();

  TTDbse::instance()->GetDefaultConnection()->StartTransaction();

  bool res;

  if (!mp.WasOK())
    res = mp.Insert();
  else
    res = mp.Update();

  if (res)
    TTDbse::instance()->GetDefaultConnection()->Commit();
  else
    TTDbse::instance()->GetDefaultConnection()->Rollback();

  CFormViewEx::OnOK();

}


// -----------------------------------------------------------------------
void CMpEdit::OnChangeMpEditCount(wxFocusEvent& evt)
{
  evt.Skip();

  short oldCount = mp.mpCount;

  ReadGridCtrl();

  FindWindow("Count")->GetValidator()->TransferFromWindow();

  m_gridCtrl->ClearGrid();

  if (m_gridCtrl->GetNumberRows() > m_count)
    m_gridCtrl->DeleteRows(m_count, m_gridCtrl->GetNumberRows() - m_count);

  if (m_gridCtrl->GetNumberRows() < m_count)
    m_gridCtrl->AppendRows(m_count - m_gridCtrl->GetNumberRows());

  for (int row = 0; row < m_gridCtrl->GetNumberRows(); row++)
  {
    for (int col = 0; col < m_gridCtrl->GetNumberCols(); col++)
    {
      m_gridCtrl->SetCellBackgroundColour(row, col, wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));            
    }
  }

  if (oldCount != m_count)
    mp.ChangeCount(m_count);

  WriteGridCtrl();

  // m_gridCtrl->AutoSize();
}


// -----------------------------------------------------------------------
void CMpEdit::WriteGridCtrl()
{
  // Col and row header
  static wxString colHeaders[3] = 
  {
    _("Res A"),
    _("Res X"),
    _("Pts")
  };

  static wxString rowHeader = _("Result %d");

  for (int row = 0; row <= m_gridCtrl->GetNumberRows(); ++row)
  {
    if (row == 0)
    {
      m_gridCtrl->SetColLabelValue(0, colHeaders[0]);
      m_gridCtrl->SetColLabelValue(1, colHeaders[1]);
      m_gridCtrl->SetColLabelValue(2, colHeaders[2]);
    } 
    else
    {
      m_gridCtrl->SetRowLabelValue(row - 1, wxString::Format(rowHeader, row));
      m_gridCtrl->SetCellValue(row - 1, 0, wxString::Format("%d", mp.mpList[row - 1].mpResA));
      m_gridCtrl->SetCellValue(row - 1, 1, wxString::Format("%d", mp.mpList[row - 1].mpResX));
      m_gridCtrl->SetCellValue(row - 1, 2, wxString::Format("%d", mp.mpList[row - 1].mpPts));
    }
  }

  m_gridCtrl->Refresh();
}


void CMpEdit::ReadGridCtrl()
{
  // Ensure we have the desired size
  mp.ChangeCount(m_gridCtrl->GetNumberRows());

  for (int row = 0; row < m_gridCtrl->GetNumberRows(); ++row)
  {
    mp.mpList[row].mpResA = wxAtoi(m_gridCtrl->GetCellValue(row, 0));
    mp.mpList[row].mpResX = wxAtoi(m_gridCtrl->GetCellValue(row, 1));
    mp.mpList[row].mpPts = wxAtoi(m_gridCtrl->GetCellValue(row, 2));
  }
}