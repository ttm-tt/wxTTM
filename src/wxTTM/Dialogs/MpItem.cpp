/* Copyright (C) Christoph Theis 2022 */

#include  "stdafx.h"

#include  "MpItem.h"


MpItem::MpItem(const MpListRec &rec)
      : ListItem(rec.mpID, rec.mpName)
{
  mp = rec;
}


int  MpItem::Compare(const ListItem *itemPtr, int col) const
{
  const MpItem *mpItem = reinterpret_cast<const MpItem *>(itemPtr);
  if (!mpItem)
    return -1;

  switch (col)
  {
    case 0 :
      return wxStrcoll(mp.mpName, mpItem->mp.mpName);
    case 1 :
      return -wxStrcoll(mp.mpDesc, mpItem->mp.mpDesc);
    default :
      return ListItem::Compare(itemPtr, col);
  }
}


void  MpItem::DrawColumn(wxDC *pDC, int col, wxRect &rect)
{
  wxRect    rcColumn = rect;

  switch (col)
  {
    case 0 :
      DrawString(pDC, rcColumn, mp.mpName);
      break;

    case 1 :
      DrawString(pDC, rcColumn, mp.mpDesc);
      break;

    default :
      break;
  }
}


void  MpItem::DrawItem(wxDC *pDC, wxRect &rect)
{
  unsigned  cW = pDC->GetTextExtent("M").GetWidth();

  wxRect  rcName = rect;
  wxRect  rcDesc = rect;

  rcName.SetRight(rcName.GetLeft() + 5 * cW);
  rcDesc.SetLeft(rcName.GetRight());

  rcName.Deflate(offset, 0);
  rcDesc.Deflate(offset, 0);

  DrawString(pDC, rcName, mp.mpName);
  DrawString(pDC, rcDesc, mp.mpDesc);
}



