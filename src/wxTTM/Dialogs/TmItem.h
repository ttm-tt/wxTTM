/* Copyright (C) 2020 Christoph Theis */

// Darstellung von Paaren in Listen

#ifndef TMITEM_H
#define TMITEM_H

#include  "ListItem.h"

#include  "TmStore.h"
#include  "TmEntryStore.h"
#include  "PlListStore.h"


class  TmItem : public ListItem
{
  public:
    TmItem();
    TmItem(long id);
    TmItem(const TmEntry &);

    // Itemhoehe bestimmen
    virtual void  MeasureItem(LPMEASUREITEMSTRUCT lpMIS);

    virtual int  Compare(const ListItem *itemPtr, int col) const;
    virtual void DrawColumn(wxDC *pDC, int col, wxRect &rect);
    virtual void DrawItem(wxDC *pDC, wxRect &rec);

    virtual bool DrawSeparatingLine();

    virtual bool HasString(const wxString &str) const;

  public:
    TmEntry  entry;
};


#endif