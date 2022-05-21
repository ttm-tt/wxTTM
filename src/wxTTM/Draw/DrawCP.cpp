/* Copyright (C) 2020 Christoph Theis */

// Auslosung zweite Stufe Oldies (Championship)
// 09.10.94 (ChT)
// 04.06.95 (ChT): Eine gueltige Tabelle wird vorrausgesetzt
// 09.05.96 (ChT): Ausgewaehlter WB wird Default
// 19.06.97 (ChT): Auslosung auch dann, wenn die Gruppe zwar nicht beendet,
//                 aber eine gueltige Tabelle vorliegt

#include "stdafx.h"
#include "TT32App.h"

#include  "DrawCP.h"

#include  "GrStore.h"
#include  "MtStore.h"
#include  "NaStore.h"
#include  "StStore.h"       // 'Setzung'
#include  "StListStore.h"
#include  "StEntryStore.h"

#include  "TbItem.h"
#include  "TBSort.h"        // Sortieren der Tabelle

#include  "InfoSystem.h"

#include  <stdlib.h>
#include  <time.h>

#include  <stdlib.h>
#include  <time.h>
#include  <assert.h>

#include  <algorithm>


# ifndef TURBO
#define random(x)   (rand() % (x))
#define randomize() srand(time(0))
# endif

#define  DEBUG
#define  DEBUG_PACK

# ifdef DEBUG_PACK
#   define  DEBUG
# endif

# ifdef DEBUG
#   include <map>
#   include <string>

    // Mapping von ID auf Name von Gruppe / Nation
    static std::map<long, wxString, std::less<long> > grList;
    static std::map<long, wxString, std::less<long> > naList;
  
    static FILE *file;
# endif


// Berechnet selbst die Tabelle anstatt stPos auszuwerten
// #define DEBUG_TABLE

/*
 (Zu ueberpruefen, keine Gewaehr fuer Richtigkeit)
 Auslosung:
 listNA ist die Liste der Nationen, jede Nation enthaelt eine Liste
 ihrer Spieler.
 listBY ist eine Liste der Freilose.
 listSC ist eine temporaere Liste mit allen Spielern / Freilosen,
 die in der grade aktuellen Stufe / Section sind
 listTM ist eine Liste aller Spieler / Freilose, die nicht gepaart
 werden konnten
 listPK ist eine Liste der Packs (gepaarte Spieler)

 Freilose werden alle gesetzt, die Position wird zu Beginn an berechnet.
 Ausnahme: Ungrade und mehr als eines: Das letzte wird frei gelost,
 je nach Anzahl der Ersten. Sonst kann es passieren, dass von 3 Freilosen
 2 nach unten kommen, von 3 Ersten 2 nach oben kommen muessen, da es
 hier Abhaengigkeiten von Setzung und Nationen gibt.

 Paarung von Packs: Packs werden zunaechst nur aus Spielern einer Nation
 gebildet, damit werden Nationen immer getrennt. Spieler, die nicht auf
 diese Art gepaart werden koennten, wandern in einen Pool, der dann am
 Ende aufgearbeitet wird. Wird auch dann kein passender Spieler gefunden
 (siehe dort), wird ein bestehendes Pack aufgebrochen. Gesetzte Spieler
 und Freilose kommen so immer auf ihren Platz.
 Um Ungleichgewichte in der Verteilung einer Nation zu vermeiden, die sich
 aus der Setzung ergeben (von 4 Plaetzen sind 2 durch fremde Nationen
 besetzt, dann duerfen dorthin nicht 2 Spieler einer weiteren Nation)
 kann dort ein Spieler temporaer fuer die naechste Stufe gesetzt werden.

 Erster Spieler in einem Pack ist
   Ein gesetzter Spieler
   Ein Freilos (auch nur ein gesetzter Spieler, Relikt)
   Ein Gruppenerster
   Irgendeiner
 Zweiter Spieler in einem Pack ist (ein passender)
   Der andere der Gruppe
   Ein gesetzer
   Zu einem Ersten ein Erster, dann ein Zweiter
   Zu einem Zweiten ein Zweiter (Freilose sind gesetzt und damit abgearbeitet)
   Irgendein Spieler
   
Gesetzte der Nation wandern immer in den Topf, wenn kein anderer der Gruppe
gefunden wird. Da die gesetzten schon bei der Verteilung der Nationen 
beruecksichtigt wurden, koennen die nicht gesetzten Spieler einer Nation
entsprechend gelost werden. Damit gibt es in der ersten Stufe keine Packs,
die einen geetzten, aber keine 2 Spieler einer Gruppe beinhalten.
Abgesehen von Randbedingungen wie Verteilung der Ersten / Freilose koennen
Packs frei gelost werden.

 Es wird implizit vorrausgesetzt, dass nur erste (und Freilose) gesetzt
 wurden. Damit werden Erste bzw. Zweite nach Moeglichkeit getrennt
 (Freilose zaehlen als Zweite).

 Auslosen der Packs:
 Packs werden in folgender Reihenfolge ausglost:
   Packs mit gesetzten Spielern (praktisch Initialisierung)
   Packs mit Ersten und Zweiten (solange noch Wahlmoeglichkeiten bestehen)
   Irgendein Pack (nur Erste / Zweite koennen immer gelost werden)

 Wird ein Pack abgearbeitet so wird rekursiv zunaechst fuer
 den ersten Spieler das Pack mit dem Gruppenpartner genommen, dann
 fuer den zweiten Spieler das Pack mit dem Gruppenpartner.
 Gruppenpartner werden auf diese Art weitestgehend getrennt.

 Randbedingungen fuer die Auslosung eines Packs
   Gesetzte Spieler kommen immer auf ihren Platz
   Packs mit nur Ersten / Zweiten duerfen immer gelost werden
   Die Differenz Erste oben / Erste unten muss am Ende minimal sein
   (durch die Freilose kann es passieren, dass mehr als ein Pack
   Erste und Zweite gemischt enthaelt)
   Zu jedem Freilos muss es einen Ersten geben

*/


// Benutzung von DrawItemTeam::pos
// pos[0] ist bei gesetzten Spielern die engueltige Position
// pos[i>1] ist die Section in Stage i
// Stage 1 ist der Anfang des Rasters, d.h. alle Spieler; pos[1] ist immer 1
// Stage lastStage+1 enthaelt die endgueltige Position in der Gruppe



bool Even(int i)
{
  return !(i & 0x1);
}


bool Odd(int i)
{
  return i & 0x1;
}



// +-------------------------------------------------------+
// +        Erweiterung der Packs und Packliste            +
// +-------------------------------------------------------+

struct  DIPack : DrawItemPack
{
  DIPack(DrawItemTeam *item1, DrawItemTeam *item2);
  DIPack();

  bool  ExchangeTeams();

  bool  DrawUp(int stg);        // 'Lose' so, dass team[0] nach oben kommt
  bool  DrawDown(int stg);      // dto., aber umgekehrt
  // int  DrawFirstUp(int stg);   // 'Lost' ersten nach oben
  // int  DrawFirstDown(int stg); // dto., aber nach unten
  bool  Draw(int stg);          // Lose beide Varianten aus

  bool  IsImpaired();       // TRUE, wenn Pack ersten und zweiten enthaelt
  bool  IsIncomplete();     // TRUE, wenn Pack nur ein Team enthaelt
  bool  IsBye();            // TRUE, wenn einer oder beide Freilose sind
  bool  Is0Bye();           // TRUE, wenn teams[0] ein Freilos ist
  bool  Is1Bye();           // TRUE, wenn teams[1] ein Freilos ist
  bool  IsTwoByes();        // TRUE, wenn beide Freilose sind
  bool  IsSeeded(int stg);  // TRUE, wenn einer der beiden Spieler gesetzt ist
  bool  Is0Seeded();        // TRUE, wenn teams[0] ein echter gesetzter ist
  bool  Is1Seeded();        // TRUE, wenn teasms[1] ein echter gesetzter ist
  bool  Is0First();         // TRUE, wenn teams[0] Gruppensieger ist
  bool  Is1First();         // TRUE, wenn teams[1] Gruppensieger ist
  bool  IsThird();          // TRUE, wenn einer oder beide Dritter sind
  bool  Is0Third();         // TRUE, wenn teams[0] ein Dritter ist
  bool  Is1Third();         // TRUE, wenn teams[1] ein Dritter ist

  bool  IsFirstUp(int stg); // Wohin wandert der Bessere?
  bool  IsUp(int stg);      // TRUE, wenn teams[0] nach oben oder teams[1] nach unten geht
};


DIPack::DIPack() : DrawItemPack()
{};


DIPack::DIPack(DrawItemTeam *item1, DrawItemTeam *item2)
       : DrawItemPack(item1, item2)
{};


bool DIPack::ExchangeTeams()
{
  DrawItemTeam *tmp = teams[0];

  teams[0] = teams[1];
  teams[1] = tmp;

  return true;
}


inline  bool DIPack::IsBye()
{
  return (Is0Bye() || Is1Bye());
}


inline  bool DIPack::IsTwoByes()
{
  return (Is0Bye() && Is1Bye());
}


inline  bool DIPack::Is0Bye()
{
  return (teams[0] && !teams[0]->lastGroup);
}


inline  bool DIPack::Is1Bye()
{
  return (teams[1] && !teams[1]->lastGroup);
}


inline  bool DIPack::IsUp(int stg)
{
  return (teams[0] && Odd(teams[0]->pos[stg]) ||
          teams[1] && Even(teams[1]->pos[stg]));
}


// Ist teams[0] ein gesetzter Spieler
bool DIPack::Is0Seeded()
{
  if (teams[0] && teams[0]->pos[0])
    return true;
  else
    return false;
}


// Ist teams[1] ein gesetzter Spieler
bool DIPack::Is1Seeded()
{
  if (teams[1] && teams[1]->pos[0])
    return true;
  else
    return false;
}


// Ist teams[0] Gruppensieger?
bool DIPack::Is0First()
{
  // exist. beide Teams?
  if (!teams[0])
    return false;

  if (Is0Bye())
    return false;

  return (teams[0]->lastPos == 1);
}


// Ist teams[1] Gruppensieger?
bool DIPack::Is1First()
{
  // exist. beide Teams?
  if (!teams[1])
    return false;

  if (Is1Bye())
    return false;

  return (teams[1]->lastPos == 1);
}


inline  bool DIPack::IsThird()
{
  return (Is0Third() || Is1Third());
}


inline  bool DIPack::Is0Third()
{
  return (teams[0] && teams[0]->lastPos > 2);
}


inline  bool DIPack::Is1Third()
{
  return (teams[1] && teams[1]->lastPos > 2);
}


bool DIPack::DrawUp(int stg)
{
  if (teams[0] && !teams[0]->pos[stg+1])
    teams[0]->pos[stg+1] = 2 * teams[0]->pos[stg] - 1;

  if (teams[1] && !teams[1]->pos[stg+1])
    teams[1]->pos[stg+1] = 2 * teams[1]->pos[stg];
    
  return false;
}


bool DIPack::DrawDown(int stg)
{
  if (teams[0] && !teams[0]->pos[stg+1])
    teams[0]->pos[stg+1] = 2 * teams[0]->pos[stg];

  if (teams[1] && !teams[1]->pos[stg+1])
    teams[1]->pos[stg+1] = 2 * teams[1]->pos[stg] - 1;
    
  return false;
}


bool DIPack::Draw(int stg)
{
  # ifdef DEBUG_PACK
  wxFprintf(file, "        Draw: ");
  # endif

  // Kein Spieler?
  if (!teams[0] && !teams[1])
  {
    # ifdef DEBUG_PACK
    wxFprintf(file, "\n");
    # endif
    return true;
  }

  // Nur ein Spieler und gesetzt?
  if (!teams[0] && teams[1]->pos[stg+1])
  {
    # ifdef DEBUG_PACK
    wxFprintf(file, "\n");
    # endif
    return true;
  }

  if (!teams[1] && teams[0]->pos[stg+1])
  {
    # ifdef DEBUG_PACK
    wxFprintf(file, "\n");
    # endif
    return true;
  }

  // Nur ein Spieler und dieser nicht gesetzt
  if (!teams[0] || !teams[1])
  {
    if (random(2))
      DrawUp(stg);
    else
      DrawDown(stg);

    # ifdef DEBUG_PACK
    wxFprintf(file, " (random)\n");
    # endif

    return true;
  }

  // 2 gesetzte Spieler
  if (teams[0]->pos[stg+1] && teams[1]->pos[stg+1])
  {
    # ifdef DEBUG_PACK
    wxFprintf(file, "\n");
    # endif
    return true;
  }

  // 1 gesetzter Spieler
  if (teams[0]->pos[stg+1])
  {
    if (Even(teams[0]->pos[stg+1]))
      DrawDown(stg);
    else
      DrawUp(stg);

    # ifdef DEBUG_PACK
    wxFprintf(file, " (must, Seeded)\n");
    # endif

    return true;
  }

  if (teams[1]->pos[stg+1])
  {
    if (Even(teams[1]->pos[stg+1]))
      DrawUp(stg);
    else
      DrawDown(stg);

    # ifdef DEBUG_PACK
    wxFprintf(file, " (must, Seeded)\n");
    # endif

    return true;
  }

  // Kein gesetzter Spieler
  if (random(2))
    DrawUp(stg);
  else
    DrawDown(stg);

  # ifdef DEBUG_PACK
  wxFprintf(file, " (random)\n");
  # endif

  return true;
}


