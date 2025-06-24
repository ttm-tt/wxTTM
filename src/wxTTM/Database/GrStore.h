/* Copyright (C) 2020 Christoph Theis */

// Tabelle der Gruppen
# ifndef  GRSTORE_H
# define  GRSTORE_H

#include  "CpListStore.h"
#include  "StoreObj.h"
#include  "Rec.h"

#include  <iostream>
#include  <string>


class  Statement;
class  ResultSet;

struct MtRec;
struct TmRec;
struct TmEntry;

struct MdRec;
struct SyRec;

inline short ld(short x)
{
  if (!x || x == 1)
    return 0;
   
  short res;
  
  for (res = 31; !(x & (1 << res)); res--)
    ;
    
  return res;
}


struct GrRec
{
  long  grID;   // Unique Key
  long  cpID;   // Foreign key CP
  long  syID;   // Foreign key SY
  long  mdID;   // Foreign key MD

  wxChar  grName[9];   // Kurzname Gruppe
  wxChar  grDesc[65];  // Beschreibung Gruppe
  wxChar  grStage[65]; // Beschreibung Stufe

  short grModus;     // 1: RR, 2: SKO, 3: DKO, 4: PLO
  short grSize;      // Anzahl Spieler
  short grWinner;    // Sieger ist Nr. XXX in Stufe  

  short grBestOf;    // Zahl der Saetze, wird nur fuer Anzeige verwendet
  
  short grQualRounds; // Erste Runde(n) sind Qualification

  short grNofRounds;  // Anzahl Runden, die in diese Gruppe gespielt wird
  short grNofMatches; // Anzahl Spiele, die in der ersten Runde gespielt werden

  short grNoThirdPlace;    // Kein Spiel um Platz 3
  short grOnlyThirdPlace;  // Nur Spiel um Platz 3

  short grPublished;       // Ready to publish on internet

  short grHasNotes;        // Flag, ob die Gruppe eine Anmerkung hat

  short grSortOrder;       // Sort order

  timestamp grPrinted;     // When was the draw printed

  GrRec() {Init();}
  GrRec & operator=(const GrRec &rec);

  void  Init() {memset(this, 0, sizeof(GrRec));}

  // Etwas API
  short NofRounds(bool chance = false) const;
  short NofMatches(short rd, bool chance = false) const;
  
  // Zumindest die FROM-Seite wird beim Drucken benoetigt
  long  QryToGroupWinner(const MtRec &);
  short QryToRoundWinner(const MtRec &);
  short QryToMatchWinner(const MtRec &);
  short QryToAXWinner(const MtRec &);

  long  QryToGroupLoser(const MtRec &);
  short QryToRoundLoser(const MtRec &);
  short QryToMatchLoser(const MtRec &);
  short QryToAXLoser(const MtRec &);

  bool  QryToWinner(const MtRec &, MtRec *, short &ax);
  bool  QryToLoser(const MtRec &, MtRec *, short &ax);   
  
  short QryFromRound(const MtRec &, int ax); 
  short QryFromMatch(const MtRec &, int ax);
};


inline  GrRec & GrRec::operator=(const GrRec &rec)
{
  memcpy(this, &rec, sizeof(GrRec)); 
  return *this;
}


inline  short GrRec::NofRounds(bool chance) const
{
  switch (grModus)
  {
    case MOD_RR :
      if (chance)
        return 0;
      return (grSize == 0 || grSize & 0x1 ? grSize : grSize-1);
      
    case MOD_SKO :
      if (chance)
        return 0;
      else
        return ld(grSize);
      
    case MOD_DKO :
    case MOD_MDK :
      return chance ?  2 * (ld(grSize) - 1) + 1 : ld(grSize) + 1;
      
    case MOD_PLO :
      if (chance)
        return 0;
      return ld(grSize);

    default :
      return 0;
  }
}


