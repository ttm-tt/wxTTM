/* Copyright (C) 2020 Christoph Theis */

// Tabellendefintion und Engine der Spielerdaten
#ifndef  UPSTORE_H
#define  UPSTORE_H

#include  "PsStore.h"
#include  "NaStore.h"

#include  "StoreObj.h"

#include  <string>


class  Statement;
class  ResultSet;

// Tabellendaten in eigene Struktur
struct UpRec : public PsRec
{
  long      upID;        // Unique ID
  long      upNr;        // Spielernummer
  long      psID;        // Foreign ID of PsRec
  long      naID;        // Foreign ID of NaRec
  wxChar    naName[9];   // Macht manches einfacher
  wxChar    naDesc[65];  // dto.
  wxChar    naRegion[65];  // dto.

  UpRec()      {Init();}
  
  void  Init() {memset(this, 0, sizeof(UpRec));}

  // Import / Export als CSV-Datei
  bool Read(const wxString &line);
  bool Write(wxString &line) const;
};


// Sicht auf Tabelle allein
class  UpStore : public StoreObj, public UpRec
{
  public:
    // Tabelle in Datenbank erzeugen
    static  bool  CreateTable();
    static  bool  UpdateTable(long version);

    static  bool  CreateConstraints();
    static  bool  UpdateConstraints(long version);

    // Import / Export
    static  bool Import(const wxString &);
    static  bool Export(wxTextBuffer &);

  public:
    UpStore(Connection *connPtr = 0);    // Defaultkonstruktor
   ~UpStore();

    virtual void  Init();

  // API: Wettbewerb anlegen, aendern, loeschen, lesen ...
  public:
    bool  SelectAll();
    bool  SelectById(long id);
    bool  SelectByNr(long nr);
    bool  SelectByName(const wxString &name);

    bool  Insert();
    bool  Update();
    bool  Remove(long id = 0);

    // Check auf cpName, ob WB existiert
    bool  InsertOrUpdate();

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