bool DIPack::IsSeeded(int stg)
{
  return (teams[0] && teams[0]->IsSeeded(stg) ||
          teams[1] && teams[1]->IsSeeded(stg));
}


// Impaiered sind 'nicht zwei gleiche'
bool DIPack::IsImpaired()
{
  // Nur ein Spieler (oder keiner)
  if (!teams[0] && !teams[1])
    return true;

  if (!teams[0])
    return !IsBye() && teams[1]->lastPos == 1;
  if (!teams[1])
    return !IsBye() && teams[0]->lastPos == 1;

  // zwei Freilose
  if (IsTwoByes())
    return false;

  // ein Freilos nur mit ersten...
  if ( IsBye() && ((teams[0]->lastPos == 1) || (teams[1]->lastPos == 1)) )
    return true;

  // erster und nicht-erster
  if ( (teams[0]->lastPos == 1) != (teams[1]->lastPos == 1) )
    return true;

  return false;
}


bool DIPack::IsIncomplete()
{
  return (teams[0] != NULL) ^ (teams[1] != NULL);
}


// Wohin geht der erste? Macht nur Sinn fuer Impaired Packs
bool DIPack::IsFirstUp(int stg)
{
  if (!IsImpaired())
    return false;

  return Is0First();
}



// -----------------------------------------------------------------------
//
struct  DLPack : DrawListPack
{
  // Add, Subtract von Teams
  bool Add(DrawItemTeam *, DrawItemTeam *);

  // Get...
  DIPack * GetSeeded(int);    // Liefert Pack mit gesetzten Spielern
  DIPack * GetBye();          // Liefert Pack mit Freilosen
  DIPack * GetThird();        // Liefert Pack mit Dritten
  DIPack * GetImpaired();     // Liefert Impaired Pack
  DIPack * GetGroupPack(DrawItemTeam *);  // Liefert Pack mit Spieler in gleicher Gruppe 
  DIPack * Get0Group(DIPack *);  // Liefert Pack mit Spieler gleicher Gruppe wie teams[0]
  DIPack * Get1Group(DIPack *);  // Liefert Pack mit Spieler gleicher Gruppe wie teams[1]

  // Auslosen testen
  bool CanPackUp(DIPack *, int);      // Kann teams[0] nach oben?

  // Auslosen
  bool DrawPackUp(DIPack *, int);     // 'Lost' pack->teams[0] nach oben
  bool DrawPack(DIPack *, int);       // Lost Pack
  bool Draw(int, int);

  int  CountImpaired();      // Anzahl Packs mit nur einem ersten
  int  CountIncomplete();    // Anzahl Packs mit nur einem Spieler

  int  totalImpaired;        // Anzahl Packs mit ersten und zweiten oder nur erstem
  int  currentImpaired;      // Uebriggebliebene Zahl impaired Packs
  int  diffImpaired;         // Differenz erste oben - erste unten
  int  diffByes;             // Differenz Byes oben / unten
  int  seededUp;             // Gesetzte oben
  int  seededDown;           // Gesetzte unten
  int  firstUp;              // Erste in oberer Haelfte
  int  firstDown;            // Erste in unterer Haelfte
  int  totalFirst;           // Anzahl Erste
  int  currentByesUp;        // Freilose in oberer Haelfte
  int  currentByesDown;      // Freilose in unterer Haelfte
  int  totalByes;            // Freilose insgesamt
  int  totalByesUp;          // Wieviele Freilose und Dritte sollen nach oben kommen
  int  totalByesDown;        // und wieviele nach unten
  int  diffUp;               // Wieviele insgesmt oben
  int  currentUp;            // Wieviele Packs mit nur einem, der nach oben koennte
};


bool DLPack::Add(DrawItemTeam *item1, DrawItemTeam *item2)
{
  DrawListPack::Add(new DIPack(item1, item2));
  return true;
}


// (2..n)
DIPack * DLPack::GetSeeded(int stg)
{
  DIPack *tmp = NULL;

  for (DLPack::iterator itPK = begin(); itPK != end(); itPK++)
  {
    DIPack *pack = (DIPack *) (*itPK);

    if (!pack->IsSeeded(stg))
      continue;
      
    // Gesetzte Freilose bevorzugt, sonst das erste mit einem gesetzten Spieler
    if (tmp == NULL)
      tmp = pack;
      
    if (pack->IsBye())
      return pack;
  }

  return tmp ? tmp : NULL;
}


DIPack * DLPack::GetBye()
{
  for (DLPack::iterator itPK = begin(); itPK != end(); itPK++)
  {
    DIPack *pack = (DIPack *) (*itPK);

    if (pack->IsBye())
      return pack;
  }

  return NULL;
}


DIPack * DLPack::GetThird()
{
  for (DLPack::iterator itPK = begin(); itPK != end(); itPK++)
  {
    DIPack *pack = (DIPack *) (*itPK);

    if (pack->IsThird())
      return pack;
  }

  return NULL;
}


DIPack * DLPack::GetImpaired()
{
  DIPack *candidate = NULL;
  for (DLPack::iterator itPK = begin(); itPK != end(); itPK++)
  {
    DIPack *pack = (DIPack *) (*itPK);

    if (!pack->IsImpaired())
      continue;

    if (!pack->IsIncomplete())
      return pack;

    if (candidate == NULL)
      candidate = pack;
  }

  return candidate;
}


DIPack * DLPack::GetGroupPack(DrawItemTeam *team)
{
  if (!team || !team->lastGroup)
    return NULL;

  DIPack *candidate = NULL;

  for (DLPack::iterator itPK = begin(); itPK != end(); itPK++)
  {
    DIPack *pack = (DIPack *) (*itPK);

    if (pack->teams[0] && pack->teams[0]->lastGroup == team->lastGroup ||
        pack->teams[1] && pack->teams[1]->lastGroup == team->lastGroup)
    {
      // Pack mit Team aus gleicher Gruppe gefunden. 
      // Jetzt nach Moeglichkeit 1-te mit 2-ten, 3-te mit 4-ten paaren
      if ( pack->teams[0] && pack->teams[0]->lastGroup == team->lastGroup && 
           (pack->teams[0]->lastPos > 2) == (team->lastPos > 2) )
        return pack;

      if ( pack->teams[1] && pack->teams[1]->lastGroup == team->lastGroup && 
           (pack->teams[1]->lastPos > 2) == (team->lastPos > 2) )
        return pack;

      candidate = pack;
    }
  }

  return candidate;
}


// Liefert Packs mit Spielern aus gleicher Gruppe
DIPack * DLPack::Get0Group(DIPack *pack)
{
  // Faelle ausschliessen
  if (!pack || !pack->teams[0] || pack->Is0Bye())
    return NULL;

  return GetGroupPack(pack->teams[0]);
}


DIPack * DLPack::Get1Group(DIPack *pack)
{
  // Faelle ausschliessen
  if (!pack || !pack->teams[1] || pack->Is1Bye())
    return NULL;

  return GetGroupPack(pack->teams[1]);
}


// Anzahl mit nur einem ersten
int  DLPack::CountImpaired()
{
  int  ci = 0;

  for (DLPack::iterator itPK = begin(); itPK != end(); itPK++)
  {
    DIPack *pack = (DIPack *) (*itPK);

    if (pack->IsImpaired())
      ci++;
  }

  return ci;
}


int DLPack::CountIncomplete()
{
  int  ci = 0;

  for (DLPack::iterator itPK = begin(); itPK != end(); itPK++)
  {
    DIPack *pack = (DIPack *) (*itPK);

    if (pack->IsIncomplete())
      ci++;
  }

  return ci;
}


