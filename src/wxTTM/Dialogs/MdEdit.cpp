/* Copyright (C) 2020 Christoph Theis */

// MdEdit.mdp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "MdEdit.h"

#include "GrStore.h"

#include "InfoSystem.h"
#include "Request.h"

IMPLEMENT_DYNAMIC_CLASS(CMdEdit, CFormViewEx)

BEGIN_EVENT_TABLE(CMdEdit, CFormViewEx)
  EVT_BUTTON(XRCID("Automatic"), CMdEdit::OnMdEditAuto)  
  EVT_GRID_CELL_CHANGED(CMdEdit::OnGridCellChanged)
  // EVT_KILL_FOCUS(XRCID("Size"), CMdEdit::OnChangeMdEditSize)
END_EVENT_TABLE()


// -----------------------------------------------------------------------
// CMdEdit
CMdEdit::CMdEdit(): CFormViewEx()
{
}


CMdEdit::~CMdEdit()
{
}


bool  CMdEdit::Edit(va_list vaList)
{
  long id = va_arg(vaList, long);

  if (id)
  {
    md.SelectById(id);
    md.Next();
    
    md.Close();
    
    short count = GrStore().CountGroups(md);

    FindWindow("Size")->Enable(false);
    FindWindow("Modus")->Enable(count == 0);
    FindWindow("Automatic")->Enable(count == 0);
  }
  else
  {
    md.ChangeSize(md.mdSize = 4);
    md.mdMtPtsWin = 2;
    md.mdMtPtsTie = 1;
    md.mdMtPtsLoss = 1;
  }

  // GridCtrl aufziehen und Daten ausgeben
  m_gridCtrl->CreateGrid(md.Rounds(), md.Matches());

  TransferDataToWindow();

  OnChangeMdEditSize(wxFocusEvent());

  // m_gridCtrl->AutoSize();

  return true;
}



// -----------------------------------------------------------------------
// CMdEdit message handlers

void CMdEdit::OnInitialUpdate() 
{
	CFormViewEx::OnInitialUpdate();	
	
	m_gridCtrl = XRCCTRL(*this, "Modus", wxGrid);
	
	FindWindow("Name")->SetValidator(CCharArrayValidator(md.mdName, sizeof(md.mdName) / sizeof(wxChar)));
	FindWindow("Description")->SetValidator(CCharArrayValidator(md.mdDesc, sizeof(md.mdDesc) / sizeof(wxChar)));
	FindWindow("PtsWin")->SetValidator(CShortValidator(&md.mdMtPtsWin));
	FindWindow("PtsTie")->SetValidator(CShortValidator(&md.mdMtPtsTie));
	FindWindow("PtsLoss")->SetValidator(CShortValidator(&md.mdMtPtsLoss));
	FindWindow("Size")->SetValidator(CShortValidator(&md.mdSize));

  FindWindow("Size")->Connect(wxEVT_KILL_FOCUS, wxFocusEventHandler(CMdEdit::OnChangeMdEditSize), NULL, this);
	
	m_gridCtrl->SetDefaultCellBackgroundColour(GetBackgroundColour());
  // m_gridCtrl->SetDefaultRowSize(40);
}  


void  CMdEdit::OnOK()
{
  TransferDataFromWindow();

  md.ChangeSize(md.mdSize);
  
  for (int row = 0; row < m_gridCtrl->GetNumberRows(); row++)
  {
    for (int col = 0; col < m_gridCtrl->GetNumberCols(); col++)
    {
      short  plA = 0, plX = 0;

      wxString str = m_gridCtrl->GetCellValue(row, col);

      wxSscanf(str, "%hd%*[^0-9]%hd", &plA, &plX);

      if ( (plA >= 1) && (plA <= md.mdSize) &&
           (plX >= 1) && (plX <= md.mdSize) )
      {
        md.SetPlayerA(row + 1, col + 1, plA);
        md.SetPlayerX(row + 1, col + 1, plX);
      }
      else
      {
        infoSystem.Error(_("Wrong players at round %d, match %d"), row + 1, col + 1);
        return;
      }
        
      // Auf Duplikate in den vorhergehenden Runden pruefen ...
      for (int tmpRow = 0; tmpRow < row; tmpRow++)
      {
        for (int tmpCol = 0; tmpCol < m_gridCtrl->GetNumberCols(); tmpCol++)
        {
          if (plA == md.GetPlayerA(tmpRow + 1, tmpCol + 1) && plX == md.GetPlayerX(tmpRow + 1, tmpCol + 1))
          {
            infoSystem.Error(_("Same match at round %d match %d and round %d match %d"), row + 1, col + 1, tmpRow + 1, tmpCol + 1);
            return;
          }
          if (plA == md.GetPlayerX(tmpRow + 1, tmpCol + 1) && plX == md.GetPlayerA(tmpRow + 1, tmpCol + 1))
          {
            infoSystem.Error(_("Same match at round %d match %d and round %d match %d"), row + 1, col + 1, tmpRow + 1, tmpCol + 1);
            return;
          }
        }
      }
      
      // ... und in dieser Runde
      for (int tmpCol = 0; tmpCol < col; tmpCol++)
      {
        if (plA == md.GetPlayerA(row + 1, tmpCol + 1) && plX == md.GetPlayerX(row + 1, tmpCol + 1))
        {
          infoSystem.Error(_("Same match at round %d match %d and match %d"), row + 1, col + 1, tmpCol + 1);
          return;
        }
        if (plA == md.GetPlayerX(row + 1, tmpCol + 1) && plX == md.GetPlayerA(row + 1, tmpCol + 1))
        {
          infoSystem.Error(_("Same match at round %d match %d and match %d"), row + 1, col + 1, tmpCol + 1);
          return;
        }
      }
    }
  }

  TTDbse::instance()->GetDefaultConnection()->StartTransaction();

  bool res;

  if (!md.WasOK())
    res = md.Insert();
  else
    res = md.Update();

  if (res)
    TTDbse::instance()->GetDefaultConnection()->Commit();
  else
    TTDbse::instance()->GetDefaultConnection()->Rollback();

  CFormViewEx::OnOK();
}


