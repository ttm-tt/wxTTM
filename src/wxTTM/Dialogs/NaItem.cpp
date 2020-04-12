/* Copyright (C) 2020 Christoph Theis */

// Darstellung von Nationen in Listen

#include  "stdafx.h"

#include  "NaItem.h"


NaItem::NaItem(const NaRec &rec)
      : ListItem(rec.naID, rec.naName)
{
  na = rec;
}


int  NaItem::Compare(const ListItem *itemPtr, int col) const
{
  const NaItem *naItem = reinterpret_cast<const NaItem *>(itemPtr);
  if (!naItem)
    return -1;

  switch (col)
  {
    case 0 :
      return wxStrcoll(na.naName, naItem->na.naName);
    case 1 :
      return wxStrcoll(na.naDesc, naItem->na.naDesc);
    case 2 :
      return wxStrcoll(na.naRegion, naItem->na.naRegion);
    default :
      return ListItem::Compare(itemPtr, col);
  }
}


void  NaItem::DrawColumn(wxDC *pDC, int col, wxRect &rect)
{
  wxRect    rcColumn = rect;

  switch (col)
  {
    case 0 :
      DrawString(pDC, rcColumn, na.naName);
      break;

    case 1 :
      DrawString(pDC, rcColumn, na.naDesc);
      break;

    case 2 :
      DrawString(pDC, rcColumn, na.naRegion);
      break;

    default :
      break;
  }
}


void  NaItem::DrawItem(wxDC *pDC, wxRect &rect)
{
  unsigned  cW = pDC->GetTextExtent("M").GetWidth();

  wxRect  rcName = rect;
  wxRect  rcDesc = rect;

  rcName.SetRight(rcName.GetLeft() + 5 * cW);
  rcDesc.SetLeft(rcName.GetRight());

  rcName.Deflate(offset, 0);
  rcDesc.Deflate(offset, 0);

  DrawString(pDC, rcName, na.naName);
  DrawString(pDC, rcDesc, na.naDesc);
}


bool NaItem::HasString(const wxString &str) const
{
  if ( !str || wxStrnicmp(na.naName, str, str.length()) )
    return false;
    
  return true;
}