// TRUE, wenn teams[0] nach oben und teams[1] nach unten kann (1..n)
bool DLPack::CanPackUp(DIPack *pack, int stg)
{
  if (!pack)
    return false;

  # ifdef DEBUG_PACK
  wxFprintf(file, "        CanPackUp: ");
  # endif

  // Sonderfall seeded: Geht immer, ausser ein gesetztes Team wandert woanders hin
  if (pack->IsSeeded(stg+1))
  {
    // Team 0 testen
    if (pack->teams[0] &&
        pack->teams[0]->pos[stg+1] &&
        Even(pack->teams[0]->pos[stg+1]))
    {
      # ifdef DEBUG_PACK
      wxFprintf(file, "No! Team 0 %s\n", (pack->teams[0]->pos[stg] ? "seeded" : "hinted"));
      # endif
      return false;
    }

    // Team 1 testen
    if (pack->teams[1] &&
        pack->teams[1]->pos[stg+1] &&
        Odd(pack->teams[1]->pos[stg+1]))
    {
      # ifdef DEBUG_PACK
      wxFprintf(file, "No! Team 1 %s\n", (pack->teams[1]->pos[stg] ? "seeded" : "hinted"));
      # endif
      return false;
    }

    # ifdef DEBUG_PACK
    int seedCount = 0;
    if (pack->teams[0] && pack->teams[0]->IsSeeded(stg+1))
      ++seedCount;
    if (pack->teams[1] && pack->teams[1]->IsSeeded(stg+1))
      ++seedCount;
    // IsSeeded(0) fragt die finale Position ab, liefert also echte gesetzte Spieler
    wxFprintf(file, "Yes %d Team(s) %s!\n", seedCount, pack->IsSeeded(0) ? "seeded" : "hinted");
    # endif
    return true;
  }

  // Incomplete muss ausgleichen
  if (pack->IsIncomplete())
  {
    if (pack->teams[0] && (diffUp + 1) > (currentUp - 1) ||
        pack->teams[1] && abs(diffUp - 1) > (currentUp - 1))
    {
      # ifdef DEBUG_PACK
      wxFprintf(file, "No! Number of players exceeded (%d diff - %d remaining\n", diffUp, currentUp);
      # endif
      return false;
    }
  }

  // Zwei Freilose der zwei Dritte koennen immer ausgelost werden
  if (pack->Is0Bye() && pack->Is1Bye())
  {
# ifdef DEBUG_PACK
    wxFprintf(file, "Yes, 2 Byes\n");
#endif
    return true;
  }

  if (pack->Is0Third() && pack->Is1Third())
  {
# ifdef DEBUG_PACK
    wxFprintf(file, "Yes, 2 Thirds\n");
#endif
  }


  // Dritte: totalUp / totalDown beachten
  if ( pack->Is0Third() ^ pack->Is1Third() )
  {
    if (pack->Is0Third() && currentByesUp >= totalByesUp && currentByesDown < totalByesDown ||
        pack->Is1Third() && currentByesDown >= totalByesDown && currentByesUp < totalByesUp)
    {
      # ifdef DEBUG_PACK
      wxFprintf(file, "No! Byes exceeded\n");
      # endif

      return false;
    }
  }
  
  // Freilose: totalUp / totalDown beachten
  if ( pack->Is0Bye() ^ pack->Is1Bye() )
  {
    if (pack->Is0Bye() && currentByesUp >= totalByesUp && currentByesDown < totalByesDown ||
        pack->Is1Bye() && currentByesDown >= totalByesDown && currentByesUp < totalByesUp)
    {
      # ifdef DEBUG_PACK
      wxFprintf(file, "No! Byes exceeded\n");
      # endif

      return false;
    }
  }
  
  // Freilose und Dritte: Nicht gesetztes Freilos wandert dorthin, 
  // wo ein Ueberschuss an ersten herrscht
  // Packs mit gesetzten wurden bereits abgehandelt.
  // Da ungesetzte Freilose gleich nach den gesetzten Spielern
  // (und Freilosen) kommen, werden hier gesetzte 1-ste ausgeglichen
  // (gesetzte Zweite werden nicht beruecksichtigt).
  // diffByes sollte an der Stelle 0 sein. Wenn nicht, wird hier
  // ein allzugrosser Ueberschuss an Freilosen verhindert.
  if (pack->IsBye() || pack->IsThird())
  {
    if ( currentByesUp < seededUp && currentByesDown >= seededDown && pack->Is1Bye() || 
         currentByesDown < seededDown && currentByesUp >= seededUp && pack->Is0Bye() )
    {
      // Umgekehrte Auslosung wuerde Setzung besser ausgleichen
      # ifdef DEBUG_PACK
      wxFprintf(file, "No, Bye shall come in same half as Seeded\n");
      # endif
      
      return false;
    }
    
    if (diffByes > 0 && (pack->Is0Bye() || pack->Is0Third()) || diffByes < 0 && (pack->Is1Bye() || pack->Is1Third()))
    {
      // Verteilung der Freilose wird nicht ausgeglichen 
      // Da es nur ein einzelnes ungesetztes Freilos gibt, kommt das nicht vor.
      # ifdef DEBUG_PACK
      wxFprintf(file, "No, Bye shall be distributed evenly\n");
      # endif
      
      return false;
    }
    
    if ( firstUp > firstDown + currentImpaired && !pack->Is0Bye() && !pack->Is0Third() || 
         firstDown > firstUp + currentImpaired && !pack->Is1Bye() && !pack->Is1Third() )
    {
      // Es kann sein, dass obige Bedingung, mehr Freilose als Erste in
      // jeder Haelfte, erfuellt ist, jetzt aber noch gepaarte Erste kommen.
      // Kommt dann das Freilos in die Haelfte mit weniger Ersten, kommt es
      // zur Situation, dass am Ende aus einer 1:0 Verteilung der Ersten eine
      // 2:1 Verteilung wird, aus einer 1:1 Verteilung der Freilose eine 1:2
      // Verteilung. Man haette also genug Freilose fuer die Ersten gehabt,
      // kann aber nicht mehr ausgleichen.
      # ifdef DEBUG_PACK
      wxFprintf(file, "No, Bye shall be in half with more Firsts\n");
      # endif
      
      return false;
    }
      
    // Freilos wandert dorthin, wo es mehr Erste gibt.
    # ifdef DEBUG_PACK
    wxFprintf(file, "Yes, Bye can be drawn (freely)\n");
    # endif
    
    return true;
  }

  // Keine gesetzten: Paired Teams duerfen immer ausgelost werden
  if (!pack->IsImpaired())
  {
    # ifdef DEBUG_PACK
    wxFprintf(file, "Yes! (Paired)\n");
    # endif
    
    return true;
  }

  if ( firstUp + currentImpaired <= currentByesUp && firstDown >= currentByesDown && pack->Is1First() ||
       firstDown + currentImpaired <= currentByesDown && firstUp >= currentByesUp && pack->Is0First() )
  {
    // Unter der Annahme, man koenne alle impaired richtig auslosen
    # ifdef DEBUG_PACK
    wxFprintf(file, "No! First shall come in same half as Byes\n");
    # endif
    
    return false;
  }

  // Einzelne Erste muessen auf bestimmte Positionen wandern
  if (diffImpaired == 0 && totalFirst == 1)
  {
    if (pack->teams[0])
    {
      int sec = pack->teams[0]->pos[stg];

      if ( ((sec % 2) != 0) ^ pack->IsFirstUp(stg))
      {
        #ifdef DEBUG_PACK
        wxFprintf(file, "No! Last first shall go %s (%i diff - %i remaining)\n", (sec % 2) ? "up" : "down", diffImpaired, currentImpaired);
        #endif
        return false;
      }
    }
    else if (pack->teams[1])
    {
      int sec = pack->teams[1]->pos[stg];

      if ( ((sec % 2) != 0) ^ pack->IsFirstUp(stg))
      {
        #ifdef DEBUG_PACK
        wxFprintf(file, "No! Last first shall go %s (%i diff - %i remaining)\n", (sec % 2) ? "up" : "down", diffImpaired, currentImpaired);
        #endif
        return false;
      }
    }

    # ifdef DEBUG_PACK
    wxFprintf(file, "Yes! Impaired (%i diff - %i remaining)\n", diffImpaired, currentImpaired);
    # endif
    return true;
  }

  // Der letzte erste, den man frei losen koennte
  // Allerdings nicht, wenn ein letztes Freilos in die andere Haelfte muesste
  if (pack->IsImpaired() && pack->IsIncomplete() && 
      (currentByesUp + currentByesDown) < totalByes && totalByes <= totalFirst)
  {
    if (pack->teams[0] && diffUp + 1 >= currentUp - 1 || 
        pack->teams[1] && abs(diffUp - 1) >= currentUp - 1)
    {
# ifdef DEBUG_PACK
      wxFprintf(file, "No! Remaining byes would come to other half\n");
# endif
      return false;
    }
  }

  // bei Impaired: Ist Limit ueberschritten?
  if ( abs(diffImpaired) < currentImpaired || 
       abs(diffImpaired) == currentImpaired && 
        (diffImpaired < 0 && pack->IsFirstUp(stg) || 
         diffImpaired > 0 && !pack->IsFirstUp(stg)) 
     )    
  {
    # ifdef DEBUG_PACK
    wxFprintf(file, "Yes! Impaired (%i diff - %i remaining)\n", diffImpaired, currentImpaired);
    # endif
    return true;
  }

  // Hier kommen sie verkehrt herum, um impaired auszugleichen
  if (abs(diffImpaired) == currentImpaired)
  {
    # ifdef DEBUG_PACK
    wxFprintf(file, "No! Impaired (%i diff - %i remaining)\n", diffImpaired, currentImpaired);
    # endif
    return false;
  }

  // testen, wohin der erste wandert
  if (pack->IsFirstUp(stg) && diffImpaired < 0 ||
      !pack->IsFirstUp(stg) && diffImpaired > 0)
  {
    # ifdef DEBUG_PACK
    wxFprintf(file, "Yes! Though impaired exceeded (%i diff - %i remaining)\n", diffImpaired, currentImpaired);
    # endif
    return true;
  }

  # ifdef DEBUG_PACK
  wxFprintf(file, "Yes! Though impaired exceeded (%i diff - %i remaining)\n", diffImpaired, currentImpaired);
  # endif

  return true;
}


bool DLPack::DrawPackUp(DIPack *pack, int stg)
{
  pack->DrawUp(stg);

  if (pack->IsImpaired())
  {
    if (pack->IsFirstUp(stg+1))
      diffImpaired++;
    else
      diffImpaired--;

    currentImpaired--;
  }

  if (pack->IsIncomplete())
  {
    if (pack->teams[0])
      diffUp++;
    else
      diffUp--;

    currentUp--;
  }
  
  if (pack->Is0Bye() || pack->Is0Third())
    ++currentByesUp;
  if (pack->Is1Bye() || pack->Is1Third())
    ++currentByesDown;
    
  diffByes = currentByesUp - currentByesDown;
    
  if (pack->Is0Seeded() && pack->Is0First())
    ++seededUp;
  if (pack->Is1Seeded() && pack->Is1First())
    ++seededDown;
    
  if (pack->Is0First())
    ++firstUp;
  if (pack->Is1First())
    ++firstDown;
    
  for (DLPack::iterator itPK = begin(); itPK != end(); itPK++)
  {
    if ( ((DIPack *) (*itPK)) == pack )
    {
      erase(itPK);
      break;
    }
  }

  // Jetzt wirds spannend: der Partner der Gruppe soll in die andere Haelfte!
  DIPack *pack0Group = Get0Group(pack);
  long    last0Group = pack->teams[0] ? pack->teams[0]->lastGroup : 0;
  long    last0Pos   = pack->teams[0] ? pack->teams[0]->lastPos : 0; 

  // pack0Group 'Losen'
  if (pack0Group)
  {
    // teams[0] nach unten?
    if (pack0Group->teams[0] &&
        pack0Group->teams[0]->lastGroup == last0Group)
      pack0Group->ExchangeTeams();
      
    # ifdef DEBUG_PACK
    wxFprintf(file, "Other from group of current team 0 found!\n");
    wxFprintf(file, "%s:%d  %s:%d\n", 
        pack0Group->teams[0] ? grList[pack0Group->teams[0]->lastGroup].data() : "",
        pack0Group->teams[0] ? pack0Group->teams[0]->lastPos : 0,
        pack0Group->teams[1] ? grList[pack0Group->teams[1]->lastGroup].data() : "",
        pack0Group->teams[1] ? pack0Group->teams[1]->lastPos : 0);
    # endif

    if (CanPackUp(pack0Group, stg))
      DrawPackUp(pack0Group, stg);
    else if (!pack0Group->IsSeeded(stg+1))
    {
      # ifdef DEBUG_PACK
      wxFprintf(file, "Other from group of current team 0 must go up!\n");
      # endif
      // warnings.push_back(wxString::Format(_("Other of group %s:%d cannot go in opposite half in stg %d"), last0Group ? grList[last0Group].data() : "", last0Pos, stg));

      DrawPackUp(pack0Group, stg);
    }
    else
    {
      # ifdef DEBUG_PACK
      wxFprintf(file, "Other from group of current team 0 seeded to same half!\n");
      # endif
    }
  }

  // Pack darf erst hier ermittelt werden, da es evt. schon oben verbraucht wurde
  DIPack *pack1Group = Get1Group(pack);
  long    last1Group = pack->teams[1] ? pack->teams[1]->lastGroup : 0;
  long    last1Pos   = pack->teams[1] ? pack->teams[1]->lastPos : 0;

  // pack1Group 'Losen'
  if (pack1Group)
  {
    // teams[1] nach oben?
    if (pack1Group->teams[1] &&
       pack1Group->teams[1]->lastGroup == last1Group)
      pack1Group->ExchangeTeams();
      
    # ifdef DEBUG_PACK
    wxFprintf(file, "Other from group of current team 1 found!\n");
    wxFprintf(file, "%s:%d  %s:%d\n", 
             pack1Group->teams[0] ? grList[pack1Group->teams[0]->lastGroup].data() : "",
             pack1Group->teams[0] ? pack1Group->teams[0]->lastPos : 0,
             pack1Group->teams[1] ? grList[pack1Group->teams[1]->lastGroup].data() : "",
             pack1Group->teams[1] ? pack1Group->teams[1]->lastPos : 0);
    # endif
    
    if (CanPackUp(pack1Group, stg))
      DrawPackUp(pack1Group, stg);      
    else if (!pack1Group->IsSeeded(stg+1))
    {
      # ifdef DEBUG_PACK
      wxFprintf(file, "Other from group of current team 1 must go up!\n");
      # endif
      // warnings.push_back(wxString::Format(_("Other of group %s:%d cannot go in opposite half in stg %d"), last1Group ? grList[last1Group].data() : "", last1Pos, stg));

      DrawPackUp(pack1Group, stg);
    }
    else
    {
      # ifdef DEBUG_PACK
      wxFprintf(file, "Other from group of current team 1 seeded to same half!\n");
      # endif
    }
  }

  delete pack;

  return true;
}


bool DLPack::DrawPack(DIPack *pack, int stg)
{
  if (!pack)
    return true;
      
  if ( pack->teams[0] && pack->teams[0]->pos[stg+1] ||
       pack->teams[1] && pack->teams[1]->pos[stg+1] )
  {
    // Position gesetzter Paare pruefen
    if ( pack->teams[0] && pack->teams[0]->pos[stg+1] && Even(pack->teams[0]->pos[stg+1]) ||
         pack->teams[1] && pack->teams[1]->pos[stg+1] && Odd(pack->teams[1]->pos[stg+1]) )         
      pack->ExchangeTeams();
  }
  else if (pack->IsImpaired() && (stg == 1 && abs(diffImpaired) || abs(diffImpaired) >= currentImpaired))
  {
    // Gruppensieger gleichmaessig auf die Haelften verteilen
    // In der ersten Stufe immer gegensteuern, da viele Packs folgen koennen
    if (diffImpaired > 0 && pack->IsFirstUp(stg) ||
        diffImpaired < 0 && !pack->IsFirstUp(stg))
      pack->ExchangeTeams();
  }
  else if (random(2) == 0)
  {
    // Zufallsauslosung
    pack->ExchangeTeams();
  }

  # ifdef DEBUG_PACK
  wxFprintf(file, "Draw Pack %s:%d  %s:%d\n", 
           pack->teams[0] ? grList[pack->teams[0]->lastGroup].data() : "",
           pack->teams[0] ? pack->teams[0]->lastPos : 0,
           pack->teams[1] ? grList[pack->teams[1]->lastGroup].data() : "",
           pack->teams[1] ? pack->teams[1]->lastPos : 0);
  # endif
  
  if (!CanPackUp(pack, stg))
    pack->ExchangeTeams();
    
  DrawPackUp(pack, stg);

  return true;
}


