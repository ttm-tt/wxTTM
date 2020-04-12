/* Copyright (C) 2020 Christoph Theis */

// Darstellung von Spielmodi in Listen

#include  "stdafx.h"

#include  "MdItem.h"
#include  "Rec.h"


MdItem::MdItem(const MdListRec &rec)
      : ListItem(rec.mdID, rec.mdName)
{
  md = rec;
  modus = MOD_RR;
}


MdItem::MdItem(short mod)
{
  modus = mod;
  
  switch (mod)
  {
    case MOD_SKO :
      wxStrcpy(md.mdDesc, _("Knock Out"));
      break;
      
    case MOD_DKO :
      wxStrcpy(md.mdDesc, _("Double Knock Out"));
      break;
      
    case MOD_MDK :
      wxStrcpy(md.mdDesc, _("Modified Double Knock Out"));
      break;
      
    case MOD_PLO :
      wxStrcpy(md.mdDesc, _("Play Off"));
      break;
      
    default :
      break;
  }
}


int  MdItem::Compare(const ListItem *itemPtr, int col) const
{
  const MdItem *mdItem = reinterpret_cast<const MdItem *>(itemPtr);
  if (!mdItem)
    return -1;

  switch (col)
  {
    case 0 :
      return wxStrcoll(md.mdName, mdItem->md.mdName);
    case 1 :
      return -wxStrcoll(md.mdDesc, mdItem->md.mdDesc);
    default :
      return ListItem::Compare(itemPtr, col);
  }
}


void  MdItem::DrawColumn(wxDC *pDC, int col, wxRect &rect)
{
  wxRect    rcColumn = rect;

  switch (col)
  {
    case 0 :
      DrawString(pDC, rcColumn, md.mdName);
      break;

    case 1 :
      DrawString(pDC, rcColumn, md.mdDesc);
      break;

    default :
      break;
  }
}


void  MdItem::DrawItem(wxDC *pDC, wxRect &rect)
{
  if (modus == MOD_RR)
  {
    unsigned  cW = pDC->GetTextExtent("M").GetWidth();

    wxRect  rcName = rect;
    wxRect  rcDesc = rect;

    rcName.SetRight(rcName.GetLeft() + 5 * cW);
    rcDesc.SetLeft(rcName.GetRight());

    rcName.Deflate(offset, 0);
    rcDesc.Deflate(offset, 0);

    DrawString(pDC, rcName, md.mdName);
    DrawString(pDC, rcDesc, md.mdDesc);
  }
  else
  {
    DrawString(pDC, rect, md.mdDesc);
  }
}
