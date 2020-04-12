/* Copyright (C) 2020 Christoph Theis */

// Tabellendefintion und Engine der Wettbewerbe
#ifndef  STLISTSTORE_H
#define  STLISTSTORE_H

#include "StoreObj.h"
#include "StStore.h"

struct  CpRec;
struct  NaRec;
struct  GrRec;
struct  StEntry;


// Tabellendaten in eigene Struktur
struct StListRec : public StRec
{
  long  naID;
  
  StListRec()  {Init();}
  StListRec(const StEntry &);

  void  Init()  {memset(this, 0, sizeof(StListRec));}
};


// Sicht auf Tabelle allein
class  StListStore : public StoreObj, public StListRec
{
  // Tabelle in Datenbank erzeugen
  public:
    static  bool  CreateView();
    static  bool  RemoveView();

  public:
    StListStore(Connection *connPtr = 0);   // Defaultkonstruktor
   ~StListStore();

    virtual void  Init();

  // API: Wettbewerbe auswaehlen
  public:
    bool  SelectAll(const GrRec &);  // Alle Setzungen
    bool  SelectById(long id);       // Setzung nach ID
    bool  SelectById(const std::set<long> &ids);       // Setzung nach ID
    bool  SelectAll(const CpRec &cp, const char *stage);
    bool  SelectByCpNa(const CpRec &cp, const NaRec &na);
    bool  SelectByCpTm(const CpRec &cp, const TmRec &tm, const timestamp * = NULL);
    bool  SelectByGrTm(const GrRec &cp, const TmRec &tm);
    
// Etwas API. Diese Funktionen aendern nicht den WB
  public:
    long  Count();

    short CountNoCons(const CpRec &cp, const char *stage = NULL);

    std::vector<std::pair<timestamp, long>> GetTimestamps(long tmID);

  private:
    wxString  SelectString(const timestamp * = NULL) const;
    bool  BindRec();
};



#endif