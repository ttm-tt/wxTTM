/* Copyright (C) 2020 Christoph Theis */

// drawrr.cpp
// Auslosung erste Stufe Oldies (Round Robin)

#include "stdafx.h"
#include "TT32App.h"

#include  "DrawRR.h"

#include  "MdStore.h"
#include  "MtStore.h"
#include  "NaStore.h"
#include  "StStore.h"       // 'Setzung'
#include  "StEntryStore.h"
#include  "TbEntryStore.h"
#include  "RkEntryStore.h"

#include  "InfoSystem.h"

// #include  "GrTmpl.h"        // Template fuer Gruppen

#include  <stdlib.h>
#include  <time.h>

#include  <set>

#include  <map>
#include  <string>

// Mapping von ID auf Name von Gruppe / Nation
static std::map<long, wxString, std::less<long> > grList;
static std::map<long, wxString, std::less<long> > naList;

// Map, in welcher Gruppe der Sieger einer Vorrundengruppe ist
static std::map<long, int> quMap;

// -----------------------------------------------------------------------


# ifndef  TURBO
#   define  random(x)    (rand() % (x))
#   define  randomize()  srand(time(0))
# endif

#define DEBUG
# ifdef DEBUG
    static FILE  *file;
# endif


// Benutzung von DrawItemTeam::pos
// pos[0] ist die nr der Gruppe, relativ zu pgr
// pos[1] ist die Position innerhalb der Gruppe


DrawRR::DrawRR(Connection *ptr)
{
  connPtr = ptr;
  groups = 0;
  pgr = 0;
  fromPos = 0;
  toPos = 0;
}


DrawRR::~DrawRR()
{
  listIRK.clear();

  if (pgr != 0)
    delete[] pgr;
    
  if (connPtr)
  {
    connPtr->Rollback();
    connPtr->Close();
    delete connPtr;
  }
}


// -----------------------------------------------------------------------

// Zum Sortieren der Gruppen
static  int compareGroupSize(const void *p1, const void *p2)
{
  return ((const GrStore *) p2)->grSize - ((const GrStore *) p1)->grSize;
}


// Einlesen der Gruppen
bool DrawRR::ReadGroups()
{
  GrStore  gr(connPtr);
  StStore  st(connPtr);
  TmEntryStore  tm(connPtr);

  int totalGR = gr.CountGroups(cp, toStage);
  int totalTM = listNA.Count(0, 0);

  // Soviel Gruppen anlegen
  if (pgr != 0)
    delete[] pgr;

  pgr = new GrStore[totalGR];

  // Gruppen der Stufe einlesen
  int  g = 0;                       // for groups
  int  totalSize = 0;               // Anzahl der Plaetze in allen Gruppen

  gr.SelectAll(cp, toStage);
  while (gr.Next())
  {
    pgr[g] = gr;

    g++;
    totalSize += gr.grSize;
  }

  // g ist die Anzahl der Gruppen
  groups = g;

  // Ist eine der Gruppen bereits gestartet? Oder gibt es bereits eine Auslosung
  bool  drawn = false;

  for (int i = g; i--; )
  {
    if ( pgr[i].QryScheduled() && (drawn || pgr[i].QryDraw()) )
    {
      infoSystem.Error(_("A draw has been performed and scheduled for group %s!"), pgr[i].grName);
      return false;
    }
    
    if (pgr[i].QryStarted())
    {
      infoSystem.Error(_("Group %s has already started!"), pgr[i].grName);
      return false;
    }

    if (!drawn && pgr[i].QryDraw())
    {
      if (!infoSystem.Confirmation(_("A draw has been performed for group %s. Continue anyway?"), pgr[i].grName))
        return false;

      drawn = true;
    }
  }

  // Ist Anzahl von Gruppen ausreichend? Ausschlaggebend ist die Anzahl der Plaetze!
  if (totalSize < totalTM)
  {
    infoSystem.Error(_("Too few groups for the draw!"));
    return false;
  }

  // Gruppen nach Groesse sortieren
  // qsort(pgr, groups, sizeof(GrStore), compareGroupSize);

  return true;
}


bool DrawRR::ReadEntries()
{
  if (fromStage.IsEmpty())
  {
    // Auf die einfache Art: alle, die nicht direct entries sind
    RkListStore  rk(connPtr);
    rk.SelectByCp(cp);
    while (rk.Next())
    {
      if (!rk.rkDirectEntry)
        listNA.AddTeam(rk);
    }

    return true;
  }
  else
  {
    // Die komplizierte Art: alle der vorherigen Stufe
    GrStore  gr(connPtr);
    GrStore *pgr = new GrStore[gr.CountGroups(cp, fromStage)];
    int idx = 0;

    gr.SelectAll(cp, fromStage);

    while (gr.Next())
    {
      pgr[idx++] = gr;

      grList.insert(std::map<long, wxString, std::less<long> >::value_type(gr.grID, wxString(gr.grName)));
    }

    grList.insert(std::map<long, std::string, std::less<long> >::value_type(0, std::string("BYE")));

    int count = idx;
    for (idx = 0; idx < count; idx++)
    {
      // Gruppe beendet?
      if (!pgr[idx].QryFinished())
      {
        infoSystem.Error(_("Group %s has not finished"), pgr[idx].grName);
        delete[] pgr;

        return false;
      }
    }

    for (int idx = 0; idx < count; idx++)
    {
      std::vector<TbEntry>  tbList;

      TbEntryStore tb(connPtr);
      tb.SelectByGr(pgr[idx], cp.cpType);

      while (tb.Next())
      {
        // Freilose uebergehen
        if (tb.tmID)
          tbList.push_back(tb);
      }

      std::sort(tbList.begin(), tbList.end(), [](const TbEntry &a, const TbEntry &b)
        {
          return a.st.stPos < b.st.stPos;
        }
      );

      int stPos = 0;
      bool modified = false;

      for (auto it : tbList)
      {
        if (it.st.stPos == 0 || it.st.stPos == stPos)
        {
          infoSystem.Error(_("Group %s has invalid final standings"), pgr[idx].grName);
          return false;
        }

        if (!modified && it.st.stPos != it.result.pos)
        {
          if (!infoSystem.Confirmation(_("The group positions of group %s have been modified. Continue anyway?"), pgr[idx].grName))
            return false;

          modified = true;
        }

        if (it.st.stPos < fromPos || toPos && it.st.stPos > toPos)
          continue;

        if (it.st.tmID == 0)
          continue;

        if (it.st.stGaveup || it.st.stDisqu)
          continue;

        DrawItemTeam *itemTM = listNA.AddTeam(it);

        // lastGroup und lastPos eintragen
        if (itemTM)
        {
          itemTM->lastGroup = it.st.grID;
          if (fromPos)
            itemTM->lastPos = it.st.stPos - fromPos + 1;
          else
            itemTM->lastPos = it.st.stPos - (pgr[idx].grSize + 1) / 2;
        }
      }
    }

    delete[] pgr;
  }

  return true;
}


