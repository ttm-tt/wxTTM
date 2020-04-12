/* Copyright (C) 2020 Christoph Theis */

// Darstellung des Ranking
#ifndef  RKITEM_H
#define  RKITEM_H


#include  "TmItem.h"

#include  "RkEntryStore.h"


class  RkItem : public TmItem
{
  public:
    RkItem();
    RkItem(const RkEntry &);

  public:
    virtual int  Compare(const ListItem *itemPtr, int col) const;
    virtual void DrawColumn(wxDC *pDC, int col, wxRect &rect);
    virtual void DrawItem(wxDC *pDC, wxRect &rec);

    virtual bool DrawSeparatingLine();
    
    virtual wxString BeginEditColumn(int col) const;
    virtual void EndEditColumn(int col, const wxString &value);

    void  SetValue(const RkEntry &);
    const RkListRec & GetValue() const {return rk;}

    bool  GetDirectEntry() const;
    short GetNatlRank() const;
    short GetIntlRank() const;

    void  SetDirectEntry(bool de);
    void  SetNatlRank(short rank);
    void  SetIntlRank(short rank);

  public:
    RkListRec  rk;
};


inline  bool RkItem::GetDirectEntry() const
{
  return (rk.rkDirectEntry ? true : false);
}


inline  void RkItem::SetDirectEntry(bool de)
{
  rk.rkDirectEntry = (de ? 1 : 0);
}


inline  short RkItem::GetNatlRank() const
{
  return rk.rkNatlRank;
}


inline  void RkItem::SetNatlRank(short rank)
{
  rk.rkNatlRank = rank;
}


inline  short RkItem::GetIntlRank() const
{
  return rk.rkIntlRank;
}


inline  void RkItem::SetIntlRank(short rank)
{
  rk.rkIntlRank = rank;
}

#endif