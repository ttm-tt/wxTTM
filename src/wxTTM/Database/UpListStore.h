/* Copyright (C) 2020 Christoph Theis */

// Tabellendefintion und Engine der Wettbewerbe
#ifndef  UPLISTSTORE_H
#define  UPLISTSTORE_H

#include  "StoreObj.h"
#include  "UpStore.h"


// Tabellendaten in eigene Struktur
struct UpListRec : public UpRec
{
  UpListRec() {Init();}
  UpListRec(const UpRec &rec) {UpRec::operator=(rec);}

  void  Init()  {memset(this, 0, sizeof(UpListRec));}
};


// Sicht auf Tabelle allein
class  UpListStore : public StoreObj, public UpListRec
{
  // Tabelle in Datenbank erzeugen
  public:
    static  bool  CreateView();
    static  bool  RemoveView();

  public:
    UpListStore(Connection *connPtr = 0);      // Defaultkonstruktor
   ~UpListStore();

    virtual void  Init();

  // API: Wettbewerbe auswaehlen
  public:
    bool  SelectAll();           // Alle Spieler
    bool  SelectById(long id);   // Spieler nach ID
    bool  SelectByNr(long nr);   // Spieler nach Nr

    long  Count();

  private:
    wxString  SelectString() const;
    bool  BindRec();
};



#endif