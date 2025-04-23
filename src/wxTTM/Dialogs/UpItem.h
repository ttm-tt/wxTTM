/* Copyright (C) 2020 Christoph Theis */

// Darstellung von Spielern in Listen

#ifndef UPITEM_H
#define UPITEM_H

#include  "ListItem.h"

#include  "UpStore.h"
#include  "UpListStore.h"


class  UpItem : public ListItem
{
  public:
    UpItem();
    UpItem(const UpRec &);

    virtual int  Compare(const ListItem *itemPtr, int col) const;
    virtual void DrawColumn(wxDC *pDC, int col, wxRect &rect);
    virtual void DrawItem(wxDC *pDC, wxRect &rec);

    virtual bool HasString(const wxString &str) const;

    virtual bool IsDeleted() const { return up.upDeleted; }
  public:
    UpRec  up;
};


#endif