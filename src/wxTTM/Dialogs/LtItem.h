/* Copyright (C) 2020 Christoph Theis */

// Darstellung von Spielern in Listen

#ifndef LTITEM_H
#define LTITEM_H

#include  "ListItem.h"

#include  "LtEntryStore.h"
#include  "PlItem.h"


class  LtItem : public PlItem
{
  public:
    LtItem(bool showNaName = true);
    LtItem(const LtEntry &, bool showNaName = true);

    virtual void DrawItem(wxDC *pDC, wxRect &rec);
    virtual void DrawColumn(wxDC *pDC, int col, wxRect &rec);
  public:
    LtRec    lt;
    bool     showNaName;

    LtEntry GetEntry() const {return LtEntry(lt, pl);}
};


#endif