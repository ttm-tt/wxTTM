/* Copyright (C) 2020 Christoph Theis */

// View ueber die Meldungen
#ifndef  NTLISTSTORE_H
#define  NTLISTSTORE_H

#include "StoreObj.h"

#include "NtStore.h"


// Tabellendaten in eigene Struktur
struct NtListRec : public NtRec
{
  long  cpID;           // Foreign key CpRec.cpID

  NtListRec()  {Init();}

  void  Init()  {memset(this, 0, sizeof(NtListRec));}
};


// Viewdaten
class  NtListStore : public StoreObj, public NtListRec
{
  // Tabelle in Datenbank erzeugen
  public:
    static  bool  CreateView();
    static  bool  RemoveView();

  public:
    NtListStore(Connection *connPtr = 0);   // Defaultkonstruktor
   ~NtListStore();

    virtual void  Init();
    
  public:
    bool  SelectByLt(const LtRec &, const timestamp * = NULL);
    short Count(const TmRec &);

    std::vector<std::pair<timestamp, long>> GetTimestamps(long ltID);

  private:
    wxString  SelectString(const timestamp * = NULL) const;
    bool  BindRec();
};



#endif