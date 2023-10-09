/* Copyright (C) 2020 Christoph Theis */

// Tabellendefintion und Engine der Spielsysteme
#ifndef  SYSTORE_H
#define  SYSTORE_H

#include  "StoreObj.h"

#include  <string>


class  Statement;
class  ResultSet;

struct SyRec;

// Definition of one team match
// Define it here so we can access the list in SyStore and SyListStore as well
struct  SyMatchRec
{
  long   syID;      // Foreign key
  short  syNr;      // Laufende Nr
  short  syType;    // Single / Double
  long   syPlayerA; // Verweis auf Setzung A
  long   syPlayerX; // Verweis auf Setzung X  

  SyMatchRec()  {Init();}

  void  Init()  {memset(this, 0, sizeof(SyMatchRec));}
};


class  SyMatchStore : public StoreObj, public SyMatchRec
{
  public:
    static  bool  CreateTable();
    static  bool  UpdateTable(long version);

    static  bool  CreateConstraints();
    static  bool  UpdateConstraints(long version);

    SyMatchStore(Connection *connPtr = 0) : StoreObj(connPtr) {}

    virtual void  Init();

    bool  Select(long id);
    bool  Insert(SyRec *);
    bool  Remove(long id);
};


// Tabellendaten in eigene Struktur
struct SyRec
{
  long      syID;        // Unique ID
  wxChar    syName[9];   // Short name ("COR")
  wxChar    syDesc[65];  // Full description ("Cobillon Cup")
  short     sySingles;   // Number of singles matches
  short     syDoubles;   // Number of doubles matches
  short     syMatches;   // Number of matches
  short     syComplete;  // Complete all matches

  struct    SyList
  {
    short   syType;      // Typ (Single / Double)
    long    syPlayerA;   // Verweis auf Setzung A
    long    syPlayerX;   // Verweis auf Setzunx X
  } *syList = nullptr;

  SyRec() {memset(this, 0, sizeof(SyRec));}
 ~SyRec() {delete[] syList; syList = nullptr;}

  SyRec & operator=(const SyRec &sy);
 
  void  ChangeSize(int mt);
};


// Sicht auf Tabelle allein
class  SyStore : public StoreObj, public SyRec
{
  public:
    // Tabelle in Datenbank erzeugen
    static  bool  CreateTable();
    static  bool  UpdateTable(long version);

    static  bool  CreateConstraints();
    static  bool  UpdateConstraints(long version);

    // Default Systeme
    static  bool  AddCorbillonCup();
    static  bool  AddModifiedCorbillonCup();
    static  bool  AddModifiedSwaythlingCup();
    static  bool  AddOlympicTeamSystem();
    static  bool  AddOlympicTeamSystem2Players();
    static  bool  AddECTeamSystem();
    static  bool  AddMixedTeamSystem();
    static  bool  AddMixedTeamSystemA();
    static  bool  AddYouthSeriesTeamSystem();
    static  bool  AddYouthSeriesTeamSystemA();

  public:
    SyStore(Connection * = 0);  // Defaultkonstruktor
   ~SyStore();

    virtual void  Init();

  // API: Wettbewerb anlegen, aendern, loeschen, lesen ...
  public:
    bool  SelectAll();
    bool  SelectById(long id);
    bool  SelectByName(const wxString &name);
    bool  Insert();
    bool  Update();
    bool  Remove(long id = 0);

    bool  Next();

    // Check auf cpName, ob WB existiert
    bool  InsertOrUpdate();

  // Etwas API. Diese Funktionen aendern nicht den WB
  public:
};



#endif
