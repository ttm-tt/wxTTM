/* Copyright (C) 2020 Christoph Theis */

// Darstellung der Nominierung

#include  "stdafx.h"

#include  "NmItem.h"

#include  "Rec.h"


NmItem::NmItem(const wxString &name_, bool b)
      : ListItem()
{
  name = name_;
  showNaName = b;
}


NmItem::NmItem(const NmEntry &rec, const wxString &name_, bool b)
      : ListItem(rec.tmID)
{
  name = name_;
  showNaName = b;
  SetValue(rec);
}


void  NmItem::SetValue(const NmEntry &rec)
{
  nm = rec;
}


bool  NmItem::AddPlayer(const LtEntry &pl)
{
  if (nm.team.cpType == CP_SINGLE)
  {
    if (!nm.ltA)
    {
      nm.team.pl.SetPlayer(pl);
      nm.ltA = pl.ltID;
    }
    else
      return false;
  }
  else
  {
    if (!nm.ltA)
    {
      nm.team.pl.SetPlayer(pl);
      nm.ltA = pl.ltID;
    }
    else if (!nm.ltB)
    {
      // In case if mixed put the girl down
      if (nm.team.pl.psSex == SEX_FEMALE && pl.psSex == SEX_MALE)
      {
        nm.team.bd = nm.team.pl;
        nm.ltB = nm.ltA;

        nm.team.pl.SetPlayer(pl);
        nm.ltA = pl.ltID;
      }
      else
      {
        nm.team.bd.SetPlayer(pl);
        nm.ltB = pl.ltID;
      }
    }
    else
      return false;
  }
  
  return true;
}


bool NmItem::RemovePlayer()
{
  if (nm.team.cpType == CP_SINGLE)
  {
    if (nm.ltA)
    {
      memset(&nm.team.pl, 0, sizeof(TmPlayer));
      nm.ltA = 0;
    }
    else
      return false;
  }
  else
  {
    if (nm.ltB)
    {
      memset(&nm.team.bd, 0, sizeof(TmPlayer));
      nm.ltB = 0;
    }
    else if (nm.ltA)
    {
      memset(&nm.team.pl, 0, sizeof(TmPlayer));
      nm.ltA = 0;
    }
    else
      return false;
  }
  
  return true;
}


// -----------------------------------------------------------------------
void NmItem::MeasureItem(LPMEASUREITEMSTRUCT lpMIS) 
{
  if ( (nm.team.cpType == CP_DOUBLE) || (nm.team.cpType == CP_MIXED) )
    lpMIS->itemHeight *= 2;
}


int  NmItem::Compare(const ListItem *itemPtr, int col) const
{
  if (nm.team.cpType == ((NmItem *) itemPtr)->nm.team.cpType)
    return nm.nmNr - ((NmItem *) itemPtr)->nm.nmNr;
  else
    return (nm.team.cpType == CP_SINGLE ? -1 : +1);
}



void NmItem::DrawItem(wxDC *pDC, wxRect &rect)
{
  unsigned  cW = pDC->GetTextExtent("M").GetWidth();

  wxRect  rcType = rect;
  wxRect  rcEntry = rect;

  rcEntry.SetLeft(rect.GetLeft() + 3 * cW);
  rcType.SetRight(rcEntry.GetLeft());

  DrawString(pDC, rcType, name);
  DrawEntry(pDC, rcEntry, nm, showNaName);
}


bool  NmItem::DrawSeparatingLine()
{
  return true;
}