// Auslosen / Verteilen der Packliste
bool DLPack::Draw(int stg, int sec)
{
  DIPack *itemPK;

  totalImpaired = 0;    // Anzahl Packs mit ersten und zweiten
  currentImpaired = 0;  // Uebriggebliebene Zahl impaired Packs
  diffImpaired = 0;     // Differenz erste oben - erste unten
  diffByes = 0;         // Differenz Byes oben - unten
  
  seededUp = seededDown = 0;
  firstUp = firstDown = totalFirst = 0;
  currentByesUp = currentByesDown = 0;  // Beinhaltet Dritte

  // Zunaechst Anzahl der Packs mit ersten und zweiten feststellen
  totalImpaired = CountImpaired();
  currentImpaired = totalImpaired;

  currentUp = CountIncomplete();

  diffUp = 0;

  totalByes = 0;
  for (DLPack::iterator it = begin(); it != end(); it++)
  {
    DIPack *pack = (DIPack *) (*it);

    if (pack->Is0Bye() || pack->teams[0] && pack->teams[0]->lastPos > 2)
      totalByes++;
    if (pack->Is1Bye() || pack->teams[1] && pack->teams[1]->lastPos > 2)
      totalByes++;

    if (pack->Is0First())
      totalFirst++;
    if (pack->Is1First())
      totalFirst++;
  }

  CalculateByes(stg, sec, ld2(2 * Count()) + stg - 1, totalByes, totalByesUp, totalByesDown);

# ifdef DEBUG_PACK
  wxFprintf(file, " %i impaired packs\n", totalImpaired);
# endif

  // Mischen der Liste
// # ifndef  DEBUG_PACK
  Shuffle();
// # endif

  // randomize();

  // Gesetzte Byes und Gesetzte Spieler abarbeiten
  while ((itemPK = GetSeeded(stg+1)) != NULL)
    DrawPack(itemPK, stg);

  // Dann ungesetzte Freilose.
  // Es ist besser, hier die gesetzten Spieler auszugleichen:
  // Bei einer unvollstaendigen Setzungsliste kann es 3 gesetzte Spieler
  // geben (Pos 1, 9, 16). Drei Freilose muessen entsprechend verteilt
  // werden (Pos 2, 10, 15). Beruecksichtigt man aber noch weitere Erste,
  // kann sich das Verhaeltnis umkehren und das freie Freilos kann auch
  // nach oben wandern, so dass es fuer 1 gesetzten 2 Freilose gibt, 
  // fuer 2 gesetzte nur 1 Freilos.
  // DrawPack mit einem Freilos sorgt dafuer, dass ein einzelnes Freilos
  // dorthin wandert, wo es die meisten 1-sten gibt.
  // Aber nur, bis die gesetzten ausgeglichen sind.
  while ((itemPK = GetBye()) != NULL)
  {
    if (seededUp <= currentByesUp && seededDown <= currentByesDown)
      break;

    DrawPack(itemPK, stg);
  }

  // Dann Dritte
  while ((itemPK = GetThird()) != NULL)
    DrawPack(itemPK, stg);
    
  // Dann die Impaired (gleichen die Byes aus)
  while ((itemPK = GetImpaired()) != NULL)
    DrawPack(itemPK, stg);

  // Dann die restelichen Freilose
  while ((itemPK = GetBye()) != 0)
    DrawPack(itemPK, stg);

  // Dann den Rest
  while (size() && (itemPK = (DIPack *) (*begin())) != NULL)
    DrawPack(itemPK, stg);

  return true;
}




// +-----------------------------------------------------------------+
// +          Eine Meta-Liste: Eintraege sind Verweise               +
// +-----------------------------------------------------------------+


struct  DIMeta : public DrawItem
{
  DIMeta()                     {itemTM = 0;}
  DIMeta(DrawItemTeam *item)   {itemTM = item;}

  // Abfragen: gesetzt?, Freilos?, Erster?, Passt mit itemin ein Pack?
  bool IsSeeded(int stg)        {return itemTM && itemTM->IsSeeded(stg);}
  bool IsBye()                  {return itemTM && !itemTM->lastGroup;}
  bool IsFirst()                {return itemTM && (itemTM->lastPos == 1);}
  bool IsSecond()               {return !IsFirst();}
  bool IsPackable(DrawItemTeam *item, int stg);

  DrawItemTeam *itemTM;
};


// Passt zusammen mit item in ein Pack?
bool DIMeta::IsPackable(DrawItemTeam *item, int stg)
{
  if (!itemTM)
    return false;

  if (!itemTM->pos[stg+1] || !item->pos[stg+1] ||
      itemTM->pos[stg+1] != item->pos[stg+1])
    return true;

  return false;
}
    

struct  DLMeta : public DrawList
{
  // Add und Subtact rufen Konstruktor / Desteruktor auf!
  DrawItemTeam * Add(DrawItemTeam *);
  DrawItemTeam * Subtract(DrawItemTeam *);

  // Funktionen, um ein _erstes_ Team zu erhalten
  DrawItemTeam * GetTeam(int);         // Holt Team aus Liste
  DrawItemTeam * GetSeededTeam(int);   // Holt gesetztes Team aus Liste
  DrawItemTeam * GetFirst(int);        // Holt ersten
  DrawItemTeam * GetBye(int);          // Holt Freilos aus Liste
  DrawItemTeam * GetSecond(int);       // Holt zweiten aus Liste 

  // Funktionen, um ein _zweites_ Team zu erhalten
  DrawItemTeam * GetTeam(DrawItemTeam *, int);        // Holt passendes Team
  DrawItemTeam * GetSeededTeam(DrawItemTeam *, int);  // Holt passendes gesetztes Team
  DrawItemTeam * GetGroupTeam(DrawItemTeam *, int);   // Holt passendes Team aus gleicher Gruppe
  DrawItemTeam * GetPairedTeam(DrawItemTeam *, int);  // Holt naechstes passendes Team, erster zu erstem,...
  DrawItemTeam * GetImpairedTeam(DrawItemTeam *, int);  // Holt naechstes passendes Team, erster zu zweitem,...
  DrawItemTeam * GetAnyTeam(DrawItemTeam *, int);     // Holt naechstes passendes Team
  DrawItemTeam * GetFirst(DrawItemTeam *, int);       // Holt passenden ersten
  DrawItemTeam * GetSecond(DrawItemTeam *, int);      // Holt zweiten
  DrawItemTeam * GetBye(DrawItemTeam *, int);         // Holt passendes Freilos zu Team
};


DrawItemTeam * DLMeta::Add(DrawItemTeam *item)
{
  push_back(new DIMeta(item));
  return item;
}


DrawItemTeam * DLMeta::Subtract(DrawItemTeam *item)
{
  DIMeta *meta = 0;
  DLMeta::iterator itMT;
  for (itMT = begin(); itMT != end(); itMT++)
  {
    meta = (DIMeta *) (*itMT);

    if (meta->itemTM == item)
      break;
  }

  if (itMT != end())
  {
    DrawList::erase(itMT);
    delete meta;
  }

  return item;
}


// Holt gesetzten Spieler aus Liste, falls es einen gibt (1..n)
DrawItemTeam * DLMeta::GetSeededTeam(int stg)
{
  // Zuerst die Gruppensieger
  for (DLMeta::iterator itMT = begin(); itMT != end(); itMT++)
  {
    DIMeta *item = (DIMeta *) (*itMT);

    // XXX Durch item->IsFirst() ersetzen
    if (item->IsSeeded(stg+1) && item->itemTM->lastPos == 1)
    {      
      return item->itemTM;
    }
  }

  // Dann den naechstbesten
  for (DLMeta::iterator itMT = begin(); itMT != end(); itMT++)
  {
    DIMeta *item = (DIMeta *) (*itMT);

    if (item->IsSeeded(stg+1))
    {      
      return item->itemTM;
    }
  }
  return NULL;
}


// Liefert echten ersten
DrawItemTeam * DLMeta::GetFirst(int stg)
{
  for (DLMeta::iterator itMT = begin(); itMT != end(); itMT++)
  {
    DIMeta *item = (DIMeta *) (*itMT);

    if (item->IsFirst() && !item->IsBye())
      return item->itemTM;
  }

  return NULL;
}


// Liefert Freilos (1..n)
DrawItemTeam * DLMeta::GetBye(int stg)
{
  DIMeta *tmp = NULL;
  
  for (DLMeta::iterator itMT = begin(); itMT != end(); itMT++)
  {
    DIMeta *item = (DIMeta *) (*itMT);
    
    if (!item->IsBye())
      continue;
      
    // Zuerst gesetzte Freilose (werden mit gesetzten Freilosen gepaart),
    // danach die (ein) ungesetztes Freilos
    if (tmp == NULL)
      tmp = item;
      
    if (item->IsSeeded(stg+1))
      return item->itemTM;
  }

  return tmp ? tmp->itemTM : NULL;
}


// Liefert echten zweiten
DrawItemTeam * DLMeta::GetSecond(int stg)
{
  for (DLMeta::iterator itMT = begin(); itMT != end(); itMT++)
  {
    DIMeta *item = (DIMeta *) (*itMT);

    if (item->IsSecond() && !item->IsBye())
      return item->itemTM;
  }

  return NULL;
}


// Auswahl nach bestimmten Regeln
// Einen echten ersten
DrawItemTeam * DLMeta::GetFirst(DrawItemTeam *item1, int stg)
{
  for (DLMeta::iterator itMT = begin(); itMT != end(); itMT++)
  {
    DIMeta *item = (DIMeta *) (*itMT);

    if (item->itemTM == item1)
      continue;

    if (item->IsFirst() && !item->IsBye() && item->IsPackable(item1, stg))
      return item->itemTM;
  }

  return NULL;
}


// Ein Freilos
DrawItemTeam * DLMeta::GetBye(DrawItemTeam *item1, int stg)
{
  for (DLMeta::iterator itMT = begin(); itMT != end(); itMT++)
  {
    DIMeta *item = (DIMeta *) (*itMT);

    if (item->itemTM == item1)
      continue;
      
    // Wir suchen Freilose
    if (!item->IsBye())
      continue;
      
    // Eine grade Anzahl von Freilosen ist gesetzt.
    // Wenn item1 also ein Bye ist (und gesetzt), 
    // dann auch ein gesetztes Freilos dazutun    
    if (item1->IsBye() && item1->IsSeeded(stg+1) && !item->IsSeeded(stg+1))
      continue;

    if (item->IsPackable(item1, stg))
      return item->itemTM;
  }

  return NULL;
}


// Einen zweiten
DrawItemTeam * DLMeta::GetSecond(DrawItemTeam *item1, int stg)
{
  DIMeta *candidate = NULL;

  for (DLMeta::iterator itMT = begin(); itMT != end(); itMT++)
  {
    DIMeta *item = (DIMeta *) (*itMT);

    if (item->itemTM == item1)
      continue;

    if (!item->IsSecond() || item->IsBye())
      continue;

    if (!item->IsPackable(item1, stg))
      continue;

    // Nach Moeglichkeit Dritte zu Freilosen
    if (item->itemTM->lastPos > 2)
      return item->itemTM;
    else if (!candidate)
      candidate = item;
  }

  return candidate ? candidate->itemTM : NULL;
}


// 1. Gesetzter Spieler
DrawItemTeam * DLMeta::GetSeededTeam(DrawItemTeam *item1, int stg)
{
  DIMeta  *item;
  DIMeta  *candidate = NULL;

  for (DLMeta::iterator itMT = begin(); itMT != end(); itMT++)
  {
    item = (DIMeta *) (*itMT);

    if (item1 == item->itemTM)
      continue;

    if (item->IsSeeded(stg+1) && item->IsPackable(item1, stg))
    {
      // Zuerst Erste mit Ersten, Zweite mit Zweiten, Bye mit Bye
      if (candidate == NULL)
        candidate = item;

      if (item->itemTM->lastPos != item1->lastPos)
        continue;

      // Aber besser Erste mit Ersten, ... als ganz gemischt
      if (candidate == NULL || candidate->itemTM->lastPos != item1->lastPos)
        candidate = item;

      if (item->IsBye() != item1->IsBye())
        continue;
        
      return item->itemTM;
    }
  }

  // Ohne diese Einschraenkung
  return candidate ? candidate->itemTM : NULL;
}


