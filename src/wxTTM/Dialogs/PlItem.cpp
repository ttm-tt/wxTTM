/* Copyright (C) 2020 Christoph Theis */

// Darstellung von Spielern in Listen

#include  "stdafx.h"

#include  "PlItem.h"
#include  "Rec.h"

#include "checked.xpm"
#include "unchecked.xpm"


PlItem::PlItem()
      : ListItem()
{
}


PlItem::PlItem(const PlRec &rec)
      : ListItem(rec.plID)
{
  pl = rec;
}


bool PlItem::HasString(const wxString &str) const
{
  wxChar tmp[10];
  _ltot(pl.plNr, tmp, 10);
  if (!wxStrcoll(tmp, str))
    return true;

  if (!_wcsnicoll(pl.psName.psLast, str.wc_str(), str.Length()))
    return true;

  return false;
}


int  PlItem::Compare(const ListItem *itemPtr, int col) const
{
  const PlItem *plItem = reinterpret_cast<const PlItem *>(itemPtr);
  if (!plItem)
    return -1;

  int  ret;

  switch (col)
  {
    case 0 :
      return pl.plNr - plItem->pl.plNr;

    case 1 :
      if ( (ret = wxStrcoll(pl.psName.psLast, plItem->pl.psName.psLast)) )
        return ret;
      return wxStrcoll(pl.psName.psFirst, plItem->pl.psName.psFirst);

    case 2 :
      return wxStrcoll(pl.naName, plItem->pl.naName);

    case 3 :
      return (pl.psSex - plItem->pl.psSex);
      
    case 4 :
      return pl.psBirthday - plItem->pl.psBirthday;

    case 5 :
      return wxStrcoll(pl.plExtID, plItem->pl.plExtID);

    case 6 :
      return -(pl.plRankPts - plItem->pl.plRankPts);

    case 7:
      return wxStrcoll(pl.naRegion, plItem->pl.naRegion);

    case 8 :
      return -(pl.psHasNote - plItem->pl.psHasNote);

    default :
      return ListItem::Compare(itemPtr, col);
  }
}


void  PlItem::DrawColumn(wxDC *pDC, int col, wxRect &rect)
{
  static wxImage checkedImg(checked);
  static wxImage uncheckedImg(unchecked);

  wxRect    rcColumn = rect;
  wxChar    buf[130];


  switch (col)
  {
    case 0 :
      if (pl.plNr)
        DrawLong(pDC, rcColumn, pl.plNr);
      break;

    case 1 :
      if (*pl.psName.psLast)
      {
        if (!*pl.psName.psFirst ||
            wxStrlen(pl.psName.psFirst) == 1 && isspace(*pl.psName.psFirst))
          wxSprintf(buf, "%s", pl.psName.psLast);
        else if (*pl.psName.psFirst)
          wxSprintf(buf, "%s, %s", pl.psName.psLast, pl.psName.psFirst);

        DrawString(pDC, rcColumn, buf);
      }
      break;

    case 2 :
      DrawString(pDC, rcColumn, pl.naName);
      break;

    case 3 :
      if (pl.psSex == SEX_MALE)
        DrawStringCentered(pDC, rect, wxT("M"));
      else if (pl.psSex == SEX_FEMALE)
        DrawStringCentered(pDC, rect, wxT("F"));
        
      break;

    case 4 :
      if (pl.psBirthday)
        DrawLong(pDC, rect, pl.psBirthday);

      break;

    case 5 :
      if (*pl.plExtID)
        DrawString(pDC, rect, pl.plExtID);
      break;

    case 6 :
      if (pl.plRankPts)
        DrawLong(pDC, rect, pl.plRankPts);
      break;

    case 7:
      DrawString(pDC, rcColumn, pl.naRegion);
      break;

    case 8 :
      if (pl.psHasNote)
        DrawImage(pDC, rect, checkedImg);
      else
        DrawImage(pDC, rect, uncheckedImg);
      break;

    default :
      break;
  }
}


void  PlItem::DrawItem(wxDC *pDC, wxRect &rect)
{
  unsigned  cW = pDC->GetTextExtent("M").GetWidth();

  wxRect  rcNr    = rect;
  wxRect  rcName  = rect;
  wxRect  rcAssoc = rect;

  rcName.SetLeft(rcNr.GetLeft() + 5 * cW);
  rcNr.SetRight(rcName.GetLeft());

  rcAssoc.SetLeft(rcAssoc.GetRight() - 5 * cW);
  rcName.SetRight(rcAssoc.GetLeft());

  rcNr.Deflate(offset, 0);
  rcName.Deflate(offset, 0);
  rcAssoc.Deflate(offset, 0);

  wxChar  buf[130];
  wxSprintf(buf, "%s, %s", pl.psName.psLast, pl.psName.psFirst);

  if (pl.plNr)
    DrawLong(pDC, rcNr, pl.plNr);
  if (*pl.psName.psLast)
  {
    DrawString(pDC, rcName, buf);
    DrawString(pDC, rcAssoc, pl.naName);
  }
}


// ==========================================================================
void PlItemEx::DrawItem(wxDC *pDC, wxRect &rect)
{
  unsigned  cW = pDC->GetTextExtent("M").GetWidth();

  wxRect left = rect;
  wxRect right = rect;
  
  right.SetLeft(rect.GetRight() - 2 * cW);
  left.SetRight(right.GetLeft());
  
  PlItem::DrawItem(pDC, left);
  DrawString(pDC, right, pl.psSex == SEX_MALE ? wxT("M") : wxT("F"));   
}