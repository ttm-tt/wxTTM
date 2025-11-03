/* Copyright (C) 2020 Christoph Theis */

// Tabellendefintion und Engine der Wettbewerbe
#ifndef  GRLISTSTORE_H
#define  GRLISTSTORE_H

#include "StoreObj.h"
#include "GrStore.h"

struct CpListRec;
struct MpRec;


// Tabellendaten in eigene Struktur
struct GrListRec : public GrRec
{
  wxChar cpName[9];
  wxChar syName[9];
  wxChar mdName[9];
  timestamp stTimestamp = {0};
  timestamp mtTimestamp = {0};

  GrListRec()  {Init();}

  void  Init()  {memset(this, 0, sizeof(GrListRec));}
};




// Sicht auf Tabelle allein
class  GrListStore : public StoreObj, public GrListRec
{
  // Tabelle in Datenbank erzeugen
  public:
    static  bool  CreateView();
    static  bool  RemoveView();

  public:
    GrListStore(Connection *connPtr = 0);   // Defaultkonstruktor
   ~GrListStore();

    virtual void  Init();

  public:
    bool  SelectAll(const CpRec &);      // All groups
    bool  SelectAll(const MdRec &);      // All RR-groups with Group Modus
    bool  SelectAll(const MpRec &);      // All groups with match points
    bool  SelectAll(const SyRec&);       // All groups with team system
    bool  SelectById(long id);           // All groups of given event
    bool  SelectById(const std::set<long> &idList);
    bool  SelectByStage(const CpRec &, const wxString &stage);
    bool  SelectByCpTm(const CpRec &, const TmRec &);
    bool  SelectByTm(const TmRec &);

    std::list<wxString> ListStages(const CpRec &);  // Alle Stufen
    
    bool  QryStarted();
    bool  QryFinished();
    bool  QryUnchecked();
    bool  QryCombined();

    long  Count();

    wxString GetNote();

    short GetLastScheduledRound(long id);

  private:
    wxString  SelectString() const;
    bool  BindRec();
};



#endif