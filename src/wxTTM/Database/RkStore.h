/* Copyright (C) 2020 Christoph Theis */

// Ranking eines Teams
#ifndef  RKSTORE_H
#define  RKSTORE_H

#include  "StoreObj.h"

struct  TmRec;
struct  NaRec;
struct  CpRec;
struct  NaListRec;
struct  CpListRec;


struct  RkRec
{
  long  tmID;          // Foreign Key TmRec
  long  naID;          // Foreign Key NaRec
  short rkNatlRank;    // Nationales Ranking
  short rkIntlRank;    // Internationales Ranking (?)
  short rkDirectEntry; // Flag direct entry

  RkRec()  {Init();}
  void  Init() {memset(this, 0, sizeof(RkRec));}
};


class  RkStore : public StoreObj, public RkRec
{
  public:
    static  bool  CreateTable();
    static  bool  UpdateTable(long version);

    static  bool  CreateConstraints();
    static  bool  UpdateConstraints(long version);

  public:
    RkStore(const RkRec &, Connection * = 0);
    RkStore(Connection * = 0);
   ~RkStore();

    virtual void Init();

  public:
    bool  Insert();
    bool  Insert(const TmRec &tm, const NaRec &na, short natlRank = 0, short intlRank = 0);
    bool  Remove(const TmRec &tm);
    bool  Remove(const CpRec &cp);
    bool  Remove(const CpRec &cp, const NaRec &na);
    bool  Remove(const CpListRec &cp, const NaListRec &na);

    bool  SelectByTm(const TmRec &);
    bool  SelectByRanking(const CpRec &, const NaRec &, int rank);
    bool  SelectByRanking(const TmRec &, const NaRec &, int rank);
    
  private:
    short  GetNextNatlRank(const NaRec &, const TmRec &, short intlRank = 0);
    bool   ExistsNatlRank(const NaRec &, const TmRec &, short natlRank);
    wxString  SelectString() const;
    bool  BindRec();
};

#endif