/* Copyright (C) 2020 Christoph Theis */

// RpEdit.cpp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "RpEdit.h"

#include "CpItem.h"
#include "NaItem.h"
#include "RkItem.h"

#include "PlStore.h"
#include "CpStore.h"
#include "RpStore.h"

#include "Rec.h"
#include "Request.h"
#include "ListItem.h"

#include <map>


IMPLEMENT_DYNAMIC_CLASS(CRpEdit, CFormViewEx)

BEGIN_EVENT_TABLE(CRpEdit, CFormViewEx)
END_EVENT_TABLE()


// Item class
class RpItem : public ListItem
{
  public:
    RpItem(short year, float pts) : m_year(year), m_pts(pts) {}

  public:
    void DrawColumn(wxDC *pDC, int col, wxRect &rect);

  public:
    short m_year;
    short m_pts;
};


void RpItem::DrawColumn(wxDC *pDC, int col, wxRect &rect)
{
  switch (col)
  {
    case 0 :
      DrawLong(pDC, rect, m_year);
      break;

    case 1 :
      DrawLong(pDC, rect, m_pts);
      break;

    default :
      break;
  }
}


// -----------------------------------------------------------------------
// CRpEdit
CRpEdit::CRpEdit() : CFormViewEx()
{
  plID = 0;
}


CRpEdit::~CRpEdit()
{
}


bool CRpEdit::Edit(va_list vaList)
{
  plID = va_arg(vaList, long);

  PlStore pl;
  pl.SelectById(plID);
  pl.Next();
  pl.Close();

  std::map<short, float> years;

  RpStore rp;
  rp.SelectByPl(plID);
  while (rp.Next())
    years[rp.rpYear] = rp.rpRankPts;
  rp.Close();

  for (std::map<short, float>::reverse_iterator it = years.rbegin(); it != years.rend(); it++)
    m_listCtrl->AddListItem(new RpItem(it->first, it->second));

  return true;
}


// -----------------------------------------------------------------------
void CRpEdit::OnInitialUpdate() 
{
	CFormViewEx::OnInitialUpdate();
	
	m_listCtrl = XRCCTRL(*this, "RankingPoints", CListCtrlEx);

  // Players, Type, Ranking
  m_listCtrl->InsertColumn(0, _("Born"));
  m_listCtrl->InsertColumn(1, _("Rank. Pts."), wxALIGN_RIGHT);

  m_listCtrl->SetResizeColumn(1);  
}


// -----------------------------------------------------------------------
// CRpEdit message handlers
void CRpEdit::OnEdit()
{
  long  idx = m_listCtrl->GetCurrentIndex();
  RpItem *itemPtr = (RpItem *) m_listCtrl->GetCurrentItem();
  
  if (itemPtr == NULL)
    return;

  long rank = ::wxGetNumberFromUser(
    wxString::Format(_("Enter new ranking for year %d"),itemPtr->m_year), _("Rank. Pts."), _("Edit Ranking Points"),
      itemPtr->m_pts, 0, 0x7FFFFFFF, this);

  if (rank >= 0)
  {
    itemPtr->m_pts = rank;
  }

  m_listCtrl->Refresh();
}


void  CRpEdit::OnOK() 
{
  RpItem *itemPtr;
  int idx = 0;
  bool ret = true;

  RpStore rp;
  rp.GetConnectionPtr()->StartTransaction();

  ret &= rp.RemoveByPl(plID);

  while ( (itemPtr = (RpItem *) m_listCtrl->GetListItem(idx++)) )
  {
    if (itemPtr->m_pts == 0)
      continue;

    rp.plID = plID;
    rp.rpYear = itemPtr->m_year;
    rp.rpRankPts = itemPtr->m_pts;
    ret &= rp.Insert();
  }

  if (ret)
    rp.GetConnectionPtr()->Commit();
  else
    rp.GetConnectionPtr()->Rollback();

  CFormViewEx::OnOK();
}


