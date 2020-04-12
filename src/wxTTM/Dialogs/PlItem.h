/* Copyright (C) 2020 Christoph Theis */

// Darstellung von Spielern in Listen

#ifndef PLITEM_H
#define PLITEM_H

#include  "ListItem.h"

#include  "PlStore.h"
#include  "PlListStore.h"


class  PlItem : public ListItem
{
  public:
    PlItem();
    PlItem(const PlRec &);

    virtual int  Compare(const ListItem *itemPtr, int col) const;
    virtual void DrawColumn(wxDC *pDC, int col, wxRect &rect);
    virtual void DrawItem(wxDC *pDC, wxRect &rec);

    virtual bool HasString(const wxString &str) const;

    virtual bool IsDeleted() const { return pl.plDeleted; }

  public:
    PlRec  pl;
};


class PlItemEx : public PlItem 
{
  public:
    PlItemEx(PlRec &pl) : PlItem(pl) {}
    
    virtual void DrawItem(wxDC *pDC, wxRect &retc);
};



#endif