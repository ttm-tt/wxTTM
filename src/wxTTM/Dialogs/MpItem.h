/* Copyright (C) Christoph Theis 2022 */
#pragma once

#include  "ListItem.h"

#include  "MpListStore.h"

class  MpItem : public ListItem
{
  public:
    MpItem(const MpListRec &);

    virtual int  Compare(const ListItem *itemPtr, int col) const;
    virtual void DrawColumn(wxDC *pDC, int col, wxRect &rect);
    virtual void DrawItem(wxDC *pDC, wxRect &rec);

  public:
    MpListRec  mp;
};