// -----------------------------------------------------------------------
bool DrawRR::ReadRanking()
{  
  // Test, ob s eine Setzung gibt
  {
    StStore st(connPtr);
    st.SelectSeeded(cp, toStage);
    if (st.Next())
    {
      infoSystem.Information(_("Ranked players ignored because players have already been seeded"));

      return true;
    }
  }

  // Map mit dem Abschneiden der vorigen Runde
  std::map<long, int> fromGroupMap;
  std::map<long,int>  fromPosMap;

  if (!fromStage.IsEmpty())
  {
    StStore st(connPtr);
    st.SelectAll(cp, fromStage);
    while (st.Next())
    {
      if (!st.tmID)
        continue;

      fromGroupMap[st.tmID] = st.grID;
      fromPosMap[st.tmID] = st.stPos;
    }
  }

  // Set mit den IDs der Teams, um kein Team mehrmals zu haben
  std::set<long> tmSet;

  // Temp. Ranking
  DrawListTeam listTMP;

  if (rkChoice == None)
  {
    // No ranking used
  }
  else if (rkChoice == World)
  {
    bool hasRanking = false;

    // Int'l ranking used, players not finishing first will be replaced with winners
    RkEntryStore rk(connPtr);
    rk.SelectByCp(cp);
    while (rk.Next())
    {
      // Neither int'l rank nor ranking points
      // if (rk.rk.rkIntlRank == 0 && rk.rankPts == 0)
      //   continue;

      hasRanking |= (rk.rk.rkIntlRank > 0 || rk.rankPts > 0);

      DrawItemTeam *itemTMP = new DrawItemTeam(rk);

      listTMP.AddItem(itemTMP);
    }

    // If no ranking found continue as if no ranking is used
    if (!hasRanking)
      listTMP.clear();
  }
  else if (rkChoice == Groups)
  {
    // Result of groups used, winner of group 1 is #1, of group 2 is #2, ...
    if (fromPos == 1)
    {
      StEntryStore st(connPtr);
      st.SelectAll(cp, fromStage);
      int rank = 0;
      while (st.Next())
      {
        if (st.st.stPos == 1)
        {
          DrawItemTeam *itemTM = new DrawItemTeam(st);
          itemTM->rkIntlRank = ++rank;
          listTMP.AddItem(itemTM);
        }
      }
    }
  }

   // Sort
  listTMP.Shuffle();
  std::stable_sort(listTMP.begin(), listTMP.end(), [](const DrawItem *a, const DrawItem *b)
    {
      const DrawItemTeam *ta = (const DrawItemTeam *)a;
      const DrawItemTeam *tb = (const DrawItemTeam *)b;

      // Bei gleichem Int'l Ranking zaehlen die Punkte
      if (ta->rkIntlRank == tb->rkIntlRank)
        return ta->rankPts > tb->rankPts;

      // Ranking 0 immer nach hinten sortieren
      if (ta->rkIntlRank)
        return tb->rkIntlRank ? ta->rkIntlRank < tb->rkIntlRank : true;

      if (tb->rkIntlRank)
        return ta->rkIntlRank ? ta->rkIntlRank < tb->rkIntlRank : false;
        
      return ta->rankPts > tb->rankPts;
    }
  );

 // Neu ranken
  int rank = 0;
  for (auto it : listTMP)
    ((DrawItemTeam *)it)->rkIntlRank = ++rank;

  // Ranking der Reihe nach durchgehen und durch den Sieger der Vorrundengruppe ersetzen
  for (auto it : listTMP)
  {
    DrawItemTeam *itTMP = (DrawItemTeam *)it;

    // Bereits in Liste
    if (tmSet.find(itTMP->tm.tmID) != tmSet.end())
      continue;

    DrawItemTeam *itemTM = listNA.GetTeam(itTMP->tm.tmID);

    // Wenn Spieler nicht gewonnen hat stattdessen den Sieger nehmen
    if (!itemTM || itemTM->lastPos > 1)
    {
      if (fromPos == 1)
        warnings.push_back(wxString::Format(_("The ranked entry of group %s finished %d, trying winner instead"), grList[fromGroupMap[itTMP->tm.tmID]], fromPosMap[itTMP->tm.tmID]));

      itemTM = listNA.GetTeam(fromGroupMap[itTMP->tm.tmID], 1);

      // Erster gehoert nicht dazu
      if (!itemTM)
        continue;

      // Oder ist bereits drin
      if (tmSet.find(itemTM->tm.tmID) != tmSet.end())
        continue;
    }

    // Don't take twice: da wir oben ausgetauscht haben könnten wir ein Team mehrmals erwischen
    if (tmSet.find(itemTM->tm.tmID) != tmSet.end())
      continue;

    tmSet.insert(itemTM->tm.tmID);

    // Ranking und Punkte uebernehmen
    itemTM->rkIntlRank = itTMP->rkIntlRank;
    itemTM->rankPts = itTMP->rankPts;

    listIRK.Add(itemTM);  // Liste aller Spieler
  }

  // Int'l Rank sortieren, aber gleiches Ranking zufaellig
  listIRK.Shuffle();

  std::stable_sort(listIRK.begin(), listIRK.end(), [](const DrawItem *a, const DrawItem *b)
    {
      const DrawItemTeam *ta = (const DrawItemTeam *)a;
      const DrawItemTeam *tb = (const DrawItemTeam *)b;

      // Bei gleichem Int'l Ranking zaehlen die Punkte
      if (ta->rkIntlRank == tb->rkIntlRank)
        return ta->rankPts > tb->rankPts;

      // Ranking 0 immer nach hinten sortieren
      if (ta->rkIntlRank)
        return tb->rkIntlRank ? ta->rkIntlRank < tb->rkIntlRank : true;

      if (tb->rkIntlRank)
        return ta->rkIntlRank ? ta->rkIntlRank < tb->rkIntlRank : false;
        
      return ta->rankPts > tb->rankPts;
    }
  );

  // Neu ranken
  rank = 0;
  for (auto it : listIRK)
    ((DrawItemTeam *)it)->rkIntlRank = ++rank;

  return true;
}


// Mischen der Nationen und Teams
bool DrawRR::Shuffle()
{
  // Mischen von Nationen
  listNA.Shuffle();

  for (DrawListNation::iterator it = listNA.begin(); it != listNA.end(); it++)
    ((DrawItemNation *) (*it))->teams.Shuffle();

  return true;
}


// Ein Array der Laenge n mit den Zahlen 1...n in zufaelliger Reihenfolge besetzen
bool DrawRR::Mix(int *mix, int size)
{
  int *seed = new int[size];
  for (int i = size; i--; )
    seed[i] = mix[i];

  int  len = size, n;
  while (len--)
  {
    n = random(len+1);
    mix[len] = seed[n];
    // memmove broken ??
    for (int j = n; j <= len-1; j++)
      seed[j] = seed[j+1];
    // memmove(seed+n, seed+n+1, len-n);
  }

  delete[] seed;

  return true;
}


