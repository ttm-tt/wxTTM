/* Copyright (C) 2020 Christoph Theis */

#pragma once

// Spieler / Teams in RR-Gruppe mit ihren Ergebnis
#include "StEntryStore.h"

struct TbEntry : public StEntry
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

  Result     result;

  TbEntry() { memset(this, 0, sizeof(TbEntry)); }

  void  Init() { memset(this, 0, sizeof(TbEntry)); }

};

class TbEntryStore : public StoreObj, public TbEntry
{
  public:
    static  bool  CreateView();
    static  bool  RemoveView();

  public:
     TbEntryStore(Connection *connPtr = 0);

     virtual void Init();
     virtual bool Next();

   public:
     bool SelectByGr(const GrRec &, short cpType);

   private:
     wxString SelectString(long grID) const;
     bool  BindRec();

     short cpType;
};
