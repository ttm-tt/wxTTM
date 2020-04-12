/* Copyright (C) 2020 Christoph Theis */

// Darstellung von Spielmodi in Listen

#ifndef MDITEM_H
#define MDITEM_H

#include  "ListItem.h"

#include  "MdListStore.h"

class  MdItem : public ListItem
{
  public:
    MdItem(const MdListRec &);
    MdItem(short modus);

    virtual int  Compare(const ListItem *itemPtr, int col) const;
    virtual void DrawColumn(wxDC *pDC, int col, wxRect &rect);
    virtual void DrawItem(wxDC *pDC, wxRect &rec);

    short  GetModus() const  {return modus;}
    void   SetModus(short mod) {modus = mod;}

  public:
    MdListRec  md;
    short  modus;
};


#endif