// Loeschen nicht gesetzter Teams
bool DrawRR::ClearGroups()
{
  // Setzung loeschen
  GrStore(connPtr).ClearDraw(cp, toStage);

  return true;
}


// Einlesen der Setzung;
// Parameter: Array mit der Anzahl der Spieler in den Gruppen
bool DrawRR::ReadSeeding(int *counts)
{
  StStore  st(connPtr);

  st.SelectSeeded(cp, toStage);
  while (st.Next())
  {
    // Gruppe finden
    for (int g = 0; g < groups; g++)
    {
      if (pgr[g].grID != st.grID)
        continue;

      DrawItemTeam  *itemTM = listNA.GetTeam(st.tmID);
      if (itemTM)
      {
        itemTM->seeded = true;
        itemTM->pos[0] = g+1;
        itemTM->pos[1] = st.stNr;
        counts[g]++;
      }
      else if (st.tmID == 0)
        counts[g]++;

      break;                                   
    } // for (g)
  }   // for (st)

  return true;
}


// Minimum von Array
int  DrawRR::Minimum(int *array, int size)
{
  int minimum = 0x7FFFFFFF;
  for (int i = 0 ; i < size; i++)
    if (array[i] < minimum)
      minimum = array[i];

  return minimum;
}


/*
   NextNumber liefert eine Gruppennummer nach folgenden Kriterien:
     - Unter allen mit kleinsten countsNA diejenige mit kleinstem count
       (Gruppen, in denen noch Spieler dieser Nation fehlen, gleichmaessig fuellen)
       - Hat eine Gruppe gesetzte Spieler einer noch nicht ausgelosten Nation 
         und hat diese nur noch einen freien Platz, diese nehmen.
       - Braucht eine Gruppe noch Spieler, diese nehmen, wenn diese Aktion nicht
         ebenfalls weitere Spieler benoetigt (2 Spieler einer anderen Nation und
         1 Spieler dieser Nation bereits in der Gruppe. Wg. der anderen muessen 
         weitere Spieler rein, wenn von dieser 2 Spiele drin sind, nochmals weitere
         Spieler. Wenn die Gruppe aber nur 4 Plaetze hat, geht das nicht.)
     - Irgendeine Gruppe mit freien Plaetzen

   Da das kleinste countsNA solange 0 ist, bis sich in jeder Gruppe ein
   Spieler befindet, werden so die Gruppen gleichmaessig gefuellt.
   Das zweite Kriterium dient nur der Sicherheit.

   Eine Gruppe benoetigt weitere Spieler, wenn mehr als die Haelfte der
   vorhandenen Spieler einer Nation angehoeren (das heisst bei 4-er Gruppen
   mindestens 2 einer Nation). needsMore wird dann so berechnet, dass
   diese Nation maximal die Haelfte der Spieler ausmacht.
   Gruppengroesse etc. werden beruecksichtigt.

   Fuer jede Nation wird die Reihenfolge, in der die Gruppen durchsucht
   werden, in TT_DRAW_RR::Mix(...) neu verwuerfelt.
*/

int  DrawRR::NextNumber(const int *mix, const int *counts, const int *countsRG, std::map<int, int> &countsNA, int *needMore, const int *seeds, int notAllowed)
{
  int minCountsRG = 0x7FFFFFFF;
  int minCountsNA = 0x7FFFFFFF;
  int minCountsGR = 0x7FFFFFFF;
  
  // Minimum von 'ountsRG, countsNA und countsGR fuer diese Gruppen
  for (int j = 0; j < groups; j++)
  {
    if (notAllowed == j+1)
      continue;

    if (countsRG[j] < minCountsRG)
      minCountsRG = countsRG[j];
  }

  for (int j = 0; j < groups; j++) 
  {
    if (notAllowed == j + 1)
      continue;

    if (countsRG[j] == minCountsRG && countsNA[j] < minCountsNA)
      minCountsNA = countsNA[j];
  }

  for (int j = 0; j < groups; j++)
  {
    if (notAllowed == j + 1)
      continue;

    if (countsRG[j] == minCountsRG && countsNA[j] == minCountsNA && counts[j] < minCountsGR)
      minCountsGR = counts[j];
  }

  int gMinGR = 0, nMinGR = 0, sMinGR = 0;   // Gruppe mit min count, needed playes, seeded players
  
  # ifdef DEBUG  
    wxFprintf(file, "\n");
    wxFprintf(file, "                Min. count RG = %d Min. count for min. RG / NA = %d\n", minCountsRG, minCountsGR);

    wxFprintf(file, "                count/countRG/size");

    for (int k = 0; k < groups; k++)
    {
      wxFprintf(file, "%s%d/%d/%d", 
              ((k % 5) == 0 ? "\n                " : "  "),
              counts[k], countsRG[k], pgr[k].grSize);
    }
    
    wxFprintf(file, "\n");
  # endif

  // 1. Kriterium: Unter den Gruppen mit kleinsten countsNA
  // die mit kleinsten counts (plus seeded, plus needMore)
  for (int nr = groups; nr--; )
  {
    int  m = mix[nr];
    
    if (notAllowed == m)
      continue;

    // Ausschlussbedingung: Gruppe muss noch freie Plaetze haben
    if (counts[m-1] >= pgr[m-1].grSize)
      continue;

    // Ausschlussbedingung: Spieler einer Nation muessen gleichmaessig
    // verteilt werden. Das heisst, Gruppen mit (zu vielen) Spielern ignorieren,
    // hier duerfen keine mehr hinein
    if (countsNA[m-1] > minCountsNA || countsRG[m-1] > minCountsRG)
      continue;

    // Ausschlussbedingung: Gruppen mit mehr als der Minumumanzahl ignorieren.
    // Hebt eigentlich die erste Bedingung, freie Plaetze in der Gruppe, auf.
    if (counts[m-1] > minCountsGR)
      continue;

    // Gruppe als letzte Fallbackloesung merken wenn noch keine gemerkt wurde
    // oder es sind schon Spieler in allen Gruppen und diese hier ist groesser
    if (gMinGR == 0)
      gMinGR = m;
    else if (minCountsRG && pgr[gMinGR-1].grSize < pgr[m-1].grSize)
      gMinGR = m;

    // Gruppen mit gesetzten bevorzugen, wenn diese gefuellt werden koennen.
    if (sMinGR > 0 && seeds[m-1] <= 0)
      continue;
    
    // Die erste Gruppe mit seeded players merken, wenn diese fast gefuellt ist.
    // Das heisst, solange genug Plaetze frei sind, lassen wir die Wahl.
    // Wenn aber nur noch einer frei ist, nehmen wir ihn, damit die Nation mit 
    // dem gesetzten Spieler diesen nicht nehmen muss und damit 2 Spieler einer
    // Nation in dieser Gruppe landen.
    // Dto. auch merken, wenn schon Spieler in allen Gruppen sind und diese hier 
    // ist groesser
    if (counts[m-1] == pgr[m-1].grSize - 1 && seeds[m-1] > 0)
    {
      if (sMinGR == 0)
        sMinGR = m;
      else if (minCountsRG && pgr[sMinGR-1].grSize < pgr[m-1].grSize)
        sMinGR = m;
    }
    
    // Gruppen mit needMore bevorzugen. Wenn wir schon eine haben,
    // alle weiteren ohne needMore ignorieren.
    if (nMinGR > 0 && needMore[m-1] <= 0 && counts[m-1] < pgr[m-1].grSize - 1)
      continue;
      
    // Wenn die Gruppe Spieler braucht und mit diesem Spieler weniger als die
    // Haelfte aus dieser Nation kommen, diese auswaehlen.
    // Das zweite ist wichtig, damit nicht diese Nation ebenfalls ein needMore
    // erzwingt, das heisst, eine 4-er Gruppe nicht nur Spieler aus 2 Nationen hat.
    // Wir setzen aber nur einmal. Damit ist es eine Fallback-Loesung:
    // Gibt es keine Gruppe mit gesetzten Spielern, wird diese genommen.
    // Gibt es eine Gruppe mit gestzten Spielern, muss diese auch needMore erfuellen.
    // Dto. auch merken, wenn schon Spieler in allen Gruppen sind und diese hier 
    // ist groesser
    if ( needMore[m-1] && (countsRG[m-1] + 1) < (pgr[m-1].grSize / 2) )
    {
      if (nMinGR == 0)
        nMinGR = m;
      else if (minCountsRG && pgr[nMinGR-1].grSize < pgr[m-1].grSize)
        nMinGR = m;
    }
      
# ifdef DEBUG
    if (gMinGR == 0)
      wxFprintf(file, "                Found no suitable group!\n");
# endif
  }

  // Gruppe benoetigt Spieler (beinhaltet seeded)
  if (nMinGR)
  {
# ifdef DEBUG
    wxFprintf(file, "                Group %d needed more players\n", nMinGR);
# endif
    needMore[nMinGR-1]--;
    return nMinGR;
  }

  // Gruppe hat seeded players 
  if (sMinGR)
  {
# ifdef DEBUG
    wxFprintf(file, "                Group %d has seeded players\n", sMinGR);
# endif    
    return sMinGR;  
  }

  // Eine Gruppe mit den wenigsten Spielern
  if (gMinGR)
  {
# ifdef DEBUG
    wxFprintf(file, "                Found Group %d, cMinGR = %d, count = %d\n",
        gMinGR, minCountsGR, counts[gMinGR-1]);
# endif
    return gMinGR;
  }

  // Sicherheit: Die erste Gruppe mit freien Plaetzen zurueck
  // Wahrscheinlich gibt es dann keine gute Loesung!
  for (int nr = groups; nr; nr--)
  {
    if (notAllowed = nr)
      continue;

    if (counts[nr-1] < pgr[nr-1].grSize)
    {
# ifdef DEBUG
      wxFprintf(file, "                Returning first not filled group %d, count = %d\n",
        nr, counts[nr-1]);
# endif
      return nr;
    }
  }
  return 0;
}


