/* Copyright (C) 2020 Christoph Theis */

// Darstellung von WB in Listen

#ifndef GRITEM_H
#define GRITEM_H

#include  "ListItem.h"

#include  "GrListStore.h"
#include  "GrStore.h"

class  GrItem : public ListItem
{
  public:
    GrItem(const GrRec &);

    virtual int  Compare(const ListItem *itemPtr, int col) const;
    virtual void DrawColumn(wxDC *pDC, int col, wxRect &rect);
    virtual void DrawItem(wxDC *pDC, wxRect &rec);
    
    virtual bool HasString(const wxString &) const;

  public:
    GrRec  gr;
};


#endif