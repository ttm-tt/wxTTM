/* Copyright (C) 2020 Christoph Theis */

// Meldungen von Spielern

#ifndef  LTENTRYSTORE_H
#define  LTENTRYSTORE_H

#include  "StoreObj.h"

#include  "LtStore.h"
#include  "PlStore.h"
#include  "CpStore.h"

struct  TmRec;

struct  LtEntry : public LtRec, public PlRec
{
  LtEntry(const LtRec &lt, const PlRec &pl) : LtRec(lt), PlRec(pl) {}
  LtEntry() {memset(this, 0, sizeof(LtEntry));}
  LtEntry(const LtEntry &lt) {memcpy(this, &lt, sizeof(LtEntry));}
  
  void  SetPlayer(const PlRec &pl) {PlRec::operator=(pl);}

  void  Init();
};


inline  void  LtEntry::Init()
{
  memset(this, 0, sizeof(LtEntry));
}



class  LtEntryStore : public StoreObj, public LtEntry
{
  public:
    static  bool  CreateView();
    static  bool  RemoveView();

  public:
    LtEntryStore(Connection *connPtr = 0);

    virtual void  Init();
    virtual bool  Next();

  public:
    bool  SelectById(long id);
    bool  SelectByCp(const CpRec &cp);
    bool  SelectByCpPl(long cpID, long plID, const timestamp * = NULL);
    bool  SelectOpenEntriesByCp(const CpRec &cp);
    bool  SelectOpenEntriesByCpNa(const CpRec &cp, const NaRec &na);

    bool  SelectForDouble(const PlRec &pl, const CpRec &cp);
    bool  SelectBuddy(const LtRec &lt, const timestamp * = NULL);

    // Spieler eines Teams
    bool  SelectPlayerByTm(const TmRec &tm);

    short CountFemaleEntries(const CpRec &cp);
    short CountMaleEntries(const CpRec &cp);

  private:
    wxString  SelectString(const timestamp * = NULL) const;
    bool  BindRec();
};

#endif