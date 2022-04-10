/* Copyright (C) 2020 Christoph Theis */

// Darstellung von Spielern in Listen

#include  "stdafx.h"

#include  "LtItem.h"
#include  "Rec.h"


LtItem::LtItem(bool b)
      : PlItem()
{
  showNaName = b;
}


LtItem::LtItem(const LtEntry &rec, bool b)
      : PlItem(rec)
{
  m_id = rec.ltID;
  lt = rec;
  showNaName = b;
}


void  LtItem::DrawItem(wxDC *pDC, wxRect &rect)
{
  DrawPlayer(pDC, rect, pl, showNaName);
}


void LtItem::DrawColumn(wxDC *pDC, int col, wxRect &rect)
{
  switch (col)
  {
    case 6 : // Rank Pts
      if (lt.ltRankPts)
        DrawLong(pDC, rect, lt.ltRankPts);
      break;

    case 7 : // Region
      DrawString(pDC, rect, pl.naRegion);
      break;

    default :
      PlItem::DrawColumn(pDC, col, rect);
  }
}



