/* Copyright (C) 2020 Christoph Theis */

#include "stdafx.h"
#include "TT32App.h"

#include  "DrawLP.h"

#include  "GrStore.h"
#include  "MtStore.h"
#include  "NaStore.h"
#include  "StStore.h"       // 'Setzung'
#include  "StListStore.h"
#include  "StEntryStore.h"
#include  "TbEntryStore.h"
#include  "RkEntryStore.h"

#include  "InfoSystem.h"

#include  <lp_lib.h>

#include  <wx/zipstrm.h>

#include  <stdlib.h>
#include  <time.h>

#include  <stdlib.h>
#include  <time.h>
#include  <assert.h>

#include  <algorithm>
#include  <set>

#define  DEBUG
#define  DEBUG_PACK

# ifdef DEBUG_PACK
#   define  DEBUG
# endif

#include <map>
#include <string>

// Mapping von ID auf Name von Gruppe / Nation
static std::map<long, wxString, std::less<long> > grList;
static std::map<long, wxString, std::less<long> > naList;

#ifdef DEBUG  
    static FILE *file;
# endif


DrawLP::DrawLP(bool champs_, bool koCons_, Connection *ptr)
{
  ziplp = NULL;
  ziplog = NULL;

  connPtr = ptr;
  
  champs = champs_;
  koCons = koCons_;

  totalDE = 0;
  totalFirst = 0;
  totalNA = 0;
  totalBY = 0;

  fromStage = "Qualification";
  toStage = (champs ? "Championship" : "Consolation");
  fromPos = 0;
  toPos   = 0;
}


DrawLP::~DrawLP()
{
  // Rollback, was noch nicht beendet ist
  if (connPtr)
  {  
    connPtr->Rollback();
    connPtr->Close();
  }
    
  delete connPtr;

  // Nicht loeschen! Spieler werden in anderer Liste verwaltet
  listIRK.clear();
  listNRK.clear();
}


// Gruppen auslesen und Liste aufbauen
bool DrawLP::ReadGroups()
{
  # ifdef  DEBUG_PACK
  // FILE *file = fopen("src\\debug", "a");
  wxFprintf(file, "           Read Groups               \n");
  wxFprintf(file, "-------------------------------------\n");
  # endif

  GrStore  gr(connPtr);
  GrStore *pgr = new GrStore[fromStage.IsEmpty() ? 0 : gr.CountGroups(cp, fromStage)];
  int idx = 0;

  if (!fromStage.IsEmpty())
  {
    gr.SelectAll(cp, fromStage);

    while (gr.Next())
    {
      pgr[idx++] = gr;
    
      grList.insert(std::map<long, wxString, std::less<long> >::value_type(gr.grID, wxString(gr.grName)));  
    }
  }
  
  grList.insert(std::map<long, std::string, std::less<long> >::value_type(0, std::string("BYE")));  

  int count = idx;
  for (idx = 0; idx < count; idx++)
  {
    // Gruppe beendet? Oder zumindest eine gueltige Tabelle?
    if (!pgr[idx].QryFinished() /* && !gr.QryTable() */ )
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

    for (auto &it : tbList)
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

      if (it.st.tmID == 0)
        continue;

      if (fromPos || toPos)
      {
        if (fromPos && it.st.stPos < fromPos || toPos && it.st.stPos > toPos)
          continue;
      }
      else if (champs && it.st.stPos > (pgr[idx].grSize + 1) / 2 ||
               !champs && it.st.stPos <= (pgr[idx].grSize + 1) / 2)
        continue;

      if (it.st.stGaveup || it.st.stDisqu)
        continue;

      if (!champs && it.st.stNocons)
        continue;

      if (champs && it.st.stNocons)
      {
        infoSystem.Information(_("Entry on pos. %d in group %s is qualified but has No Consolation flag set"), it.st.stPos, pgr[idx].grName);
        continue;
      }

      DrawItemTeam *itemTM = listNA.AddTeam(it);

      // lastGroup und lastPos eintragen
      if (itemTM)
      {
        itemTM->lastGroup = it.st.grID;
        if (fromPos)
          itemTM->lastPos = it.st.stPos - fromPos + 1;
        else
          itemTM->lastPos = champs ? it.st.stPos : it.st.stPos - (pgr[idx].grSize + 1) / 2;
        itemTM->pos[1] = 1;               // Init der Positionen
        
        if (itemTM->lastPos == 1)
          totalFirst++;
      }
    }
  }   // for (gr)

  # ifdef DEBUG_PACK
  // fclose(file);
  # endif

  delete[] pgr;

  return true;
}


// Verlierer der ersten Runden der vorigen Stufe auslesen
bool DrawLP::ReadMatches()
{
  GrStore gr(connPtr);
  gr.SelectAll(cp, fromStage);
  gr.Next();
  gr.Close();

  if (!gr.grID)
  {
    infoSystem.Error(_("No groups found for stage %s!"), fromStage.wx_str());
    return false;
  }

  if (!gr.QryStarted())
  {
    infoSystem.Error(_("Group %s has not yet started!"), gr.grDesc);
    return false;
  }

  std::map<long, StEntry> stMap;

  StEntryStore st(connPtr);
  st.SelectByGr(gr, cp.cpType);

  while (st.Next())
  {
    // Freilose uebergehen
    if (st.tmID)
      stMap[st.st.stID] = st;
  }

  // Read all losers
  MtStore mt(connPtr);
  mt.SelectByGr(gr);
  while (mt.Next())
  {
    if (mt.mtEvent.mtRound < fromPos)
      continue;

    if (mt.mtEvent.mtRound > toPos)
      continue;

    if (mt.IsAByeOrXBye())
      continue;

    if (!mt.QryWinnerAX())
    {
      infoSystem.Error(_("Not all required matches of group %s have been played"), gr.grDesc);
      return false;
    }

    DrawItemTeam *itemTM = listNA.AddTeam(stMap[mt.QryWinnerAX() == 1 ? mt.stX : mt.stA]);

    // Fake groups and pos
    int lastGroup = 1 + ((mt.mtEvent.mtMatch - 1) / (1 << (toPos - mt.mtEvent.mtRound)));
    int lastPos = 1 + (toPos - mt.mtEvent.mtRound);

    itemTM->lastGroup = lastGroup;
    itemTM->lastPos = lastPos;

    itemTM->pos[1] = 1;

    if (lastPos == 1)
      totalFirst++;
  }

  // Dummy Gruppen
  for (int i = 1; i <= gr.NofMatches(toPos); i++)
    grList.insert(std::map<long, wxString, std::less<long> >::value_type(i, wxString::Format("MT-%d", i)));

  grList.insert(std::map<long, std::string, std::less<long> >::value_type(0, std::string("BYE")));

  return true;
}


