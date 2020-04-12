/* Copyright (C) 2020 Christoph Theis */

// Zugehoerigkeit eines Spielers (Meldung) zu einem Team

#ifndef  NTSTORE_H
#define  NTSTORE_H

#include  "StoreObj.h"

struct  LtRec;
struct  TmRec;


struct  NtRec
{
  long  ltID;   // Foreign Key LtRec
  long  tmID;   // Foreign Key TmRec;
  long  ntNr;   // Durchnumerierung je Team

  NtRec() {Init();}
  void  Init() {memset(this, 0, sizeof(NtRec));}
};


class  NtStore : public StoreObj, public NtRec
{
  public:
    static  bool  CreateTable();
    static  bool  UpdateTable(long version);

    static  bool  CreateConstraints();
    static  bool  UpdateConstraints(long version);

  public:
    NtStore(Connection * = 0);
   ~NtStore();

    virtual void Init();

  public:    
    bool  Insert(const LtRec &lt, const TmRec &tm, short nr = 0);
    bool  Remove(const LtRec &lt);
    bool  Remove(const TmRec &tm, short nr = 0);

    bool  SelectByLt(const LtRec &);
    bool  SelectByTmNr(const TmRec &, short nr = 0);

  private: 
    wxString  SelectString() const;
    bool  BindRec();

    short GetNextNumber(long tmID);
};

#endif