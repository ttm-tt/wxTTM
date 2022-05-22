/* Copyright (C) 2020 Christoph Theis */

// Auslosung a'la CSD

#include "stdafx.h"

#include "TT32App.h"

#include "DrawCSD.h"

#include "RkListStore.h"
#include "StEntryStore.h"

#include "InfoSystem.h"


# ifndef  TURBO
#   define  random(x)    (rand() % (x))
#   define  randomize()  srand(time(0))
# endif

                  // dedif, dif      
const int  OP_NN = 0; // 0, 0
const int  OP_PP = 1; // +, +
const int  OP_PN = 2; // +, 0
const int  OP_PM = 3; // +, -
const int  OP_NP = 4; // 0, +


DrawCSD::DrawCSD(Connection *ptr)
{
  connPtr = ptr;
}


DrawCSD::~DrawCSD()
{
  // Wenn hier noch kein Commit stattgefunden hat, wiederaufrollen.
  if (connPtr)
  {
    connPtr->Rollback();
    connPtr->Close();
  }
    
  delete connPtr;
}


bool  DrawCSD::Draw(const CpRec &cp, wxString deStage, wxString quStage, int seed)
{
  // Initialize random generator
  srand(seed ? seed : time(NULL));
  
  // Rankinginfo einlesen
  ReadRanking(cp);

  // Gruppen einlesen  
  GrStore gr(connPtr);
  gr.SelectAll(cp);
  
  nofQGroups = 0;
  sizeDGroup = 0;
  
  int nofQualRounds = 0;

  int sizeQGroups = 0;
  
  while (gr.Next())
  {
    if (!quStage.IsEmpty() && !wxStrcoll(quStage, gr.grStage) ||
        !deStage.IsEmpty() && !wxStrcoll(deStage, gr.grStage))
    {
      if (!quStage.IsEmpty() && !wxStrcoll(quStage, gr.grStage))
      {
        nofQGroups++;
        sizeQGroups += gr.grSize;
      }
      else if (!deStage.IsEmpty() && !wxStrcoll(deStage, gr.grStage))
      {
        if (sizeDGroup)
        {
          // More than the one and only?
          return false;
        }
        else
        {
          sizeDGroup = gr.grSize;
          nofQualRounds = gr.grQualRounds;
        }        
      }
    }
  }
  
  // Wenn es keine Qualifier gibt, brauche ich auch keine Q-Gruppen
  if (qu == 0)
  {
    quStage = wxEmptyString;
    nofQGroups = 0;
  }
  
  // de + nofQGroups ist die Mindesgroesse des Hauptfeldes. 
  int minSizeDGroup = exp2(ld2(de + nofQGroups));
  if (minSizeDGroup < (de + nofQGroups))
    minSizeDGroup *= 2;
    
  if (sizeDGroup == 0)
  {
    sizeDGroup = minSizeDGroup;    
    
    // Wenn es Qualis gibt, aber keine Qualigruppen, das Hauptfeld aufblasen
    if (qu > 0 && quStage.IsEmpty())
    {
      sizeDGroup = exp2(ld2(de + qu));
      if (sizeDGroup < de + qu)
        sizeDGroup *= 2;      
    }
  }
  else if (nofQGroups == 0 && sizeDGroup >= (2 * de + qu))
  {
    // Keine Q-Gruppen angelegt und Qualifiers koennen in einer weiteren
    // Runde spielen (DE bekommen ein Freilos in der ersten Runde):
    // Qualification-Stufe ignorieren
    quStage = wxEmptyString;
  }
  
  // Wenn es Qualifier gibt und quStage definiert ist, muss es auch 
  // Q-Gruppen geben. Q-Gruppen haben dann per Definition Spieler drin
  if (!quStage.IsEmpty())
  {
    // Quali-Gruppen: Anzahl berechnen
    if (nofQGroups == 0)
    {
      // Wenn es keine Gruppen gibt, Gruppen mit 8 Spielern anlegen
      nofQGroups = (qu + 7) / 8;
    }
     else if (sizeQGroups < qu)
     {
       // Wenn es Gruppen gibt, muessen genuegend Plaetze frei sein
       infoSystem.Error(_("To few groups for the draw"));
       return false;
     }
  }
  else if (qu)
  {
    // Quali-Runde: Groesse fuer das Hauptfeld berechnen
    if (sizeDGroup < 2 * de + qu)
    {
      // Hauptfeld zu klein
      infoSystem.Error(_("No group or group to small!"));
      return false;
    }

    // Mindestgroesse fuer Hauptfeld    
    // Wenn Qual-Runden angegeben wurden, dann diese nehmen, sonst berechnen
    if (nofQualRounds)
    {
      minSizeDGroup = sizeDGroup >> nofQualRounds;
    }
    else
    {
      int tmpQu = qu;
      minSizeDGroup = sizeDGroup;

      // Solange Platz ist fuer DE und der Rest von QU
      while (minSizeDGroup > 2 * (de + tmpQu / 2))
      {
        minSizeDGroup /= 2;
        tmpQu /= 2;
      }
    }
  }

  if (sizeDGroup < minSizeDGroup)
  {
    infoSystem.Error(_("No group or group to small!"));
    return false;
  }
    
  // Jetzt sollte ich wissen wieviele Gruppen denn so gebraucht werden
  pgr = new GrStore[1 + nofQGroups];

  // Sicherstellen, dass die Gruppen vorhanden sind  
  if (!CreateGroups(cp, deStage, quStage))
    return false;
  
  // Seedinginfo lesen und Gruppen freimachen
  if (!ReadSeeding(cp))
    return false;
  
  gr.ClearDraw(cp, deStage);
  if (!quStage.IsEmpty())
    gr.ClearDraw(cp, quStage);
    
  // Noetige Anzahl von QG anlegen
  while (listQG.Count() < nofQGroups)
  {
    DrawItemTeam *itemQG = new DrawItemTeam(TmListRec());
    itemQG->rkDirectEntry = 1;
    itemQG->pos[1] = 1;
    listQG.AddItem(itemQG);
  }

  // Wenn es keine Q-Gruppen gibt, eine entsprechende Anzahl faelschen
  if (quStage.IsEmpty())
  {
    // Anzahl der Qualifier je Platz im Hauptfeld
    int factor = sizeDGroup / minSizeDGroup;
    
    while (listQG.Count() < qu / factor)
    {
      DrawItemTeam *itemQG = new DrawItemTeam(TmListRec());
      itemQG->rkDirectEntry = 1;
      itemQG->pos[1] = 1;
      listQG.AddItem(itemQG);
    }
  }
      
  // Entsprechende Anzahl an Freilosen anlegen
  while (listBY.Count() < minSizeDGroup - (de + listQG.Count()))
  {
    DrawItemTeam *itemBY = new DrawItemTeam(TmListRec());
    itemBY->rkDirectEntry = 1;
    itemBY->pos[1] = 1;
    listBY.AddItem(itemBY);
  }
  
  int lastStage = ld2(minSizeDGroup);
  int lastQStage = lastStage;
  
  if (quStage.IsEmpty())
    lastQStage = pgr[0].NofRounds();
  else if (qu > 0)
    lastQStage += pgr[1].NofRounds();
  
  // Auslosen, aber nur bis zu den Qualirunden
  if ( !DrawThem(1, lastStage, lastStage) )
    return false;
  
  // Jetzt weiter auf die Plaetze oder in die Q-Gruppen
  // if (minSizeDGroup < sizeDGroup)
  if (qu == 0)
    ;  // Keine Qualifier
  else if (nofQGroups == 0)
  {
    // Jetzt die Q-Gruppen rauswerfen und Freilose auffuellen
    DrawItem *itemQG;
    while ( (itemQG = listQG.CutFirst()) )
      delete itemQG;
      
    if ( !DrawThem(lastStage + 1, lastQStage, lastQStage) )
      return false;
  }
  else
  {
    // Jetzt die Q-Gruppen rauswerfen und Freilose auffuellen
    // Die Auslosung beginnt bei lastStage + 2, damit hier nur noch QU sind
    // Damit die anderen nicht ausgelost werden wird fuer sie lastStage + 1 auf -1 gesetzt.
    DrawItem *itemQG;
    int qidx = 0;
    
    while ( (itemQG = listQG.CutFirst()) )
    {
      int sec = ((DrawItemTeam *) itemQG)->pos[lastStage + 1];
      int count = pgr[++qidx].grSize - listNA.Count(lastStage + 1, sec);
      while (count--)
      {
        DrawItemTeam *itemBY = new DrawItemTeam(TmListRec());
        itemBY->rkDirectEntry = 0;
        itemBY->pos[lastStage + 2] = sec;
        listBY.AddItem(itemBY);
      }
      
      delete itemQG;
    }

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

        if (!itemTM->rkDirectEntry)
          itemTM->pos[lastStage + 2] = itemTM->pos[lastStage+1];
        else
          itemTM->pos[lastStage + 2] = -1;
      }    
    }

    for (DrawListTeam::iterator itBY = listBY.begin();
         itBY != listBY.end(); itBY++)
    {
      DrawItemTeam *itemBY = (DrawItemTeam *) (*itBY);
      if (itemBY->rkDirectEntry)
        itemBY->pos[lastStage + 2] = -1;
    }
  
    if ( !DrawThem(lastStage + 2, lastQStage + 1, lastQStage + 1) )
      return false;
  }
    
  gr.GetConnectionPtr()->StartTransaction();

  if (!DistributeEntries())
  {
    gr.GetConnectionPtr()->Rollback();
    return false;
  }

  gr.GetConnectionPtr()->Commit();

  return true;
}


