/* Copyright (C) 2020 Christoph Theis */

// Darstellung von Teams in Listen
#include  "stdafx.h"

#include  "TmItem.h"

#include  "Rec.h"


TmItem::TmItem()
      : ListItem(), entry()
{
}


TmItem::TmItem(long id)
      : ListItem(id), entry()
{
}


TmItem::TmItem(const TmEntry &tm)
      : ListItem(tm.team.cpType == CP_GROUP ? tm.team.gr.xxStID : tm.tmID)
{
  entry = tm;
}



void  TmItem::DrawItem(wxDC *pDC, wxRect &rect)
{
  wxRect  rc = rect;

  if (entry.team.cpType)
    DrawEntry(pDC, rc, entry);
}


void  TmItem::DrawColumn(wxDC *pDC, int col, wxRect &rect)
{
  if (entry.team.cpType)
    DrawEntry(pDC, rect, entry);
}


int  TmItem::Compare(const ListItem *itemPtr, int col) const
{
  if (!itemPtr)
    return -1;
    
  TmItem *tmItem = (TmItem *) itemPtr;
    
  if (entry.team.cpType != tmItem->entry.team.cpType)
    return entry.team.cpType - tmItem->entry.team.cpType;

  switch (entry.team.cpType)
  {
    case CP_SINGLE :
      switch (col)
      {
        case 0 :
          return (entry.team.pl.plNr - 
                  ((TmItem *) itemPtr)->entry.team.pl.plNr);
        case 1 :
          return (wxStrcoll(entry.team.pl.psName.psLast, 
                         ((TmItem *) itemPtr)->entry.team.pl.psName.psLast));
                         
        case 2 :
          return (wxStrcoll(entry.team.pl.naName, 
                         ((TmItem *) itemPtr)->entry.team.pl.naName));
                         
        default :
          return 0;
      }                                                  
    
    case CP_DOUBLE :
      switch (col)
      {
        case 0 :
          return (entry.team.pl.plNr - 
                  ((TmItem *) itemPtr)->entry.team.pl.plNr);
                  
        case 1 :
          return (wxStrcoll(entry.team.pl.psName.psLast, 
                         ((TmItem *) itemPtr)->entry.team.pl.psName.psLast));
                         
        case 2 :
          return (wxStrcoll(entry.team.pl.naName, 
                         ((TmItem *) itemPtr)->entry.team.pl.naName));
                         
        default :
          return 0;
      }
    
    case CP_MIXED :
      switch (col)
      {
        case 0 :
          return (entry.team.pl.plNr - 
                  ((TmItem *) itemPtr)->entry.team.pl.plNr);
                  
        case 1 :
          return (wxStrcoll(entry.team.pl.psName.psLast, 
                         ((TmItem *) itemPtr)->entry.team.pl.psName.psLast));
                         
        case 2 :
          return (wxStrcoll(entry.team.pl.naName, 
                         ((TmItem *) itemPtr)->entry.team.pl.naName));
                         
        default :
          return 0;
      }                  

    case CP_TEAM :
      switch (col)
      {
        case 0 :
          return wxStrcoll(entry.team.tm.tmName, 
                        ((TmItem *) itemPtr)->entry.team.tm.tmName);

        case 1 :
          return wxStrcoll(entry.team.tm.tmDesc, 
                        ((TmItem *) itemPtr)->entry.team.tm.tmDesc);

        case 2 :
          return (wxStrcoll(entry.team.tm.naName, 
                         ((TmItem *) itemPtr)->entry.team.tm.naName));
                         
        default :
          return 0;
      }  
                      
    case CP_GROUP :
    {
      // TBD an das Ende
      if (entry.team.gr.grID == 0)
        return +1;
        
      if (tmItem->entry.team.gr.grID == 0)
        return -1;
        
      int res = wxStrcoll(
          entry.team.gr.grName, tmItem->entry.team.gr.grName);
                       
      if (res == 0)
        res = entry.team.gr.grPos - tmItem->entry.team.gr.grPos;
        
      return res;
    }

    default :
      return 0;      
  }
}


bool  TmItem::DrawSeparatingLine()
{
  if ( (entry.team.cpType == CP_DOUBLE) || (entry.team.cpType == CP_MIXED) )
    return true;

  return false;
}


bool  TmItem::HasString(const wxString &str) const
{
  return entry.HasString(str);
}


// -----------------------------------------------------------------------
void TmItem::MeasureItem(LPMEASUREITEMSTRUCT lpMIS) 
{
  if ( (entry.team.cpType == CP_DOUBLE) || (entry.team.cpType == CP_MIXED) )
    lpMIS->itemHeight *= 2;
}


