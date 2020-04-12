/* Copyright (C) 2020 Christoph Theis */

// Tabellendefintion und Engine der Systeme
#ifndef  SYLISTSTORE_H
#define  SYLISTSTORE_H

#include "StoreObj.h"
#include "SyStore.h"

struct GrRec;


// Tabellendaten in eigene Struktur
struct SyListRec : public SyRec
{
  SyListRec()   {Init();}
  
  void  Init()  {memset(this, 0, sizeof(SyListRec));}
};


// Sicht auf Tabelle allein
class  SyListStore : public StoreObj, public SyListRec
{
  // Tabelle in Datenbank erzeugen
  public:
    static  bool  CreateView();
    static  bool  RemoveView();

  public:
    SyListStore(Connection *connPtr = 0);   // Defaultkonstruktor
   ~SyListStore();

    virtual void  Init();

  // API: Wettbewerbe auswaehlen
  public:
    bool  SelectAll();                  // Alle Spielsysteme
    bool  SelectById(long id);          // Spielsystem nach ID
    bool  SelectByGr(const GrRec &gr);  // Spielsystem fuer Gruppe

  private:
    wxString  SelectString() const;
    bool  BindRec();
  
};



#endif