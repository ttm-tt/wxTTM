/* Copyright (C) 2020 Christoph Theis */

// View ueber die Mannschaften
#ifndef  TMLISTSTORE_H
#define  TMLISTSTORE_H

#include "StoreObj.h"
#include "TmStore.h"

struct CpRec;
struct PlRec;


// Tabellendaten in eigene Struktur
struct TmListRec : TmRec
{
  long  naID;

  TmListRec()  {Init();}

  void  Init()  {memset(this, 0, sizeof(TmListRec));}
};


// Viewdaten
class  TmListStore : public StoreObj, public TmListRec
{
  // Tabelle in Datenbank erzeugen
  public:
    static  bool  CreateView();
    static  bool  RemoveView();

  public:
    TmListStore(Connection *connPtr = 0);   // Defaultkonstruktor
   ~TmListStore();

    virtual void  Init();

  public:
    bool  SelectById(long id);
    bool  SelectByCp(const CpRec &, long naID = 0);
    bool  SelectByPl(const PlRec &);
    bool  SelectByCpPl(const CpRec &, const PlRec &, const timestamp * = NULL);
    bool  SelectByLt(const LtRec &, const timestamp * = NULL);
    bool  SelectByName(const wxString &, const CpRec &);

  private:
    wxString  SelectString(const timestamp * = NULL) const;
    bool  BindRec();
};



#endif