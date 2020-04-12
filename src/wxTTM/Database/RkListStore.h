/* Copyright (C) 2020 Christoph Theis */

// View ueber das Ranking
#ifndef  RKLISTSTORE_H
#define  RKLISTSTORE_H

#include "StoreObj.h"

#include "RkStore.h"


// Tabellendaten in eigene Struktur
struct RkListRec : public RkRec
{
  long  cpID;           // Foreign key CpRec.cpID

  RkListRec()  {Init();}

  void  Init()  {memset(this, 0, sizeof(RkListRec));}
};


// Viewdaten
class  RkListStore : public StoreObj, public RkListRec
{
  // Tabelle in Datenbank erzeugen
  public:
    static  bool  CreateView();
    static  bool  RemoveView();

  public:
    RkListStore(Connection *connPtr = 0);   // Defaultkonstruktor
   ~RkListStore();

    virtual void  Init();
    
  public:
    bool   SelectByCp(const CpRec &);
    bool   SelectByTm(const TmRec &tm);
    short  CountDirectEntries(const CpRec &, const NaRec &);
    short  CountQualifiers(const CpRec &, const NaRec &);

  private:
    wxString  SelectString() const;
    bool  BindRec();
};



#endif