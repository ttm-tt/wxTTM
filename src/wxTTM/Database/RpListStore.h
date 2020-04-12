/* Copyright (C) 2020 Christoph Theis */

// View fuer RpRec
#ifndef RPLISTSTORE_H
#define RPLISTSTORE_H

#include "StoreObj.h"
#include "RpStore.h"

// Tabellendaten in eigene Struktur
struct RpListRec : public RpRec
{
  RpListRec()   {Init();}
  RpListRec(const RpRec &);

  void  Init()  {memset(this, 0, sizeof(RpListRec));}
};


// Sicht auf Tabelle allein
class  RpListStore : public StoreObj, public RpListRec
{
  // Tabelle in Datenbank erzeugen
  public:
    static  bool  CreateView();
    static  bool  RemoveView();

  public:
    RpListStore(Connection *connPtr = 0);   // Defaultkonstruktor
   ~RpListStore();

    virtual void  Init();

  // API: Wettbewerbe auswaehlen
  public:
    bool  SelectAll();           // Alle Wettbewerbe
    bool  SelectById(long id);   // Wettbewerb nach ID

  // Etwas API. Diese Funktionen aendern nicht den WB
  public:

  private:
    wxString  SelectString() const;
    bool  BindRec();
};




#endif