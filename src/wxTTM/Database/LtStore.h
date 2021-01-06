/* Copyright (C) 2020 Christoph Theis */

// Relation CpRec <--> PlRec

# ifndef  LTSTORE_H
# define  LTSTORE_H

#include  "StoreObj.h"

class  Connection;

struct CpRec;
struct PlRec;
struct TmRec;
struct NaRec;


struct  LtRec
{
  long  ltID;       // Unique ID
  long  plID;       // Foreign key PlRec.plID
  long  cpID;       // Foreign key CpRec.cpID
  double ltRankPts; // Ranking points des Spielers in diesem WB

  LtRec() {Init();}

  void  Init() {memset(this, 0, sizeof(LtRec));}
};


class  LtStore : public StoreObj, public LtRec
{
  public:
    static  bool CreateTable();
    static  bool UpdateTable(long version);
    static  bool CreateConstraints();
    static  bool UpdateConstraints(long version);

    // Import / Export
    static  bool Import(wxTextBuffer &is);
    static  bool Export(wxTextBuffer &os, long version = 1);
    static  bool RemoveFromDoubles(wxTextBuffer &is);

  public:
    LtStore(Connection * = 0);
   ~LtStore();

    virtual void Init();

  public:
    bool Insert(const CpRec &, const PlRec &);
    bool Update();
    bool Remove(long id = 0);
    bool InsertOrUpdate(const CpRec &, const PlRec &);

    bool SelectById(long id);
    bool SelectByPl(long id);
    bool SelectByCp(long id);
    bool SelectByTm(long id);
    bool SelectBuddy(const LtRec &);

    bool SelectByCpPl(long cpId, long plId);
    bool SelectByCpPlNr(long cpId, long plNr);

  private:
    wxString  SelectString() const;
    bool  BindRec();

    short GetNextNumber(long tmID);
};




# endif