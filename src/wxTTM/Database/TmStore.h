/* Copyright (C) 2020 Christoph Theis */

// Record Teams
# ifndef  TMSTORE_H
# define  TMSTORE_H

#include  "StoreObj.h"

class  Connection;

struct  CpRec;
struct  LtRec;


struct  TmRec
{
  long    tmID;        // Unique ID
  long    cpID;        // Foreign key CpRec.cpID
  short   tmDisqu;     // Disqualified
  short   tmGaveup;    // Gave up
  wxChar  tmName[9];   // Name bei Mannschaften
  wxChar  tmDesc[65];  // Beschreibung bei Mannschaften

  TmRec()      {Init();}
  
  void  Init() {memset(this, 0, sizeof(TmRec));}
};


class  TmStore : public StoreObj, public TmRec
{
  public:
    static  bool  CreateTable();
    static  bool  UpdateTable(long version);

    static  bool  CreateConstraints();
    static  bool  UpdateConstraints(long version);

  public:
    TmStore(Connection * = 0);
   ~TmStore();

    virtual void Init();

    bool  SelectAll();
    bool  SelectById(long id);
    bool  SelectByName(const wxString &name);
    bool  SelectByCp(long id);
    bool  SelectByLt(const LtRec &);
    bool  SelectByCpPlNr(const CpRec &cp, long plNr);
    bool  SelectByCpTmName(const CpRec &cp, const wxString &tmName);

    bool  Insert(const CpRec &cp);
    bool  Update();
    bool  Remove(long id = 0);

    bool  RemoveAllEntries();
    bool  RemoveEntry(const LtRec &);
    bool  AddEntry(const LtRec &, short ntNr = 0);

    long  GetMaxNameLength();

  private:
    wxString  SelectString() const;
    bool  BindRec();
};

# endif
