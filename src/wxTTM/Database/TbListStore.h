/* Copyright (C) 2021 Christoph Theis */

#pragma once

#include "StoreObj.h"

#include "GrStore.h"

// Result of RR group calculation
struct TbListRec
{
  struct  Result
  {
    short  pos;         // Abschliesende Position
    short  nrMatches;   // Zahl der Spiele
    short  matchPoints; // ITTF Punkte Regeleung
    short  points[2];   // Punkteverhaeltnis
    short  matches[2];  // Mannschaft: Spielverhaeltnis
    short  sets[2];     // Satzverhaeltnis
    short  balls[2];    // Ballverhaeltnis
  };

  long    tbID;
  Result  result;

  TbListRec() { memset(this, 0, sizeof(TbListRec)); }

  void  Init() { memset(this, 0, sizeof(TbListRec)); }  
};


class TbListStore : public StoreObj, public TbListRec
{
  public:
     TbListStore(Connection *connPtr = 0);

     virtual void Init();

   public:
     bool SelectByGr(const GrRec &);
     bool SelectByGr(const GrRec &, const std::set<long> &ids);

   private:
     wxString SelectString(long grID) const;
     wxString SelectString(long grID, const std::set<long> &ids) const;
     bool  BindRec();
};