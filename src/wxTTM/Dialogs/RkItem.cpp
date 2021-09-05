/* Copyright (C) 2020 Christoph Theis */

// Darstellung des Ranking

#include  "stdafx.h"

#include  "RkItem.h"

#include  "Rec.h"


RkItem::RkItem()
      : TmItem()
{
}


RkItem::RkItem(const RkEntry &rec)
      : TmItem(rec.tmID)
{
  rk = rec.rk;
  entry = rec;
}


void  RkItem::SetValue(const RkEntry &rec)
{
  rk = rec.rk;
  entry = rec;
}


// -----------------------------------------------------------------------
int  RkItem::Compare(const ListItem *itemPtr, int col) const
{
  const RkItem *tmp = (const RkItem *) itemPtr;

  switch (col)
  {
    case 4 :
    {
      // If one has Int'l rank:
      if (rk.rkIntlRank)
      {
        if (tmp->rk.rkIntlRank)
          return rk.rkIntlRank - tmp->rk.rkIntlRank;
        else
          return -1;
      }
      else if (tmp->rk.rkIntlRank)
        return +1;
    }

    // Weder dieses noch das andere item hat Int'l Rank: fall through
    case 5 :
    {
      double ret = tmp->entry.rankPts - entry.rankPts;

      if (ret == 0)
        ret = tmp->rk.rkDirectEntry - rk.rkDirectEntry;

      if (ret == 0)
      {
        if (rk.rkIntlRank && tmp->rk.rkIntlRank)
          ret = rk.rkIntlRank - tmp->rk.rkIntlRank;
        else if (rk.rkIntlRank)
          ret = -1;
        else if (tmp->rk.rkIntlRank)
          ret = +1;
        else
          ret = 0;
      }

      if (ret == 0)
        ret = rk.naID - tmp->rk.naID;

      if (ret == 0)
        ret = rk.rkNatlRank - tmp->rk.rkNatlRank;

      return ret;
    }

    default :
      return (rk.rkNatlRank - tmp->rk.rkNatlRank);
  }
}


void RkItem::DrawColumn(wxDC *pDC, int col, wxRect &rect)
{
  switch (col)
  {
    case 0 :
      TmItem::DrawColumn(pDC, 0, rect);
      break;

    case 1 : 
      DrawString(pDC, rect, (rk.rkDirectEntry ? wxT("D") : wxT("Q")));
      break;

    case 2 :
      DrawLong(pDC, rect, rk.rkNatlRank);
      break;
      
    case 3 :
      DrawLong(pDC, rect, rk.rkIntlRank);   
      break;

    case 4 :
      DrawDouble(pDC, rect, entry.rankPts);
      break;
  }
}


void RkItem::DrawItem(wxDC *pDC, wxRect &rect)
{
}


bool  RkItem::DrawSeparatingLine()
{
  if ( (entry.team.cpType == CP_DOUBLE) || (entry.team.cpType == CP_MIXED) )
    return true;

  return false;
}

wxString RkItem::BeginEditColumn(int col) const
{
  wxString str;
  if (col == 3)
    str = wxString::Format("%d", rk.rkIntlRank);
    
  return str;
}

void RkItem::EndEditColumn(int col, const wxString &value)
{
  const wxChar *str = value.t_str();
  wxChar *endPtr = NULL;
  int rank = wxStrtod(str, &endPtr);
  
  if (!*endPtr)
    rk.rkIntlRank = rank;
}
