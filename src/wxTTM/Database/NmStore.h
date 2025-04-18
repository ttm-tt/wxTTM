/* Copyright (C) 2020 Christoph Theis */

// Tabellendefintion und Engine der Wettbewerbe
#ifndef  NMSTORE_H
#define  NMSTORE_H

#include  "StoreObj.h"

#include  <string>


class  Statement;
class  ResultSet;

class  NmStore;

struct MtRec;
struct TmRec;
struct TmEntry;


// Meldung Single / Double
struct NmSingle
{
  long  nmID;         // Referenz NmRec
  short nmNr;         // Nummer der Meldung
  long  ltA;          // Referenz LtRec
  bool  nmOptional;

  void  Init() {memset(this, 0, sizeof(NmSingle));}
};


struct  NmDouble
{
  long  nmID;          // Referenz NmRec
  short nmNr;          // Nummer der Meldung
  long  ltA;           // Referenz LtRec
  long  ltB;           // Referenz LtRec
  bool  nmOptional;

  void  Init() {memset(this, 0, sizeof(NmDouble));}
};


class  NmSingleStore : public StoreObj, public NmSingle
{
  public:
    // Tabelle in Datenbank erzeugen
    static  bool  CreateTable();
    static  bool  UpdateTable(long version);


    static  bool  CreateConstraints();
    static  bool  UpdateConstraints(long version);

  public:
    NmSingleStore(NmStore &);
   ~NmSingleStore();

    virtual void Init();

  public:
    bool  Insert();
    bool  Update();
    bool  Remove();

    bool  SelectAll();

  private:
    NmSingleStore();
};


class  NmDoubleStore : public StoreObj, public NmDouble
{
  public:
    // Tabelle in Datenbank erzeugen
    static  bool  CreateTable();
    static  bool  UpdateTable(long version);


    static  bool  CreateConstraints();
    static  bool  UpdateConstraints(long version);

  public:
    NmDoubleStore(NmStore &);
   ~NmDoubleStore();

    virtual void Init();

  public:
    bool  Insert();
    bool  Update();
    bool  Remove();

    bool  SelectAll();

  private:
    NmDoubleStore();
};


// -----------------------------------------------------------------------
// Tabellendaten in eigene Struktur
struct NmRec
{
  long  nmID;  // Primary key
  long  tmID;  // Referenz TmRec
  long  mtID;  // Referenz MtRec

  NmRec();
  NmRec(const NmRec &);
 ~NmRec();

  void  Init();

  NmRec & operator=(const NmRec &);

  // idx ist 0-basiert
  long  GetSingle(short idx)  const  {return (idx < nofSingles ? nmSingle[idx].ltA : 0);}
  long  GetDoubleA(short idx) const  {return (idx < nofDoubles ? nmDouble[idx].ltA : 0);}
  long  GetDoubleB(short idx) const  {return (idx < nofDoubles ? nmDouble[idx].ltB : 0);}

  short GetNofSingles() const        {return nofSingles;}
  short GetNofDoubles() const        {return nofDoubles;}

  void  SetSingle(short idx, long lt, bool optional = false);
  void  SetDoubles(short idx, long ltA, long ltB, bool optional = false);

  void  SetNofSingles(short nof);
  void  SetNofDoubles(short nof);

  // Folgendes steht in eigenen Tabellen
  short nofSingles;
  short nofDoubles;

  struct  NmSingle
  {
    long  ltA = 0;
    bool  nmOptional = false;

    bool  isNew;        // Not persisted
  }  *nmSingle;

  struct  NmDouble
  {
    long  ltA = 0;
    long  ltB = 0;
    bool  nmOptional = false;

    bool  isNew;        // Not persisted
  }  *nmDouble;
};


// Sicht auf Tabelle allein
class  NmStore : public StoreObj, public NmRec
{
  public:
    // Tabelle in Datenbank erzeugen
    static  bool  CreateTable();
    static  bool  UpdateTable(long version);


    static  bool  CreateConstraints();
    static  bool  UpdateConstraints(long version);

    // Import / Export
    static bool Import(wxTextBuffer& is);
    static bool Export(wxTextBuffer& is, short cpType, const std::vector<long> & idList, bool append, long version);
    static long GetMaxSupportedExportVersion() { return 1; }

  public:
    NmStore(Connection * = 0);  // Defaultkonstruktor
   ~NmStore();
     
    virtual void Init();

  public:
    bool  Insert(const MtRec &mt, const TmEntry &tm);
    bool  Insert(long mtID, long tmID);
    bool  Update();
    bool  Remove();

    bool  Next();
  
    bool  SelectByMtTm(const MtRec &mt, const TmEntry &tm);
    bool  SelectByMtTm(long mtID, long tmID);

  private:
    wxString  SelectString() const;
    void  BindRec();
};


// -----------------------------------------------------------------------
inline  NmRec::NmRec()
{
  nmSingle = 0;
  nmDouble = 0; 

  Init();
}


inline  NmRec::NmRec(const NmRec &nm)
{
  nmSingle = 0;
  nmDouble = 0;

  *this = nm;
}


inline  NmRec::~NmRec()
{
  delete[] nmSingle;
  delete[] nmDouble;
}


inline  void  NmRec::  Init()
{
  delete[] nmSingle; 
  delete[] nmDouble; 

  memset(this, 0, sizeof(NmRec));
}



#endif