// -----------------------------------------------------------------------
// Read ranking information
bool  DrawCSD::ReadRanking(const CpRec &cp)
{
  de = 0;
  qu = 0;
  
  RkListStore rk(connPtr);
  
  // Erst die Zahl der Direct Entries auslesen.
  // Ein Wettbewerb ohne DE wird wie ein Wettbewerb nur mit DE behandelt
  bool quAsDE = (rk.CountDirectEntries(cp, NaRec()) == 0);
  
  rk.SelectByCp(cp);
  while (rk.Next())
  {
    if (quAsDE)
      rk.rkDirectEntry = 1;
      
    listNA.AddTeam(rk);
    if (rk.rkDirectEntry)
      de++;
    else
      qu++;
  }
  
  char text[100];
  sprintf(text, "de = %d, qu = %d", de, qu);
  // ::MessageBox(NULL, text, "ReadRanking", MB_OK);
  
  return true;
}


// Read (and clear) groups
bool  DrawCSD::ReadSeeding(const CpRec &cp)
{
  // Hauptgruppe einlesen
  
  // Besser waere ein Join von StListStore und XxListStore,
  // statt auch noch die Namen einzulsen. Auf der anderen Seite
  // braucht man das eh nur fuer die Hauptgruppe.
  StEntryStore st(connPtr);
  st.SelectByGr(pgr[0], cp.cpType);
  
  bool warnNoDE = false;
  
  while (st.Next())
  {
    if (st.st.stSeeded)
    {
      if (st.st.tmID)
      {
        // Gesetzte Mannschaft
        DrawItemTeam *itemTM = listNA.GetTeam(st.tmID);
        if (itemTM)
        {
          if (de > 0 && itemTM->rkDirectEntry == 0)
          {
            if (!warnNoDE)
            {
              if (!infoSystem.Question(_("Seeded entry on pos. %d is not a direct entry. Continue anyway?"), st.st.stPos))
                return false;
              
              warnNoDE = true;
            }
          }
          
          itemTM->pos[0] = st.st.stNr;

          for (int stg = 1, sec = pgr[0].grSize; sec; stg++, sec /= 2)
            itemTM->pos[stg] = (st.st.stNr - 1) / sec + 1;
        }
      }        
      else if (st.team.cpType == CP_GROUP)
      {
        // Gruppe
        DrawItemTeam *itemQG = new DrawItemTeam(TmListRec());
        itemQG->rkDirectEntry = 1;
        itemQG->tm.tmID = st.team.gr.grID;
        itemQG->pos[0] = st.st.stNr;
        
        for (int stg = 1, sec = pgr[0].grSize; sec; stg++, sec /= 2)
          itemQG->pos[stg] = (st.st.stNr - 1) / sec + 1;
          
        listQG.AddItem(itemQG);
      }
      else 
      {
        // Freilos
        DrawItemTeam *itemBY = new DrawItemTeam(TmListRec());
        itemBY->rkDirectEntry = 1;
        itemBY->pos[0] = st.st.stNr;
                
        for (int stg = 1, sec = pgr[0].grSize; sec; stg++, sec /= 2)
          itemBY->pos[stg] = (st.st.stNr - 1) / sec + 1;
          
        listBY.AddItem(itemBY);
      }
    }
  }
  
  return true;  
}


