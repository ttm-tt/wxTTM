/* Copyright (C) 2020 Christoph Theis */

// Tabellendefintion und Engine der Mdsteme
#ifndef  MDLISTSTORE_H
#define  MDLISTSTORE_H

#include "StoreObj.h"
#include "MdStore.h"


// Tabellendaten in eigene Struktur
struct MdListRec : public MdRec
{
  MdListRec()    {Init();}
  
  void  Init()   {memset(this, 0, sizeof(MdListRec));}
};


// Sicht auf Tabelle allein
class  MdListStore : public StoreObj, public MdListRec
{
  // Tabelle in Datenbank erzeugen
  public:
    static  bool  CreateView();
    static  bool  RemoveView();

  public:
    MdListStore(Connection *connPtr = 0);   // Defaultkonstruktor
   ~MdListStore();

    virtual void  Init();

  // API: Wettbewerbe auswaehlen
  public:
    bool  SelectAll();           // Alle Modi
    bool  SelectById(long id);   // Modus nach ID
    bool  SelectById(const std::set<long> idList);   // Modus nach ID

  // Etwas API. Diese Funktionen aendern nicht den WB
  public:

  private:
    wxString  SelectString() const;
    bool  BindRec();
};



#endif