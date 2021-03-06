/* Copyright (C) 2020 Christoph Theis */

// Tabellendefintion und Engine der Spielsysteme
#ifndef  SYSTORE_H
#define  SYSTORE_H

#include  "StoreObj.h"

#include  <string>


class  Statement;
class  ResultSet;

// Tabellendaten in eigene Struktur
struct SyRec
{
  long      syID;        // Unique ID
  wxChar    syName[9];   // Short name ("COR")
  wxChar    syDesc[65];  // Full description ("Cobillon Cup")
  short     sySingles;   // Number of Singles matches
  short     syDoubles;   // Number of Doubles matches
  short     syMatches;   // Number of matches
  short     syComplete;  // Complete all matches

  struct    SyList
  {
    short   syType;      // Typ (Single / Double)
    long    syPlayerA;   // Verweis auf Setzung A
    long    syPlayerX;   // Verweis auf Setzunx X
  } *syList;

  SyRec() {memset(this, 0, sizeof(SyRec));}
 ~SyRec() {delete[] syList;}
 
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
