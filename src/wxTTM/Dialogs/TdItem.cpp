/* Copyright (C) 2020 Christoph Theis */

// Darstellung von Spielern in Listen

#include  "stdafx.h"

#include  "TdItem.h"


TdItem::TdItem()
      : TmItem()
{
}


TdItem::TdItem(long id)
      : TmItem(id)
{
}


TdItem::TdItem(const TmEntry &rec)
      : TmItem(rec)
{
}


void  TdItem::DrawColumn(wxDC *pDC, int col, wxRect &rect)
{
  wxRect    rcColumn = rect;

  switch (col)
  {
    case 0 :
      DrawString(pDC, rcColumn, entry.team.tm.tmName);
      break;

    case 1 :
      DrawString(pDC, rcColumn, entry.team.tm.tmDesc);
      break;

    case 2 :
      DrawString(pDC, rcColumn, entry.team.tm.naName);
      break;

    case 3:
      DrawString(pDC, rcColumn, entry.cpName);
      break;

    default :
      break;
  }
}