bool DrawRR::DrawRanking(int *counts)
{
  // Gibt es ueberhaupt Spieler mit Ranking
  if (listIRK.Count() == 0)
    return true;

  // wirlaufen die Gruppen von 1..n bzw. von n..1 durch bis wir eine
  // Gruppe haben, die noch nicht voll ist und in die die Nation passt.
  // Vorwaerts, wenn der aktuelle (i-te) / Anzahl der Gruppen gerade ist (0, 2, ...)
  // Rueckwaerts, wenn der atkuelle (i-te) / Anzahl der Gruppen ungerade ist (1, 3, ...)

  int currentSeed = 0;   // Welchen nahmen wir gerade
  std::map<long, int> currentNA;  // Anzahl bereits gesetzter per Nation
  std::map<long, int> currentRG;  // Anzahl bereits gesetzter per Region
  std::vector<std::map<long, int>> currentGroupNA;
  std::vector<std::map<long, int>> currentGroupRG;
  std::vector<DrawItemTeam *> lastTeams(groups);

  for (int i = 0; i < groups; i++)
  {
    currentGroupNA.push_back(std::map<long, int>());
    currentGroupRG.push_back(std::map<long, int>());
  }

  // Setup Regionen
  std::map<long, long> idMapping;

  {
    std::map<wxString, long> tmpMap;
    NaListStore na(connPtr);
    na.SelectAll();
    while (na.Next())
    {
      if (wxStrlen(na.naRegion) == 0)
      {
        // Leere Region wird wie eine eigene Region behandelt
        idMapping[na.naID] = na.naID;
        continue;
      }

      // Bereits bekannt?
      if (tmpMap.find(wxString(na.naRegion)) != tmpMap.end())
        idMapping[na.naID] = tmpMap[wxString(na.naRegion)];
      else
      {
        idMapping[na.naID] = na.naID;
        tmpMap[wxString(na.naRegion)] = na.naID;
      }
    }
  }

  for (auto tm : listIRK)
  {
    DrawItemTeam *itemTM = (DrawItemTeam *) tm;
    long naID = itemTM->tm.naID;
    int expected = currentSeed / groups;                  // How many may be in the group
    int expectedNA = currentNA[naID] / groups;            // How many of this nation may be in the group
    int expectedRG = currentRG[idMapping[naID]] / groups; // How many of this region may be in the group
    int candidate = -1;

    bool found = false;

    int i = (expected % 2) ? groups : -1;

    while (!found)
    {
      // Reihenfolge von Increment / Decrement ist wichtig
      if (expected % 2)
      {
        // Rueckwaerts, stop wenn wir bereits bei 0 sind
        if (i-- == 0)
          break;
      }
      else
      {
        // Vorwwaerts, stop wenn wir dann groups erreichen
        if (++i == groups)
          break;
      }

      // Bedingung: 
      //   Anzahl in Gruppe <= grSize
      //   Anzahl in Gruppe <= expected
      //   Anzahl Nation in Gruppe < expectedNA
      if (counts[i] == pgr[i].grSize)
        continue;

      if (counts[i] > expected)
        continue;

      if (currentGroupNA[i][naID] > expectedNA)
        continue;

      if (currentGroupRG[i][idMapping[naID]] > expectedRG)
        continue;

      found = true;
      break;
    }

    // Falls nicht gefunden muessen in eine Gruppe, die schon genug Spieler haben, einer mehr rein
    // i ist jetzt je nach Lauf am Anfang (0) oder Ende (groups)
    while (!found)
    {
      // Reihenfolge von Increment / Decrement ist wichtig
      if ((expected + 1) % 2)
      {
        // Rueckwaerts, stop wenn wir bereits bei 0 sind
        if (i-- == 0)
          break;
      }
      else
      {
        // Vowrwaerts, stop wenn wir dann groups erreichen
        if (++i == groups)
          break;
      }

      // Bedingung: 
      //   Anzahl in Gruppe <= expected
      //   Anzahl Nation in Gruppe < expectedNA
      //   Anzahl in Gruppe <= grSize
      if (counts[i] > (expected + 1))
        continue;

      if (currentGroupNA[i][naID] > expectedNA)
        continue;

      if (currentGroupRG[i][idMapping[naID]] > expectedNA)
        continue;

      if (counts[i] == pgr[i].grSize)
      {
        // If not the same country, it might be a candidate
        // It was the last one added (or so) so it has a low enough ranking
        // Check for lastTeams[i] is not necessary, this group is full so someone has been added recenty
        // But check if that was not a seeded entry! we use a special flag to test, because after the entry
        // is processed it looks like a seeded one.
        if (candidate == -1 && lastTeams[i] && !lastTeams[i]->seeded && lastTeams[i]->tm.naID != itemTM->tm.naID)
          candidate = i;

        continue;
      }

      found = true;
      break;
    }

    // If we didn't find a solution, but have a candidate:
    // swap the players ...
    if (!found && candidate != -1)
    {
      // Update counters as if we have found a team at candidate position
      currentNA[naID]++;
      currentRG[idMapping[naID]]++;
      currentGroupNA[candidate][naID]++;
      currentGroupRG[candidate][idMapping[naID]]++;

      itemTM->pos[0] = candidate + 1;      // Group Nr
      itemTM->pos[1] = counts[candidate];  // Group Pos
        
      if (itemTM->lastPos == 1)
        quMap[itemTM->lastGroup] = itemTM->pos[0];

      // Swap the teams
      DrawItemTeam *itemTM2 = itemTM;
      itemTM = lastTeams[candidate];
      lastTeams[candidate] = itemTM2;
      naID = itemTM->tm.naID;

      // And decrement the counter incremented by the candidate
      currentNA[naID]--;
      currentRG[idMapping[naID]]--;
      currentGroupNA[candidate][naID]--;
      currentGroupRG[candidate][idMapping[naID]]--;

      expectedNA = currentNA[naID] / groups;            // How many of this nation may be in the group
      expectedRG = currentRG[idMapping[naID]] / groups; // How many of this region may be in the group
    }

    // ... and try with the candidate, same direction as the original run
    while (!found)
    {
      // Reihenfolge von Increment / Decrement ist wichtig
      if (expected % 2)
      {
        // Rueckwaerts, stop wenn wir bereits bei 0 sind
        if (i-- == 0)
          break;
      }
      else
      {
        // Vorwwaerts, stop wenn wir dann groups erreichen
        if (++i == groups)
          break;
      }

      // Bedingung: 
      //   Anzahl in Gruppe <= grSize
      //   Anzahl in Gruppe <= expected
      //   Anzahl Nation in Gruppe < expectedNA
      if (counts[i] == pgr[i].grSize)
        continue;

      if (counts[i] > expected)
        continue;

      if (currentGroupNA[i][naID] > expectedNA)
        continue;

      if (currentGroupRG[i][idMapping[naID]] > expectedRG)
        continue;

      found = true;
      break;
    }

    // And finally any group with a free place, going in original direction
    // Starting at the beginning of this run we expect to find first non-full group where we stopped
    if (!found)
    {
      i = (expected % 2) ? groups : -1;
      while (!found)
      {
        // Reihenfolge von Increment / Decrement ist wichtig
        if ((expected) % 2)
        {
          // Rueckwaerts, stop wenn wir bereits bei 0 sind
          if (i-- == 0)
            break;
        }
        else
        {
          // Vowrwaerts, stop wenn wir dann groups erreichen
          if (++i == groups)
            break;
        }

        if (counts[i] == pgr[i].grSize)
          continue;

        found = true;
        break;
      }
    }

    if (!found)
    {
      infoSystem.Error(_("Could not find solution to draw ranked entries"));
      return false;
    }

    // Put into group
    counts[i]++;
    currentSeed++;
    currentNA[naID]++;
    currentRG[idMapping[naID]]++;
    currentGroupNA[i][naID]++;
    currentGroupRG[i][idMapping[naID]]++;
    lastTeams[i] = itemTM;

    itemTM->pos[0] = i + 1;      // Group Nr
    itemTM->pos[1] = counts[i];  // Group Pos
        
    if (itemTM->lastPos == 1)
      quMap[itemTM->lastGroup] = itemTM->pos[0];
  }
   
  return true;
}