bool  DrawCSD::DrawThem(int startStage, int lastDrawStage, int lastStage)
{
  for (int stg = startStage; stg <= lastDrawStage; stg++)
  {
    // DrawStage muss nur das Ende des gesamten Feldes wissen
    if (!DrawStage(stg, lastStage))
      return false;
  }
    
  return true;
}


bool  DrawCSD::DrawStage(int stg, int lastStage)
{
  for (int sec = 1; sec <= exp2(stg-1); sec++)
  {
    if (!DrawSection(stg, sec, lastStage))
      return false;
  }
    
  return true;
}


bool  DrawCSD::DrawSection(int stg, int sec, int lastStage)
{
  dde = dtn = 0;
  deSec = quSec = 0;
  
  for (int i = 0; i < 5; i++)
    opCounter[i] = 0;
    
  BuildPackList(stg, sec);
  
  // Freilose auffuellen. Kann man hier machen, 
  // weil hier direct entries und qualifiers in der Stufe bekannt sind
  int qgCount = listQG.Count(stg, sec);
  int byCount = listBY.Count(stg, sec);
  
  int count = deSec + qgCount + byCount;
  int deCount = count;
  
  // Wenn es keine Q-Gruppen gibt, aber Qualifiers, 
  // diese mitzaehlen
  if (qgCount == 0 && quSec != 0)
    count += quSec;
    
  int needed = 2;
  while (needed < count)
    needed *= 2;

  for (; count < needed; count++)
  {
    DrawItemTeam *itemBY = new DrawItemTeam(TmListRec());
    // Freilose sind direct entries, wenn es auch direct entries gibt
    itemBY->rkDirectEntry = 1;
    itemBY->pos[stg] = sec;
    listBY.Add(itemBY);
  }
  
  // Auslesen / Auslosen der gesetzten Packs  
  DrawSeededPacks(stg, sec);
  // Verteilen der Freilose
  DistributeByes(stg, sec, lastStage);
  // Verteilen der Q-Gruppen. 
  // Diese brauchen die freien Plaetze nach obigen Prozeduren
  DistributeQGroups(stg, sec, lastStage);
  
  // SeparateNations(stg, sec);
  
  // Test, ob eine Loesung existiert. 
  // Wenn es am Anfang eine gibt, kann diese nicht mehr verloren gehen.
  if (!SolutionExists(OP_NN))
  {
    // Warnung ausgeben
    infoSystem.Error(_("No solution exists at stage %d, section %d"), stg, sec);
    return false;
  }
  
  // Jetzt die Packs auslosen
  listPK.Shuffle();
  
  DrawItemPack * itemPK;
  
  while ( (itemPK = (DrawItemPack *) listPK.CutFirst()) )
  {
    // Gesetzte wurden bereits ausgelost
    if (itemPK->teams[0]->pos[stg+1] == 0)
      DrawPack(itemPK, stg, sec);
    delete itemPK;
  }
  
  return true;
}


