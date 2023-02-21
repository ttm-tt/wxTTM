/* Copyright (C) 2020 Christoph Theis */

// Tabelle der Spiele
# ifndef  MTSTORE_H
# define  MTSTORE_H

#include  "StoreObj.h"
#include  "Rec.h"

#include  <string>


class  Statement;
class  ResultSet;

struct GrRec;
struct StRec;
struct MtRec;

class  MtStore;
class  MtListStore;
class  MtEntryStore;


// Ergebnis eines einzelnen Satzes. mtSet == 0 enthaelt die Summe
struct  MtSet
{
  long  mtID;         // Foreign key MT
  short mtMS;         // Mannschaftsspiel
  short mtSet;        // Satznr
  short mtResA;       // Ergebnis A
  short mtResX;       // Ergebnis X

  MtSet()       {Init();}

  void  Init()  {memset(this, 0, sizeof(MtSet));}

  MtSet & operator=(const MtSet &);  
};


class  MtSetStore : public StoreObj, public MtSet
{
  public:
    static  bool  CreateTable();
    static  bool  UpdateTable(long version);

    static  bool  CreateConstraints();
    static  bool  UpdateConstraints(long version);

  public:
    MtSetStore(Connection *);
    MtSetStore(const MtRec &, Connection *);

    virtual void  Init();

    // Auswahl aller Saetze. ms != 0: Nur dieses Mannschaftsspiel
    bool  SelectAll(short ms = 0);

    // Auswahl vieler Spiele
    bool  SelectAll(const std::set<long> &ids);

  private:
    MtSetStore() {}
};


// Ein einzelnes Spiel. mtMS == 0 enthaelt die Summe
struct  MtMatch
{
  long  mtID;
  short mtMS;
  short mtResA;
  short mtResX;
  short mtWalkOverA;
  short mtWalkOverX;
  short mtInjuredA;
  short mtInjuredX;
  short mtDisqualifiedA; // e.g. failed racket control 
  short mtDisqualifiedX;
  // short mtNotPlayed;

  MtMatch() {Init();}

  void  Init() {memset(this,0, sizeof(MtMatch));}

  MtMatch & operator=(const MtMatch &);
};


class  MtMatchStore : public StoreObj, public MtMatch
{
  public:
    static  bool  CreateTable();
    static  bool  UpdateTable(long version);

    static  bool  CreateConstraints();
    static  bool  UpdateConstraints(long version);

  public:
    MtMatchStore(const MtRec &, Connection *);

    virtual void  Init();

    // Auswahl aller Saetze. ms != 0: Nur dieses Mannschaftsspiel
    bool  SelectAll(short ms = 0);

  private:
    MtMatchStore() {}
};


struct  MtRec
{
  long  mtID;       // Unique ID
  long  mtNr;       // Unique Nr all matches

  long  stA;        // Foreign key ST-A
  long  stX;        // Foreign key ST-X

  long  mtUmpire;   // SchiRi-Team
  long  mtUmpire2;  // SchiRi-Team Assistant

  short mtReverse;       // A-X vertauschen 
  short mtScorePrinted;  // Scoresheet gedruckt
  short mtScoreChecked;  // Result checked

  // For an individual match this is a copy of MtMatch
  // For team event those can be set seperately
  short mtWalkOverA;     // w/o A
  short mtWalkOverX;     // w/o X
  short mtInjuredA;
  short mtInjuredX;
  short mtDisqualifiedA; // e.g. failed racket control
  short mtDisqualifiedX;

  short mtMatches;       // Anzahl der Spiele
  short mtBestOf;        // Zahl der Saetze

  struct MtEvent
  {
    long   grID;     // Foreign key GR
    short  mtRound;  // Runde
    short  mtMatch;  // Spiel in Runde
    short  mtChance; // DKO: Trostrunde
    short  mtMS;     // Mannschaftsspiel
  } mtEvent;

  struct MtPlace
  {
    timestamp   mtDateTime; // yyyy-mm-dd hh:mm:ss
    short  mtTable;        
  } mtPlace;

  // Ergebnissummen aus MtSets (0), MtMatches (0)
  short  mtBallsA;
  short  mtBallsX;
  short  mtSetsA;
  short  mtSetsX;

  // Gewonnene / verlorene Einzelspiele, aus DB
  short  mtResA;
  short  mtResX;

  // Team-IDs hinter stA, stX. Ergeben sich aus JOIN
  long   tmA;
  long   tmX;

  // Verknuepfung zwischen Gruppen
  // long   grA;
  // long   grX;
  
  long   xxAstID;
  long   xxXstID;

  bool   mtComplete;

  MtRec();
  MtRec(const MtRec &rec);
  MtRec & operator=(const MtRec &);

  void  Init();

  // Ist A / X Freilos?
  bool  IsABye() const  {return stA != 0 && tmA == 0 && xxAstID == 0;}
  bool  IsXBye() const  {return stX != 0 && tmX == 0 && xxXstID == 0;}
  bool  IsBye() const   {return IsABye() && IsXBye();}
  bool  IsAByeOrXBye() const {return IsABye() || IsXBye();}

  short QryWinnerAX() const;  // A: +1, X: -1
  
  bool  IsFinished() const;
};


inline  MtRec::MtRec() 
{
  Init();
}


inline  MtRec::MtRec(const MtRec &rec)
{
  operator=(rec);
}


inline  void  MtRec::Init()
{
  memset(this, 0, sizeof(MtRec));
}


// -----------------------------------------------------------------------
class  MtStore : public StoreObj, public MtRec
{
  public:
    static  bool CreateTable();
    static  bool UpdateTable(long version);

    static  bool  CreateConstraints();
    static  bool  UpdateConstraints(long version);

