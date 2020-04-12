/* Copyright (C) 2020 Christoph Theis */

// Darstellung der Setzung

#include  "stdafx.h"

#include  "StItem.h"

#include  "Rec.h"

#include "checked.xpm"
#include "unchecked.xpm"


StItem::StItem()
      : TmItem()
{
  boldStNr = false;
}


StItem::StItem(const StRec &rec, short cpType)
      : TmItem(rec.stID)
{
  st = rec;
  entry.tmID = rec.tmID;
  entry.team.cpType = cpType;
  
  boldStNr = false;
}


StItem::StItem(const StEntry &rec)
      : TmItem(rec.st.stID)
{
  st = rec.st;
  entry = rec;
  
  boldStNr = false;
}


void  StItem::SetValue(const StEntry &rec)
{
  st = rec.st;
  entry = rec;
}


// -----------------------------------------------------------------------
int  StItem::Compare(const ListItem *itemPtr, int col) const
{
  return (st.stNr - ((StItem *) itemPtr)->st.stNr);
}


void StItem::DrawColumn(wxDC *pDC, int col, wxRect &rect)
{
  static wxImage checkedImg(checked);
  static wxImage uncheckedImg(unchecked);

  switch (col)
  {
    case 0 :
    {
      if (boldStNr)
      {
        wxFont oldFont = pDC->GetFont();
        wxFont boldFont = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
        boldFont.MakeBold();
        pDC->SetFont(boldFont);
        
        DrawLong(pDC, rect, st.stNr);
        
        pDC->SetFont(oldFont);
      }
      else
      {
        DrawLong(pDC, rect, st.stNr);
      }
      break;
    }

    case 1 :
      DrawImage(pDC, rect, st.stSeeded ? checkedImg : uncheckedImg);
      break;

    case 2 :
      DrawImage(pDC, rect, st.stGaveup ? checkedImg : uncheckedImg);
      break;

    case 3 :
      DrawImage(pDC, rect, st.stDisqu ? checkedImg : uncheckedImg);
      break;

    case 4 :
      DrawImage(pDC, rect, st.stNocons ? checkedImg : uncheckedImg);
      break;

    case 5 :
      TmItem::DrawColumn(pDC, 0, rect);
      break;
  }
}


void StItem::DrawItem(wxDC *pDC, wxRect &rect)
{
}


bool  StItem::DrawSeparatingLine()
{
  if ( (entry.team.cpType == CP_DOUBLE) || (entry.team.cpType == CP_MIXED) )
    return true;

  return false;
}