bool DrawRR::DrawThem(int *counts)
{
  // Vermischen der Reihenfolge der Gruppe
  int  *mix      = (int *) calloc(groups, sizeof(int));    // 'Auslosung'
  int  *needMore = (int *) calloc(groups, sizeof(int));    // Ungleichgewicht der Nationen
  int  *seeds    = (int *) calloc(groups, sizeof(int));    // Anzahl gesetzter Spieler in Gruppe

  DrawItemNation *itemNA;
  DrawItemTeam   *itemTM;
  DrawListTeam::iterator itTM;
  DrawListNation::iterator itNA;
  
  // Anzahl gesetzter Spieler steht bereits in counts
  memcpy(seeds, counts, groups * sizeof(int));

  // Verteilen der Teams mit mehr Spielern als Gruppen
  for (itNA = listNA.begin(); itNA != listNA.end(); itNA++)
  {
    itemNA = (DrawItemNation *) (*itNA);

    if (itemNA->teams.size() < (unsigned) groups)
      continue;
      
    // Anzahl der Teilnehmer in jeder Gruppe    
    int max = (itemNA->teams.size() / groups);
    
    int *tmpCounts = (int *) calloc(groups, sizeof(int));

    // Setzung auslesen
    for (itTM = itemNA->teams.begin(); itTM != itemNA->teams.end(); itTM++)
    {
      itemTM = (DrawItemTeam *) (*itTM);
      if (itemTM->pos[0])
        tmpCounts[itemTM->pos[0] - 1]++;
    }
    
    // Jetzt verteilen: Zunaechst Sieger einer Vorrunde
    for (int step = 1; step <= 2; step++)
    {
      for (itTM = itemNA->teams.begin(); itTM != itemNA->teams.end(); itTM++)
      {
        itemTM = (DrawItemTeam *) (*itTM);
      
        // Bereits vergeben (also Setzung oder erste)
        if (itemTM->pos[0])
          continue;

        // Im ersten Schritt nur Sieger der Vorrunde
        if (step == 1 && itemTM->lastPos != 1)
          continue;

        // Solange suchen, bis eine Nation zugewiesen werden kann
        int i;   // Dient auch als Abbruchbedingung
        for (i = 0; i < groups; i++)
        {
          // Vorgruppenspieler trennen
          if (itemTM->lastPos > 1 && quMap[itemTM->lastGroup] == i+1)
            continue;

          if (tmpCounts[i] < max)
          {
            tmpCounts[i]++;
            counts[i]++;
            // seeds[i]++;

            itemTM->pos[0] = i + 1;

            // Gruppe der Sieger vermerken
            if (itemTM->lastPos == 1)
              quMap[itemTM->lastGroup] = itemTM->pos[0];

            break;
          }
        }
      
        // Keinen Platz mehr gefunden, also Ende
        if (i == groups)
          break;
      }
    }

    free(tmpCounts);
  }

  // Jetzt die Regionalverbaende
  // Initialize mapping fuer listRG
  std::map<long, NaRec> idMapping;

  {
    std::map<wxString, NaRec> tmp;
    NaStore na(connPtr);
    na.SelectAll();
    while (na.Next())
    {
      // Nationen ohne Regionalverband werden wie ein Regionalverband behandelt.
      // Damit hab ich nacher nur noch eine Schleife ueber die Regionen.
      if (wxStrlen(na.naRegion) == 0)
      {
        idMapping[na.naID] = na;

        continue;
      }

      if (tmp.find(wxString(na.naRegion)) == tmp.end())
        tmp[wxString(na.naRegion)] = na;

      idMapping[na.naID] = tmp[wxString(na.naRegion)];
    }

    na.Close();
  }

  DrawListRegion listRG;

  for (itNA = listNA.begin(); itNA != listNA.end(); itNA++)
  {
    itemNA = (DrawItemNation *) (*itNA);

    if (itemNA->Count() == 0)
      continue;

    if (listRG.Get(idMapping[itemNA->na.naID]) == NULL)
      listRG.Add(idMapping[itemNA->na.naID]);

    for (itTM = itemNA->teams.begin(); itTM != itemNA->teams.end(); itTM++)
    {
      listRG.AddTeamItem((DrawItemTeam *) (*itTM), idMapping[itemNA->na.naID].naID);
    }
  }

  // Jetzt innerhalb einer Region mischen
  for (itNA = listRG.begin(); itNA != listRG.end(); itNA++)
  {
    ((DrawItemNation *) (*itNA))->teams.Shuffle();
  }

  // Und jetzt verteilen, was mehr Teams als Gruppen hat
  // Da die Nationen mit mehr Spielern als Gruppen bereits aufgeteilt wurden,
  // koennen hier nur noch ein Spieler je Nation dazukommen.
  std::map<long, std::map<int, int>> countsNA;

  for (itNA = listRG.begin(); itNA != listRG.end(); itNA++)
  {
    itemNA = (DrawItemNation *) (*itNA);
    
    if (itemNA->teams.size() < (unsigned) groups)
      continue;
      
    // Anzahl der Teilnehmer in jeder Gruppe    
    int max = (itemNA->teams.size() / groups);

    // Anzahl frei vergebbarer Spieler
    int remaining = itemNA->teams.size() - max * groups;
    
    int *tmpCounts = (int *) calloc(groups, sizeof(int));

    // Setzung auslesen
    for (itTM = itemNA->teams.begin(); itTM != itemNA->teams.end(); itTM++)
    {
      itemTM = (DrawItemTeam *) (*itTM);
      if (itemTM->pos[0])
      {
        tmpCounts[itemTM->pos[0] - 1]++;
        countsNA[itemTM->tm.naID][itemTM->pos[0]-1]++;
      }
    }

    // Gruppen, die schon max haben, mit weiteren Spielern auffuellen, 
    // wenn sie groesser sind als der Rest: Wenn in eine Gruppe mehrere
    // Spieler einer Region kommen sollen, dann nach Moeglichkeit in eine
    // grosse (5-er) Gruppe und nicht in eine kleine (3-er) Gruppe.
    for (int i = 0; i < groups; i++)
    {
      if (tmpCounts[i] >= max && remaining && pgr[i].grSize > pgr[groups-1].grSize)
      {
        for (int step = 1; step <= 2; step++)
        {
          for (DrawList::iterator it = itemNA->teams.begin(); it != itemNA->teams.end(); it++)
          {
            itemTM = (DrawItemTeam *) (*it);

            if  (itemTM->pos[0])
              continue;

            if (step == 1 && itemTM->lastPos != 1)
              continue;

            if (itemTM->lastPos > 1 && quMap[itemTM->lastGroup] == i+1)
              continue;

            if ( countsNA[itemTM->tm.naID][i] > 0)
              continue;

            tmpCounts[i]++;
            counts[i]++;
            countsNA[itemTM->tm.naID][i]++;

            itemTM->pos[0] = i + 1;

            // Gruppe von Vorrundensieger vermerken
            if (itemTM->lastPos == 1)
              quMap[itemTM->lastGroup] = itemTM->pos[0];

            remaining--;

            break;
          }
        }
      }
    }
    
    // Jetzt verteilen
    for (int step = 1; step <= 2; step++)
    {
      for (itTM = itemNA->teams.begin(); itTM != itemNA->teams.end(); itTM++)
      {
        itemTM = (DrawItemTeam *) (*itTM);
      
        // Bereits vergeben
        if (itemTM->pos[0])
          continue;

        // Erstmal nur die Vorrundensieger
        if (step == 1 && itemTM->lastPos != 1)
          continue;
                
        // Solange suchen, bis ein Nation zugewiesen werden kann
        int i;   // Dient auch als Abbruchbedingung
        for (i = 0; i < groups; i++)
        {
          if (itemTM->lastPos > 1 && quMap[itemTM->lastGroup] == i+1)
            continue;

          if (countsNA[itemTM->tm.naID][i] > 0)
            continue;

          if (tmpCounts[i] < max)
          {
            tmpCounts[i]++;
            counts[i]++;
            // seeds[i]++;
            countsNA[itemTM->tm.naID][i]++;

            itemTM->pos[0] = i + 1;

            // Position Vorrundensieger merken
            if (itemTM->lastPos == 1)
              quMap[itemTM->lastGroup] = itemTM->pos[0];

            break;
          }
        }
      
        // Keinen Platz mehr gefunden, also Ende
        if (i == groups)
          break;

        // Wenn eine grosse Gruppe gefuellt wurde und es gibt noch frei vergebbare Spieler,
        // dann einen freien einer anderen Nation suchen (s.o.)
        if (tmpCounts[i] == max && remaining && pgr[i].grSize > pgr[groups-1].grSize)
        {
          for (DrawList::iterator it = itemNA->teams.begin(); it != itemNA->teams.end(); it++)
          {
            itemTM = (DrawItemTeam *) (*it);

            if  (itemTM->pos[0])
              continue;

            if (step == 1 && itemTM->lastPos != 1)
              continue;

            if ( countsNA[itemTM->tm.naID][i] > 0)
              continue;

            if (itemTM->lastPos > 1 && quMap[itemTM->lastGroup] == i+1)
              continue;

            tmpCounts[i]++;
            counts[i]++;
            countsNA[itemTM->tm.naID][i]++;

            itemTM->pos[0] = i + 1;

            // Position Vorrundensieger merken
            if (itemTM->lastPos == 1)
              quMap[itemTM->lastGroup] = i+1;

            remaining--;
          }
        }
      }
    }
    
    free(tmpCounts);
  }
  
  // Verteilen der Teams auf die Gruppen, Regionenweise.
  // Da jede Region weniger Spieler hat als Gruppen koennen nicht 2 Spieler
  // einer Nation in einer Gruppe landen. Ausser von der Verteilung davor.

  int * countsRG = (int *) calloc(groups, sizeof(int));

  // Und countsNA wieder leeren. Die Liste wird hier neu berechnet.
  countsNA.clear();

  for (int step = 1; step <= 2; step++)
  {
    for (itNA = listRG.begin(); itNA != listRG.end(); itNA++)
    {
      itemNA = (DrawItemNation *) (*itNA);
    
      // Nichts zu tun, wenn es keine Teilnehmer gibt.
      if (itemNA->Count() == 0)
        continue;

      # ifdef DEBUG
        wxFprintf(file, "%s\n---\n", wxStrlen(itemNA->na.naRegion) ? itemNA->na.naRegion : itemNA->na.naName);
      #endif

      // Array loeschen
      for (int i = 0; i < groups; i++)
      {
        countsRG[i] = 0;
        mix[i] = i+1;
      }
    
      // Jede Region wird neu verteilt
      Mix(mix, groups);

      // Setzungsinfo auslesen
      for (itTM = itemNA->teams.begin(); itTM != itemNA->teams.end(); itTM++)
      {
        itemTM = (DrawItemTeam *) (*itTM);

        if (itemTM->pos[0])
        {
          // seeds[itemTM->pos[0]-1]--;
          countsRG[itemTM->pos[0]-1]++;

          countsNA[itemTM->tm.naID][itemTM->pos[0]-1]++;

          // Musste noch ein Spieler in diese Gruppe?
          // Die Setzungsinfo wurde schon beruecksichtigt
          // if (needMore[itemTM->pos[0]-1])
          //    needMore[itemTM->pos[0]-1]--;

          #ifdef DEBUG
            int pos = itemTM->pos[0];
            wxFprintf(file, "  %3i (Seeded)  %s countsRG = %i  counts = %i  needMore = %i\n",
                     pos, pgr[pos-1].grName, countsRG[pos-1],  counts[pos-1], needMore[pos-1]);
          # endif
        }
      }

      // Den Rest verteilen
      for (itTM = itemNA->teams.begin(); itTM != itemNA->teams.end(); itTM++)
      {
        itemTM = (DrawItemTeam *) (*itTM);

        if (itemTM->pos[0])              // Gesetzter Spieler
          continue;

        if (step == 1 && itemTM->lastPos != 1)
          continue;

        // entlang von mix ein gleichzeitiges Minimum von counts, countsRG und countsNA suchen
        // Aber verboten ist die Gruppe vom Vorrundensieger
        int notAllowed = (itemTM->lastPos > 1 ? quMap[itemTM->lastGroup] : 0);
        int nr = NextNumber(mix, counts, countsRG, countsNA[itemTM->tm.naID], needMore, seeds, notAllowed);

        // Fehler aufgetreten?
        if (nr == 0)
        {
          free(mix);
          free(countsRG);
          return false;
        }

        counts[nr-1]++;
        countsRG[nr-1]++;
        countsNA[itemTM->tm.naID][nr-1]++;

        itemTM->pos[0] = nr;            // nr geht von 1 ... n

        if (itemTM->lastPos == 1)
          quMap[itemTM->lastGroup] = itemTM->pos[0];

        # ifdef DEBUG
          wxFprintf(file,  "  %3i           %s countsRG = %i  counts = %i  needMore = %i\n",
                    nr, pgr[nr-1].grName, countsRG[nr-1], counts[nr-1], needMore[nr-1]);
        # endif
      }

      // Jetzt werden die Gruppen getestet: Jede Gruppe, in der die Haelfte
      // der Spieler dieser Region angehoert, braucht mehr Spieler.
      // Der Zaehler wird entweder beim setzen oder in NextNumber runtergezaehlt
      for (int j = 0; j < groups; j++)
      {
        if ( (counts[j] > 1) &&  (countsRG[j] > counts[j] / 2) )
        {
          // Die Gruppe muss soweit gefuellt werden, dass ebensoviele andere
          // Spieler drin sind
          needMore[j] += countsRG[j] - (counts[j] - countsRG[j]);
        }

        // Aber nicht mehr, als noch Spieler reinpassen
        if (counts[j] + needMore[j] > pgr[j].grSize)
          needMore[j] = pgr[j].grSize - counts[j];
  # ifdef DEBUG
        if (needMore[j])
          wxFprintf(file, "Group %d needs %d more players\n", j+1, needMore[j]);
  # endif
      }

      #ifdef DEBUG
        wxFprintf(file, "\n\n");
      #endif
    }
  }
  free(mix);
  free(countsRG);
  free(needMore);

  #ifdef DEBUG
  wxFprintf(file, "-----------------------------------------------------------\n");
  #endif

  return true;
}


