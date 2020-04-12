/* Copyright (C) 2020 Christoph Theis */

// Darstellung der Setzung
#ifndef  STITEM_H
#define  STITEM_H


#include  "TmItem.h"

#include  "StEntryStore.h"
#include  "StListStore.h"


class  StItem : public TmItem
{
  public:
    StItem();
    StItem(const StRec &, short cpType);
    StItem(const StEntry &);

  public:
    virtual int  Compare(const ListItem *itemPtr, int col) const;
    virtual void DrawColumn(wxDC *pDC, int col, wxRect &rect);
    virtual void DrawItem(wxDC *pDC, wxRect &rec);

    virtual bool DrawSeparatingLine();

    void  SetValue(const StEntry &);

  public:
    StRec  st;
    bool   boldStNr;
};


#endif