// Team aus selber Gruppe wie item1
DrawItemTeam * DLMeta::GetGroupTeam(DrawItemTeam *item1, int stg)
{
  for (DLMeta::iterator itMT = begin(); itMT != end(); itMT++)
  {
    DIMeta *item = (DIMeta *) (*itMT);

    if (item1 == item->itemTM)
      continue;

    // Freilose in Liste?
    if (item->IsBye())
      continue;

    if (item->itemTM->lastGroup == item1->lastGroup)
      return (item->IsPackable(item1, stg) ? item->itemTM : 0);
  }

  return NULL;
}


// Liefert ersten fuer ersten, zweiten fuer zweiten
DrawItemTeam * DLMeta::GetPairedTeam(DrawItemTeam *item1, int stg)
{
  DIMeta *candidate = NULL;

  // Durchlauf ohne Gruppen-Paare aufzubrechen
  for (DLMeta::iterator itMT = begin(); itMT != end(); itMT++)
  {
    DIMeta *item = (DIMeta *) (*itMT);

    // Zwei erste oder zwei nicht-erste.
    // Zweite und Dritte werden gleich behandelt.
    if ( (item->itemTM->lastPos == 1) != (item1->lastPos == 1) )
      continue;
      
    if (!item->IsPackable(item1, stg))
      continue;

    if (GetGroupTeam(item->itemTM, stg))
      continue;

    // Nach Moeglichkeit 2-te mit 2-ten paaren
    if (item1->lastPos == item->itemTM->lastPos)
      return item->itemTM;
    else
      candidate = item;
  }

  // Wenn es einen candidate gibt, diesen nehmen
  if (candidate)
    return candidate->itemTM;

  // Ohne obige Einschraenkung
  for (DLMeta::iterator itMT = begin(); itMT != end(); itMT++)
  {
    DIMeta *item = (DIMeta *) (*itMT);

    if (item->itemTM == item1)
      continue;

    if ( (item->itemTM->lastPos == 1) != (item1->lastPos == 1) )
      continue;
      
    if (!item->IsPackable(item1, stg))
      continue;

    // Nach Moeglichkeit 2-te mit 2-ten paaren
    if (item1->lastPos == item->itemTM->lastPos)
      return item->itemTM;
    else
      candidate = item;
  }

  return candidate ? candidate->itemTM : NULL;
}


// Liefert ersten fuer zweiten, zweiten fuer ersten
DrawItemTeam * DLMeta::GetImpairedTeam(DrawItemTeam *item1, int stg)
{
  DIMeta *candidate = NULL;

  // Durchlauf ohne Gruppen-Paare aufzubrechen
  for (DLMeta::iterator itMT = begin(); itMT != end(); itMT++)
  {
    DIMeta *item = (DIMeta *) (*itMT);

    // Zwei erste oder zwei nicht-erste.
    // Zweite und Dritte werden gleich behandelt.
    if ( (item->itemTM->lastPos == 1) == (item1->lastPos == 1) )
      continue;
      
    if (!item->IsPackable(item1, stg))
      continue;

    if (GetGroupTeam(item->itemTM, stg))
      continue;

    // Nach Moeglichkeit 1-ten mit 2-ten und nicht dritten paaren
    if (abs(item1->lastPos - item->itemTM->lastPos) == 1)
      return item->itemTM;
    else if (!candidate)
      candidate = item;
  }

  // Wenn es einen candidate gibt, dann diesen nehmen
  if (candidate)
    return candidate->itemTM;

  // Ohne obige Einschraenkung
  for (DLMeta::iterator itMT = begin(); itMT != end(); itMT++)
  {
    DIMeta *item = (DIMeta *) (*itMT);

    if (item->itemTM == item1)
      continue;

    if ( (item->itemTM->lastPos == 1) == (item1->lastPos == 1) )
      continue;
      
    if (!item->IsPackable(item1, stg))
      continue;

    // Nach Moeglichkeit 1-ten mit 2-ten und nicht dritten paaren
    if (abs(item1->lastPos - item->itemTM->lastPos) == 1)
      return item->itemTM;
    else if (!candidate)
      candidate = item;
  }

  return candidate ? candidate->itemTM : NULL;
}


// Liefert das erste Team, das passt, ohne weitere Ruecksicht
DrawItemTeam * DLMeta::GetAnyTeam(DrawItemTeam *item1, int stg)
{
  DIMeta *candidate = NULL;

  // Durchlauf ohne Gruppen-Paare aufzubrechen
  for (DLMeta::iterator itMT = begin(); itMT != end(); itMT++)
  {
    DIMeta *item = (DIMeta *) (*itMT);
    
    if (!item->IsPackable(item1, stg))
      continue;

    if (GetGroupTeam(item->itemTM, stg))
      continue;

    // Nach Moeglichkeit 2 Dritte oder einen Dritten mit einem nicht geesetzten
    if ( (item->itemTM->lastPos > 2) == (item1->lastPos > 2) )
      return item->itemTM;
    
    if (item->itemTM->lastPos > 2 && !item1->pos[stg+1])
      return item->itemTM;
    
    if (item1->lastPos > 2 && !item->itemTM->pos[stg+1])
      return item->itemTM;
    
    if (!candidate)
      candidate = item;
  }

  if (candidate)
    return candidate->itemTM;

  // Ohne diese Einschraenkung
  for (DLMeta::iterator itMT = begin(); itMT != end(); itMT++)
  {
    DIMeta *item = (DIMeta *) (*itMT);

    if (item->itemTM == item1)
      continue;
      
    if (!item->IsPackable(item1, stg))
      continue;

    // Nach Moeglichkeit 2 Dritte oder einen Dritten mit einem nicht geesetzten
    if ( (item->itemTM->lastPos > 2) == (item1->lastPos > 2) )
      return item->itemTM;
    
    if (item->itemTM->lastPos > 2 && !item1->pos[stg+1])
      return item->itemTM;
    
    if (item1->lastPos > 2 && !item->itemTM->pos[stg+1])
      return item->itemTM;
    
    if (!candidate)
      candidate = item;
  }

  return candidate ? candidate->itemTM : NULL;
}


// Liefert Team aus Liste
// Reihenfolge:
// Gesetzte mit Gesetzten, Byes mit zweiten, Paired, Irgendeinen
DrawItemTeam *  DLMeta::GetTeam(int stg)
{
  if (size() == 0)
    return NULL;
    
  DrawItemTeam *itemTM;
  
  // Zuerst Gesetzte, dann Freilose, dann den Rest
  if ( (itemTM = GetSeededTeam(stg)) != 0)
    return itemTM;

  if ( (itemTM = GetBye(stg)) != 0)
    return itemTM;
    
  // Die ersten muessen gleichmaessig verteilt werden    
  if ( (itemTM = GetFirst(stg)) != 0)
    return itemTM;

  return ((DIMeta *) (*begin()))->itemTM;
}


// Liefert Team, das mit item1 in ein Pack passt
// Kriterien:
// gesetzten mit gesetzten, Paired (beinhaltet Byes), irgendeinen (1..n)
DrawItemTeam * DLMeta::GetTeam(DrawItemTeam *item1, int stg)
{
  DrawItemTeam *itemTM = 0;
  
  // Gruppen trennen hat absolute Prioritaet
  if ( (itemTM = GetGroupTeam(item1, stg)) != 0 )
    return itemTM;

  // Gesetzte miteinander, die muessen eh getrennt werden    
  if ( item1->IsSeeded(stg+1) && (itemTM = GetSeededTeam(item1, stg)) != 0)
    return itemTM;

  // Freilose miteinander
  if ( item1->IsBye() && (itemTM = GetBye(item1, stg)) != 0)
    return itemTM;

  // Wenn das Freilos gesetzt ist, es gibt ein ungesetztes und einen Zweiten,
  // dann das Freilos nicht mit einem Zweiten kombinieren
  if (item1->IsBye() && GetBye(stg) && GetSecond(stg) && (itemTM = GetFirst(item1, stg) )!= 0)
    return itemTM;
    
  // Freilos mit Zweiten. Damit wandert zumindest ein zweiter fort
  // vom Freilos
  if ( item1->IsBye() && (itemTM = GetSecond(item1, stg)) != 0)
    return itemTM;
    
# ifdef DEBUG    
  if (item1->IsBye() && Count() > 1)
  {
    wxFprintf(file, "No second found for BYE\n");
    for (DLMeta::iterator it = begin(); it != end(); it++)
    {
      DIMeta *item = (DIMeta *) (*it);
      wxFprintf(file, "%s:%d, ", grList[item->itemTM->lastGroup].data(), item->itemTM->lastPos);
    }
    wxFprintf(file, "\n");
  }
# endif  

  // 20090513
  // Es gibt auch Zweite, die gesetzt sind, daher Paired und nicht Erste
  // Ausserdem nur in der ersten Stufe, ab der zweiten sind Packs unabhaengig
  // if ( item1->IsSeeded(stg+1) && (itemTM = GetFirst(item1, stg)) != 0)
  //   return itemTM;

  // if ( stg == 1 && item1->IsSeeded(stg+1) && (itemTM = GetPairedTeam(item1, stg)) != 0)
  //   return itemTM;

  if ( (itemTM = GetPairedTeam(item1, stg)) != 0)
    return itemTM;

  // In der ersten Stufe sind die Packs voneinander abhaengig, die 
  // ersten muessen aber dennoch gleichmaessig verteilt werden (die
  // Freilose sind es auch, und es soll entsprechend erste geben).
  // item1 ist, wenn moeglich, ein Erster. Ist es keiner, gibt es
  // hier auch keinen weiteren ersten. Ist aber einfacher, das so
  // zu schreiben, statt lastPos zu testen.  
  
  // item1 muss nicht ein erster sein, da auch zweite gesetzt sind
  // if ( stg == 1 && (itemTM = GetFirst(item1, stg)) != 0)
  //   return itemTM;

  // if ( stg == 1 && (itemTM = GetImpairedTeam(item1, stg)) != 0)
  //   return itemTM;
    
  // Fuenftens: egal welcher, Hauptsache passt
  return GetAnyTeam(item1, stg);
}



// +-----------------------------------------------------------------+ 

DrawCP::DrawCP(bool fl, Connection *ptr)
{
  connPtr = ptr;
  
  champs = fl;

  totalFirst = 0;
  totalNA = 0;
  totalBY = 0;

  frStage = "Qualification";
  toStage = (fl ? "Championship" : "Consolation");
}


DrawCP::~DrawCP()
{
  // Rollback, was noch nicht beendet ist
  if (connPtr)
  {  
    connPtr->Rollback();
    connPtr->Close();
  }
    
  delete connPtr;
}


