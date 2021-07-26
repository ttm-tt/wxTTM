/* Copyright (C) 2020 Christoph Theis */

// Drawinf.h
// Verwaltung der Auslosung
// 22.04.94 (ChT)
// Neufassung 18.09.94 (ChT)

#ifndef  DRAWINF_H
#define  DRAWINF_H

#include  "TmListStore.h"
#include  "NaListStore.h"
#include  "StListStore.h"
#include  "RkListStore.h"
#include  "RkEntryStore.h"
#include  <list>


// -----------------------------------------------------------------------
typedef enum
{
  None,     // No ranking used
  Groups,   // Results of groups: winner of group 1 is #1, of group 2 is #2, ...
  World     // Int'l ranking used, winners of group take ranking position of highest ranked player in group
} RankingChoice;


// zwei Hilfen: 2-er Potenz und 2-er Logarithmus
static int  exp2(int i)
{
	return 1 << i;
}


static int  ld2(int i)
{
  int j;
  for (j = 0; exp2(j+1) <= i; j++)
		;
  return j;
}


// Berechnung, wieviele Freilose nach oben / unten muessen
extern void CalculateByes(int stg, int sec, int lastStage, 
                          int dbc, int &dbu, int &dbl);


// Vorwaerts Referenzen
class  DrawItemPack;

// Basis fuer Eintrage in Liste
class  DrawItem 
{
  public:
    virtual ~DrawItem();
    
    struct  CompareItemFunctional
    {
      bool  operator()(DrawItem *&i1, DrawItem *&i2) 
      { 
        return *i1 < *i2;
      }
    }; 
    
    
    bool operator<(const DrawItem &item) const {return key < item.key;}

  public:
    // Sortierschluessel    
    int  key;
};


typedef  std::list<DrawItem *> DrawItemList;

// Basis fuer Listen
class  DrawList : public DrawItemList
{
  public:
    virtual ~DrawList();

  public:
    // Mischen der Liste
    void  Shuffle();
    void  AddItem(DrawItem *itemPtr) {push_back(itemPtr);}
    DrawItem * CutFirst();
    DrawItem * CutLast();

    int   Count() {return size();}
    void  Sort()  {sort(DrawItem::CompareItemFunctional());}
};



// Eintrag in Liste der Teams
// Entspricht einem Team der DBK
class  DrawItemTeam : public DrawItem
{
  public:
    // Konstanten
    enum  {MAX_POS = 32};  // Mehr als ein 4Mrd-Feld wird es nicht geben

    // Konstruktor
    DrawItemTeam();
    DrawItemTeam(const TmListRec &);
    DrawItemTeam(const RkEntry &);
   ~DrawItemTeam();

    bool IsSeeded(int stg)    {return pos[stg] != 0;}       // Spieler ist gesetzt
    bool IsBye()              {return tm.tmID == 0;}        // Spieler ist ein Freilos

    // Variablen zur Verwaltung
    TmListRec  tm;            // Team
    int   pos[MAX_POS];       // Nr der Gruppe / Position in Gruppe

    // Qualifcation --> CP + CO
    long  lastGroup;          // id der letzten Gruppe
    int   lastPos;            // Plazierung in Gruppe
    
    // Qualification + CP
    bool  rkDirectEntry;      // Qualifier / Direct Entry
    int   rkNatlRank;         // Nationales ranking
    int   rkIntlRank;         // Internationalies Ranking
    int   rankPts;            // Ranking points
    
    bool  mask;
    
    // Variablen zur Listenverwaltung
    DrawItemPack *pack;      // Verweis auf Pack
};



// Liste der Teams
class  DrawListTeam : public DrawList
{
  public:
    DrawItemTeam * Add(const TmListRec &);
    DrawItemTeam * Get(const TmListRec &);
    DrawItemTeam * GetTeam(long tmID);

    // Erstes Team auf pos[stg] == sec
    DrawItemTeam * GetFirst(int stg, int sec);
    
