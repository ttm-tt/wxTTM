/* Copyright (C) 2020 Christoph Theis */

// Darstellung der Nominierung
#ifndef  NMITEM_H
#define  NMITEM_H


#include  "TmItem.h"

#include  "NmEntryStore.h"


class  NmItem : public ListItem
{
  public:
    NmItem(const wxString &name, bool showNaName = true);
    NmItem(const NmEntry &, const wxString &name, bool showNaName = true);

  public:
    // Itemhoehe bestimmen
    virtual void  MeasureItem(LPMEASUREITEMSTRUCT lpMIS);

    virtual int  Compare(const ListItem *itemPtr, int col) const;
    virtual void DrawItem(wxDC *pDC, wxRect &rec);

    virtual bool DrawSeparatingLine();

    void  SetValue(const NmEntry &);

    bool  AddPlayer(const LtEntry &pl);
    bool  RemovePlayer();

  public:
    NmEntry  nm;
    wxString name;
    bool     showNaName; 
};


#endif