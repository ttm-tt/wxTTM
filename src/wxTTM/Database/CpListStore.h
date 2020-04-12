/* Copyright (C) 2020 Christoph Theis */

// Tabellendefintion und Engine der Wettbewerbe
#ifndef  CPLISTSTORE_H
#define  CPLISTSTORE_H

#include "StoreObj.h"
#include "CpStore.h"

struct  CpRec;


// Tabellendaten in eigene Struktur
struct CpListRec : public CpRec
{
  CpListRec()   {Init();}
  CpListRec(const CpRec &);

  void  Init()  {memset(this, 0, sizeof(CpListRec));}
};


// Sicht auf Tabelle allein
class  CpListStore : public StoreObj, public CpListRec
{
  // Tabelle in Datenbank erzeugen
  public:
    static  bool  CreateView();
    static  bool  RemoveView();

  public:
    CpListStore(Connection *connPtr = 0);   // Defaultkonstruktor
   ~CpListStore();

    virtual void  Init();

  // API: Wettbewerbe auswaehlen
  public:
    bool  SelectAll();           // Alle Wettbewerbe
    bool  SelectById(long id);   // Wettbewerb nach ID
    bool  SelectById(const std::set<long> &ids);   // Wettbewerb nach ID
    bool  SelectByName(const wxString &);

  // Etwas API. Diese Funktionen aendern nicht den WB
  public:

  private:
    wxString  SelectString() const;
    bool  BindRec();
};



#endif