    // Eintrag einfuegen /entfernen
    DrawItemTeam * Add(DrawItemTeam *item)      {push_back(item); return item;}
    DrawItemTeam * Subtract(DrawItemTeam *item) {remove(item); return item;}

    // Zaehlen aller Teams
    int  Count()  {return size();}
    
    // Zaehlen der Teams auf pos[stage]
    int  Count(int stage);    

    // Zaehlen der Teams auf pos[stage] == sec
    int  Count(int stage, int sec);
};


// Eintrag in Liste der Nationen
class  DrawItemNation : public DrawItem
{
  public:
    // Konstruktor
    DrawItemNation(const NaRec &);

    // Add, Get eines Teams
    DrawItemTeam * Add(const TmListRec &tm)  {return teams.Add(tm);}
    DrawItemTeam * Get(const TmListRec &tm)  {return teams.Get(tm);}
    DrawItemTeam * GetTeam(long tmID)        {return teams.GetTeam(tmID);}
    
    DrawItemTeam * GetTeam(long grID, short pos);

    DrawItemTeam * Add(DrawItemTeam *item)       {teams.push_back(item); return item;}
    DrawItemTeam * Subtract(DrawItemTeam *item)  {teams.remove(item); return item;}

    // Erstes Team auf (stg, sec)
    DrawItemTeam * GetFirst(int stg, int sec)  {return teams.GetFirst(stg, sec);}

    // Liste der Teams einer Nation
    DrawListTeam  teams;

    NaRec na;

    // Zaehlen aller Teams
    int  Count()        {return teams.Count();}

    // Zaehlen der Teams auf pos[stage]
    int  Count(int stage) {return teams.Count(stage);}
    // Zaehlen der Teams auf pos[stage] == sec
    int  Count(int stage, int sec) {return teams.Count(stage, sec);}
};


// Liste der Nationen
class  DrawListNation : public DrawList
{
  public:
    // Add, Get Nation
    DrawItemNation * Add(const NaRec &);
    DrawItemNation * Get(const NaRec &na);    
    
    DrawItemNation * Subtract(DrawItemNation *item) {remove(item); return item;}
    DrawItemNation * Add(DrawItemNation *item)      {push_back(item); return item;}

    // Add, Get Team 
    DrawItemTeam * AddTeam(const TmListRec &);
    DrawItemTeam * AddTeam(const StListRec &);
    DrawItemTeam * AddTeam(const RkListRec &);
    DrawItemTeam * AddTeam(const RkEntry &);
    DrawItemTeam * GetTeam(const TmListRec &);
    DrawItemTeam * GetTeam(const RkListRec &);
    DrawItemTeam * GetTeam(long);
    DrawItemTeam * GetTeam(long grID, short pos);

    // Add, Subtract eines Eintrags aus Teamliste
    DrawItemTeam * Add(DrawItemTeam *);
    DrawItemTeam * Subtract(DrawItemTeam *);

    // Zahl aller Teams
    int  Count() {return size();}
    // Zaehlen der Teams auf pos[stage] == sec
    int  Count(int stage, int sec);

  protected:
    virtual DrawItemNation * GetNation(long);
};


// Liste der Regionalverbaende
class DrawListRegion : public DrawListNation
{
  public:
    ~DrawListRegion();

  public:
    void AddTeamItem(DrawItemTeam *item, long id) { GetNation(id)->teams.Add(item); }
};


// Eintrag in Liste der Packs
class DrawItemPack : public DrawItem
{
  public:
    // Konstruktor
    DrawItemPack();
    DrawItemPack(DrawItemTeam *item1, DrawItemTeam *item2);
   ~DrawItemPack();

    // enthaelt 2 Teams
    DrawItemTeam * teams[2];

    int  Add(DrawItemTeam *);
};



// Liste der Packs
class  DrawListPack : public DrawList
{
  public:
    void  Add(DrawItemPack *ptr) {push_back(ptr);}
};



#endif