// Packliste (Paare, die getrennt werden muessen / koennen) bilden.
// Ein Pack besteht immer aus Spielern einer Nation, ueberzaehlige 
// Spieler werden einzeln ausgelost. Ansonsten wuerden 2 Qualifier
// getrennt werden, auch wenn es nur eine Q-Gruppe gibt und beide
// daher in die gleiche Haelfte laufen muessen.
bool  DrawCSD::BuildPackList(int stg, int sec)
{
  DrawItemPack *itemPK;
  
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
      
      if (itemTM->pos[stg] == 0 || itemTM->pos[stg] == sec)
      {
        if (itemTM->rkDirectEntry)
          deSec++;
        else
          quSec++;
      }
    }
    
    while ( (itemPK = GetPack(itemNA->teams, stg, sec)) )
    {
      opCounter[abs(GetOperator(itemPK))]++;
      listPK.Add(itemPK);
    }            
  }
  
  return true;
}


// Liefert das naechste Pack aus der Liste in der Reihenfolge:
// Erster Spieler: das kleinste Ranking
// Zweiter Spieler: Ein Spieler aus 1-2, 3-4, 5-8, 9-16, ...
DrawItemPack * DrawCSD::GetPack(DrawListTeam &list, int stg, int sec)
{
  if (list.Count() == 0)
    return 0;
    
  DrawItemTeam *firstItem = 0, *secondItem = 0, *itemPtr = 0;
  
  // Erstes Item holen: Derjenige mit dem hoechsten Ranking
  for (DrawListTeam::iterator itTM = list.begin();
       itTM != list.end(); itTM++)
  {
    DrawItemTeam *itemPtr = (DrawItemTeam *) (*itTM);

    if (itemPtr->pos[stg] && itemPtr->pos[stg] != sec)
      continue;
    
    if (itemPtr->mask)
      continue;
    
    if (!firstItem)
      firstItem = itemPtr;
    else if (firstItem->rkNatlRank > itemPtr->rkNatlRank)
      firstItem = itemPtr;
  }
  
  if (firstItem == 0)
    return 0;
    
  firstItem->mask = true;
  
  // Zweites Item: Eines, das dazu passt
  for (DrawListTeam::iterator itTM = list.begin();
       itTM != list.end(); itTM++)
  {
    DrawItemTeam *itemPtr = (DrawItemTeam *) (*itTM);
    
    if (itemPtr->pos[stg] && itemPtr->pos[stg] != sec)
      continue;
    
    if (itemPtr->mask)
      continue;
    
    // Erstes Item erwischt
    if (firstItem == itemPtr)
      continue;
    
    // Zwei gesetzte  
    if (firstItem->pos[stg+1] && 
        firstItem->pos[stg+1] == itemPtr->pos[stg+1])
      continue;
      
    // Kein zweites Item bislang
    if (!secondItem)
    {
      secondItem = itemPtr;
      continue;
    }
           
    // Bereich pruefen: 1+2, 3+4, 5-8, ... 
    int ub = exp2(ld2(firstItem->rkNatlRank)+1);
    int lb = ub / 2 + 1;
    
    if ( itemPtr->tm.naID == firstItem->tm.naID && 
         itemPtr->rkNatlRank >= lb && itemPtr->rkNatlRank <= ub &&
         (secondItem->rkNatlRank < lb || secondItem->rkNatlRank > ub)
       )
     secondItem = itemPtr;         
  }
  
  if (secondItem)
    secondItem->mask = true;
    
  // Gesetzten Spieler nach vorne bringen
  if (secondItem && secondItem->pos[stg+1] && !firstItem->pos[stg+1])
    return new DrawItemPack(secondItem, firstItem);
  else
    return new DrawItemPack(firstItem, secondItem);
  
  return 0;
}


