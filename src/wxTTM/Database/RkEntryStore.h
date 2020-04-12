/* Copyright (C) 2020 Christoph Theis */

#ifndef  RKENTRYSTORE_H
#define  RKENTRYSTORE_H


#include  "TmEntryStore.h"
#include  "RkListStore.h"

struct  CpRec;
struct  NaRec;


struct  RkEntry : public TmEntry
{
  RkListRec  rk;

  RkEntry()  {Init();}

  void  Init()  {memset(this, 0, sizeof(RkEntry));}
};


class  RkEntryStore : public StoreObj, public RkEntry
{
  public:
    static  bool  CreateView();
    static  bool  RemoveView();

  public:
    RkEntryStore(Connection *connPtr = 0);

    virtual void  Init();
    virtual bool  Next();

  public:
    bool  SelectByTm(const TmRec &tm, short type);
    bool  SelectByCp(const CpRec &cp);
    bool  SelectByCpNa(const CpRec &cp, const NaRec &na);

  private:
    wxString  SelectString() const;
    bool  BindRec();
};

#endif