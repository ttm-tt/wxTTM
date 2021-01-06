/* Copyright (C) 2020 Christoph Theis */

// Tabellendefiniton der Nationen

#ifndef  NASTORE_H
#define  NASTORE_H

#include "StoreObj.h"

#include  <string>


class  Statement;
class  ResultSet;


struct  NaRec
{
  long   naID;       // Unique ID
  wxChar naName[9];  // (Olympic) abbraviation
  wxChar naDesc[65]; // Full Name
  wxChar naRegion[65];   // (Full) Name of regional association

  NaRec()      {Init();}
  
  void  Init() {memset(this, 0, sizeof(NaRec));}

  // Import / Export als CSV-Datei
  bool Read(const wxString &);
  bool Write(wxString &) const;
};


class  NaStore : public StoreObj, public NaRec
{
  public:
    // Create / Alter Tables
    static  bool  CreateTable();
    static  bool  UpdateTable(long version);

    // Import / Export
    static  bool  Import(wxTextBuffer &is);
    static  bool  Export(wxTextBuffer &os, long version = 1);

  public:
    NaStore(Connection * = 0);                  // Defaultkonstruktor
    NaStore(const NaRec &, Connection * =0);
   ~NaStore();

    virtual void  Init();

  // API: Wettbewerb anlegen, aendern, loeschen, lesen ...
  public:
    bool  SelectAll();
    bool  SelectById(long id);
    bool  SelectByName(const wxString &name);
    bool  Insert();
    bool  Update();
    bool  Remove(long id = 0);

    // Check auf naName, ob Nation existiert
    bool  InsertOrUpdate();

  // Etwas API. Diese Funktionen aendern nicht die Nation
    long  NameToID(const wxString &name);

    // Max len of naName, naDesc, naRegion
    long  GetMaxNameLength();
    long  GetMaxDescLength();
    long  GetMaxRegionLength();

  private:
    wxString  SelectString() const;
    void  BindRec();
};
    


#endif
