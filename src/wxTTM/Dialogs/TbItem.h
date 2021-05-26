/* Copyright (C) 2020 Christoph Theis */

// Erweiterung von StItem um Tabellenberechnung
#ifndef  TBITEM_H
#define  TBITEM_H

#include  "StItem.h"
#include  "TbEntryStore.h"

#include  <stack>

class  TbItem : public StItem
{
  public:
    TbItem();
    TbItem(const StRec &, short cpType);
    TbItem(const StEntry &);
    TbItem(const TbEntry &);

  public:
    virtual int  Compare(const ListItem *itemPtr, int col) const;
    virtual void DrawColumn(wxDC *pDC, int col, wxRect &rect);
    virtual void DrawItem(wxDC *pDC, wxRect &rec);

  public:
    TbListRec::Result     result;

    void SetResult(const TbListRec &r) { result = r.result; }

    void  PushResult();
    void  PopResult();

    bool hidden = false;
    std::stack<TbEntryStore::Result *>  resultStack;
};

#endif