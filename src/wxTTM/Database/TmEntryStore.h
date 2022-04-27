/* Copyright (C) 2020 Christoph Theis */

// Teams
#ifndef  TMENTRYSTORE_H
#define  TMENTRYSTORE_H

#include  "StoreObj.h"
#include  "PlStore.h"
#include  "GrStore.h"
#include  "TmListStore.h"
#include  "LtEntryStore.h"

struct  CpRec;
struct  GrRec;
struct  StRec;
struct  TmRec;


struct  TmPlayer : public LtEntry
{
  void  SetPlayer(const LtEntry &pl);
};


struct  TmTeam : public TmRec, public NaRec
{
  void  SetTeam(const TmRec &tm);
};


struct  TmGroup : public GrRec
{
  long    xxStID = 0;
  short   grPos = 0;

  void  SetGroup(const GrRec &gr, short pos);
};


struct  TmEntry : public TmListRec
{
  struct
  {
    short  cpType = CP_NONE;
    TmPlayer  pl;
    TmPlayer  bd;
    TmTeam    tm;
    TmGroup   gr;
  }  team;

  double rankPts = 0.;

  TmEntry() {memset(this, 0, sizeof(TmEntry));}
  TmEntry(const TmEntry &tm) {memcpy(this, &tm, sizeof(TmEntry));}

  void  SetSingle(const LtEntry &pl);
  void  SetDouble(const LtEntry &pl, const LtEntry &bd);
  void  SetMixed(const LtEntry &pl, const LtEntry &bd);
  void  SetTeam(const TmRec &tm);
  void  SetGroup(const GrRec &gr, short pos);

  bool  HasString(const wxString &str) const;

  void  Init();
};


inline  void  TmEntry::Init()
{
  short type = team.cpType;
  memset(this, 0, sizeof(TmEntry));
  team.cpType = type;
}


class  TmEntryStore : public StoreObj, public TmEntry
{
  public:
    static  bool  CreateView();
    static  bool  RemoveView();

  public:
    TmEntryStore(Connection *connPtr = 0);

    virtual void  Init();

  public:
    bool  SelectTeamById(long id, short type, const timestamp * = NULL);
    bool  SelectTeamById(const std::set<long> &ids, short type);
    bool  SelectTeamByCp(const CpRec &cp);
    bool  SelectTeamForSeeding(const GrRec &gr, short cpType);
    short CountTeams(const CpRec &cp);

  private:
    wxString  SelectString(const timestamp * = NULL) const;
    bool  BindRec();
};

#endif
