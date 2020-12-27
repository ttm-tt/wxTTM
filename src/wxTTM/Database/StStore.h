/* Copyright (C) 2020 Christoph Theis */

// Tabelle der "Setzung"
# ifndef  STSTORE_H
# define  STSTORE_H


#include  "StoreObj.h"

struct  CpRec;
struct  GrRec;
struct  TmRec;
struct  XxRec;
struct  TmEntry;

struct  StRec
{
  long  stID;      // Prim. Key
  long  grID;      // Foreign key GrStore
  long  tmID;      // Foreign key TmStore
  short stNr;      // Position in Gruppe
  short stPos;     // Abschneiden in Gruppe
  short stSeeded;  // Gesetzter Spieler
  short stGaveup;  // Hat aufgegeben
  short stDisqu;   // Wurde disqualifiziert
  short stNocons;  // Keine Consolation

  StRec()      {Init();}
  
  void  Init() {memset(this, 0, sizeof(StRec));}
};


class  StStore : public StoreObj, public StRec
{
  public:
    enum
    {
      ST_SEEDED = 1,
      ST_GAVEUP = 2,
      ST_DISQU  = 4,
      ST_NOCONS = 8,
      ST_KEEPSEEDED = 16
    };
    
  public:
    static  bool CreateTable();
    static  bool UpdateTable(long version);

    static  bool CreateConstraints();
    static  bool UpdateConstraints(long version);

    // Import / Export
    static  bool  Import(const wxString &name);
    static  bool  Export(wxTextBuffer &os, short cpType, const std::vector<long> &, bool append);

  public:
    StStore(Connection *ptr = 0);

    virtual void  Init();

  public:
    bool  Insert(const GrRec &);
    bool  Update();
    bool  UpdatePosition();
    bool  Remove(const GrRec &);

  public:
    bool  SelectAll(const GrRec &);
    bool  SelectById(long id);
    bool  SelectByNr(long grID, short nr);

    bool  SelectAll(const CpRec &, const wxString &stage);
    bool  SelectSeeded(const CpRec &, const wxString & stage);
    
  public:
    bool  RemoveTeam(long id);

  public:
    // API fuer Setzung
    bool  ClearAllTeams(const GrRec &gr);
    
    bool  SetTeam(long tmID, unsigned flags = 0);
    bool  SetTeam(const TmEntry &tm, unsigned flags = 0);

    bool  SetTeam(const GrRec &rec, short nr, long tmID, unsigned flags = 0);
    bool  SetTeam(const GrRec &rec, short nr, const TmEntry &tm, unsigned flags = 0);

    long  GetId(const GrRec &, short nr);

    bool  SetSeeded(bool seeded);
    bool  SetGaveup(bool gaveup);
    bool  SetDisqualified(bool disqu);
    bool  SetNoConsolation(bool nocons);

  private:
    wxString  SelectString() const;
    bool  BindRec();
};

# endif
