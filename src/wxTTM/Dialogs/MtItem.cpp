/* Copyright (C) 2020 Christoph Theis */

// Darstellung der Spiele

#include  "stdafx.h"

#include  "MtItem.h"
#include  "TmEntryStore.h"

#include  "StrUtils.h"
#include  "Rec.h"


MtItem::MtItem()
      : ListItem()
{
}


MtItem::MtItem(const MtEntry &rec, bool times)
      : ListItem(rec.mt.mtID)
{
  showTimes = times;

  SetValue(rec);
}


void  MtItem::SetValue(const MtEntry &rec)
{
  mt = rec;

  SetSchedule(rec.mt.mtPlace);
}


void  MtItem::SetSchedule(const MtRec::MtPlace &mtPlace)
{
  mt.mt.mtPlace = mtPlace;

  if (mtPlace.mtDateTime.year)
  {
    wxSprintf(date, "%02i.%02i.%02i", 
              mtPlace.mtDateTime.day,
              mtPlace.mtDateTime.month,
              mtPlace.mtDateTime.year % 100);
  }
  else
    memset(date, 0, sizeof(date));
  if (mtPlace.mtDateTime.hour)
  {
    wxSprintf(time, "%02i:%02i", 
              mtPlace.mtDateTime.hour,
              mtPlace.mtDateTime.minute);
  }
  else
    memset(time, 0, sizeof(time));
}


// -----------------------------------------------------------------------
int  MtItem::Compare(const ListItem *itemPtr, int col) const
{
  return (mt.mt.mtNr - ((MtItem *) itemPtr)->mt.mt.mtNr);
}


void  MtItem::DrawPair(wxDC *pDC, wxRect &rect, const MtEntry &mt)
{
  unsigned cW = pDC->GetTextExtent("M").GetWidth();

  wxRect rcA = rect;
  wxRect rcX = rect;
  wxRect rcSep = rect;

  rcA.SetRight(rect.GetLeft() + rect.GetWidth() / 2 - cW);
  rcSep.SetLeft(rcA.GetRight());

  rcX.SetLeft(rcA.GetRight() + 2 * cW);
  rcX.SetRight(rect.GetRight());
  rcSep.SetRight(rcX.GetLeft());

  if (mt.IsABye())
    DrawBye(pDC, rcA);
  else if (true && mt.mt.cpType == CP_INDIVIDUAL && mt.mt.mtReverse)
    DrawEntry(pDC, rcA, mt.tmX);
  else
    DrawEntry(pDC, rcA, mt.tmA);

  DrawString(pDC, rcSep, wxT("-"), wxALIGN_CENTER);

  if (mt.IsXBye())
    DrawBye(pDC, rcX);
  else if (true && mt.mt.cpType == CP_INDIVIDUAL && mt.mt.mtReverse)
    DrawEntry(pDC, rcX, mt.tmA);
  else
    DrawEntry(pDC, rcX, mt.tmX);
}


void MtItem::DrawColumn(wxDC *pDC, int col, wxRect &rect)
{
  wxRect  rc = rect;

  rc.Deflate(offset);

  switch (col)
  {
    case 0 :
      if (mt.mt.cpType == CP_INDIVIDUAL)
        DrawLong(pDC, rc, mt.mt.mtEvent.mtMS);
      else
        DrawLong(pDC, rc, mt.mt.mtNr);
      break;

    case 1 :      
      DrawPair(pDC, rc, mt);
      break;

    case 2 :
      if (mt.mt.mtEvent.mtLeg)
        DrawLong(pDC, rc, mt.mt.mtEvent.mtLeg);
      else
        DrawString(pDC, rc, "");
      break;

    case 3 :
      if (showTimes)
        DrawString(pDC, rc, date);
      else
      {
        if ( (mt.mt.cpType == CP_INDIVIDUAL) && mt.mt.mtReverse )
          DrawResult(pDC, rc, mt.mt.mtResX, mt.mt.mtResA);
        else
          DrawResult(pDC, rc, mt.mt.mtResA, mt.mt.mtResX);
      }
      break;

    case 4 :
      if (showTimes)
        DrawString(pDC, rc, time);
      break;

    case 5 :
      if (showTimes && mt.mt.mtPlace.mtTable)
        DrawLong(pDC, rc, mt.mt.mtPlace.mtTable);
      break;

    case 6 :
      if (showTimes && mt.mt.mtUmpire)
        DrawLong(pDC, rc, mt.mt.mtUmpire);
      break;
  }
}


void MtItem::DrawItem(wxDC *pDC, wxRect &rect)
{
}


bool  MtItem::DrawSeparatingLine()
{
  if ( (mt.mt.cpType == CP_DOUBLE) || (mt.mt.cpType == CP_MIXED) )
    return true;

  return false;
}


bool  MtItem::HasString(const wxString &str) const
{
  if (mt.tmA.HasString(str) || mt.tmX.HasString(str))
    return true;

  return false;
}