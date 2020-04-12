/* Copyright (C) 2020 Christoph Theis */

// Das Ergebnis im SQLEditor

#ifndef SQLITEM_H
#define SQLITEM_H

#include  "ListItem.h"
#include  "ResultSet.h"

#include  <map>

#pragma warning(disable:4786)


typedef  std::map<short, wxString, std::less<short> >  ColInfoMap;


class  SQLItem : public ListItem
{
  public:
    SQLItem(ResultSet *);

    virtual int  Compare(const ListItem *itemPtr, int col) const;
    virtual void DrawColumn(wxDC *pDC, int col, wxRect &rect);

    wxString  GetColumn(short idx) const;
    unsigned GetColumnSize(short idx) const;

  private:
    ColInfoMap  colInfoMap;
};


inline  wxString  SQLItem::GetColumn(short idx) const
{
  return (*colInfoMap.find(idx)).second;
}


inline  unsigned  SQLItem::GetColumnSize(short idx) const
{
  return (*colInfoMap.find(idx)).second.size();
}


#endif