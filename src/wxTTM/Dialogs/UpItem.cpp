/* Copyright (C) 2020 Christoph Theis */

// Darstellung von Schiedsrichtern in Listen

#include  "stdafx.h"

#include  "UpItem.h"
#include  "Rec.h"


UpItem::UpItem()
      : ListItem()
{
}


UpItem::UpItem(const UpRec &rec)
      : ListItem(rec.upID)
{
  up = rec;
}


bool UpItem::HasString(const wxString &str) const
{
  wxChar tmp[10];
  _ltot(up.upNr, tmp, 10);
  if (!wxStrnicmp(tmp, str, str.length()))
    return true;

  if (!wxStrnicmp(up.psName.psLast, str, str.length()))
    return true;

  return false;
}


int  UpItem::Compare(const ListItem *itemPtr, int col) const
{
  const UpItem *upItem = reinterpret_cast<const UpItem *>(itemPtr);
  if (!upItem)
    return -1;

  int  ret;

  switch (col)
  {
    case 0 :
      return up.upNr - upItem->up.upNr;

    case 1 :
      if ( (ret = wxStrcoll(up.psName.psLast, upItem->up.psName.psLast)) )
        return ret;
      return wxStrcoll(up.psName.psFirst, upItem->up.psName.psFirst);

    case 2 :
      return wxStrcoll(up.naName, upItem->up.naName);

    case 3 :
      return (up.psSex - upItem->up.psSex);
      
    default :
      return ListItem::Compare(itemPtr, col);
  }
}


void  UpItem::DrawColumn(wxDC *pDC, int col, wxRect &rect)
{
  wxRect    rcColumn = rect;
  wxChar    buf[130];

  switch (col)
  {
    case 0 :
      if (up.upNr)
        DrawLong(pDC, rcColumn, up.upNr);
      break;

    case 1 :
      if (*up.psName.psLast)
      {
        if (!*up.psName.psFirst ||
            wxStrlen(up.psName.psFirst) == 1 && isspace(*up.psName.psFirst))
          wxSprintf(buf, "%s", up.psName.psLast);
        else if (*up.psName.psFirst)
          wxSprintf(buf, "%s, %s", up.psName.psLast, up.psName.psFirst);

        DrawString(pDC, rcColumn, buf);
      }
      break;

    case 2 :
      DrawString(pDC, rcColumn, up.naName);
      break;

    case 3 :
      if (up.psSex == SEX_MALE)
        DrawStringCentered(pDC, rect, wxT("M"));
      else if (up.psSex == SEX_FEMALE)
        DrawStringCentered(pDC, rect, wxT("F"));
        
      break;

    default :
      break;
  }
}


void  UpItem::DrawItem(wxDC *pDC, wxRect &rect)
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
  wxSprintf(buf, "%s, %s", up.psName.psLast, up.psName.psFirst);

  if (up.upNr)
    DrawLong(pDC, rcNr, up.upNr);
  if (*up.psName.psLast)
  {
    DrawString(pDC, rcName, buf);
    DrawString(pDC, rcAssoc, up.naName);
  }
}