bool  DrawCSD::DrawSeededPacks(int stg, int sec)
{
  for (DrawListPack::iterator itPK = listPK.begin();
       itPK != listPK.end(); itPK++)
  {
    DrawItemPack *itemPK = (DrawItemPack *) (*itPK);
    
    if (itemPK->teams[0] && itemPK->teams[0]->pos[stg+1])
    {
      DrawPack(itemPK, stg, sec);
    }
  }
   
  return true;
}


bool  DrawCSD::DistributeByes(int stg, int sec, int lastStage)
{
  // Erster Schritt: Zaehlen
  int dbc = 0, dbu = 0, dbl = 0;  // count, obere, untere Haelfte
  int ndbu = 0, ndbl = 0;         // obere, untere Haelfte gesetzt
  int qbc = 0;                    // Anzahl Q-Byes
  
  for (DrawListTeam::iterator itBY = listBY.begin(); 
       itBY != listBY.end(); itBY++)
  {
    DrawItemTeam *itemBY = (DrawItemTeam *) (*itBY);
    
    if (itemBY->pos[stg] && itemBY->pos[stg] != sec)
      continue;
      
    // Eines gefunden
    itemBY->rkDirectEntry ? dbc++ : qbc++;
    
    if (itemBY->pos[stg+1]) 
      (itemBY->pos[stg+1] % 2) ? ndbl++ : ndbu++;
  }
  
  // Gibt es ueberhaupt was zu tun?
  if (dbc == 0 && qbc == 0)
    return true;
    
  // Es sollte immer nur Q-Byes oder D-Byes geben
  wxASSERT((dbc > 0) ^ (qbc > 0));
    
  // Benoetigte Freilose berechnen
  if (dbc > 0)
  {
    CalculateByes(stg, sec, lastStage, dbc, dbu, dbl);
  }
  else
  {
    // Q-Byes: Hier wird stg und sec manipuliert
    int stgOffset = pgr[0].NofRounds() + 1;  // Abzueglich Hauptrunde
    int secOffset = 1 << (stg - stgOffset);  // Section relativ zur Hauptrunde
    CalculateByes(stg - stgOffset, ((sec - 1) % secOffset) + 1, lastStage - stgOffset, qbc, dbu, dbl);  
  }
  
  // Jetzt noch ndbu, ndbl beruecksichtigen
  if (dbu < ndbu)
  {
    dbu = ndbu;
    dbl = dbc - dbu;
  }
  
  if (dbl < ndbl)
  {
    dbl = ndbl;
    dbu = dbc - dbl;
  }

  // Den Rest verteilen
  for (DrawListTeam::iterator itBY = listBY.begin();
       itBY != listBY.end(); itBY++)
  {
    DrawItemTeam *itemBY = (DrawItemTeam *) (*itBY);
    
    if (itemBY->pos[stg] && itemBY->pos[stg] != sec)
      continue;
    
    if (itemBY->pos[stg+1])
      ; // Nichts zu tun
    else if (ndbu < dbu)
    {
      itemBY->pos[stg+1] = 2 * sec - 1;
      ndbu++;
    }
    else if (ndbl < dbl)
    {
      itemBY->pos[stg+1] = 2 * sec;
      ndbl++;
    }
    // Freie Freilose vergeben
    else if (random(2))
    {
      itemBY->pos[stg+1] = 2 * sec - 1;
      ndbu++;
    }
    else
    {
      itemBY->pos[stg+1]= 2 * sec;
      ndbl++;
    }
  }
  
  if (dbc > 0)
  {
    dde += (ndbu - ndbl);
    dtn += (ndbu - ndbl);
  } 
  else
  {
    // Byes sind keine DE, also aendert sich dde nicht.
    dtn += (ndbu - ndbl);
  }
  
  return true;  
}


