/* Copyright (C) 2020 Christoph Theis */

// Darstellung von Nationen in Listen

#ifndef NAITEM_H
#define NAITEM_H

#include  "ListItem.h"

#include  "NaStore.h"

class  NaItem : public ListItem
{
  public:
    NaItem(const NaRec &);

    virtual int  Compare(const ListItem *itemPtr, int col) const;
    virtual void DrawColumn(wxDC *pDC, int col, wxRect &rect);
    virtual void DrawItem(wxDC *pDC, wxRect &rec);
    
    virtual bool HasString(const wxString &) const;

  public:
    NaRec  na;
};


#endif