inline  short GrRec::NofMatches(short rd, bool chance) const
{
  if (rd < 0 || rd > NofRounds(chance))
    return 0;

  switch (grModus)
  {
    case MOD_RR :
      if (chance)
        return 0;
        
      if (rd > 0)
        return (grSize / 2);
      else
        return NofRounds() * (grSize / 2);

    case MOD_SKO :
      if (chance)
        return 0;
        
      if (rd > 0)
        return (1 << (NofRounds() - rd));
      else
        return grSize - 1;

    case MOD_DKO :
    case MOD_MDK :
      if (rd == NofRounds(chance))
        return 1;
      if (chance)
        return (0x1 << ((NofRounds(chance) - rd - 1) / 2));
      else if (rd > 0)
        return (1 << (NofRounds() - rd - 1));
      else
        return grSize - 1;

    case MOD_PLO :
      if (chance)
        return 0;
        
      if (grOnlyThirdPlace)
      {
        if (rd == NofRounds())
          return 2;
        else if (rd > 0)
          return (1 << (NofRounds() - rd));
        else
          return grSize;  // grSize - 1 (fuer KO) + 1 (3-ter Platz)
      }
      else
      {
        if (rd > 0)
          return (grSize / 2);
        else
          return NofRounds() * (grSize / 2);
      }

    default :
      return 0;
  }
}


class  GrStore : public StoreObj, public GrRec
{
  public:
    struct CreateGroupStruct
    {
      Connection *connPtr;
      CpRec       cp;
      GrRec       gr;
      wxString nameTempl;
      wxString descTempl;
      short       start;
      short       count;
      short       numeric;
      bool        delConnPtr;
    };

  public:
    // Tabelle in Datenbank erzeugen
    static  bool  CreateTable();
    static  bool  UpdateTable(long version);

    static  bool  CreateConstraints();
    static  bool  UpdateConstraints(long version);

    // Import / Export
    static  bool  Import(wxTextBuffer &is);
    static  bool  Export(wxTextBuffer &os, short cpType, const std::vector<long> &, bool append, long version = 1);
    static  long  GetMaxSupportedExportVersion() {return 1;}

  public:
    GrStore(Connection *ptr = 0) : StoreObj(ptr) {};
    GrStore(const GrRec &rec, Connection *ptr = 0) : StoreObj(ptr), GrRec(rec) {}
    GrStore & operator=(const GrRec &rec);

    virtual void Init();
  
  public:
    bool  Insert(const CpRec &);
    bool  Update();
    bool  Remove(long id = 0);

    // Check auf cpID + grName, ob WB existiert
    bool  InsertOrUpdate();

    bool  SelectAll(const CpRec &);
    bool  SelectAll(const CpRec &, const wxString &stage);
    bool  SelectById(long id);
    bool  SelectByName(const wxString &name, const CpRec &);

  // API
  public:
    static  unsigned CreateGroup(void *arg);
    static  unsigned UpdateGroup(void *arg);
    
    bool  ClearAllTeams();

    bool  SetTeam(short pos, const TmEntry &tm, unsigned flags = 0);
    bool  SetTeam(short pos, const TmRec &tm, unsigned flags = 0);

    bool  SetWinner(const MtRec &);
    bool  SetLoser(const MtRec &);

    bool  SetPublish(bool publish);

    // Tabelle berechnen und speichern bzw. Tabelle loeschen
    bool  SetTable();
    bool  ClearTable();

    // Flags berechnen
    bool  QryStarted();
    bool  QryFinished();
    bool  QryUnchecked();
    bool  QryDraw();
    bool  QryScheduled();
    bool  QryCombined();

    // Fuer die Auslosung, Drucken, ...
    short CountGroups();
    short CountGroups(const CpRec &);
    short CountGroups(const CpRec &, const wxString &stage);
    bool  ClearDraw(const CpRec &, const wxString &stage);
    bool  ClearDraw(long grID);

    // Fuer MdStore und SyStore
    short CountGroups(const MdRec &);
    short CountGroups(const SyRec &);

    // Notizen an Gruppe
    bool  InsertNote(const wxString &note);
    wxString GetNote();

    // Calculate sort order
    short CalculateSortOrder();

    // Set printed time
    bool SetPrinted(timestamp &ts);
private:
    wxString  SelectString() const;
    bool  BindRec();    

};


inline  GrStore & GrStore::operator=(const GrRec &rec)
{
  ((GrRec *) this)->operator=(rec);
  return *this;
}

# endif