void CMdEdit::OnChangeMdEditSize(wxFocusEvent &evt) 
{
  evt.Skip();

  short oldSize = md.mdSize;
  
  FindWindow("Size")->GetValidator()->TransferFromWindow();
  
  m_gridCtrl->ClearGrid();
  
  if (m_gridCtrl->GetNumberRows() > md.Rounds())
    m_gridCtrl->DeleteRows(md.Rounds(), m_gridCtrl->GetNumberRows() - md.Rounds());
  
  if (m_gridCtrl->GetNumberCols() > md.Matches())
    m_gridCtrl->DeleteCols(md.Matches(), m_gridCtrl->GetNumberCols() - md.Matches());
    
  if (m_gridCtrl->GetNumberRows() < md.Rounds())
    m_gridCtrl->AppendRows(md.Rounds() - m_gridCtrl->GetNumberRows());
    
  if (m_gridCtrl->GetNumberCols() < md.Matches())
    m_gridCtrl->AppendCols(md.Matches() - m_gridCtrl->GetNumberCols());
    
  for (int row = 0; row < m_gridCtrl->GetNumberRows(); row++)
  {
    for (int col = 0; col < m_gridCtrl->GetNumberCols(); col++)
    {
      m_gridCtrl->SetCellBackgroundColour(row, col, wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));            
    }
  }

  if (oldSize != md.mdSize)
    md.ChangeSize(md.mdSize);

  WriteGridCtrl();

  m_gridCtrl->AutoSize();

  Layout();
}


void CMdEdit::OnMdEditAuto(wxCommandEvent &) 
{
  // Fuellen
  // Zahl der Runden und Spiele
  int  even = md.mdSize & 0x1 ? 0 : 1;
  int  lastRd = md.Rounds();
  int  lastMt = md.Matches();

  for (int rd = 0; rd < lastRd; rd++)
  {
    for (int mt = 0; mt < lastMt; mt++)
    {
      if (even && mt == (lastMt - 1))
      {
        md.SetPlayerA(rd+1, mt+1, rd+1);
        md.SetPlayerX(rd+1, mt+1, md.mdSize);
      }
      else
      {
        short plA = ((md.mdSize - even + rd - mt - 1) % (md.mdSize - even)) + 1;
        short plX = ((1 + mt + rd) % (md.mdSize - even)) + 1;

        // Kleinere Nummer soll vorne stehen
        if (plA > plX)
        {
          short tmp = plA;
          plA = plX;
          plX = tmp;
        }

        md.SetPlayerA(rd+1, mt+1, plA);
        md.SetPlayerX(rd+1, mt+1, plX);
      } // Nicht letztes Spiel in Runde
    }   // Alle Spiele
  }     // Alle Runden

  // Daten ausgeben
  WriteGridCtrl();
}


void CMdEdit::WriteGridCtrl()
{
  wxString strRd = _("Round %d");
  wxString strMt = _("Match %d");

  for (short row = 0; row <= m_gridCtrl->GetNumberRows(); row++)
  {
    for (short col = 0; col <= m_gridCtrl->GetNumberCols(); col++)
    {
      if (row == 0 && col == 0)
        ; // m_gridCtrl->SetCellValue(row, col, "");
      else if (row == 0)
        m_gridCtrl->SetColLabelValue(col - 1, wxString::Format(strMt, col));
      else if (col == 0)
        m_gridCtrl->SetRowLabelValue(row - 1, wxString::Format(strRd, row));
      else
        m_gridCtrl->SetCellValue(row - 1, col - 1, wxString::Format("%d : %d", md.GetPlayerA(row, col), md.GetPlayerX(row, col)));
    }
  }

  m_gridCtrl->Refresh();
}


void CMdEdit::OnGridCellChanged(wxGridEvent &evt)
{
  int row = evt.GetRow(), col = evt.GetCol();
  wxString str = m_gridCtrl->GetCellValue(row, col);
  short plA, plX;
  
  if (wxSscanf(str, "%hd%*[^0-9]%hd", &plA, &plX) != 2)
    str = wxString::Format("%d : %d", md.GetPlayerA(row, col), md.GetPlayerX(row, col));
  else
    str = wxString::Format("%hd : %hd", plA, plX);
    
  m_gridCtrl->SetCellValue(row, col, str);
}