/* Copyright (C) 2020 Christoph Theis */

// Tabelle der personenbezogenen Daten

#ifndef  PSSTORE_H
#define  PSSTORE_H

#include  "StoreObj.h"
#include  <string>



class  Statement;
class  ResultSet;

// Tabellendaten in eigene Struktur
struct PsRec
{
  long  psID;           // Unique ID
  struct 
  {
    wxChar psLast[64];    // Family name
    wxChar psFirst[64];   // Given name
  } psName;
  long  psBirthday;       // Year of Birth
  short psSex;            // Sex (1: Male. 2: Female)
  short psArrived;        // Flag: Arrived (0/1)
  wxChar psEmail[64]; 
  wxChar psPhone[64];
  short  psHasNote;       // Flag if there is a note attached
  timestamp psTimestamp;  // Update timestamp
  timestamp psDeleteTime; // Time recoord was deleted

  PsRec()       {Init();}
  
  void  Init()  {memset(this, 0, sizeof(PsRec));}
};


// Sicht auf Tabelle allein
class  PsStore : public StoreObj, public PsRec
{
  public:
    // Tabelle in Datenbank erzeugen
    static  bool  CreateTable();
    static  bool  UpdateTable(long version);
    static  bool  CreateConstraints();
    static  bool  UpdateConstraints(long version);

  public:
    PsStore(Connection *connPtr = 0);    // Defaultkonstruktor
    PsStore(const PsRec &rec, Connection *connPtr);
   ~PsStore();

    virtual void  Init();

  // API: Wettbewerb anlegen, aendern, loeschen, lesen ...
  public:
    bool  SelectAll();
    bool  SelectById(long id);
    bool  Insert();
    bool  Update();
    bool  Remove(long id = 0);

    bool  InsertNote(const wxString &note);
    wxString GetNote();

    // Check auf Name, ob Person existiert
    bool  InsertOrUpdate();
};



#endif