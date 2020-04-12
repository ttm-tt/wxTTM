/* Copyright (C) 2020 Christoph Theis */

// View fuer Verknuepfungen zwischen Gruppen
#ifndef  XXLISTSTORE_H
#define  XXLISTSTORE_H

#include "StoreObj.h"
#include "XxStore.h"



// Tabellendaten in eigene Struktur
struct XxListRec : public XxRec
{
  char  grName[9];
  char  grDesc[65];

  XxListRec()   {Init();}
  XxListRec(const XxRec &rec)  {Init(); XxRec::operator=(rec);}

  void  Init()  {memset(this, 0, sizeof(XxListRec));}
};




// Sicht auf Tabelle allein
class  XxListStore : public StoreObj, public XxListRec
{
  // Tabelle in Datenbank erzeugen
  public:
    static  bool  CreateView();
    static  bool  RemoveView();

  private:
    // So no one will use this
    XxListStore(Connection * = 0) {};
};


#endif
