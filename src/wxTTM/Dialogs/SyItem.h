/* Copyright (C) 2020 Christoph Theis */

// Darstellung von Spielsystemen in Listen

#ifndef SYITEM_H
#define SYITEM_H

#include  "ListItem.h"

#include  "SyListStore.h"

class  SyItem : public ListItem
{
  public:
    SyItem(const SyListRec &);

    virtual int  Compare(const ListItem *itemPtr, int col) const;
    virtual void DrawColumn(wxDC *pDC, int col, wxRect &rect);
    virtual void DrawItem(wxDC *pDC, wxRect &rec);

  public:
    SyListRec  sy;
};


#endif