// Setzen der Teams auf die Gruppen
bool DrawRR::Distribute(int *counts)
{
  // Loeschen der Setzungen der Gruppen, soweit nicht gesetzt
  if (!ClearGroups())
    return false;

  DrawItemNation *itemNA;
  DrawItemTeam   *itemTM;
  
  // Damit man nicht so viel merkt...
  Shuffle();

#if 1  
  // Teams nach Gruppe sortieren
  DrawListTeam listGR;
  
  for (DrawListNation::iterator itNA = listNA.begin(); itNA != listNA.end(); itNA++)
  {
    itemNA = (DrawItemNation *) (*itNA);

    while (itemNA->teams.Count())    
    {
      itemTM = (DrawItemTeam *) (*itemNA->teams.begin());
      
      itemNA->teams.Subtract(itemTM);
      itemTM->key = itemTM->pos[0];
      listGR.Add(itemTM);
    }
  }
  
  listGR.Sort();
  
  MdStore  md(connPtr);
  
  // Jetzt durch diese Liste gehen und alle einer Gruppe in die naechste Liste tun
  while (listGR.Count())
  {
    DrawListTeam list;
    
    do
    {
      itemTM = (DrawItemTeam *) (*listGR.begin());
      listGR.Subtract(itemTM);
      itemTM->key = itemTM->tm.naID;
      list.Add(itemTM);
    } while ( listGR.begin() != listGR.end() && 
              ((DrawItemTeam *) (*listGR.begin()))->pos[0] == itemTM->pos[0] );
              
    if (list.Count() == 0)
      continue;
    
    // list nach Nation sortieren
    list.Sort();
    
    itemTM = (DrawItemTeam *) (*list.begin());
    const GrRec &gr = pgr[itemTM->pos[0]-1];

    // Belegte Plaetze markieren    
    int *count = (int *) calloc(gr.grSize, sizeof(int));
    // Vergabe-Reihenfolge
    int *seq = (int *) calloc(gr.grSize, sizeof(int));
      
    for (DrawListTeam::iterator itTM = list.begin(); itTM != list.end(); itTM++)
    {
      itemTM = (DrawItemTeam *) (*itTM);
      if (itemTM->pos[1])
        count[itemTM->pos[1]-1]++;
    }
    
    // Ausserdem sind die letzten Plätze belegt
    for (int i = gr.grSize; i-- > list.Count(); )
      count[i]++;
    
    if (md.mdID != gr.mdID)
    {
      md.SelectById(gr.mdID);
      md.Next();
      md.Close();
    }
    
    // Jetzt Paare in erster Runde spielen lassen
    for (DrawListTeam::iterator itTM = list.begin(); itTM != list.end(); )
    {
      itemTM = (DrawItemTeam *) (*itTM);
      itTM++;
      
      if (itTM == list.end())
        break;
      
      DrawItemTeam *itemTMNext = (DrawItemTeam *) (*itTM);
      
      if (itemTMNext->tm.naID != itemTM->tm.naID)
        continue;
        
      // Beide Spieler sind gesetzt
      if (itemTM->pos[1] && itemTMNext->pos[1])
        continue;
        
      // Spiele in der ersten oder zweiten, ... Runde      
      for(int idx = 0; idx < gr.NofRounds(); idx++)
      {        
        for (int mt = gr.NofMatches(idx+1); mt--; )
        {
          seq[md.GetPlayerA(idx+1, mt+1)-1] = md.GetPlayerX(idx+1, mt+1);
          seq[md.GetPlayerX(idx+1, mt+1)-1] = md.GetPlayerA(idx+1, mt+1);
        }
        
        if (itemTM->pos[1] && !count[seq[itemTM->pos[1]-1]-1])
        {
          // Erster Spieler ist gesetzt
          itemTMNext->pos[1] = seq[itemTM->pos[1]-1];
          count[itemTMNext->pos[1]-1]++;
        }
        else if (itemTMNext->pos[1] && !count[seq[itemTMNext->pos[1]-1]-1])
        {
          // Zweiter Spieler ist gesetzt
          itemTM->pos[1] = seq[itemTMNext->pos[1]-1];
          count[itemTM->pos[1]-1]++;
        }
        else if (!itemTM->pos[1] && !itemTMNext->pos[1])
        {
          for (int i = 0; i < gr.NofMatches(idx+1); i++)
          {
            // Positionen > list.Count() vermeiden (i ist 0-basiert)
            // Wenn das hier der Fall ist, kann man abbrechen
            if (i >= list.Count())
              break;
              
            // Das gleiche gilt fuer das andere Team, allerdings hier weitermachen
            if (seq[i] > list.Count())
              continue;
              
            if (!count[i] && !count[seq[i]-1])
            {
              itemTM->pos[1] = i+1;
              itemTMNext->pos[1] = seq[i];
              
              count[itemTM->pos[1]-1]++;
              count[itemTMNext->pos[1]-1]++;
              
              break;
            }
          }
        }
        
        // Freie Plaetze gefunden?
        if (itemTM->pos[1] && itemTMNext->pos[1])
          break;
      }      
    }
    
    // Jetzt die restlichen Plaetze vergeben, aber zunaechst die Vorrundensieger
    for (int step = 1; step <= 2; step++)
    {
      for (DrawListTeam::iterator itTM = list.begin(); itTM != list.end(); itTM++)
      {
        itemTM = (DrawItemTeam *) (*itTM);

        if (step == 1 && itemTM->lastPos != 1)
          continue;

        if (!itemTM->pos[1])
        {
          for (int i = 0; i < pgr[itemTM->pos[0]-1].grSize; i++)
          {
            if (count[i] == 0)
            {
              itemTM->pos[1] = i+1;
              count[i]++;
            
              break;
            }
          }
        }        
        
        pgr[itemTM->pos[0]-1].SetTeam(itemTM->pos[1], itemTM->tm, StStore::ST_KEEPSEEDED);
      }
    }
    
    free(seq);        
    free(count);
  }
  
#else
  for (DrawListNation::iterator itNA = listNA.begin(); itNA != listNA.end(); itNA++)
  {
    itemNA = (DrawItemNation *) (*itNA);

    for (DrawListTeam::iterator itTM = itemNA->teams.begin(); 
         itTM != itemNA->teams.end(); itTM++)
    {
      itemTM = (DrawItemTeam *) (*itTM);

      int nr = itemTM->pos[0];         // s. o.: Bereich 1 ... n

      // Ist Team bereits gesetzt?
      if (itemTM->pos[1])              // ja      
        continue;

      // Frei Position suchen
      StStore  st(connPtr);
      st.SelectAll(pgr[nr-1]);
      while (st.Next())
      {
        if (!st.tmID)
        {
          st.Close();
          pgr[nr-1].SetTeam(st.stNr, itemTM->tm);

          break;
        }
      }
    }
  }
  
#endif  

  return true;
}