// Direct entries
bool DrawLP::ReadDirectEntries()
{
  // Int'l ranking used, players not finishing first will be replaced with winners
  RkEntryStore rk(connPtr);

  rk.SelectByCp(cp);
  while (rk.Next())
  {
    // Only direct entries
    if (!rk.rk.rkDirectEntry)
      continue;

    DrawItemTeam *itemTM = listNA.AddTeam(rk);
    itemTM->pos[1] = 1;
    // And treat them as group winner
    itemTM->lastPos = 1;
  }

  return true;
}

bool DrawLP::ReadRegions()
{
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

  for (DrawListNation::iterator it = listNA.begin(); it != listNA.end(); it++)
    ((DrawItemNation *)(*it))->na = naMap[((DrawItemNation *)(*it))->na.naID];

  return true;
}


// Einlesen vom Ranking
bool DrawLP::ReadRanking()
{
  // Do we use a ranking at all?
  if (rkChoice == None)
    return true;

  // Map mit dem Abschneiden der vorigen Runde
  std::map<long, int> fromGroupMap;
  std::map<long, int> fromPosMap;

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

  // Test, ob es eine Setzung von gibt
  {
    StStore st(connPtr);
    st.SelectSeeded(cp, toStage);
    while (st.Next())
    {
      // If group ranking no QU may be seeded
      if (rkChoice == Groups && fromGroupMap.find(st.tmID) != fromGroupMap.end())
      {
        infoSystem.Information(_("Ranked players ignored because players have already been seeded"));

        return true;
      }
    }

    st.Close();
  }

  // Set mit den IDs der Teams, um kein Team mehrmals zu haben
  std::set<long> tmSet;

  // Temp. Ranking
  DrawListTeam listTMP;

  if (rkChoice == World)
  {
    // Int'l ranking used, players not finishing first will be replaced with winners
    RkEntryStore rk(connPtr);

    rk.SelectByCp(cp);
    while (rk.Next())
    {
      // If we use WR then we don't care if the ranked player was a DE or QU:
      // If we don't have DE and do not use Groups, then take WR.
      // Or we have DE and the top seeds are DE and we don't care about QU
      //
      // But: we ignore entries without rank
      if (rk.rk.rkIntlRank == 0)
        continue;

      DrawItemTeam *itemTMP = new DrawItemTeam(rk);

      // Only those we have
      if (listNA.GetTeam(rk))
        listTMP.AddItem(itemTMP);
    }
  }
  else if (rkChoice == Groups)
  {
    // Result of groups used, winner of group 1 is #1, of group 2 is #2, ...
    // Plus offset of DE
    // If we start from pos 1 include DE
    if (fromPos == 1)
    {
      int rank = 0;

      // DE lesen
      RkEntryStore rk(connPtr);
      rk.SelectByCp(cp);
      while (rk.Next())
      {
        DrawItemTeam *itemTMP = new DrawItemTeam(rk);
        listTMP.AddItem(itemTMP);        
      }

      rank = listTMP.Count();

      StEntryStore st(connPtr);
      st.SelectAll(cp, fromStage);

      // Keep 2nd in seperate list
      DrawListTeam listSecond;

      while (st.Next())
      {
        if (st.st.stPos < fromPos || st.st.stPos > toPos)
          continue;

        DrawItemTeam *itemTM = new DrawItemTeam(st);

        if (st.st.stPos == fromPos)
        {
          itemTM->rkIntlRank = ++rank;
          listTMP.AddItem(itemTM);
        }
        else
        {
          listSecond.AddItem(itemTM);
        }
      }

      if (listSecond.Count())
      {
        DrawItemTeam *itemTM;

        while ((itemTM = (DrawItemTeam *) listSecond.CutLast()))
        {
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

      // rkDirectEntry nach vorn sortieren
      if (ta->rkDirectEntry != tb->rkDirectEntry)
        return ta->rkDirectEntry ? true : false;

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
    DrawItemTeam *itTMP = (DrawItemTeam *) it;

    // Bereits in Liste
    if (tmSet.find(itTMP->tm.tmID) != tmSet.end())
      continue;

    DrawItemTeam *itemTM = listNA.GetTeam(itTMP->tm.tmID);

    if (rkChoice == World)
    {
      // Wenn Spieler nicht gewonnen hat stattdessen den Sieger nehmen
      if (!itemTM || itemTM->lastPos > 1)
      {
        // Erster gehoert nicht dazu
        if (!itemTM)
        {
          if ((champs || fromPos == 1))
            warnings.push_back(wxString::Format(_("The ranked entry of group %s finished %d, trying winner instead"), grList[fromGroupMap[itTMP->tm.tmID]], fromPosMap[itTMP->tm.tmID]));

          itemTM = listNA.GetTeam(fromGroupMap[itTMP->tm.tmID], 1);
        }
      }
    }

    if (!itemTM)
      continue;

    // Don't take twice: da wir oben ausgetauscht haben könnten wir ein Team mehrmals erwischen
    if (tmSet.find(itemTM->tm.tmID) != tmSet.end())
      continue;

    tmSet.insert(itemTM->tm.tmID);

    // Ranking und Punkte uebernehmen
    itemTM->rkIntlRank = itTMP->rkIntlRank;
    itemTM->rankPts = itTMP->rankPts;

    listIRK.Add(itemTM);  // Liste aller Spieler
    listNRK.Add(itemTM);  // Augedroeselt nach Nation
  }

  // Int'l Rank neu sortieren, aber gleiches Ranking zufaellig
  listIRK.Shuffle();

  std::stable_sort(listIRK.begin(), listIRK.end(), [](const DrawItem *a, const DrawItem *b) 
    {
      const DrawItemTeam *ta = (const DrawItemTeam *)a;
      const DrawItemTeam *tb = (const DrawItemTeam *)b;

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
    ((DrawItemTeam *) it)->rkIntlRank = ++rank;

  // Nat'l Rank sortieren: Nat'l richtet sich nach Int'l, also nach Int'l sortieren und dann neues Nat'l setzen
  for (auto it : listNRK)
  {
    DrawItemNation *itemNA = (DrawItemNation *) it;
    itemNA->teams.sort([](const DrawItem *a, const DrawItem *b)
      {
        return ((const DrawItemTeam *)a)->rkIntlRank < ((const DrawItemTeam *)b)->rkIntlRank;
      }
    );

    int rank = 0;
    for (auto it : itemNA->teams)
      ((DrawItemTeam *) it)->rkNatlRank = ++rank;
  }

  return true;
}

// Auslesen der Setzung und Loeschen der Gruppe
bool DrawLP::ReadSeeding(GrStore &gr)
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
bool DrawLP::Distribute(GrStore &gr)
{
# ifdef DEBUG
  wxFprintf(file, "\n\n\n            Distribute Teams              \n");
  wxFprintf(file, "------------------------------------------\n");
# endif

  if (toGrID == 0)
    gr.ClearDraw(cp, toStage);
  else
    gr.ClearDraw(toGrID);
  
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
    // Mark direct entries
    bool de = itemTM->rkDirectEntry && itemTM->lastGroup == 0;

    wxFprintf(file, "%04i: %s:%i\n", 
             itemTM->pos[lastStage+1],
             de ? "DE" : grList[itemTM->lastGroup].data(),
             itemTM->lastPos);
# endif
  }

  return true;
}


// Mischen der Listen
bool DrawLP::Shuffle()
{
  listNA.Shuffle();

  for (DrawListNation::iterator itNA = listNA.begin(); itNA != listNA.end(); itNA++)
    ((DrawItemNation *) (*itNA))->teams.Shuffle();

  listBY.Shuffle();

  return true;
}


int __WINAPI writeModelHandler(void *userData, char *buf)
{
  if (buf != NULL && strlen(buf) > 0)
    ((wxZipOutputStream *) userData)->WriteAll(buf, strlen(buf));

  return 0;
}


void __WINAPI writeLogHandler(lprec *lp, void *userData, char *buf)
{
  // Change \n to \r\n
  if (buf != NULL && strlen(buf) > 0)
  {
    wxString tmp(buf);
    tmp.Replace("\n", "\r\n", true);

    const char * str = tmp.ToStdString().c_str();

    ((wxZipOutputStream *) userData)->WriteAll(str, tmp.Length());
  }
}


bool DrawLP::DrawSection(int stg, int sec)
{
  DrawList listSC;

  /*
    lpsolve berechnet fuer jedes Team eine Wert (xi) von 0 oder 1. 
    0 bedeutet obere Haelfte, 1 bedeutet untere Haelfte

    Konventionen
      SUMME(...) Summe der Variablen, also Anzahl nach unten
      COUNT(...) Anzahl, z.B. Anzahl Teams GER

    Randbedingungen
      1) Alle Spieler gleichmaessig oben und unten verteilen
      2) Gesetzte in ihre Haelfte. Dazu gehoren auch Freilose
      3) Gruppensieger und -zweite trennen
      4) Gruppensieger gleichmaessig verteilen. 
         Damit koennen sie in der ersten Runde nicht gegeneinander kommen
      5) Gruppensieger mit Freilosen syncen: 
         Ein einzelner Gruppensieger muss gegen ein einzelnes Freilos kommen, 
         d.h. bis zur letzten Stufe in die gleiche Haelfte, dann in die andere Haelfte

    Optimierungsziel
      1) Nationen gleichmaessig verteilen (Gewichtung 1000)
         Gleichmaessig heisst: 
           Die Anzahl, die nach oben kommt, minus die Haelfte der Anzahl, moeglichst 0 werden lassen
           Oder: abs(SUMME(Nationen) - COUNT(Nationen) / 2) minimieren
      2) Regionen (wie Nationen) gleichmaessig verteilen (Gewichtung 100) 
      3) Einen Zufaellig gewaehlten Spieler in die untere Haelfte bringen (Gewichtung 1)
  */

  // Mapping, welches Team in welcher Spalte der Matrix steht
  // Spalten beginnen bei 1
  std::map<DrawItemTeam *, int> colMapping;

  // Als erstes die Freilose in Liste eintragen ...
  for (DrawListTeam::iterator itBY = listBY.begin(); itBY !=listBY.end(); itBY++)
  {
    DrawItemTeam *itemTM = (DrawItemTeam *) (*itBY);

    if (itemTM->pos[stg] == sec)
    {
      colMapping[itemTM] = colMapping.size() + 2;
      listSC.push_back(itemTM);
    }
  }

  // Byes verteilen
  int dbc = listSC.size(), dbu = 0, dbl = 0;

  CalculateByes(stg, sec, lastStage, dbc, dbu, dbl);
  
  for (DrawList::iterator it = listSC.begin(); it != listSC.end(); ++it)
  {
    if (dbu > 0)
    {
      --dbu;
      ((DrawItemTeam *) (*it))->pos[stg+1] = 2 * sec - 1;
    }
    else if (dbl > 0)
    {
      --dbl;
      ((DrawItemTeam *) (*it))->pos[stg+1] = 2 * sec;
    }
    else
      break;
  }

  // Nationen und Regionn in dieser section zaehlen und Teams in listSC merken
  int countNA = 0;
  int countRG = 0;
  std::set<wxString> rgSet;

  for (DrawListNation::iterator itNA = listNA.begin(); itNA != listNA.end(); itNA++)
  {
    DrawItemNation *itemNA = (DrawItemNation *) (*itNA);

    bool found = false;

    for (DrawListTeam::iterator itTM = itemNA->teams.begin(); itTM != itemNA->teams.end(); itTM++)
    {
      DrawItemTeam *itemTM = (DrawItemTeam *) (*itTM);

      if (itemTM->pos[stg] == sec)
      {
        if (!found)
        {
          countNA++;

          if (idMapping.find(itemNA->na.naID) != idMapping.end())
          {            
            wxString region = itemNA->na.naRegion;

            if (rgSet.find(region) == rgSet.end())
            {
              rgSet.insert(region);
              countRG++;
            }
          }

          found = true;
        }

        // Spalten beginnen bei 1, die erste Spalte ist die Konstante, 
        // die naechste Spalte ist also bisherige Anzahl + 2
        colMapping[itemTM] = colMapping.size() + 2;
        listSC.push_back(itemTM);
      }
    }
  }

  int countSC = listSC.Count();                // Anzahl Teams mit Freilosen
  int colNA = countSC + 2;                     // Erste Spaltennummer (1-based) fuer Nationen: Anzahl Teams + 1 (Konstante) + 1 (lpsolve ist 1-based)
  int cols = 1 + countSC + countNA + countRG;  // Max. Anzahl Spalten in der Matrix
  int rows = 0;                                // Anzahl Zeilen der Matrix, die in der Folge aufgebaut wird. Wir beginnen bei 0
  int naCountAbs = 0;                          // Anzahl Spalten fuer abs()-Berechnung fuer Nationen

  // Erste Spalte ist ein Offset fuer die Gewichtung, immer 1
  // Dann kommen die Teams
  // Dann die Wert fuer abs(Nation)
  // 0  1      2           countSC+3    countSC+4
  // 1, Q01:1, Q03:2, ..., abs(AUT), abs(GER), ...
  int *colvec = new int[cols];    
  REAL *rowvec = new REAL[cols];  

  // Matrix initialisieren. Zu Beginn gibt es noch keine Zeilen
  lprec *lp = make_lp(rows, cols);    

  // Spalten fuer Konstante und Teams sind binary
  for (int i = 0; i < countSC + 1; i++)
    set_binary(lp, i + 1, TRUE);  // 1-based

  // Spalten fuer Nationen und Regionen sind int
  for (int i = 0; i < countNA + countRG; i++)
    set_int(lp, colNA + i, TRUE); // 1-based

  // Set col names
  // set_col_name(lp, "Q01:02");
  // set_col_name(lp, 1, "Const");

  // Spaltennamen setzen
  // Konstante: "C"
  // Teams: "<grName>#<stPos>#<colNr>" bzw. "BYE#2#<colNr>" fuer Freilose
  // Nationen: "abs[<naName>]"
  // Regionen: "abs[<naRegion>]"

  set_col_name(lp, 1, wxString("C").char_str());

  for (std::map<DrawItemTeam *, int>::iterator it = colMapping.begin(); it != colMapping.end(); it++)
  {
    // Direct entries did not play groups
    bool de = it->first->lastGroup == 0 && it->first->rkDirectEntry;

    wxString tmp = wxString::Format("%s#%02d#%02d#%02d", de ? "DE" : grList[it->first->lastGroup].data(), it->first->lastPos, it->second, it->first->rkIntlRank);
    // Name darf kein " ", "-" enthalten
    tmp.Replace(" ", "_", true);
    tmp.Replace("-", "_", true);
    set_col_name(lp, it->second, tmp.char_str());
  }

  // Add constraints
  set_add_rowmode(lp, TRUE);

  // Im folgenden ist
  //   colvec: Ein 0-based Array mit den Spaltennummern, die betroffen sind
  //   rowvec: Ein 0-based Array mit den Faktoren fuer die Spalten von colvec
  // Z.B. colvec = {1, 4}, rowvec {2, 8}, EQ 1 ergibt: 2 * x1 + 0*x2 + 0*x3 + 8 * x4 = 1

  if (true)
  {
    // Konstante "C", immer 1
    memset(colvec, 0, cols * sizeof(int));
    memset(rowvec, 0, cols * sizeof(REAL));

    colvec[0] = 1;
    rowvec[0] = 1;

    add_constraintex(lp, 1, rowvec, colvec, EQ, 1);
  }

  if (true)
  {
    // Haelfte der Mannschaften + Freilose ist unten (und die andere Haelfte damit oben)
    // Das heisst, die Summe aller Resultate (gleich Anzahl unten) ist countSC / 2
    memset(colvec, 0, cols* sizeof(int));
    memset(rowvec, 0, cols * sizeof(REAL));

    for (int j = 0; j < countSC; j++)
    {
      colvec[j] = j+2;  // Teams beginnen ab Spalte 2
      rowvec[j] = 1;
    }

    add_constraintex(lp, countSC, rowvec, colvec, EQ, countSC / 2);
  }

  if (true)
  {
    // Trennen von Gruppensieger, -zweite
    // Das heisst, SUMME(Gruppe) = COUNT(Gruppe) / 2

    // Erstemal alle Teams nach Gruppen gruppieren
    // TODO: Kombinieren 1+2, 3+4, ..., dann 1-4, 5-8, ..., dann 1-8, 9-16, ..., etc.
    std::map<int, std::list<DrawItemTeam *>> mapGR;
    for (DrawListTeam::iterator it = listSC.begin(); it != listSC.end(); it++)
    {
      DrawItemTeam *itemTM = (DrawItemTeam *) (*it);

      // Freilose und direct entries werden extra behandelt
      if (itemTM->IsBye() || itemTM->rkDirectEntry)
        continue;

      mapGR[itemTM->lastGroup].push_back(itemTM);
    }

    for (std::map<int, std::list<DrawItemTeam *>>::iterator it = mapGR.begin(); it != mapGR.end(); it++)
    {
      std::list<DrawItemTeam *> &listGR = it->second;

      // Nichts zu tun, wenn es weniger als 2 Teams sind
      if (listGR.size() < 2)
        continue;

      memset(colvec, 0, cols * sizeof(int));
      memset(rowvec, 0, cols * sizeof(REAL));

      int j = 0;
      for (std::list<DrawItemTeam *>::iterator itTM = listGR.begin(); itTM != listGR.end(); itTM++)
      {
        DrawItemTeam *itemTM = (DrawItemTeam *) (*itTM);
        colvec[j] = colMapping[itemTM];
        rowvec[j] = 1;

        j++;
      }

      if ( (listGR.size() % 2) )
      {
        // Ungerade: (COUNT - 1) / 2 <= SUMME <= (COUNT + 1) / 2
        add_constraintex(lp, j, rowvec, colvec, LE, (int) (listGR.size() + 1) / 2);
        add_constraintex(lp, j, rowvec, colvec, GE, (int) (listGR.size() - 1) / 2);
      }
      else
      {
        // Gerade: SUMME == COUNT/2
        add_constraintex(lp, j, rowvec, colvec, EQ, listGR.size() / 2);
      }
    }
  }

  // Gesetzte auf ihre Plaetze
  if (true) 
  {
    int j = 0;
    for (DrawListTeam::iterator it = listSC.begin(); it != listSC.end(); it++)
    {
      DrawItemTeam *itemTM = (DrawItemTeam *) (*it);
      if (itemTM->pos[stg+1] == 0)
        continue;

      memset(colvec, 0, cols * sizeof(int));
      memset(rowvec, 0, cols * sizeof(REAL));

      colvec[0] = colMapping[itemTM];
      rowvec[0] = 1;

      // 1 * xi == (oben ? 0 : 1)
      add_constraintex(lp, 1, rowvec, colvec, EQ, (itemTM->pos[stg+1] % 2) ? 0 : 1);
    }
  }

  if (true) 
  {
    // Sieger gleichmaessig verteilen
    memset(colvec, 0, cols * sizeof(int));
    memset(rowvec, 0, cols * sizeof(REAL));

    DrawItemTeam *itemFirst = NULL;

    int j = 0;
    for (DrawListTeam::iterator it = listSC.begin(); it != listSC.end(); it++)
    {
      DrawItemTeam *itemTM = (DrawItemTeam *) (*it);

      // Keine Freilose
      if (itemTM->IsBye())
        continue;

      // Nur Sieger
      if (itemTM->lastPos != 1)
        continue;

      // Wenn es nur einen gibt brauch ich weiter unten.
      // Wenn es mehr als einen gibt spielt es auch keine Rolle
      itemFirst = itemTM;

      colvec[j] = colMapping[itemTM];
      rowvec[j] = 1;

      j++;
    }

    if (j > 1)
    {
      // Mehr als einen Sieger gleichmaessig verteilen
      if (j % 2)
      {
        // Ungerade: (COUNT - 1) / 2 <= SUMME <= (COUNT + 1) / 2
        add_constraintex(lp, j, rowvec, colvec, LE, (int) (j + 1) / 2);
        add_constraintex(lp, j, rowvec, colvec, GE, (int) (j - 1) / 2);
      }
      else
      {
        // Gerade: COUNT / 2 == SUMME
        add_constraintex(lp, j, rowvec, colvec, EQ, (int) j / 2);
      }
    }
    else if (itemFirst && itemFirst->pos[stg+1] == 0)
    {
      // Durch das if steht fest, dass es genau einen gibt und dieser ist nicht gesetzt
      // Solche einzelnen Sieger wandern auf eine bestimmte Position 
      // Wenn es mehr als ein Freilos gibt gibt es auch ein Freilos fuer den Ersten
      // Wenn es nur ein Freilos gibt, dann wandert dieses
      //   - Bis zur letzten Stufe
      //     - Wenn ungerade: nach oben
      //     - Wenn gerade: nach unten
      //   - In der letzten Stufe dann umgekehrt
      //     - Wenn ungerade: nach unten
      //     . Wenn gerade: nach oben
      // Erste wandern bis zur letzten Stufe mit, dann in die andere Haelfte
      //   - Wenn ungerade: nach oben
      //   - Wenn gerade: nach unten
      if (sec % 2)
        add_constraintex(lp, j, rowvec, colvec, EQ, 0);
      else
        add_constraintex(lp, j, rowvec, colvec, EQ, 1);
    }
  }

  // Freilose gegen die (top) Spieler
  if (true)
  {
    int j = 0;  // count
    int count = 0;
    int byes = 0;
    int de = 0;
    int total = listNA.Count(stg, sec);

    for (DrawListTeam::iterator it = listSC.begin(); it != listSC.end(); it++)
    {
      if (((DrawItemTeam *) (*it))->rkDirectEntry)
        ++de;
    }

    // If we got byes or DE and we are not in the last stage, top seed entries follow byes
    if ((dbc > 0 || de > 0) && stg < lastStage)
    {
      // Byes go against top ranked players, so we must have same number of players as byes in each half.
      // The formula we use is #1 + #2 + ... #dbc - BYE#1 - BYE#2 - ... - BYE#dbc = 0
      memset(colvec, 0, cols * sizeof(int));
      memset(rowvec, 0, cols * sizeof(REAL));

      for (DrawListTeam::iterator it = listSC.begin(); it != listSC.end(); it++)
      {
        DrawItemTeam *itemTM = (DrawItemTeam *) (*it);

        // Byes go against top ranked players
        // rkIntlRank is the seeding order we use
        // All DE must go into the list, they'll get the least ranked 2nd if no byes are left
        // To find the highest ranked QU we look at all QU,not only those who won their group.
        // Which means even 2nd could get a bye in first round
        //
        //  Don't care
        if (rkChoice == None)
          continue;

        // Not a bye, but no or too low intl rank
        if (!itemTM->IsBye() && (!itemTM->rkIntlRank || itemTM->rkIntlRank > listBY.Count()))
          continue;

        colvec[j] = colMapping[itemTM];
        if (itemTM->IsBye())
        {
          rowvec[j] = -1;
          byes++;
        }
        else
        {
          rowvec[j] = 1;
          count++;
        }

        j++;
      }

      while (byes < de)
      {
        // Add the lowest ranked 2nd to play against DE, i.e. as if they were byes
        for (DrawListTeam::iterator it = listSC.begin(); it != listSC.end(); it++)
        {
          DrawItemTeam *itemTM = (DrawItemTeam *) (*it);

          if (itemTM->IsBye())
            continue;
            
          // total is the number of real entries. They are ranked 1..n, so total is also
          // the highest rank position
          if (itemTM->rkIntlRank && itemTM->rkIntlRank <= total - listBY.Count())
            continue;

          colvec[j] = colMapping[itemTM];
          rowvec[j] = -1;

          byes++;
          j++;
        }
      }

      if (count == byes)
      {
        add_constraintex(lp, j, rowvec, colvec, EQ, 0);
      }
    }
  }

  // Int'l Ranking verteilen, wenn nicht gesetzt
  if (true)
  {    
    // Only with ranking. "Groups" requires snake system in the previous stage and no DE
    if (rkChoice == World || rkChoice == Groups && totalDE == 0)
    {
      // pos points to the end of the faction, i.e. 1, 2, 4, 8, 16, ...
      // But only as far as half of max with IRK.
      // XXX: Only max totalFirst?
      for (int pos = 1; pos / 2 < listIRK.Count(); pos *= 2)
      {
        memset(colvec, 0, cols * sizeof(int));
        memset(rowvec, 0, cols * sizeof(REAL));

        int j = 0;

        for (auto it = listIRK.begin(); it != listIRK.end(); it++)
        {
          DrawItemTeam *itemTM = (DrawItemTeam *) (*it);

          if (itemTM->pos[stg] != sec)
            continue;

          // List ist nach int'l rank sortiert
          if (itemTM->rkIntlRank <= pos / 2)
            continue;

          if (itemTM->rkIntlRank > pos)
            break;

          colvec[j] = colMapping[itemTM];
          rowvec[j] = 1;

          j++;
        }

        if (j == 0)
          continue;

        if ((1 << stg) == pos)
        {
          // Wenn es nur einen mit Ranking geben kann, dann wandert dieser beim ersten Mal
          //     - Wenn ungerade: nach unten
          //     - Wenn gerade: nach oben
          if (sec % 2)
            add_constraintex(lp, j, rowvec, colvec, EQ, 1);
          else
            add_constraintex(lp, j, rowvec, colvec, EQ, 0);

        }
        else if ((1 << stg) > pos / 2)
        {
          // Wenn es nur einen mit Ranking geben kann, dann wandert dieser die weiteren Male
          //     - Wenn gerade: nach unten
          //     - Wenn ungerade: nach oben
          if (sec % 2)
            add_constraintex(lp, j, rowvec, colvec, EQ, 0);
          else
            add_constraintex(lp, j, rowvec, colvec, EQ, 1);
        }
        else 
        {
          // Max Haelfte moegliche Anzahl in Stufe, min was gefunden wurde als Haelfte max moeglich
          // Moegliche Anzahl in Stufe ist pos / 2
          // Max: pos / (2 << stg); Min: max(0, j - pos / (2 << stg))
          add_constraintex(lp, j, rowvec, colvec, LE, (int)(pos / (2 << stg)));
          add_constraintex(lp, j, rowvec, colvec, GE, std::max(0, (int)(j - pos / (2 << stg))));
        }
      }
    }
  }

  // Nat'l Ranking verteilen
  // Kopie von oben, aber bezogen auf nat'l rank und ohne fixe Positionen
  if (true)
  {
    // Only with ranking. "Groups" requires snake system in the previous stage and no DE
    if (rkChoice == World || rkChoice == Groups && totalDE == 0)
    {
      for (auto it : listNRK)
      {
        DrawListTeam &list = ((DrawItemNation *) it)->teams;

        // pos points to the end of the faction, i.e. 2, 4, 8, 16, ...
        // No fix place for 1st, only 1st and 2nd in different halfes, but that is done by separating nations.
        // So we start with 2 and end at half of entries, remaining ones are left to optimize function
        for (int pos = 2; pos < list.Count(); pos *= 2)
        {
          memset(colvec, 0, cols * sizeof(int));
          memset(rowvec, 0, cols * sizeof(REAL));

          int j = 0;
          for (auto it : list)
          {
            DrawItemTeam *itemTM = (DrawItemTeam *) it;
            if (itemTM->pos[stg] != sec)
              continue;

            // List ist nach nat'l rank sortiert
            if (itemTM->rkNatlRank > pos)
              break;

            colvec[j] = colMapping[itemTM];
            rowvec[j] = 1;

            j++;
          }

          if (j == 0)
            continue;

          // Wenn es mehr als einen gibt, gleichmaessig verteilen
          // Ansonsten ist es egal, wohin er rutscht
          if (j > 1)
          {
            if (j % 2)
            {
              // Ungerade: (COUNT - 1) / 2 <= SUMME <= (COUNT + 1) / 2
              add_constraintex(lp, j, rowvec, colvec, LE, (int)(j + 1) / 2);
              add_constraintex(lp, j, rowvec, colvec, GE, (int)(j - 1) / 2);
            }
            else
            {
              // Gerade: COUNT / 2 == SUMME
              add_constraintex(lp, j, rowvec, colvec, EQ, (int)j / 2);
            }
          }
        }
      }
    }
  }

  // Verbaende gleichmaessig verteilen
  // Das heisst, Ziel ist es abs(SUMME(GER) - COUNT(GER) / 2) + ... zu minimieren
  // abs(...) wird ueber zwei Regeln definiert: 
  //  1) SUMME - COUNT/2 - abs <= 0
  //  2) SUMME - COUNT/2 + abs >= 0
  // S.a. http://lpsolve.sourceforge.net/5.5/absolute.htm

  if (true)
  {
    for (DrawListNation::iterator itNA = listNA.begin(); itNA != listNA.end(); itNA++)
    {
      DrawItemNation *itemNA = (DrawItemNation *) (*itNA);

      memset(colvec, 0, cols * sizeof(int));
      memset(rowvec, 0, cols * sizeof(REAL));

      int j = 0;      

      // DrawListTeam, damit ich eine zufaellige Reihenfolge habe
      DrawListTeam naList;

      // GER1 + GER2 + ... 
      for (DrawListTeam::iterator itTM = itemNA->teams.begin(); itTM != itemNA->teams.end(); itTM++)
      {
        DrawItemTeam *itemTM = (DrawItemTeam *) (*itTM);

        if (colMapping.find(itemTM) == colMapping.end())
          continue;

        naList.AddItem(itemTM);
      }

      // Nur wenn es auch mehr als ein Team dieser Nation gibt
      if (naList.Count() < 2)
      {
        // Alle ausschneiden
        while (naList.CutFirst())
          ;

        continue;
      }

      naList.Shuffle();
      bool oddFound = (naList.Count() % 2) == 0;

      while (DrawItemTeam *itemTM = (DrawItemTeam *) naList.CutFirst())
      {
        // Wenn es eine ungerade Anzahl ist, einen frei auslosbaren Spieler nicht beruecksichtigen.
        // Ansonsten versucht die Optimierung, einen Spieler mehr nach oben als unten zu bringen, da COUNT(oben) immer 0 ist
        // 
        // TODO: 
        // Wenn es oben oder unten zu wenig freie Plaetze gibt, diesen Spieler in die andere Haelfte bringen
        // Z.B., 15 Teams, 16-er Feld, oben ein anderer gesetzter und Freilos. 
        // Dann gibt es oben nur 7 verfuegbare Spiele, unten 8. 
        // Es ist also besser, oben 7 und unten 8 zu haben. 
        // Umgekehrt muessten sonst oben 2 in der ersten Runde gegeneinander spielen.
        // Sollte man aber besser ueber Optimierungsziel loesen als feste Regel, denn es muss ja keine Loesung dafuer geben.
        if (!oddFound && itemTM->pos[stg+1] == 0)
        {
          oddFound = true;
          continue;
        }

        colvec[j] = colMapping[itemTM];
        rowvec[j] = 1;  

        j++;
      }

      // Spaltennamen setzen
      set_col_name(lp, colNA + naCountAbs, wxString::Format("abs[%s]#%02d", itemNA->na.naName, colNA + naCountAbs).char_str());

      // ... - j/2 (definiert ueber die Konstante)
      colvec[j] = 1;
      rowvec[j] = - (int) (j / 2);

      j++;

      // ... - abs(Nation)
      colvec[j] = colNA + naCountAbs;
      rowvec[j] = -1;

      add_constraintex(lp, j + 1, rowvec, colvec, LE, 0);

      // ... + abs(Nation)
      rowvec[j] = 1;

      add_constraintex(lp, j + 1, rowvec, colvec, GE, 0);

      naCountAbs++;
    }
  }

  // Dto. Regionalverbaende (listRG)
  int colRG = colNA + naCountAbs;
  int rgCountAbs = 0;

  if (true)
  {
    DrawListRegion listRG;

    // Im ersten Schritt alle Teams nach Regionalverband gruppieren
    for (DrawListNation::iterator itNA = listNA.begin(); itNA != listNA.end(); itNA++)
    {
      DrawItemNation *itemNA = (DrawItemNation *) (*itNA);

      if (idMapping.find(itemNA->na.naID) == idMapping.end())
        continue;

      for (DrawListTeam::iterator itTM = itemNA->teams.begin(); itTM != itemNA->teams.end(); itTM++)
      {
        DrawItemTeam *itemTM = (DrawItemTeam *) (*itTM);

        // Per pointer uebergeben. Der Destruktor loescht nicht
        if (listRG.Get(idMapping[itemTM->tm.naID]) == NULL)
            listRG.Add(idMapping[itemTM->tm.naID]);

          listRG.AddTeamItem(itemTM, idMapping[itemTM->tm.naID].naID);
      }
    }

    // Dann mit Regionen weiter, als seien es Nationen
    for (DrawListNation::iterator itRG = listRG.begin(); itRG != listRG.end(); itRG++)
    {
      memset(colvec, 0, cols * sizeof(int));
      memset(rowvec, 0, cols * sizeof(REAL));

      int j = 0;      

      // DrawListTeam, damit ich eine zufaellige Reihenfolge habe
      DrawListTeam rgList;

      DrawItemNation *itemRG = (DrawItemNation *) (*itRG);

      // GER1 + GER2 + ... 
      for (DrawListTeam::iterator itTM = itemRG->teams.begin(); itTM != itemRG->teams.end(); itTM++)
      {
        DrawItemTeam *itemTM = (DrawItemTeam *) (*itTM);

        if (colMapping.find(itemTM) == colMapping.end())
          continue;

        rgList.AddItem(itemTM);
      }

      // Nur wenn es auch mehr als ein Team dieser Nation gibt
      if (rgList.Count() < 2)
      {
        // Alle ausschneiden
        while (rgList.CutFirst())
          ;

        continue;
      }

      rgList.Shuffle();
      bool oddFound = (rgList.Count() % 2) == 0;

      while (DrawItemTeam *itemTM = (DrawItemTeam *) rgList.CutFirst())
      {
        // Wenn es eine ungerade Anzahl ist, einen frei auslosbaren Spieler nicht beruecksichtigen.
        // Ansonsten versucht die Optimierung, einen Spieler mehr nach oben als unten zu bringen, da COUNT(oben) immer 0 ist
        // 
        // TODO: 
        // Wenn es oben oder unten zu wenig freie Plaetze gibt, diesen Spieler in die andere Haelfte bringen
        // Z.B., 15 Teams, 16-er Feld, oben ein anderer gesetzter und Freilos. 
        // Dann gibt es oben nur 7 verfuegbare Spiele, unten 8. 
        // Es ist also besser, oben 7 und unten 8 zu haben. 
        // Umgekehrt muessten sonst oben 2 in der ersten Runde gegeneinander spielen.
        // Sollte man aber besser ueber Optimierungsziel loesen als feste Regel, denn es muss ja keine Loesung dafuer geben.
        if (!oddFound && itemTM->pos[stg+1] == 0)
        {
          oddFound = true;
          continue;
        }

        colvec[j] = colMapping[itemTM];
        rowvec[j] = 1;  

        j++;
      }

      // Spaltennamen setzen
      set_col_name(lp, colRG + rgCountAbs, wxString::Format("abs[%s]#%02d", itemRG->na.naRegion, colRG + rgCountAbs).char_str());

      // ... - j/2 (definiert ueber die Konstante)
      colvec[j] = 1;
      rowvec[j] = - (int) (j / 2);

      j++;

      // ... - abs(Region)
      colvec[j] = colRG + rgCountAbs;
      rowvec[j] = -1;

      add_constraintex(lp, j + 1, rowvec, colvec, LE, 0);

      // ... + abs(Region)
      rowvec[j] = 1;

      add_constraintex(lp, j + 1, rowvec, colvec, GE, 0);

      rgCountAbs++;
    }
  }

  // objective
  set_add_rowmode(lp, FALSE);

  // min: 1000 * abs(GER) + 1000 * abs(AUT) + ... + 100 * abs(Region) + ...

  if (true)
  {
    memset(colvec, 0, cols * sizeof(int));
    memset(rowvec, 0, cols * sizeof(REAL));

    int j = 0;

    for (int c = 0; c < naCountAbs; c++)
    {
      colvec[j] = colNA + c;
      rowvec[j] = 1000;  // Gewichtung Nation

      j++;
    }

    // Dto fuer Regionen, aber mit Gewicht 100
    if (true)
    {
      for (int c = 0; c < rgCountAbs; c++)
      {
        colvec[j] = colRG + c;
        rowvec[j] = 100;  // Gewichtung Region

        j++;
      }
    }

    // Add random

    listSC.Shuffle();

    for (DrawListTeam::iterator it = listSC.begin(); it != listSC.end(); it++)
    {
      DrawItemTeam *itemTM = (DrawItemTeam *) (*it);
      if (itemTM->pos[stg+1] != 0)
        continue;

      colvec[j] = colMapping[itemTM];
      rowvec[j] = 1;  // Gewichtung freier Spieler

      j++;

      break;
    }

    set_obj_fnex(lp, j, rowvec, colvec);
  }

  resize_lp(lp, get_Nrows(lp), colRG + rgCountAbs - 1);

  // Minimize weights
  set_minim(lp);

  // Log output
  set_verbose(lp, NORMAL);
  put_logfunc(lp, writeLogHandler, ziplog);

  ziplp->PutNextEntry(wxString::Format("stg%d-sec%02d.lp", stg, sec));
  ziplog->PutNextEntry(wxString::Format("stg%d-sec%02d.txt", stg, sec));

  write_lpex(lp, ziplp, writeModelHandler);

  int ncols = get_Ncolumns(lp);
  int nrows = get_Nrows(lp);

  set_solutionlimit(lp, 10000);

  // Und loesen
  int res = solve(lp);

#if 0
  // Es gibt nie eine zweite Loesung, warum auch immer :(
  if (res == OPTIMAL)
  {
    int solc = get_solutioncount(lp);
    if (solc > 1)
    {
      solc = (rand() % solc) + 1;
      wxString buf = wxString::Format("\nsolution: %d*/\n", solc); 
      ziplp->WriteAll(buf.char_str(), buf.Length() + 1);
      set_solutionlimit(lp, solc);

      res = solve(lp);
    }
  }
#endif

  if (res != OPTIMAL)
  {
    infoSystem.Error(_("Error solving the draw in stg %d sec %d: res = %d"), stg, sec, res);
  }

  bool ret = res == OPTIMAL;

  if (ret)
  {
    // Ergebnis in Logfile schreiben
    if (true)
    {
      // print_solution schreibt nach outputstream und nicht logstream
      // Daher ein temp. file aufsetzen, dorthin schreiben, und den 
      // Inhalt des temp. files dann in den writeLogHandler fuettern
      char *tmpname = _tempnam(NULL, "ttm");
      FILE *tmplog = fopen(tmpname, "w+b");
      set_outputstream(lp, tmplog);

      print_solution(lp, ncols);

      // Wieder auf stdout zuruecksetzen
      set_outputstream(lp, NULL);

      fseek(tmplog, 0, SEEK_SET);
      char buf[256];
      size_t r;

      // Leerzeile
      ziplog->WriteAll("\r\n", 2);

      fgets(buf, 255, tmplog);   // Log beginnt mit Leerzeile
      fgets(buf, 255, tmplog);   // Titel
      buf[255] = 0;
      ziplog->WriteAll(buf, strlen(buf));
      ziplog->WriteAll("\r\n", 2);

      // Weiter geht es mit fixed size Bloecken
      while ( (r = fread(buf, 1, 40, tmplog)) > 0 )
      {
        ziplog->WriteAll(buf, std::min(r, (size_t) 40));
        ziplog->WriteAll("\r\n", 2);
      }

      fclose(tmplog);
      unlink(tmpname);
    }

    memset(rowvec, 0, cols * sizeof(REAL));

    get_variables(lp, rowvec);

    // Naechste section eintragen, wenn sie denn stimmt
    for (std::map<DrawItemTeam *, int>::iterator itCOL = colMapping.begin(); itCOL != colMapping.end(); itCOL++)
    {
      DrawItemTeam *itemTM = itCOL->first;
      int col = itCOL->second - 1;

      int pos = rowvec[col] > 0 ? 2 * sec : 2 * sec - 1;
      if (!itemTM->pos[stg+1])
        itemTM->pos[stg + 1] = pos;
      else if (itemTM->pos[stg+1] != pos)
      {
        infoSystem.Error(_("Team seeded to pos %d was drawn to different pos %d in std %d sec %d"), itemTM->pos[stg+1], pos, stg, sec);
        ret = false;
      }
    }
  }

  delete_lp(lp);

  // Weitere Pruefungen in letzter Stufe
  if (listSC.Count() == 2)
  {
    DrawListTeam::iterator it = listSC.begin();
    DrawItemTeam *itemOne = (DrawItemTeam *) (*it);
    ++it;
    DrawItemTeam *itemTwo = (DrawItemTeam *) (*it);

    // 2 Erste gegeneinander
    if ((totalDE + totalFirst) < (totalNA + totalBY) / 2)
    {
      if (itemOne->lastPos == 1 && itemTwo->lastPos == 1)
        warnings.push_back(wxString::Format(_("Two first playing in first round at pos %d - %d"), 2 * sec - 1, 2 * sec));
    }

    // Zweiter hat unerwartet ein Freilos
    if (totalBY <= totalDE + totalFirst)
    {
      if (!itemOne->rkDirectEntry && itemOne->lastGroup == 0 && itemTwo->lastPos != 1 || !itemTwo->rkDirectEntry && itemTwo->lastGroup == 0 && itemOne->lastPos != 1)
        warnings.push_back(wxString::Format(_("Second got an unexpected bye in first round at pos %d - %d"), 2 * sec - 1, 2 * sec));
    }

    // DE hat nicht das erwartete Freilos
    if (totalBY >= totalDE)
    {
      if (itemOne->rkDirectEntry && !itemTwo->IsBye() || !itemOne->IsBye() && itemTwo->rkDirectEntry)
        warnings.push_back(wxString::Format(_("DE did not get an expected bye in first round at pos %d - %d"), 2 * sec - 1, 2 * sec));
    }

    // Erster hat nicht das erwartete Freilos
    if (totalBY >= totalDE + totalFirst)
    {
      if (itemOne->lastGroup && itemTwo->lastPos == 1 || itemTwo->lastGroup && itemOne->lastPos == 1)
        warnings.push_back(wxString::Format(_("First did not get an expected bye in first round at pos %d - %d"), 2 * sec - 1, 2 * sec));
    }
  }

  listSC.clear();

  return ret;
}

bool DrawLP::DrawStage(int stg)
{
  for (int sec = 1; sec <= exp2(stg-1); sec++)
    if (!DrawSection(stg, sec))
    {
      infoSystem.Error(_("No solution exists in stg %d sec %d"), stg, sec);
      return false;
    }

  return true;
}


bool DrawLP::DrawThem(GrStore &gr)
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

  // And make sure everyone in the list will be drawn
  for  (auto itNA : listNA)
    for (auto itTM : ((DrawItemNation *) itNA)->teams)
      ((DrawItemTeam *) itTM)->pos[1] = 1;

  lastStage = ld2(gr.grSize);

  for (int stg = 1; stg <= lastStage; stg++)
    if (!DrawStage(stg))
      return false;

  return true;
}


bool DrawLP::DrawImpl(long seed)
{
  // Name wird als Dateiname verwendet
  wxString tmpStage = toStage;
  tmpStage.Replace("/", "__", true);
  
  GrStore  gr(connPtr);

  // Als erstes testen, ob die naechste Gruppe ex.ist.
  if (toGrID == 0)
    gr.SelectAll(cp, toStage);
  else
    gr.SelectById(toGrID);

  gr.Next();
  gr.Close();

  if (!gr.grID)
  {
    infoSystem.Error(_("No group!"));
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
      CTT32App::instance()->GetPath().data(), cp.cpName, tmpStage.c_str(),
      tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

  file = wxFopen(fname, "w");
  // file = fopen("Debug.txt", "w");
  #endif

# ifdef DEBUG
  wxFprintf(file, "Seed: %ld\n", seed);
# endif

  // Gruppen auslesen
  if (!koCons && !ReadGroups())               
  {
#ifdef DEBUG
    fclose(file);
#endif      
    return false;       // ... nicht erfolgreich
  }

  // Oder Spiele lesen
  if (koCons && !ReadMatches())
  {
#ifdef DEBUG
    fclose(file);
#endif      
    return false;       // ... nicht erfolgreich
  }

  // And direct entries
  if (!koCons && !ReadDirectEntries())
  {
#ifdef DEBUG
    fclose(file);
#endif      

    return false;
  }

  if (!ReadRegions())
  {
#ifdef DEBUG
    fclose(file);
#endif      
    return false;
  }

  // Setzung loeschen
  if (toGrID == 0)
    gr.ClearDraw(cp, toStage);
  else
    gr.ClearDraw(toGrID);

  // Setzung einlesen
  if (!ReadSeeding(gr))
  {
#ifdef DEBUG
    fclose(file);
#endif      
    return false;
  }

  // Int'l Ranking einlesen
  if (!ReadRanking())
  {
#ifdef DEBUG
    fclose(file);
#endif      
    return false;       // ... nicht erfolgreich
  }

  totalNA = listNA.Count(0, 0);

  // Spieler zaehlen und Groesse der Gruppe pruefen
  if (gr.grSize < totalNA)
  {
#ifdef DEBUG
    fclose(file);
#endif    
    infoSystem.Error(_("Group too small for %d entries!"), totalNA);
    return false;
  }

  if (gr.grSize / 2 >= totalNA)
  {
#ifdef DEBUG
    fclose(file);
#endif    
    infoSystem.Error(_("Group too large for %d entries!"), totalNA);
    return false;
  }

  wxSprintf(fname, "%s\\%s_%s_%04d%02d%02dT%02d%02d%02d-lpsolve.zip",
      CTT32App::instance()->GetPath().data(), cp.cpName, tmpStage.c_str(),
      tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

  wxFileOutputStream outlp(fname);
  ziplp = new wxZipOutputStream(outlp);

  wxSprintf(fname, "%s\\%s_%s_%04d%02d%02dT%02d%02d%02d-lplogs.zip", 
      CTT32App::instance()->GetPath().data(), cp.cpName, tmpStage.c_str(),
      tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

  wxFileOutputStream outlog(fname);
  ziplog = new wxZipOutputStream(outlog);

  bool ret = DrawThem(gr);

  ziplp->Close();
  delete ziplp;
  outlp.Close();

  ziplog->Close();
  delete ziplog;
  outlog.Close();

  if (ret && !warnings.empty())
  {
    wxString str = "Ignore the warnings and accept the draw?\n";

    for (std::list<wxString>::iterator it = warnings.begin(); it != warnings.end(); it++)
    {
      str += "\n";
      str += (*it);
    }

    if (!infoSystem.Question(str))
      ret = false;
  }

  warnings.clear();

  if (!ret )
  {
#ifdef DEBUG
    fclose(file);
#endif 

    return false;
  }

  // In der Gruppe verteilen
  Distribute(gr);
  
#ifdef DEBUG
  fclose(file);
#endif    
  
  return true;
}


// Gruppen auslesen und Auslosung durchfuehren
bool DrawLP::Draw(const CpRec &cp_, const wxString &fr_, const wxString &to_, int fromPos_, int toPos_, long toGrID_, RankingChoice rkChoice_, int seed_)
{
  cp = cp_;
  fromStage = fr_;
  toStage = to_;
  fromPos = fromPos_;
  toPos   = toPos_;
  toGrID = toGrID_;
  rkChoice = rkChoice_;

  // Um nicht imm fragen zu muessen, ob connPtr oder default
  Connection *tmpConn = connPtr ? connPtr : TTDbse::instance()->GetDefaultConnection();

  tmpConn->StartTransaction();

  bool ret = DrawImpl(seed_ ? seed_ : time(NULL));

  // Commit
  if (ret)
    tmpConn->Commit();
  else
    tmpConn->Rollback();

  return ret;
}





