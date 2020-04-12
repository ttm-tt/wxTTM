/* Copyright (C) 2020 Christoph Theis */

// Liste der Nationen
#ifndef  NALISTSTORE_H
#define  NALISTSTORE_H

#include "StoreObj.h"
#include "NaStore.h"


// Tabellendaten in eigene Struktur
struct NaListRec : public NaRec
{
  NaListRec()   {Init();}
  NaListRec(const NaListRec &rec) {memcpy(this, &rec, sizeof(NaListRec));}

  void  Init()  {memset(this, 0, sizeof(this));}
};


// Sicht auf Tabelle allein
class  NaListStore : public StoreObj, public NaListRec
{
  // Tabelle in Datenbank erzeugen
  public:
    static  bool  CreateView();
    static  bool  RemoveView();

  public:
    NaListStore(Connection *connPtr = 0);     // Defaultkonstruktor
   ~NaListStore();

    virtual void  Init();

  // API: Verbaende auswaehlen
  public:
    bool  SelectAll();                 // Alle Nationen
    bool  SelectById(long id);         // Nation nach ID
    bool  SelectByName(const wxChar *);  // Nation nach Name

  // Etwas API. Diese Funktionen aendern nicht die Nation
  public:
};



#endif