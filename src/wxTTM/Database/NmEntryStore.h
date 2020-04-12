/* Copyright (C) 2020 Christoph Theis */

#ifndef  NMENTRYSTORE_H
#define  NMENTRYSTORE_H

#include  "TmEntryStore.h"

struct  MtRec;
struct  TmEntry;

struct  NmEntry : public TmEntry
{
  short  nmNr;    // Fortlaufende Nummer der Meldung
  long   nmID;
  long   mtID;

  long   ltA;
  long   ltB;

  NmEntry() {Init();}
  
  void Init() {memset(this, 0, sizeof(NmEntry));}
};


class  NmEntryStore : public StoreObj, public NmEntry
{
  public:
    static  bool  CreateView();
    static  bool  RemoveView();

  public:
    NmEntryStore(Connection *connPtr = 0);

    virtual void Init();

  public:
    bool  SelectByMtTm(const MtRec & mt, const TmEntry &tm);
    bool  ExistsForMt(const MtRec &mt);

  private:
    wxString  SelectString() const;
    void  BindRec();
};

#endif