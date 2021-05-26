/* Copyright (C) 2020 Christoph Theis */

// Erweiterung von StItem um Ergebnis der Tabellenberechnung

#include  "stdafx.h"

#include  "TbItem.h"
#include  "TT32App.h"

#include  "TbEntryStore.h"

#include  "Rec.h"

#include "checked.xpm"
#include "unchecked.xpm"

TbItem::TbItem() : StItem() 
{
  memset(&result, 0, sizeof(result));
}


TbItem::TbItem(const StRec &rec, short type)
      : StItem(rec, type)
{
  memset(&result, 0, sizeof(result));
}


TbItem::TbItem(const StEntry &rec)
      : StItem(rec)
{
  memset(&result, 0, sizeof(result));
}

TbItem::TbItem(const TbEntry &rec)
  : StItem(rec)
{
  result.pos = rec.result.pos;
  result.nrMatches = rec.result.nrMatches;
  result.matchPoints = rec.result.matchPoints;
  result.points[0] = rec.result.points[0];
  result.points[1] = rec.result.points[1];
  result.matches[0] = rec.result.matches[0];
  result.matches[1] = rec.result.matches[1];
  result.sets[0] = rec.result.sets[0];
  result.sets[1] = rec.result.sets[1];
  result.balls[0] = rec.result.balls[0];
  result.balls[1] = rec.result.balls[1];
}



int  TbItem::Compare(const ListItem *itemPtr, int col) const
{
  if (col == 8)
  {
    // Unterschiedliche Positionen, aber Platz 0 ans Ende ruecken
    int myPos = (st.stPos ? st.stPos : result.pos);
    int itemPos = ((TbItem *) itemPtr)->st.stPos ? 
                  ((TbItem *) itemPtr)->st.stPos : 
                  ((TbItem *) itemPtr)->result.pos;
    
    if (myPos != itemPos)
    {
      if (myPos == 0)
        return +1;
      else if ( itemPos == 0)
        return -1;
      else 
        return myPos - itemPos;
    }
      
    return StItem::Compare(itemPtr, col);
  }
  else
    return StItem::Compare(itemPtr, col);
}


void TbItem::DrawColumn(wxDC *pDC, int col, wxRect &rect)
{
  static wxImage checkedImg(checked);
  static wxImage uncheckedImg(unchecked);

  // Keine Ergebnisse oder Plazierung, wenn Freilos
  if (col >= 6 && entry.tmID == 0)
    return;

  wxColor oldFg = pDC->GetTextForeground();
  if (hidden)
    pDC->SetTextForeground(*wxLIGHT_GREY);
    
  switch (col)
  {
    case 0 :
    case 1 :
    case 2 :
    case 3 :
    case 4 :
    case 5 :
      StItem::DrawColumn(pDC, col, rect);
      break;

    case 6 :  // Mt Pts
      if (CTT32App::instance()->GetTable() == TT_DTTB)
        DrawResult(pDC, rect, result.points[0], result.points[1]);
      else
        DrawLong(pDC, rect, result.matchPoints);
      break;

    case 7 : // Result (Matches / Games)
      if (entry.team.cpType == CP_TEAM)
        DrawResult(pDC, rect, result.matches[0], result.matches[1]);
      else
        DrawResult(pDC, rect, result.sets[0], result.sets[1]);
      break;

    case 8: // Games
      DrawResult(pDC, rect, result.sets[0], result.sets[1]);
      break;

    case 9: // Points
      DrawResult(pDC, rect, result.balls[0], result.balls[1]);
      break;

    case 10 :
    {
      if (st.stPos == result.pos)
        DrawLong(pDC, rect, result.pos);
      else
      {
        wxColor oldColor = pDC->GetTextForeground();
        pDC->SetTextForeground(wxColor(255, 0, 0));
        
        if (st.stPos == 0)
          DrawLong(pDC, rect, result.pos);
        else
          DrawLong(pDC, rect, st.stPos);
        
        pDC->SetTextForeground(oldColor);
      }
      break;
    }

    case 11 :
      DrawImage(pDC, rect, hidden ? uncheckedImg : checkedImg);
      break;
  }

  pDC->SetTextForeground(oldFg);
}


void TbItem::DrawItem(wxDC *pDC, wxRect &rect)
{
  StItem::DrawItem(pDC, rect);
}



// -----------------------------------------------------------------------
void  TbItem::PopResult()
{
  TbEntry::Result *resPtr = resultStack.top();
  resultStack.pop();  // Liefert keinen Wert

  memcpy(&result, resPtr, sizeof(TbEntry::Result));
  delete resPtr;

}


void  TbItem::PushResult()
{
  TbEntry::Result *resPtr = new TbEntry::Result;
  memcpy(resPtr, &result, sizeof(TbEntry::Result));
  memset(&result, 0, sizeof(TbEntry::Result));

  resultStack.push(resPtr);
}