/* Copyright (C) 2020 Christoph Theis */

// Darstellung der Spiele
#ifndef  MTITEM_H
#define  MTITEM_H


#include  "StItem.h"

#include  "MtEntryStore.h"


class  MtItem : public ListItem
{
  public:
    MtItem();
    MtItem(const MtEntry &, bool showTime);

  public:
    virtual int  Compare(const ListItem *itemPtr, int col) const;
    virtual void DrawColumn(wxDC *pDC, int col, wxRect &rect);
    virtual void DrawItem(wxDC *pDC, wxRect &rec);

    virtual bool DrawSeparatingLine();
    virtual bool HasString(const wxString &str) const;

    void  SetToTime(bool  fl) {showTimes = fl;}
    void  SetValue(const MtEntry &);
    void  SetResult(short resA, short resX);
    void  SetSchedule(const MtRec::MtPlace &mtPlace);

  protected:
    void  DrawPair(wxDC *pDC, wxRect &rect, const MtEntry &mt);

  public:
    MtEntry  mt;

  protected:
    wxChar  time[6];
    wxChar  date[9];
    bool    showTimes;
};


inline  void MtItem::SetResult(short resA, short resX)
{
  mt.mt.mtResA = resA;
  mt.mt.mtResX = resX;
}


#endif