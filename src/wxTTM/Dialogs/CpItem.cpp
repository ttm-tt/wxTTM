/* Copyright (C) 2020 Christoph Theis */

// Darstellung von WB in Listen

#include  "stdafx.h"

#include  "CpItem.h"


CpItem::CpItem(const CpRec &rec)
      : ListItem(rec.cpID, rec.cpName)
{
  cp = rec;
}


int  CpItem::Compare(const ListItem *itemPtr, int col) const
{
  const CpItem *cpItem = reinterpret_cast<const CpItem *>(itemPtr);
  if (!cpItem)
    return -1;

  switch (col)
  {
    case 0 :
      return wxStrcoll(cp.cpName, cpItem->cp.cpName);
    case 1 :
      return wxStrcoll(cp.cpDesc, cpItem->cp.cpDesc);
    case 2 :
      return -(cp.cpYear - cpItem->cp.cpYear);
    case 3 :
      return (cp.cpType - cpItem->cp.cpType);
    case 4 :
      return cp.cpSex - cpItem->cp.cpSex;
    default :
      return ListItem::Compare(itemPtr, col);
  }
}


void  CpItem::DrawColumn(wxDC *pDC, int col, wxRect &rect)
{
  static const char *strType[] = {"S", "D", "X", "T"};
  static const char *strSex[]  = {"M", "F", "X"};

  wxRect    rcColumn = rect;

  switch (col)
  {
    case 0 :
      DrawString(pDC, rcColumn, cp.cpName);
      break;

    case 1 :
      DrawString(pDC, rcColumn, cp.cpDesc);
      break;

    case 2 :
      DrawLong(pDC, rcColumn, cp.cpYear);
      break;

    case 3 :
      if (cp.cpType >= 1 && cp.cpType <= 4)
        DrawString(pDC, rcColumn, strType[cp.cpType - 1]);
      break;

    case 4 :
      if (cp.cpSex >= 1 && cp.cpSex <= 4)
        DrawString(pDC, rcColumn, strSex[cp.cpSex - 1]);
      break;

    default :
      break;
  }
}


void  CpItem::DrawItem(wxDC *pDC, wxRect &rect)
{
  unsigned  cW = pDC->GetTextExtent("M").GetWidth();

  wxRect  rcName = rect;
  wxRect  rcDesc = rect;

  rcName.SetRight(rcName.GetLeft() + 5 * cW);
  rcDesc.SetLeft(rcName.GetRight());

  rcName.Deflate(offset, 0);
  rcDesc.Deflate(offset, 0);

  DrawString(pDC, rcName, cp.cpName);
  DrawString(pDC, rcDesc, cp.cpDesc);
}


bool CpItem::HasString(const wxString &str) const
{
  if ( !str || wxStrnicmp(cp.cpName, str, str.length()) )
    return false;
    
  return true;
}