bool  DrawCSD::DistributeQGroups(int stg, int sec, int lastStage)
{
  if (listQG.Count() == 0)
    return true;
    
  // Wie bei den Freilosen: Erste Runde: Zaehlen
  int qgc = 0, qgu = 0, qgl = 0;  // Count, obere, untere Haelfte
  int nqgu = 0, nqgl = 0;         // Gesetzt obere untere Haelfte
  
  for (DrawListTeam::iterator itQG = listQG.begin();
       itQG != listQG.end(); itQG++)
  {
    DrawItemTeam *itemQG = (DrawItemTeam *) (*itQG);
    
    if (itemQG->pos[stg] && itemQG->pos[stg] != sec)
      continue;
      
    qgc++;
    if (!itemQG->pos[stg+1])
      ; // Nicht gesetzt
    else if (itemQG->pos[stg+1] % 2)
      nqgu++;
    else
      nqgl++;
  }
  
  // Keine Q-Gruppen hier
  if (qgc == 0)
    return true;
  
  // Zweite Stufe: Benoetigte QG ausrechnen
  qgu = qgc / 2;
  qgl = qgc / 2;
  
#if 0  
  // Einzelne Q-Gruppen umgekehrt wie Freilose verteilen.
  // Damit rutschen sie von gesetzten Plaetzen weg.
  // Eine ungerade Anzahl kann frei verteilt werden.
  if (qgc == 1)
  {
    if (stg == lastStage)
      (sec % 2) ? qgu++ : qgl++;
    else
      (sec % 2) ? qgl++ : qgu++;      
  }  
  else if (qgc % 2)
    random(2) ? qgu++ : qgl++;
#else
  if (qgc % 2)
    random(2) ? qgu++ : qgl++;
#endif  

  // Wenn es in einer Haelfte mehr direct entries gibt, als durch 
  // Paare ausgeglichen werden kann, nochmals korrigieren.
  // Die gesetzten Spieler sind schon alle weg, der Rest kann also
  // frei ausgelost werden.
  //                                    + / +          + / 0          + / -
  int diff =  abs(dde + qgu - qgl) - (opCounter[1] + opCounter[2] + opCounter[3]);
  if (diff > 0)
  {
    if (dde < 0)
    {
      // Mehr direct entries unten
      qgl -= diff;
      qgu += diff;
    }
    else if (dde > 0)
    {
      // Mehr direct entries oben
      qgu -= diff;
      qgl -= diff;
    }
  }
  
  // Auf gesetzte Q-Gruppen korrigieren
  if (qgu < nqgu)
  {
    qgu = nqgu;
    qgl = qgc - qgu;
  }
  
  if (qgl < nqgl)
  {
    qgl = nqgl;
    qgu = qgc - qgl;
  }
  
  // Anzahl der Qualifiers oben / unten ausrechnen
  int nquu = quSec * qgu / qgc;
  int nqul = quSec * qgl / qgc;
  
  int qbu = 0, qbl = 0;  // "Freilose" oben unten
  
  while ( (nquu + nqul + qbu + qbl) < quSec )
  {
    if (random(2))
    {
      if (qbu < qgu)
        qbu++;
      else
        qbl++;
    }
    else
    {
      if (qbl < qgl)
        qbl++;
      else
        qbu++;
    }
  }    
  
  // dtn faelschen, damit die Qualifies richtig wandern
  dtn -= (nquu + qbu - nqul - qbl);   
  
  // Korrektur, falls durch die Setzung schon Qualifiers gewandert sind
  // dtn += (dtn - dde);  // Differenz, die durch Qualifiers verursacht wurde
  
  // Setzen und verteilen
  for (DrawListTeam::iterator itQG = listQG.begin(); 
       itQG != listQG.end(); itQG++)
  {
    DrawItemTeam *itemQG = (DrawItemTeam *) (*itQG);
    
    if (itemQG->pos[stg] && itemQG->pos[stg] != sec)
      continue;
      
    if (itemQG->pos[stg+1])
      ;
    else if (nqgu < qgu)
    {
      nqgu++;
      itemQG->pos[stg+1] = 2 * sec - 1;
    }
    else // if (nqgl < qgl)
    {
      nqgl++;
      itemQG->pos[stg+1] = 2 * sec;
    }
  }
  
  dde += (qgu - qgl);
  dtn += (qgu - qgl);  
  
  return true;
}


bool  DrawCSD::DrawPack(DrawItemPack *itemPK, int stg, int sec)
{
  int op = GetOperator(itemPK);

  bool down = random(2) ? true : false;
  
  // Pruefen, ob erstes Team gesetzt ist. 
  // Wenn Pack eine Setzung enthaelt, dann ist auf jedem Fall das
  // erste Team gesetzt.
  if (itemPK->teams[0] && itemPK->teams[0]->pos[stg+1])
    down = ( (itemPK->teams[0]->pos[stg+1] % 2) ? false : true );
  
  if (down)
    op = -op;

  // Gesetzte Spieler nicht vertauschen! Die Loesung muss existieren.
  if ( (!itemPK->teams[0] || !itemPK->teams[0]->pos[stg+1]) && 
       !SolutionExists(op))
  {
    op = -op;
    down = !down;
  }

  if (itemPK->teams[0])
    itemPK->teams[0]->pos[stg+1] = 2 * sec - (down ? 0 : 1);
  if (itemPK->teams[1])
    itemPK->teams[1]->pos[stg+1] = 2 * sec - (down ? 1 : 0);
    
  ActualizeDrawParameters(op);
    
  return true;
}


int  DrawCSD::GetOperator(DrawItemPack *itemPK)
{
  int dif = 0, dedif = 0;
  
  static int op[][3] = 
  {
    {-OP_PP, -OP_PN, -OP_PM}, // dedif/dif: --, -0, -+
    {-OP_NP,  OP_NN,  OP_NP}, // dedif/dif: 0-, 00, 0+
    { OP_PM,  OP_PN,  OP_PP}  // dedif/dif: +-, +0, ++
  };
  
  if (itemPK->teams[0])
  {
    dif++;
    if (itemPK->teams[0]->rkDirectEntry)
      dedif++;
  }
  
  if (itemPK->teams[1])
  {
    dif--;
    if (itemPK->teams[1]->rkDirectEntry)
      dedif--;
  }
  
  return op[dedif+1][dif+1];  
}


