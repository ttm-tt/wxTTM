/* Copyright (C) 2020 Christoph Theis */

// Darstellung der Ergebnisse des SQLEdtiors

#include  "stdafx.h"

#include  "SQLItem.h"


SQLItem::SQLItem(ResultSet *resPtr) : ListItem()
{
  if (!resPtr)
    return;

  wxChar  str[128];
  int     idx = 0;
  try
  {
    while (resPtr->GetData(++idx, str, 128))
    {
      if (resPtr->WasNull())
        colInfoMap.insert(ColInfoMap::value_type(idx-1, "<null>"));
      else
        colInfoMap.insert(ColInfoMap::value_type(idx-1, str));
    }
  }
  catch(...)
  {
  }
}


int  SQLItem::Compare(const ListItem *itemPtr, int col) const
{
  const char *str1 = (*colInfoMap.find(col)).second.data();
  const char *str2 = (*( ((SQLItem *) itemPtr)->colInfoMap.find(col)) ).second.data();

  return strcoll(str1, str2);
}


void  SQLItem::DrawColumn(wxDC *pDC, int col, wxRect &rect)
{
  wxRect    rcColumn = rect;
  rcColumn.Deflate(offset, 0);

  ColInfoMap::iterator  it = colInfoMap.find(col);

  switch (col)
  {
    case 0 :
      DrawString(pDC, rcColumn, it == colInfoMap.end() ? 
                                wxT("") : (*it).second.data());
      break;

    default :
      DrawString(pDC, rcColumn, it == colInfoMap.end() ? 
                                wxT("") : (*it).second.data());
      break;
  }
}