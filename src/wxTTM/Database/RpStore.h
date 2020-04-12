/* Copyright (C) 2020 Christoph Theis */

// Rankingpunkte per Altersklasse
#ifndef RPSTORE_H
#define RPSTORE_H

#include "StoreObj.h"

struct RpRec
{
  long rpID;           // Primary key
  long plID;           // Foreign key PlStore
  short  rpYear;       // Altersklasse
  double rpRankPts;    // Ranking points

  RpRec()  {Init();}
  void  Init() {memset(this, 0, sizeof(RpRec));}
};


class RpStore : public StoreObj, public RpRec
{
  public:
    static  bool  CreateTable();
    static  bool  UpdateTable(long version);

    static  bool  CreateConstraints();
    static  bool  UpdateConstraints(long version);

    // Import / Export
    static  bool Import(const wxString &);
    static  bool Export(const wxString &);

  public:
    RpStore(const RpRec &, Connection * = 0);
    RpStore(Connection * = 0);
   ~RpStore();

    virtual void Init();

  public:
    bool Insert();
    bool RemoveByPl(long plID);

    bool SelectByPl(long plID);

  private:
    wxString SelectString() const;
    bool BindRec();
};

#endif