bool DrawCSD::SolutionExists(int op)
{
  bool f = (op == OP_NN);  // ASSERT, wenn keine Loesung existieren kann
  bool solution = true;
  
  int tmpOpCounter[5];
  int tmpDDE = dde;
  int tmpDTN = dtn;
  
  for (int i = 0; i < 5; i++)
    tmpOpCounter[i] = opCounter[i];
    
  ActualizeDrawParameters(op);
  
  int am = opCounter[1];
  int bm = opCounter[2];
  int cm = opCounter[3];
  int dm = opCounter[4];
    
  if ( (bm == 0) && (cm == 0) && (dm == 0) && (dde == dtn) )
  {
    solution = (abs(dde) <= am);
    
    if (f)
      wxASSERT(solution);
  }
  else
  {
    int t1 = dde + am + bm + cm;
    int t2 = dtn + am - cm + dm;

    // t1, t2 must be even!??
    int g1 = t1 / 2;
    int g2 = t1 / 2 - bm;
    int g3 = -t2 / 2 + dm;
    int g4 = -t2 / 2;
    
    solution = 
      (( t1 % 2) == 0) && ((t2 % 2) == 0) &&   // solution integer
      (                                        // any solution exist
        (                                      // only one solution
          ( (bm + dm) == 0 )     &&
          ( (g1 + g4) % 2 == 0 ) &&
          ( (g1 + g4) >= 0 ) && ( (g1 + g4) <= 2 * cm ) && // 0 <= x1 <= cm
          ( (g1 - g4) >= 0 ) && ( (g1 - g4) <= 2 * am )    // 0 <= x1 <= am
        )                                      // bracket one solution
        ||
        (                                      // more than one solution
          ( (bm + dm) >= 0 )                 && 
          ( g1 >= 0 ) && ( g2 <= (cm + am) ) &&
          ( g3 >= -am ) && ( g4 <= cm )      &&          
          ( (g2 + g4) <= 2 * cm )            &&  // y4 <= cm
          ( (g1 + g3) >= 0 )                 &&  // y3 >= 0
          ( (g2 - g3) <= 2 * am )            &&  // x2 <= am
          ( (g1 - g4) >= 0 )                     // x1 >= 0
        )                                      // bracket more solutions
      );                                       // bracket any solution
      
      if (f)
        wxASSERT(solution);
  }
 
  // tmp Werte wieder restaurieren
  for (int i = 0; i < 5; i++)
    opCounter[i] = tmpOpCounter[i];
  dde = tmpDDE;
  dtn = tmpDTN;    
  
  return solution;
}


bool  DrawCSD::ActualizeDrawParameters(int op)
{
  int absOp = abs(op);
  if (absOp > 4)
    return false;
    
  int sign = (op > 0 ? +1 : (op < 0 ? -1 : 0));
    
  opCounter[absOp]--;
  
  switch (absOp)
  {
    case 1 :  // +, +
      dde += sign;
      dtn += sign;
      break;
      
    case 2 :  // +, 0
      dde += sign;
      break;
      
    case 3 : // +, -
      dde += sign;
      dtn -= sign;
      break;
      
    case 4 : // 0, +
      dtn += sign;
      break;
      
    default :
      break;
  }
  
  return true;
}


