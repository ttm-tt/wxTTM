/* Copyright (C) 2020 Christoph Theis */

// Tabellendefintion und Engine der Wettbewerbe
#ifndef  CPSTORE_H
#define  CPSTORE_H

#include  "StoreObj.h"

#include  <string>


class  Statement;
class  ResultSet;

struct PlRec;
struct NaRec;
struct LtRec;
struct LtEntry;
class  LtStore;
class  TmStore;

// Tabellendaten in eigene Struktur
struct CpRec
{
  long      cpID;        // Unique ID
  wxChar    cpName[9];   // Short name ("MS")
  wxChar    cpDesc[65];  // Full description ("Mens Singles")
  short     cpType;      // Type of event (Singles, Doubles, ...)
  short     cpSex;       // Sex of Playes (Men, Women, Mixed)
  long      cpYear;      // Age of participants
  long      syID;        // Default team system
  short     cpBestOf;    // Default best-of
  short     cpPtsToWin;  // Points to win
  short     cpPtsAhead;  // Points ahead of opponent to win
  short     cpPtsToWinLast;  // Points to win last game
  short     cpPtsAheadLast;  // Points ahead to win last game

  CpRec() {Init();}

  void  Init() {memset(this, 0, sizeof(CpRec));}

  // Spieler darf spielen ?
  bool  IsAllowed(const PlRec &pl) const; 
  
  // Import / Export als CSV-Datei
  bool Read(const wxString &);
  bool Write(wxString &) const;
};


// Sicht auf Tabelle allein
class  CpStore : public StoreObj, public CpRec
{
  public:
    // Tabelle in Datenbank erzeugen
    static  bool  CreateTable();
    static  bool  UpdateTable(long version);

    static  bool  CreateConstraints();
    static  bool  UpdateConstraints(long version);

    // Import / Export
    static  bool  Import(const wxString &name);
    static  bool  Export(wxTextBuffer &);

  public:
    CpStore(Connection * = 0);  // Defaultkonstruktor
   ~CpStore();
     
     virtual void Init();

  // API: Wettbewerb anlegen, aendern, loeschen, lesen ...
  public:
    bool  SelectAll();
    bool  SelectById(long id);
    bool  SelectByName(const wxString &name);
    bool  Insert();
    bool  Update();
    bool  Remove(long id = 0);

    // Check auf cpName, ob WB existiert
    bool  InsertOrUpdate();

  // Etwas API. Diese Funktionen aendern nicht den WB
  public:
    // Spieler fuer WB melden
    bool  EnlistPlayer(const PlRec &pl);
    // Spieler fuer WB melden, Meldung stht in lt
    bool  EnlistPlayer(const PlRec &pl, LtStore &lt);
    // Spieler von WB abmelden
    bool  RemovePlayer(const PlRec &pl);
    // Spieler anhand seiner Meldung vom WB abmelden
    bool  RemovePlayer(LtStore &lt);
    // Einzel anhand Spieler in WB aufnehmen
    bool  CreateSingle(const PlRec &pl, short natlRank = 0, short intlRank = 0);
    // Einzel anhand Meldung in WB aufnehmen
    bool  CreateSingle(LtStore &lt, const NaRec &na, short natlRank = 0, short intlRank = 0);
    // Doppel anhand Spieler in WB aufnehmen
    bool  CreateDouble(const PlRec &pl, const PlRec &bd, 
                       const NaRec &na, short natlRank = 0, short intlRank = 0);
    // Doppel anhand Meldung in WB aufnehmen
    bool  CreateDouble(const LtRec &ltpl, const LtRec &ltbd, 
                       const NaRec &na, short natlRank = 0, short intlRank = 0);
    // Mannschaft erzeugen
    bool  CreateTeam(TmStore &tm, const NaRec &na, short natlRank = 0, short intlRank = 0);

    // Reihenfolge festlegen: false, wenn sie umgedreht werden muessen
    bool  CheckDoubleOrder(const LtEntry &pl, const LtEntry &bd, const NaRec &na);

  private:
    wxString  SelectString() const;
    void  BindRec();
};



#endif