// Gruppen auslesen und Liste aufbauen
bool DrawCP::ReadGroups()
{
  # ifdef  DEBUG_PACK
  // FILE *file = fopen("src\\debug", "a");
  wxFprintf(file, "           Read Groups               \n");
  wxFprintf(file, "-------------------------------------\n");
  # endif

  // Initialize mapping fuer listRG
  std::map<wxString, NaRec> tmp;
  std::map<int, NaRec> naMap;

  naMap[0] = NaRec();

  NaStore na(connPtr);
  na.SelectAll();
  while (na.Next())
  {
    naMap[na.naID] = na;
    if (wxStrlen(na.naRegion) == 0)
      continue;

    if (tmp.find(wxString(na.naRegion)) == tmp.end())
      tmp[wxString(na.naRegion)] = na;

    idMapping[na.naID] = tmp[wxString(na.naRegion)];
  }

  na.Close();

  GrStore  gr(connPtr);
  GrStore *pgr = new GrStore[gr.CountGroups(cp, frStage)];
  int idx = 0;

  gr.SelectAll(cp);

  while (gr.Next())
  {
    if (wxStricmp(gr.grStage, frStage))
      continue;

    pgr[idx++] = gr;
    
#ifdef DEBUG  
    grList.insert(std::map<long, wxString, std::less<long> >::value_type(gr.grID, wxString(gr.grName)));  
#endif
  }
  
  grList.insert(std::map<long, std::string, std::less<long> >::value_type(0, std::string("BYE")));  
  

  int count = idx;
  for (idx = 0; idx < count; idx++)
  {
    // Gruppe beendet? Oder zumindest eine gueltige Tabelle?
# ifndef DEBUG_TABLE
    if (!pgr[idx].QryFinished() /* && !gr.QryTable() */ )
    {
      infoSystem.Error(_("Group %s has not finished"), pgr[idx].grName);
      delete[] pgr;

      return false;
    }
# endif
  }
  
  for (int idx = 0; idx < count; idx++)       
  {
    std::vector<TbItem *> tbList;
    std::vector<MtRec>    mtList;
    
    StEntryStore st(connPtr);
    st.SelectByGr(pgr[idx], cp.cpType);
    
    while (st.Next())
    {
      // Freilose uebergehen
      if (st.tmID)
        tbList.push_back(new TbItem(st));
    }
    
    MtStore  mt(connPtr);
    mt.SelectByGr(pgr[idx]);
    while (mt.Next())
      mtList.push_back(mt);
    
    TbSort::Sort(pgr[idx], cp.cpType, tbList, mtList);      
    
    int stPos = 0;
    bool modified = false;
    
    for (std::vector<TbItem *>::iterator it = tbList.begin(); it != tbList.end(); it++)
    {    
      StRec &st = (*it)->st;
      
      if (st.tmID == 0)
        continue;
        
      if (st.stGaveup || st.stDisqu || st.stNocons)
        continue;

  # ifndef DEBUG_TABLE
      if ( st.stPos == stPos || st.stPos == 0 )
      {
        infoSystem.Error(_("Group %s has invalid final standings"), pgr[idx].grName);             
        return false;
      }
      
      if (!modified && st.stPos != (*it)->result.pos)
      {
        if (!infoSystem.Confirmation(_("The group positions of group %s have been modified. Continue anyway?"), pgr[idx].grName))
          return false;
          
        modified = true;
      }
      
  # endif

  # ifdef DEBUG_TABLE
      st.stPos = ++stPos;
  # endif

      if (fromPos || toPos)
      {
        if (fromPos && st.stPos < fromPos || toPos && st.stPos > toPos)
          continue;
      }
      else if (champs && st.stPos > (pgr[idx].grSize + 1) / 2 ||
          !champs && st.stPos <= (pgr[idx].grSize + 1) / 2)
        continue;

      DrawItemTeam *itemTM = listNA.AddTeam( (*it)->entry );      
      totalNA++;
      
      // lastGroup und lastPos eintragen
      if (itemTM)
      {
        itemTM->lastGroup = st.grID;
        if (fromPos)
          itemTM->lastPos = st.stPos - fromPos + 1;
        else
          itemTM->lastPos = champs ? st.stPos : st.stPos - (pgr[idx].grSize + 1) / 2;
        itemTM->pos[1] = 1;               // Init der Positionen
        
        if (itemTM->lastPos == 1)
          totalFirst++;
      }

      stPos = st.stPos;
    }

    for (std::vector<TbItem *>::iterator it = tbList.begin(); it != tbList.end(); it++)
      delete (*it);
    
  }   // for (gr)

  for (DrawListNation::iterator it = listNA.begin(); it != listNA.end(); it++)
    ((DrawItemNation *) (*it))->na = naMap[ ((DrawItemNation *) (*it))->na.naID ];

  # ifdef DEBUG_PACK
  // fclose(file);
  # endif

  delete[] pgr;

  return true;
}


// Auslesen der Setzung und Loeschen der Gruppe
bool DrawCP::ReadSeeding(GrStore &gr)
{
  StEntryStore  st(connPtr);
  st.SelectByGr(gr, cp.cpType);
  while (st.Next())
  {
    // Nicht gesetzte Spieler uebergehen, sonst kann man keine 
    // Auslosung wiederholen.
    if (st.st.stSeeded == 0)
      continue;
      
    // Freilos
    if (st.st.tmID == 0)
    {
      DrawItemTeam *itemBY = new DrawItemTeam();
       listBY.push_back(itemBY);
       
      itemBY->pos[0] = st.st.stNr;

      for (int stg = 1, sec = gr.grSize; sec; stg++, sec /= 2)
        itemBY->pos[stg] = (st.st.stNr - 1) / sec + 1;  
        
      continue;                    
    }
            
    DrawItemTeam  *itemTM = listNA.GetTeam(st.st.tmID);
    
    if (itemTM == 0)
    {
      // Gyula spcial: Gesetzte Mannschaften muessen nicht
      // aus der Quali kommen. Sie werden dann wie Gruppenerste
      // (und direct entries) behandelt.
      itemTM = listNA.AddTeam(st);
      itemTM->lastPos = 1;
      itemTM->rkDirectEntry = 1;
      totalNA++;
    }
    
    if (itemTM)
    {
      itemTM->pos[0] = st.st.stNr;

      for (int stg = 1, sec = gr.grSize; sec; stg++, sec /= 2)
        itemTM->pos[stg] = (st.st.stNr - 1) / sec + 1;   
    }
  } // for (st)

  // Zweiter Durchgang: Die jeweils zweiten testen
  st.Close();
  st.SelectByGr(gr, cp.cpType);
  while (st.Next())
  {
    // Nicht gesetzte und Freilose uebergehen
    if (st.st.stSeeded == 0)
      continue;

    if (st.st.tmID == 0)
      continue;

    DrawItemTeam *itemTM = listNA.GetTeam(st.st.tmID);

    // Zur Sicherheit, wenn nicht gefunden
    if (!itemTM)
      continue;

    // Positionen fangen immer mit 1 an, auch in Consolation.
    // Nur erste und zweite trennen, ein dritter darf woanders hin (I don't care)
    if (itemTM->lastPos != 1)
      continue;
        
    DrawItemTeam *itemOther;
    long grID = itemTM->lastGroup;

    // Siehe oben, es geht hier nur um den zweiten, nicht um weitere.
    short pos = itemTM->lastPos + 1;  
      
    if ( grID != 0 && (itemOther = listNA.GetTeam(grID, pos)) != 0 )
    {
      if (itemTM->pos[2] == itemOther->pos[2])
      {
        // Auch der zweite ist gesetzt, aber in der gleichen Haelfte!
        if (!infoSystem.Question(_("Second of group %s is seeded into the same half as first. Continue anyway?"), gr.grDesc))
          return false;
      }
      else
      {
        // Der zweite ist entweder nicht gesetzt oder in der anderen Haelfte.
        // Beides ist hier aber egal, er soll ja in die andere Haelfte kommen.
        if (itemTM->pos[2] == 1)
          itemOther->pos[2] = 2;
        else
          itemOther->pos[2] = 1;
      }
          
      # ifdef DEBUG
        wxFprintf(file, "Hint second %s:%d %s\n", 
                 grList[itemOther->lastGroup].data(), itemOther->lastPos,
                 itemOther->pos[2] == 1 ? "up" : "down");
      # endif          
    }
  } // for (st)

  return true;
}


struct  CompareTeamFunctional
{
  bool  operator()(DrawItemTeam *&i1, DrawItemTeam *&i2) 
  {
    if (i1->lastGroup < i2->lastGroup)
      return true;
    else if (i1->lastGroup > i2->lastGroup)
      return false;
    else if (i1->lastPos < i2->lastPos)
      return true;
    else
      return false;
  }
}; 


// Verteilen nach der Auslosung
bool DrawCP::Distribute(GrStore &gr)
{
# ifdef DEBUG
  wxFprintf(file, "\n\n\n            Distribute Teams              \n");
  wxFprintf(file, "------------------------------------------\n");
# endif

  gr.ClearDraw(cp, toStage);
  
  std::vector<DrawItemTeam *> tmList;

  for (DrawListNation::iterator itNA = listNA.begin(); itNA != listNA.end(); itNA++)
  {
    DrawItemNation *itemNA = (DrawItemNation *) (*itNA);
    
    for (DrawListTeam::iterator itTM = itemNA->teams.begin();
         itTM != itemNA->teams.end(); itTM++)
    {
      DrawItemTeam *itemTM = (DrawItemTeam *) (*itTM);
      
      tmList.push_back(itemTM);
    }
  }
        
  // Und die Freilose
  for (DrawListTeam::iterator itTM = listBY.begin(); itTM != listBY.end(); itTM++)
    tmList.push_back( (DrawItemTeam *) (*itTM) );
        
  std::sort(tmList.begin(), tmList.end(), CompareTeamFunctional());
  
  for (std::vector<DrawItemTeam *>::iterator itTM = tmList.begin();
       itTM != tmList.end(); ++itTM)
  {
    DrawItemTeam *itemTM = (DrawItemTeam *) (*itTM);
    
    TmListRec  tm = itemTM->tm;
    gr.SetTeam(itemTM->pos[lastStage+1], tm, StStore::ST_KEEPSEEDED);

# ifdef DEBUG
    wxFprintf(file, "%04i: %s:%i\n", 
             itemTM->pos[lastStage+1],
             grList[itemTM->lastGroup].data(),
             itemTM->lastPos);
# endif
  }

  return true;
}


// Mischen der Listen
bool DrawCP::Shuffle()
{
  listNA.Shuffle();

  for (DrawListNation::iterator itNA = listNA.begin(); itNA != listNA.end(); itNA++)
    ((DrawItemNation *) (*itNA))->teams.Shuffle();

  listBY.Shuffle();

  return true;
}


// Aufbau der Packlist
// In die Packliste kommen zuerst gepaarte Spieler einer Nation.
// Die Spieler, die nicht gepaart werden koennen, wandern in einen Pool (listTM)
// Im allgemeinen sollen das nur einzelne Spieler einer Nation sein, ausser
// aus der Setzung ergibt es sich anders.
// Nachdem alle Nationen abgearbeitet wurden, wird dieser Pool abgearbeitet.
// Kann zu einem Spieler kein passender Partner gefunden werden, wird ein
// bereits bestehendes Pack aufgebrochen (das kann passieren, wenn auf
// 4 Plaetze 1 gesetzter Spieler, ein Freilos und 2 Spieler einer Nation
// kommen. Die beiden Spieler werden im ersten Schritt gepaart, die beiden
// gesetzten (Spieler + Freilos) koennen nicht kombiniert werden und daher
// muss das Pack aufgebrochen werden. Nachteil: Es muessen bereits im ersten
// Spiel 2 Spieler einer Nation gegeneinander spielen.
// Hints: Es duerfen nicht mehr als halbsoviele Spieler einer Nation in
// eine Haelfte gelost werden, als freie Plaetze fuer sie vorhanden sind.
// Freie Plaetze sind alle, auf denen kein gesetzter Spieler ist plus
// die mit gesetzten Spielern dieser Nation.
// Deswegen werden zuerst die freien Plaetze berechnet (Gesamtzahl Plaetze
// minus Freilose (sind alle gesetzt) minus gesetzte Spieler (freeUp / freeDown).
// Fuer jede Nation wird dieser Wert korrigiert (naFreeUp / naFreeDown).
// Fuer jeden gepaarten Spieler wird gezaehlt, wohin er kommen kann
// (naUp / naDown erhoehen). Fuer die einzelnen Spieler, die in den Pool
// wandern, wird geprueft, ob diese unter diesen Vorraussetzungen noch frei
// auslosbar sind und wenn nicht, wird ein Wert fuer diese Stufe vorgegeben.
// freeUp / freeDown werden entsprechend korrigiert.
// Das ist nur moeglich, solange freie Plaetze verfuegbar sind, sonst kaemen
// mehr Spieler in eine Haelfte, als Plaetze verfuegbar sind.
// Das Verfahren schlaegt nur zu, wenn mindestens ein Spieler oben oder unten
// ist, das heisst, wenn sich eine zu fruehe Paarung ergaebe, und in der
// anderen Haelfte aber noch Platz fuer ihn ist.

