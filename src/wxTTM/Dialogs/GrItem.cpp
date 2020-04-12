/* Copyright (C) 2020 Christoph Theis */

// Darstellung von WB in Listen

#include  "stdafx.h"

#include  "GrItem.h"

#include "checked.xpm"
#include "unchecked.xpm"


GrItem::GrItem(const GrRec &rec)
      : ListItem(rec.grID, wxString::Format("%s %s", rec.grStage, rec.grName))
{
  gr = rec;
}


int  GrItem::Compare(const ListItem *itemPtr, int col) const
{
  const GrItem *grItem = reinterpret_cast<const GrItem *>(itemPtr);
  if (!grItem)
    return -1;

  switch (col)
  {
    case 0 :
      return wxStrcoll(gr.grName, grItem->gr.grName);

    case 1 :
      return wxStrcmp(gr.grDesc, grItem->gr.grDesc);

    case 2 :
    {
      int ret = wxStrcoll(gr.grStage, grItem->gr.grStage);

      // Wenn gleich (was immer der Fall sein kann), dann auf den Namen zurueckgreifen
      // So hat man eine definierte Sortierung, ohne auf sich auf die Reihenfolge in der DB zu verlassen
      if (ret == 0)
        ret = wxStrcoll(gr.grName, grItem->gr.grName);

      return ret;
    }

    case 3 :
      return gr.grHasNotes - grItem->gr.grHasNotes;

    default :
      return ListItem::Compare(itemPtr, col);
  }
}


void  GrItem::DrawColumn(wxDC *pDC, int col, wxRect &rect)
{
  static wxImage checkedImg(checked);
  static wxImage uncheckedImg(unchecked);

  wxRect    rcColumn = rect;

  switch (col)
  {
    case 0 :
      DrawString(pDC, rcColumn, gr.grName);
      break;

    case 1 :
      DrawString(pDC, rcColumn, gr.grDesc);
      break;

    case 2 :
      DrawString(pDC, rcColumn, gr.grStage);
      break;

    case 3 :
      if (gr.grHasNotes)
        DrawImage(pDC, rect, checkedImg);
      else
        DrawImage(pDC, rect, uncheckedImg);
      break;

    default :
      break;
  }
}


void  GrItem::DrawItem(wxDC *pDC, wxRect &rect)
{
  unsigned  cW = pDC->GetTextExtent("M").GetWidth();

  wxRect  rcName = rect;
  wxRect  rcDesc = rect;
  wxRect  rcStage = rect;

  rcName.SetRight(rcName.GetLeft() + 5 * cW);
  rcDesc.SetLeft(rcName.GetRight());
  rcDesc.SetRight(rcDesc.GetLeft() + rcDesc.GetWidth() / 2);
  rcStage.SetLeft(rcDesc.GetRight());

  rcName.Deflate(offset, 0);
  rcDesc.Deflate(offset, 0);
  rcStage.Deflate(offset, 0);

  DrawString(pDC, rcName, gr.grName);
  DrawString(pDC, rcDesc, gr.grDesc);
  DrawString(pDC, rcStage, gr.grStage);
}


bool GrItem::HasString(const wxString & str) const
{
  if ( !str || wxStrnicmp(gr.grName, str, str.length()) )
    return false;
    
  return true;
}