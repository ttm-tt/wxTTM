/* Copyright (C) 2020 Christoph Theis */

#pragma once

// Spieler / Teams in RR-Gruppe mit ihren Ergebnis
#include "StEntryStore.h"
#include "TbListStore.h"

struct TbEntry : public StEntry, public TbListRec
{
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