bool DrawCP::BuildPacklist(int stg, int sec)
{
  DLPack        listPK;      // Packliste
  DLMeta        listTM;      // Liste einzelner Spieler, die nicht sofort in Packlist koennen

  // Als erstes die Freilose in Liste eintragen ...
  for (DrawListTeam::iterator itBY = listBY.begin(); itBY !=listBY.end(); itBY++)
  {
    DrawItemTeam *itemTM = (DrawItemTeam *) (*itBY);

    if (itemTM->pos[stg] != sec)
      continue;

    listTM.Add(itemTM);
  }

  // Byes verteilen
  int dbc = listTM.size(), dbu = 0, dbl = 0;

  CalculateByes(stg, sec, lastStage, dbc, dbu, dbl);
  
  for (DLMeta::iterator it = listTM.begin(); it != listTM.end(); ++it)
  {
    if (dbu > 0)
    {
      --dbu;
      ((DIMeta *) (*it))->itemTM->pos[stg+1] = 2 * sec - 1;
    }
    else if (dbl > 0)
    {
      --dbl;
      ((DIMeta *) (*it))->itemTM->pos[stg+1] = 2 * sec;
    }
    else
      break;
  }

  // Nationen und Teams mischen, bevor die dritten verteilt werden
  listNA.Shuffle();

  // Freie Plaetze oben / unten zaehlen
  int  freeUp, freeDown;      

  freeUp = freeDown = ( 1 << (lastStage - stg) );

  freeUp   -= listNA.Count(stg+1, 2*sec-1);
  freeDown -= listNA.Count(stg+1, 2*sec);

  freeUp   -= listBY.Count(stg+1, 2*sec-1);
  freeDown -= listBY.Count(stg+1, 2*sec);

  // Markierung zuuecksetzen und vermischen
  for (DrawListNation::iterator itNA = listNA.begin(); 
       itNA != listNA.end(); itNA++)
  {
    DrawItemNation *itemNA = (DrawItemNation *) (*itNA);

    itemNA->teams.Shuffle();
    
    for (DrawListTeam::iterator itTM = itemNA->teams.begin();
         itTM != itemNA->teams.end(); itTM++)
    {    
      DrawItemTeam *itemTM = (DrawItemTeam *) (*itTM);
      itemTM->mask = false;
    }
  }
  
#ifdef DEBUG
  wxFprintf(file, "Pack by associations\n");
#endif

  BuildPackList(stg, sec, listNA, listPK, listTM, freeUp, freeDown);

  // Regionalverbaende auslosen.
  // Dazu ueber die nicht zuordnenbaren Teams iterieren und wenn sie
  // zu einem Regionalverband gehoeren, nach listRG aufnehmen.
  DrawListRegion listRG;
  for (DLMeta::iterator it = listTM.begin(); it != listTM.end(); )
  {
    DIMeta *item = (DIMeta *) (*it);

    if (idMapping.find(item->itemTM->tm.naID) != idMapping.end())
    {
      // itemTM hat bereits die Maske gesetzt.
      item->itemTM->mask = false;

      // Per pointer uebergeben. Der Destruktor loescht nicht
      if (listRG.Get(idMapping[item->itemTM->tm.naID]) == NULL)
        listRG.Add(idMapping[item->itemTM->tm.naID]);

      listRG.AddTeamItem(item->itemTM, idMapping[item->itemTM->tm.naID].naID);

      // DIMeta kann man loeschen
      delete item;

      it = listTM.erase(it);
    }
    else
      it++;
  }

#ifdef DEBUG
  wxFprintf(file, "Pack by regions\n");
#endif

  BuildPackList(stg, sec, listRG, listPK, listTM, freeUp, freeDown);

#ifdef DEBUG
  wxFprintf(file, "Pack remaing teams\n");
#endif  

  // Die letzten zwei kann man immer kombinieren
  if (listTM.Count() == 2)
  {
    DIMeta *item = (DIMeta *) (*listTM.begin());

    // S.o.
    item->itemTM->mask = false;

    DrawItemTeam *itemTM = item->itemTM;

    // DIMeta kann man loeschen
    delete item;
    listTM.erase(listTM.begin());

    item = (DIMeta *) (*listTM.begin());

    // S.o.
    item->itemTM->mask = false;

    if ( !itemTM->IsSeeded(stg) || item->itemTM->IsSeeded(stg) || itemTM->pos[stg] != item->itemTM->pos[stg] )
    {
      listPK.Add(itemTM, item->itemTM);
    }
    else
    {
      listPK.Add(itemTM, NULL);
      listPK.Add(item->itemTM, NULL);
    }

    // DIMeta kann man loeschen
    delete item;
    listTM.erase(listTM.begin());
  }

  while (listTM.Count())
  {
    DIMeta *item = (DIMeta *) (*listTM.begin());

    // S.o.
    item->itemTM->mask = false;

    listPK.Add(item->itemTM, NULL);

    // DIMeta kann man loeschen
    delete item;
    listTM.erase(listTM.begin());
  }
  
  int  countPK = listPK.Count();

  // Fehler, wenn die Anzahl der Spieler nicht stimmt
  if ( countPK * 2 - listPK.CountIncomplete() != (1 << (lastStage - stg + 1)) )
  {
    int found = countPK * 2 - listPK.CountIncomplete();
    int exp = 1 << (lastStage - stg + 1);

    infoSystem.Error(_("Error in stage %d section %d: Wrong number of players found (found %d, expected %d)!"), stg, sec, found, exp);
    return false;
  }

  // Ueberpruefungen
  DIPack *pack = (DIPack *) (*listPK.begin());

  // Warnung, wenn in der letzten Stufe 2 Packs existieren
  if (stg == lastStage && countPK > 1)
  {
#ifdef DEBUG
    wxFprintf(file, "   More than 1 pack in last stage\n");
#endif

    warnings.push_back(wxString::Format(_("More than 1 pack in last stage at pos %d - %d"), 2 * sec - 1, 2 * sec));
  }

  // Warnung, wenn zwei erste unerwartet gegeneinander spielen
  if (countPK == 1 && totalFirst <= (totalNA + totalBY) / 2 && pack->Is0First() && pack->Is1First())
  {
#ifdef DEBUG
    wxFprintf(file, "   Error in pack\n");
#endif

    warnings.push_back(wxString::Format(_("Two first playing in first round at pos %d - %d"), 2 * sec - 1, 2 * sec));
  }

  // Warnung, wenn ein Zweiter ein (unerwartetes) Freilos bekommt
  if (countPK == 1 && totalBY <= totalFirst && pack->IsBye() && !(pack->Is0First() || pack->Is1First()))
  {
#ifdef DEBUG
    wxFprintf(file, "   Error in pack\n");
#endif

    warnings.push_back(wxString::Format(_("Second got an unexpected bye in first round at pos %d - %d"), 2 * sec - 1, 2 * sec));
  }

  // Warnung, wenn ein Erster kein erwartetes Freilos bekommt

  if (countPK == 1 && totalBY >= totalFirst && !pack->IsBye() && (pack->Is0First() || pack->Is1First()))
  {
#ifdef DEBUG
    wxFprintf(file, "   Error in pack\n");
#endif

    warnings.push_back(wxString::Format(_("First did not get an expected bye in first round at pos %d - %d"), 2 * sec - 1, 2 * sec));
  }

  // Und auslosen
  listPK.Draw(stg, sec);

  return true;
}