bool DrawRR::DrawImpl()
{
  if (!ReadEntries())
  {
    return false;
  }

  if (!ReadGroups())
    return false;

  time_t ct = time(NULL);
  #ifdef DEBUG
    struct tm *tm = localtime(&ct);

    wxString tmpStage = toStage;
    tmpStage.Replace("/", "__", true);
  
    wxChar fname[256];
    wxSprintf(fname, "%s\\%s_%s_%04d%02d%02dT%02d%02d%02d.txt", 
        CTT32App::instance()->GetPath().data(), cp.cpName, tmpStage.t_str(),
        tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

    file = wxFopen(fname, "w");
    wxFprintf(file, "Seed: %ld\n", (long) ct);
  #endif

  // Vereinfachtes Verfahren: Teams der Nationen werden nacheinander auf
  // die Gruppen verteilt; es gibt eine Loesung, aber nicht alle Loesungen
  // sind (wahrscheinlich) gleichberechtigt; ist aber so das einfachste

  // Anzahl Teams in Gruppe;
  int *counts = (int *) calloc(groups, sizeof(int));

  GrStore(connPtr).ClearDraw(cp, toStage);

  // Setzung einlesen
  if (!ReadSeeding(counts))
  {
    free(counts);
    return false;                          // irgendwas != 0
  } 

  if (!ReadRanking())
  {
    free(counts);
    return false;
  }

  if (!DrawRanking(counts))
  {
    free(counts);
    return false;
  }

  Shuffle();                           // Mischen der Nationen und Teams

  // 'Auslosung'
  if (!DrawThem(counts))
  {
    free(counts);
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
    {
      free(counts);
      return false;
    }
  }

  warnings.clear();

  // In Gruppen eintragen
  Distribute(counts);

  free(counts);

  #ifdef DEBUG
  fclose(file);
  #endif

  return true;
}

bool DrawRR::Draw(const CpRec& cp_, const wxString &fromStage_, const wxString &toStage_, int fromPos_, int toPos_, RankingChoice rkChoice_, int seed_)
{
  cp = cp_;
  fromStage = fromStage_;
  toStage = toStage_;
  fromPos = fromPos_;
  toPos = toPos_;
  rkChoice = rkChoice_;

  // Einziges randomize in Auslosung
  srand(seed_ ? seed_ : time(NULL));

  // Um nicht immer auf connPtr zu testen
  Connection *tmpConn = connPtr ? connPtr : TTDbse::instance()->GetDefaultConnection();

  tmpConn->StartTransaction();

  bool ret = DrawImpl();

  if (ret)
    tmpConn->Commit();
  else
    tmpConn->Rollback();

  return ret;
}