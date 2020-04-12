/* Copyright (C) 2020 Christoph Theis */

// Darstellung von WB in Listen

#ifndef CPITEM_H
#define CPITEM_H

#include  "ListItem.h"

#include  "CpStore.h"

class  CpItem : public ListItem
{
  public:
    CpItem(const CpRec &);

    virtual int  Compare(const ListItem *itemPtr, int col) const;
    virtual void DrawColumn(wxDC *pDC, int col, wxRect &rect);
    virtual void DrawItem(wxDC *pDC, wxRect &rec);
    
    virtual bool HasString(const wxString &) const;

  public:
    CpRec  cp;
};


#endif