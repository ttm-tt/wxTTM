/* Copyright (C) 2020 Christoph Theis */

// Darstellung von Spielsystemem in Listen

#include  "stdafx.h"

#include  "SyItem.h"


SyItem::SyItem(const SyListRec &rec)
      : ListItem(rec.syID, rec.syName)
{
  sy = rec;
}


int  SyItem::Compare(const ListItem *itemPtr, int col) const
{
  const SyItem *syItem = reinterpret_cast<const SyItem *>(itemPtr);
  if (!syItem)
    return -1;

  switch (col)
  {
    case 0 :
      return wxStrcoll(sy.syName, syItem->sy.syName);
    case 1 :
      return -wxStrcoll(sy.syDesc, syItem->sy.syDesc);
    default :
      return ListItem::Compare(itemPtr, col);
  }
}


void  SyItem::DrawColumn(wxDC *pDC, int col, wxRect &rect)
{
  wxRect    rcColumn = rect;

  switch (col)
  {
    case 0 :
      DrawString(pDC, rcColumn, sy.syName);
      break;

    case 1 :
      DrawString(pDC, rcColumn, sy.syDesc);
      break;

    default :
      break;
  }
}


void  SyItem::DrawItem(wxDC *pDC, wxRect &rect)
{
  unsigned  cW = pDC->GetTextExtent("M").GetWidth();

  wxRect  rcName = rect;
  wxRect  rcDesc = rect;

  rcName.SetRight(rcName.GetLeft() + 5 * cW);
  rcDesc.SetLeft(rcName.GetRight());

  rcName.Deflate(offset, 0);
  rcDesc.Deflate(offset, 0);

  DrawString(pDC, rcName, sy.syName);
  DrawString(pDC, rcDesc, sy.syDesc);
}