bool  DrawCSD::CreateGroups(const CpRec &cp, wxString deStage, wxString quStage)
{
  // pgroups[0] ist Championship, die folgenden die Q-Gruppen
  GrStore gr(connPtr);
  gr.SelectAll(cp, deStage);
  if (!gr.Next())
  {
    // Neu anlegen
    GrStore::CreateGroupStruct  *cgs = new GrStore::CreateGroupStruct;

    cgs->nameTempl = "CP";
    cgs->descTempl = "Competition Proper";
    wxStrcpy(cgs->gr.grStage, deStage);
    cgs->gr.grSize = sizeDGroup;
    cgs->gr.grModus = MOD_SKO;
    cgs->gr.grBestOf = (cp.cpType == CP_SINGLE ? 7 : 5);

    cgs->cp = cp;
    cgs->start = 1;
    cgs->count = 1;
    cgs->connPtr = connPtr; 
    cgs->delConnPtr = false;

    GrStore::CreateGroup(cgs);

    gr.SelectByName("CP", cp);
    gr.Next();
  }
  
  gr.Close();
  
  pgr[0] = gr;
  
  bool drawn = false;
      
  if (pgr[0].QryDraw())
  {
    drawn = true;
    
    if (pgr[0].QryScheduled())
    {
      infoSystem.Error(_("A draw has been performed and scheduled!"));
      return false;
    }
    
    if (!infoSystem.Confirmation(_("A draw has been performed. Continue anyway?")))
      return false;
  }
  
  if (pgr[0].QryStarted())
  {
    infoSystem.Error(_("Group %s has already started!"), pgr[0].grName);
    return false;
  }

  // Qualigruppen noch anlegen / suchen...
  if (!quStage.IsEmpty() && qu > 0)
  {
    if (nofQGroups == 0)
      nofQGroups = (qu / 8) + 1;
      
    int cnt = nofQGroups;
    
    gr.SelectAll(cp, quStage);
    if (gr.Next())
    {
      do
      {
        cnt--;
        pgr[nofQGroups - cnt] = gr;
      }
      while (gr.Next() && cnt);
      
      if (cnt > 0)
      {
        // Zu wenig Q-Gruppen
        infoSystem.Error(_("To few groups for the draw"));
        return false;
      }
      
      for (int idx = 1; idx < nofQGroups; idx++)
      {
        if ( pgr[idx].QryScheduled() && (drawn || pgr[idx].QryDraw()) )
        {
          infoSystem.Error(_("A draw has been performed and scheduled!"));
          return false;
        }
        
        if (pgr[idx].QryStarted())
        {
          infoSystem.Error(_("Group %s has already started!"), pgr[idx].grName);
          return false;
        }
        
        if (!drawn && pgr[idx].QryDraw())
        {
          if (!infoSystem.Confirmation(_("A draw has been performed. Continue anyway?")))
            return false;
            
          drawn = true;
        }
      }
    }
    else
    {
      GrStore::CreateGroupStruct  *cgs = new GrStore::CreateGroupStruct;

      cgs->nameTempl = "Q-";
      cgs->descTempl = "Q-Group";
      wxSprintf(cgs->gr.grStage, quStage);
      cgs->gr.grSize = 8;
			cgs->gr.grModus = MOD_SKO;
      cgs->gr.grBestOf = 5;

      cgs->connPtr = connPtr;
      cgs->cp = cp;
      cgs->start = 1;
      cgs->count = nofQGroups;

      GrStore::CreateGroup(cgs);
      
      gr.SelectAll(cp, quStage);
      while (gr.Next() && cnt)
      {
        cnt--;
        pgr[nofQGroups - cnt] = gr;
      }
    }
  }        
  
  return true;
}


bool  DrawCSD::DistributeEntries()
{
  if (!pgr || !pgr[0].grSize)
    return false;
    
  int lastStage = pgr[0].NofRounds() + 1;
  int lastQStage = lastStage + (nofQGroups > 0 ? pgr[1].NofRounds() : 0);
  
  // Die Manschaften nach Position sortieren
  DrawListTeam listTM;
  
  for (DrawListNation::iterator itNA = listNA.begin();
       itNA != listNA.end(); itNA++)
  {
    DrawItemNation *itemNA = (DrawItemNation *) (*itNA);
    
    for (DrawListTeam::iterator itTM = itemNA->teams.begin();
         itTM != itemNA->teams.end(); itTM++)
    {
      DrawItemTeam *itemTM = (DrawItemTeam *) (*itTM);
      
      itemTM->key = itemTM->pos[lastQStage];
      listTM.AddItem(itemTM);
    }
  }
  
  // Und die Freilose
  for (DrawListTeam::iterator itTM = listBY.begin(); itTM != listBY.end(); itTM++)
  {
    DrawItemTeam *itemTM = (DrawItemTeam *) (*itTM);
      
    itemTM->key = itemTM->pos[lastQStage];
    listTM.AddItem(itemTM);
  }
        
  listTM.Sort();
  
  // Jetzt der Reihe nach abarbeiten
  int qidx = 0;  // Index der aktuellen Q-Gruppe in pgr
  int qpos = 0;  // Position der aktuellen Q-Gruppe in pgr
  
  DrawItemTeam *itemTM;
  
  while ( (itemTM = (DrawItemTeam *) listTM.CutFirst()) )
  {
    if (itemTM->rkDirectEntry || nofQGroups == 0)
    {
      // Also DE, direkt setzen
      pgr[0].SetTeam(itemTM->pos[lastStage], itemTM->tm, StStore::ST_KEEPSEEDED);
    }
    else
    {
      // Qualifier, Q-Gruppe bestimmen
      if (itemTM->pos[lastStage] != qpos)
      {
        qidx++;
        qpos = itemTM->pos[lastStage];
        
        TmEntry tm;
        tm.SetGroup(pgr[qidx], 1);
        pgr[0].SetTeam(qpos, tm, StStore::ST_KEEPSEEDED);
      }
      
      int pos = itemTM->pos[lastStage + pgr[qidx].NofRounds() + 1];
      pos = (pos - 1) % pgr[qidx].grSize;
      pgr[qidx].SetTeam(pos + 1, itemTM->tm, StStore::ST_KEEPSEEDED);
    }
  }
  
  return true; 
}