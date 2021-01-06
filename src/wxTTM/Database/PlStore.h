/* Copyright (C) 2020 Christoph Theis */

// Tabellendefintion und Engine der Spielerdaten
#ifndef  PLSTORE_H
#define  PLSTORE_H

#include  "PsStore.h"
#include  "NaStore.h"
#include  "CpStore.h"

#include  "StoreObj.h"

#include  <string>


class  Statement;
class  ResultSet;

// Tabellendaten in eigene Struktur
struct PlRec : public PsRec
{
  long      plID;        // Unique ID
  long      plNr;        // Spielernummer
  long      psID;        // Foreign ID of PsRec
  long      naID;        // Foreign ID of NaRec
  wxChar    naName[9];   // Macht manches einfacher
  wxChar    naDesc[65];  // dto.
  wxChar    naRegion[65];  // dto.
  wxChar    plExtID[65]; // ITTF / DTTB ID
  double    plRankPts;   // Ranking points (ITTF / ...)
  bool      plDeleted;   // Flag for deleted players

  PlRec()      {Init();}
  
  void  Init() {memset(this, 0, sizeof(PlRec));}
};


// Sicht auf Tabelle allein
class  PlStore : public StoreObj, public PlRec
{
  public:
    // Tabelle in Datenbank erzeugen
    static  bool  CreateTable();
    static  bool  UpdateTable(long version);

    static  bool  CreateConstraints();
    static  bool  UpdateConstraints(long version);

    // Import / Export
    static  bool Import(wxTextBuffer &is);
    static  bool Export(wxTextBuffer &o, long version = 1);

  public:
    PlStore(Connection *connPtr = 0);    // Defaultkonstruktor
    PlStore(const PlRec &rec, Connection *ptr = 0) : StoreObj(ptr), PlRec(rec) {}

   ~PlStore();

    virtual void  Init();

  // API: Wettbewerb anlegen, aendern, loeschen, lesen ...
  public:
    bool  SelectAll();
    bool  SelectById(long id);
    bool  SelectByNr(long nr);
    bool  SelectByExtId(const wxString &extId);
    bool  SelectByName(const wxString &name);

    bool  Insert();
    bool  Update();
    bool  Remove(long id = 0);

    // Check auf cpName, ob WB existiert
    bool  InsertOrUpdate();

    // Copy from PsStore
    bool  InsertNote(const wxString &note);
    wxString  GetNote();

  // Etwas API. Diese Funktionen aendern nicht den WB
  public:
    long  NrToID(long nr);

    long  GetHighestNumber();

  // Hilfen
  private:
    long  GetNextNumber();
    wxString  SelectString() const;
    void  BindRec();
};



#endif
