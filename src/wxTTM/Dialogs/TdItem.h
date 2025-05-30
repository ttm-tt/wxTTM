/* Copyright (C) 2020 Christoph Theis */

// Darstellung von Mannschaften in Listen

#ifndef TDITEM_H
#define TDITEM_H

#include  "TmItem.h"

class  TdItem : public TmItem
{
  public:
    TdItem();
    TdItem(long id);
    TdItem(const TmEntry &);

    virtual void DrawColumn(wxDC *pDC, int col, wxRect &rect);
};


#endif