bool DrawCP::BuildPackList(int stg, int sec, DrawListNation &list, DLPack &listPK, DLMeta &listTM, int &freeUp, int &freeDown)
{
  DLMeta        listSC;      // temp. Liste der Spieler in Section sec
  DrawItemTeam *itemTM;      // Team aus Liste

  // Nacheinander in Packs eintragen
  for (int run = 0; run < 2; run++)
  {
#ifdef DEBUG
    wxFprintf(file, "Pack associations run %d\n", run);
#endif
        
    // 2 runs: Zuerst die Gesetzten, dann den Rest
    for (DrawListNation::iterator itNA = list.begin(); itNA != list.end(); itNA++)
    {
      // listSC enthaelt im ersten Durchlauf noch die ungesetzten Spieler
      while (listSC.CutFirst())
        ;
      
      DrawItemNation *itemNA = (DrawItemNation *) (*itNA);

      // Liste der Teams mischen
      itemNA->teams.Shuffle();

      // Spieler dieser Nation , die nach oben / unten wandern
      int naUp = 0, naDown = 0;
      // Freie Plaetze fuer diese Nation. Das sind alle freien Plaetze
      // plus die, auf denen gesetzte Spieler dieser Nation sitzen
      int naFreeUp = freeUp, naFreeDown = freeDown;

      //  Spieler heraussuchen
      for (DrawListTeam::iterator itTM = itemNA->teams.begin();
          itTM != itemNA->teams.end(); itTM++)
      {
        DrawItemTeam *itemTM = (DrawItemTeam *) (*itTM);

        if (itemTM->pos[stg] != sec)
          continue;

        // Gesetzter Spieler ? Freie Plaetze hochzaehlen
        if (itemTM->pos[stg+1])
        {
          (itemTM->pos[stg+1] & 0x1 ? naUp++     : naDown++);
          (itemTM->pos[stg+1] & 0x1 ? naFreeUp++ : naFreeDown++);
        }
        
        if (itemTM->mask)
          continue;
        
        // In temporaere Liste einfuegen
        listSC.Add(itemTM);
      }

      // Liste der Spieler in (stg, sec) mischen
      listSC.Shuffle();
      while ((itemTM = listSC.GetTeam(stg)) != 0)
      {
        // assert(itemTM->mask == false);
        if (run == 0 && !itemTM->IsSeeded(stg+1))
          break;
          
        listSC.Subtract(itemTM);
        
        // Markierung setzen
        itemTM->mask = true;

        DrawItemTeam *itemNext = listSC.GetTeam(itemTM, stg);

        if (run > 0 && stg == lastStage - 1 && listTM.Count() == 2)
        {
          // Die letzten 4, aber in listTM gibt es bereits 2, die ich nicht kombinieren kann, da beide gesetzt
          DrawListTeam::iterator it = listTM.begin();
          DrawItemTeam *item1 = ((DIMeta *) (*it))->itemTM;
          it++;
          DrawItemTeam *item2 = ((DIMeta *) (*it))->itemTM;

          if (item1->IsSeeded(stg+1) && item2->IsSeeded(stg+1) && item1->pos[stg+1] == item2->pos[stg+1])
          {
            listTM.Add(itemTM);
            continue;
          }
        }

        // assert(itemNext == NULL || itemNext->mask == false);
        
  #if 1      
        // Wenn es mindestens 2 Teams gibt und zu diesem Team gibt es
        // keinen gleichwertigen, dieses spaeter abarbeiten.
        // Ebenso in der ersten Runde keine Impaired einer Nation
        // zulassen, da durch die Randbedingung, erster und zweiter 
        // einer Gruppe zu trennen, die ersten nicht gleichmaessig 
        // verteilt werden.
        // 20090517 Nur stg == 1, da nur hier Packs voneinander abhaengig sind
        //          (danach sollten Gruppen getrennt sein)
        //          Nur wenn beide nicht gesetzt sind.
        //          Der Fall, dass einer gesetzt ist, wird unten abgehandelt.
        //          Sind beide gesetzt verbaue ich mir nichts, da beide ohnehin
        //          in unterschiedliche Haelften kommen.
        //          Mehr als ein Team: Da versucht wird, paarweise zu suchen
        //          bedeutet ein ungepaartes Pack, dass die anderen besser passen.
        // Ausserdem nur erster und zweiter        
        
        if ( itemNext != NULL && stg == 1 && listSC.Count() > 1 &&
            !itemTM->IsSeeded(stg+1) && !itemNext->IsSeeded(stg+1) &&
            itemTM->lastPos <= 2 && itemNext->lastPos <= 2 &&
            itemTM->lastGroup != itemNext->lastGroup && 
            itemTM->lastPos != itemNext->lastPos ) 
            
        {
  #ifdef DEBUG
          wxFprintf(file, "   Team %s:%d skipped, other team with different group pos.\n", 
                   grList[itemNext->lastGroup].data(), itemNext->lastPos);
  #endif        
          itemNext = NULL;
        }
  #endif      

        // In der ersten Runde nur gesetzte Spieler mit Spielern der
        // gleichen Gruppe zulassen      
        // 20090513: Nur wenn der andere nicht gesetzt ist
        //           Packs sind voneinander abhaengig, ein gesetzter kann verhindern,
        //           dass ich das Pack so auslosen kann, dass ein Gruppenzweiter in die
        //           andere Haelfte kommt wie ein Gruppenerster (und umgekehrt).
        //           Gaebe es einen zweiten Gesetzten, waere er im Pack. 
        //           Gibt es keinen weiteren mehr dieser Nation, setze ich einen 
        //           entsprechenden Hint
        //           Wenn itemTM nicht gesetzt ist, ist es itemNext auch nicht, 
        //           da zuerst nach gesetzten gesucht wird.
        // Ausserdem nur erster und zweiter
        if ( itemNext != NULL && stg == 1 && 
            itemTM->lastPos <= 2 && itemNext->lastPos <= 2 &&
            itemTM->IsSeeded(stg+1) && !itemNext->IsSeeded(stg+1) )                       
        {
          // 20090517 Ein gesetzter kommt mit einem nicht-gesetzten in ein Pack.
          //          Da mit jedem gesetzten auch der entsprechende andere aus der 
          //          Gruppe einen Hint gesetzt bekommt, koennen die beiden nicht aus
          //          der gleichen Gruppe stammen.
          //          In der ersten Stufe sind die Packs voneinander abhaengig, wird
          //          ein Team nach unten gelost wird als naechstes das Pack mit dem
          //          anderen der Gruppe entsprechend ausgelost. Das funktioniert nur
          //          dann verlaesslich, wenn in dem Pack beide gesetzt sind (das Pack 
          //          wurde dann entsprechend gebildet) oder keiner.
          //          Packe ich zu einem gesetzten einen nicht-gesetzten Spieler, muss
          //          der nicht-gesetzte in die andere Haelfte, der andere aus dessen
          //          Gruppe dann wieder in die gleiche wie der gesetzte. Dieser Spieler
          //          braucht also ebenfalls einen Hint. Das kann ich aber nur machen,
          //          wenn er noch nicht in einem Pack steckt, ansonsten muesste ich die 
          //          Packs weiter abarbeiten und komm eventuell an einen Punkt, an dem
          //          ich nicht weiter kann.
          //          Der andere der Gruppe kann nicht bereits gesetzt sein, sonst haette
          //          ich hier bereits einen Hint.
          //          Allerdings kann es passieren, dass ich mit diesem Hint 2 Spieler
          //          der anderen Nation in die gleiche Haelfte zwinge, da ich dort dann
          //          keine Wahlfreiheit mehr habe.
          if (listPK.GetGroupPack(itemNext))
          {
  #ifdef DEBUG
            wxFprintf(file, "   Team %s:%d skipped, other team seeded and from different group\n", 
                     grList[itemNext->lastGroup].data(), itemNext->lastPos);
  #endif        
            itemNext = NULL;
          }
          else 
          {
            // 1-te und 2-te trennen, 3-te und 4-te (wenn es ueberhaupt so viele gibt)
            short pos = Odd(itemNext->lastPos) ? itemNext->lastPos + 1 : itemNext->lastPos - 1;
            
            DrawItemTeam * itemGroup = list.GetTeam(itemNext->lastGroup, pos);
            
            // Wenn itemGroup bereits woanders hin ausgelost wurde, brauch ich darauf
            // keine Ruecksicht mehr zu nehmen
            if (itemGroup && itemGroup->pos[stg] != sec)
              itemGroup = NULL;
            
            // Hint kann gesetzt werden, wenn itemGroup nicht existiert (eigentlich nur Consolation)
            // oder von der Auslosung passt
            if (itemGroup == NULL || !itemGroup->pos[stg+1] || itemGroup->pos[stg+1] == itemTM->pos[stg+1])
            {
              // Hint itemNext
              itemNext->pos[stg+1] = Odd(itemTM->pos[stg+1]) ? itemTM->pos[stg+1] + 1 : itemTM->pos[stg+1] - 1;
              
  #ifdef DEBUG
              wxFprintf(file, "   Hint team %s:%d to go %s\n", 
                       grList[itemNext->lastGroup].data(), itemNext->lastPos, Odd(itemNext->pos[stg+1]) ? "Up" : "Down");
  #endif

              if (itemGroup) 
              {
                // Hint itemGroup: Andere Haelfte als itemNext, also gleiche wie itemTM
                itemGroup->pos[stg+1] = itemTM->pos[stg+1];
              
  #ifdef DEBUG
                wxFprintf(file, "   Hint team %s:%d to go %s\n", 
                         grList[itemGroup->lastGroup].data(), itemGroup->lastPos, Odd(itemGroup->pos[stg+1]) ? "Up" : "Down");
  #endif
              }
            }
            else
            {
  #ifdef DEBUG
              wxFprintf(file, "   Team %s:%d skipped, other team with different group pos and cannot set hint.\n", 
                       grList[itemNext->lastGroup].data(), itemNext->lastPos);
  #endif        
              itemNext = NULL;
            }
          }
        }
          
  #ifdef DEBUG
        if (!itemNext && listSC.Count() > 0)
        {
          wxFprintf(file, "   No next team found for %s:%d\n", 
                   grList[itemTM->lastGroup].data(), itemTM->lastPos);
                  
          for (DLMeta::iterator it = listSC.begin(); it != listSC.end(); ++it)
          {
            if ( !((DIMeta *) (*it))->IsPackable(itemTM, stg) )
              continue;
              
            DrawItemTeam *item = ((DIMeta *) (*it))->itemTM;

            wxFprintf(file, "   Candidate %s:%d\n", 
                     grList[item->lastGroup].data(), item->lastPos);
          }                
        }
  #endif      
  
        if (itemNext)
        {
          // Die Spieler belegen jeweils einen Platz oben und unten
          // Gestzte Spieler wurden aber bereits gezaehlt.
          // Deshalb werden sie zuerst runtergezaehlt und dann wieder hoch
          if (itemTM->pos[stg+1])
            (itemTM->pos[stg+1] & 0x1 ? naUp-- : naDown--);
          if (itemNext->pos[stg+1])
            (itemNext->pos[stg+1] & 0x1 ? naUp-- : naDown--);

          naUp++;
          naDown++;

  #ifdef DEBUG
          wxFprintf(file, "   Pair team %s:%d with %s:%d\n", 
                   grList[itemTM->lastGroup].data(), itemTM->lastPos, 
                   grList[itemNext->lastGroup].data(), itemNext->lastPos);
  #endif
                
          // Markierung setzen
          itemNext->mask = true;
          
          // Und ein Pack bilden
          listSC.Subtract(itemNext);
          listPK.Add(itemTM, itemNext);  
        }
        else 
        {
          // 1-te und 2-te trennen, 3-te und 4-te (wenn es ueberhaupt so viele gibt)
          short pos = Odd(itemTM->lastPos) ? itemTM->lastPos + 1 : itemTM->lastPos - 1;
          
          DrawItemTeam * itemGroup = list.GetTeam(itemTM->lastGroup, pos);
          
          // itemGroup kann ignoriert werden, wenn es nicht im gleichen Bereich ist
          if (itemGroup && itemGroup->pos[stg] != sec)
            itemGroup = NULL;
          
          // Ansonsten Hilfestellung geben:
          // Wenn der Spieler nicht gesetzt war
          // Wenn es schon Spieler in einer Haelfte gibt und sie
          // muessten gegeneinander spielen und es gibt einen besseren
          // Weg in der anderen Haelfte, dann dorthinein losen
          // Das geht aber nur, wenn der Spieler nicht gesetzt ist und wenn der andere
          // der Gruppe nicht schon in einem Pack ist (siehe oben)
          if ( !itemTM->pos[stg+1] && !listPK.GetGroupPack(itemTM) && 
               (itemGroup == NULL || itemGroup->pos[stg+1] == 0) )
          {
            int naf2Up = (naFreeUp + 1) / 2;
            int naf2Down = (naFreeDown + 1) / 2;
            
            if ( (naUp || naDown) &&
                 (naUp >= naf2Up) && (naDown < naf2Down) )
            {
              // Hint: Zu viele Spieler bereits in oberer Haelfte
              naDown++;
              freeDown--;
              itemTM->pos[stg+1] = 2 * sec;
              
#ifdef DEBUG
              wxFprintf(file, "   Hint team %s:%d to go down\n", 
                       grList[itemTM->lastGroup].data(), itemTM->lastPos);
#endif
            }
            else if ( (naUp || naDown) &&
                      (naDown >= naf2Down) && (naUp < naf2Up) )
            {
              naUp++;
              freeUp--;
              itemTM->pos[stg+1] = 2 * sec - 1;
              
    #ifdef DEBUG
              wxFprintf(file, "   Hint team %s:%d to go up\n", 
                       grList[itemTM->lastGroup].data(), itemTM->lastPos);
    #endif
             }
             
             if (itemTM->pos[stg+1] && itemGroup)
             {
               if (Odd(itemTM->pos[stg+1]))
                 itemGroup->pos[stg+1] = itemTM->pos[stg+1] + 1;
               else
                 itemGroup->pos[stg+1] = itemTM->pos[stg+1] - 1;

    #ifdef DEBUG
              wxFprintf(file, "   Hint team %s:%d to go up\n", 
                       grList[itemGroup->lastGroup].data(), itemGroup->lastPos);
    #endif
             }
           }

          listTM.Add(itemTM);
        }

      } // Abbau der Liste
    } // for NA
  } // for run

  return true;
}



// Packlist auslosen
bool DrawCP::DrawPacklist(int stg, int sec)
{
  return true;
}


bool DrawCP::DrawSection(int stg, int sec)
{
  // DEBUG
# ifdef DEBUG_PACK
  // FILE *file = fopen("src\\debug", "a");
  wxFprintf(file, "\n\n Draw Packs: Stage = %2d, Section = %2d\n", stg, sec);
  wxFprintf(file, "-------------------------------\n");
  // fclose(file);
# endif

  if (!BuildPacklist(stg, sec))     // Lost auch aus
    return false;
  // DrawPacklist(stg, sec);

  return true;
}


bool DrawCP::DrawStage(int stg)
{
  for (int sec = 1; sec <= exp2(stg-1); sec++)
    if (!DrawSection(stg, sec))
      return false;

  return true;
}


bool DrawCP::DrawThem(GrStore &gr)
{
  warnings.clear();

  // Freilose einfuegen
  for (int i = totalNA + listBY.size(); i < gr.grSize; i++)
    listBY.push_back(new DrawItemTeam());
    
  totalBY = listBY.size();

  // Init der Bye-Liste
  for (DrawListTeam::iterator itTM = listBY.begin(); itTM != listBY.end(); itTM++)
  {
    DrawItemTeam *itemBye = (DrawItemTeam *) (*itTM);

    itemBye->lastPos = 2;         // Alle byes sind zweite
    itemBye->pos[1] = 1;          // allgemeines init hier
  }

  lastStage = ld2(gr.grSize);

  for (int stg = 1; stg <= lastStage; stg++)
    if (!DrawStage(stg))
      return false;

  return true;
}


bool DrawCP::DrawImpl(long seed)
{
  GrStore  gr(connPtr);

  // Als erstes testen, ob die naechste Gruppe ex.ist.
  gr.SelectAll(cp);
  while (gr.Next())
  {
    if (!wxStricmp(gr.grStage, toStage))
      break;
  }

  gr.Close();

  if (!gr.grID)
  {
    infoSystem.Error(_("No group or group to small!"));
    return false;
  }

  if (gr.QryStarted())
  {
    infoSystem.Error(_("Group %s has already started!"), gr.grDesc);
    return false;
  }

  if (gr.QryDraw())
  {
    if (gr.QryScheduled())
    {
      infoSystem.Error(_("A draw has been performed and scheduled for group %s!"), gr.grDesc);
      return false;
    }
    
    if (!infoSystem.Confirmation(_("A draw has been performed for group %s. Continue anyway?"), gr.grDesc))
      return false;
  }

  // Einziges Randomize in der Auslosung
  srand(seed);

  time_t ct = time(NULL);
  #ifdef DEBUG
  struct tm *tm = localtime(&ct);
  
  wxChar fname[256];
  wxSprintf(fname, "%s\\%s_%s_%04d%02d%02dT%02d%02d%02d.txt", 
      CTT32App::instance()->GetPath().data(), cp.cpName, toStage.c_str(),
      tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

  file = wxFopen(fname, "w");
  // file = fopen("Debug.txt", "w");
  #endif

# ifdef DEBUG
  wxFprintf(file, "Seed: %ld\n", seed);
# endif

  if (!ReadGroups())               // Gruppen auslesen
  {
#ifdef DEBUG
    fclose(file);
#endif      
    return false;                     // ... nicht erfolgreich
  }

  // Setzung einlesen
  if (!ReadSeeding(gr))
  {
#ifdef DEBUG
    fclose(file);
#endif      
    return false;
  }

  // Spieler zaehlen und Groesse der Gruppe pruefen
  if (gr.grSize < totalNA)
  {
#ifdef DEBUG
    fclose(file);
#endif    
    infoSystem.Error(_("No group or group to small!"));
    return false;
  }

  if (!DrawThem(gr))
  {
#ifdef DEBUG
    fclose(file);
#endif      
    return false;
  }

  if (!warnings.empty())
  {
    wxString str = "Ignore the warnings and accept the draw?\n";

    for (std::list<wxString>::iterator it = warnings.begin(); it != warnings.end(); it++)
    {
      str += "\n";
      str += (*it);
    }

    if (!infoSystem.Question(str))
      return false;
  }

  warnings.clear();

  // In der Gruppe verteilen
  if (connPtr)
    connPtr->StartTransaction();
  else
    TTDbse::instance()->GetDefaultConnection()->StartTransaction();

  Distribute(gr);
  
#ifdef DEBUG
  fclose(file);
#endif    
  
  // Commit
  if (connPtr)
    connPtr->Commit();
  else
    TTDbse::instance()->GetDefaultConnection()->Commit();

  return true;
}


// Gruppen auslesen und Auslosung durchfuehren
bool DrawCP::Draw(const CpRec &cp_, const wxString &fr_, const wxString &to_, int fromPos_, int toPos_, int seed_)
{
  cp = cp_;
  frStage = fr_;
  toStage = to_;
  fromPos = fromPos_;
  toPos   = toPos_;

  bool ret = DrawImpl(seed_ ? seed_ : time(NULL));

  return ret;
}
  