    // Import / Export Results
    static  bool  ImportResults(wxTextBuffer &is);
    static  bool  ExportResults(wxTextBuffer &os, short cpType, const std::vector<long> &, bool append, long version = 1);
    static  bool  ExportForRanking(wxTextBuffer &os, short cpType, const std::vector<long> &, bool append, long version = 1);
    static  bool  ExportForRankingTTM(wxTextBuffer &os, short cpType, const std::vector<long> &, bool append, long versin = 1);
    static  bool  ExportForRankingITTF(wxTextBuffer &os, short cpType, const std::vector<long> &, bool append, long versin = 1);
    static  bool  ExportForRankingETTU(wxTextBuffer &os, short cpType, const std::vector<long> &, bool append, long version = 1);

    // Import / Export Schedule
    static  bool  ImportSchedule(wxTextBuffer &is);
    static  bool  ExportSchedule(wxTextBuffer &os, short cpType, const std::vector<long> &, bool append, long version = 1);
    static  long  GetMaxSupportedExportVersion() {return 1;}
    
  private:
    static  bool UpdateStoredProcedure(long version);

  public:
    MtStore(Connection *ptr = 0) : StoreObj(ptr) {};

    virtual void  Init();

  public:
    bool  Insert(const GrRec &);
    bool  Update();
    bool  Remove(const GrRec &);

  // Etwas API
  public:
    bool  SetTeamAByNr(short stNr);
    bool  SetTeamXByNr(short stNr);
    bool  SetTeamsByNr(short stNrA, short stNrX);

    bool  SetTeamAById(long stID);
    bool  SetTeamXById(long stID);

    bool  SetBestOf(short bestOf);
    
    // Erste / Letzte Spielzeit einer Runde abfragen
    timestamp GetEarliestMatchTime(const MtEvent &event);
    timestamp GetLatestMatchTime(const MtEvent &event);
    short GetLastPlayedRound(const MtEvent &event);

    short GetHighestTableNumber(const MtPlace &place);

    // Anzahl Spiele Gruppen (combined), Runde in Gruppe, Spiel in Runde in Gruppe
    short CountMatchesInGroupGroup(const MtEvent &evt);
    short CountMatchesInRoundGroup(const MtEvent &evt);
    short CountMatchesInMatchGroup(const MtEvent &evt);

    // Reverse-Flag aktualisieren
    bool  UpdateReverseFlag();

    // W/O (im Mannschaftsspiel) aktualisieren
    bool  UpdateWalkOver();

    // Ergebnis akutalisieren. Liste hat immer mtBestOf Elemente
    bool  UpdateResult(
      short bestOf, MtSet *sets, 
      short walkOverA = 0, short walkOverX = 0, 
      short injuredA = 0, short injuredX = 0, 
      short disqualifiedA = 0, short disqualifiedX = 0);

    // Zeitplan aktualisieren
    bool  UpdateSchedule();

    bool  UpdateScheduleMatch(MtStore::MtEvent &mtEvent,
                              MtStore::MtPlace &mtPlace,
                              short umpire, short umpire2);

    bool  UpdateScheduleRound(MtStore::MtEvent &mtEvent,
                              MtStore::MtPlace &mtPlace,
                              short &nofTables, bool decTable, 
                              short umpire, short umpire2);

    bool  UpdateScheduleRoundExcludeByes(
                              MtStore::MtEvent &mtEvent,
                              MtStore::MtPlace &mtPlace,
                              short &nofTables, bool decTable,
                              short umpire, short umpire2);

    bool  UpdateScheduleRoundsGroup(
                              MtStore::MtEvent &mtEvent,
                              MtStore::MtPlace &mtPlace,
                              short umpire, short umpire2);

    bool  UpdateScheduleRoundsGroup(
                              MtStore::MtEvent &mtEvent,
                              MtStore::MtPlace &mtPlace,
                              short &nofTables, bool decTable,
                              short umpire, short umpire2);

    bool  UpdateScheduleRoundsGroupExcludeByes(
                              MtStore::MtEvent &mtEvent,
                              MtStore::MtPlace &mtPlace,
                              short &nofTables, bool decTable,
                              short umpire, short umpire2);

    bool  UpdateScheduleMatchesGroup(
                              MtStore::MtEvent &mtEvent,
                              MtStore::MtPlace &mtPlace,
                              short &nofTables, bool decTable,
                              short umpire, short umpire2);

    bool  UpdateScheduleGroup(MtStore::MtEvent &mtEvent,
                              MtStore::MtPlace &mtPlace,
                              short umpire, short umpire2);

    bool  UpdateScheduleGroup(MtStore::MtEvent &mtEvent,
                              MtStore::MtPlace &mtPlace,
                              short &nofTables, bool decTable,
                              short umpire, short umpire2);

    bool  UpdateScoreChecked(long id, bool checked);

    bool  UpdateScorePrinted(long id, bool printed);
    bool  UpdateScorePrintedForRound(long id, short round, bool printed);
    bool  UpdateScorePrintedForGroup(long id, bool printed);
    bool  UpdateScorePrintedScheduled(const MtStore::MtPlace &from, 
                                      const MtStore::MtPlace &to, bool printed);
    bool  UpdateScorePrintedForTeam(const StRec &st, bool printed);
                                      
    // Update Timestamp
    bool  UpdateTimestamp(const StRec &);                                      

    // Auswahl nach Gruppe, Event, ID, Nr.
    bool  SelectByGr(const GrRec &);
    bool  SelectByEvent(const MtEvent &);
    bool  SelectById(long id);
    bool  SelectByNr(long nr);

  private:
    long  GetNextNumber();
    bool  UpdateScheduleMatch(long id, MtStore::MtPlace &mtPl, short umpire, short umpire2);

    wxString  SelectString() const;
    bool  BindRec();
};


# endif