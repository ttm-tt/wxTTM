/* Copyright (C) 2020 Christoph Theis */

// Tabellendefintion und Engine der Wettbewerbe
#ifndef  PLLISTSTORE_H
#define  PLLISTSTORE_H

#include  "StoreObj.h"
#include  "PlStore.h"


// Tabellendaten in eigene Struktur
struct PlListRec : public PlRec
{
  PlListRec() {Init();}
  PlListRec(const PlRec &rec) {PlRec::operator=(rec);}

  void  Init()  {memset(this, 0, sizeof(PlListRec));}
};


// Sicht auf Tabelle allein
class  PlListStore : public StoreObj, public PlListRec
{
  // Tabelle in Datenbank erzeugen
  public:
    static  bool  CreateView();
    static  bool  RemoveView();

  public:
    PlListStore(Connection *connPtr = 0);      // Defaultkonstruktor
   ~PlListStore();

    virtual void  Init();

  // API: Wettbewerbe auswaehlen
  public:
    bool  SelectAll(long naID = 0); // Alle Spieler
    bool  SelectById(long id, const timestamp * = NULL);   // Spieler nach ID
    bool  SelectByNr(long nr);   // Spieler nach Nr

    // Alle potentiellen Partner fuer Spieler
    bool  SelectForDouble(const PlRec &pl, const CpRec &cp);
    
    // Alle Spieler, die noch nocht im WB gemeldet sind
    bool  SelectForCp(const CpRec &cp);

    long  Count();

    wxString GetNote();

    std::vector<timestamp> GetTimestamps(long psID);

  private:
    wxString  SelectString(const timestamp * = NULL) const;
    bool  BindRec();
};



#endif