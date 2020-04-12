/* Copyright (C) 2020 Christoph Theis */

// View ueber die Meldungen
#ifndef  LTLISTSTORE_H
#define  LTLISTSTORE_H

#include "StoreObj.h"

#include "LtStore.h"


// Tabellendaten in eigene Struktur
// Viewdaten
class  LtListStore : public StoreObj, public LtRec
{
  // Tabelle in Datenbank erzeugen
  public:
    static  bool  CreateView();
    static  bool  RemoveView();

  public:
    LtListStore(Connection *connPtr = 0);   // Defaultkonstruktor
   ~LtListStore();

    virtual void  Init();
    
  public:
    bool  SelectById(long id);
    bool  SelectByPl(long plID, const timestamp * = NULL);

    std::vector<std::pair<timestamp, long>> GetTimestamps(long plID);

  private:
    wxString  SelectString(const timestamp * = NULL) const;
    bool  BindRec();
};



#endif