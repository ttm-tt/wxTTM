/* Copyright (C) 2020 Christoph Theis */

// Tabellenberechnung
#include  "stdafx.h"

#include  "TbSort.h"
#include  "TbItem.h"

#include  "GrListStore.h"
#include  "MtStore.h"

#include  "TT32App.h"

#include  <map>
#include  <algorithm>


typedef  std::map<long, TbItem *, std::less<long> >   TbItemMap;


// statische Variable (*Comp)
int  (* TbSort::Comp)(const short *,const short *) = TbSort::Quot;
int  (* TbSort::CompareResult)(TbItem *, TbItem *) = 0;  


// Einsprungspunkt: Tabellenberechnung fuer Gruppe GR
bool  TbSort::Sort(const GrRec &gr, short cpType, 
                   std::vector<TbItem *> &tbList, 
                   std::vector<MtRec> &mtList)
{
  if (tbList.size() == 0)
    return true;
    
  if (CTT32App::instance()->GetTable() == TT_DTTB)
  {
    DttbSort sort;
    sort.DoSort(0, tbList.size()-1, gr, cpType, tbList, mtList);
  }
  else
  {
    IttfSort sort;
    sort.DoSort(0, tbList.size()-1, gr, cpType, tbList, mtList);
  }

  // grWinner draufaddieren
  for (std::vector<TbItem *>::iterator it = tbList.begin();
       it != tbList.end(); it++)
  {
    (*it)->result.pos += gr.grWinner;
  }

  return true;
}


bool TbSort::SortRange(short from, short to, std::vector<TbItem *> &tbList,
                       int (*f)(TbItem *, TbItem *))
{
  CompareResult = f;
  
  // Unter gleichen Positionen sortieren
  for (int first = from, next = first; first <= to; )
  {
    while (next < to)
    {
      if (tbList[first]->result.pos != tbList[next+1]->result.pos)
        break;
        
      next++;
    }
    
    std::sort(tbList.begin() + first, tbList.begin() + next+1, CompareResultFunctional());
    NumberRange(first, next, tbList, f);
    
    first = next+1;
    next = first;
  }
  
  // std::sort(tbList.begin() + from, tbList.begin() + (to+1), CompareResultFunctional());
  // NumberRange(from, to, tbList, f);

  return true;
}


bool  TbSort::NumberRange(short from, short to, std::vector<TbItem *> &tbList, 
                          int (*f)(TbItem *, TbItem *))
{
  tbList[from]->result.pos = from;
  TbItem *tmpPtr = tbList[from];

  short curPos = from;  // Aktuelle Position, die vergeben wird

  for (int i = from+1; i <= to; i++)
  {
    TbItem *tmpPtr = tbList[i];
    // Wenn es einen Unterschied gibt, gilt ab hier eine neue Position
    if ( (*f)(tbList[i], tbList[i-1]) )
      curPos = i;

    tbList[i]->result.pos = curPos;
  }

  return true;
}





// -----------------------------------------------------------------------
// Sortierfunktionen: 
// Entscheidung anhand des Quotienten treffen
int  TbSort::Quot(const short *p1,const short *p2)
{
  // Wenn beide zu 0 gespielt haben,
  // entscheidet der groessere Zaehler
  if (!p1[1] && !p2[1])
	 return (p2[0] - p1[0]);

  // Wenn nur einer zu 0 gespielt hat:
  // Bei 0 : 0 entscheidet Differenz des anderen
  // ansonsten hat dieser gewonnenn (der andere muss schlechter sein)
  if (!p1[1] && p2[1])
  {
	  if (!p1[0])
		  return (p2[0] - p2[1]);
	   else
      return -1;
  }

  if (p1[1] && !p2[1])
  {
    if (!p2[0])
		  return -(p1[0] - p1[1]);
    else
      return 1;
  }

  // Besseres Verhaeltniss
  if ( (float) (p1[0]) / (float) (p1[1]) >
       (float) (p2[0]) / (float) (p2[1]) )
    return -1;

  // Schlechteres Verhaeltniss
  if ( (float) (p1[0]) / (float) (p1[1]) <
       (float) (p2[0]) / (float) (p2[1]) )
	 return 1;

  // Gleichstand
  return 0;
}


// Entscheidung anhand der Differenz
int  TbSort::Diff(const short *p1,const short *p2)
{
  int d2 = p2[0] - p2[1];
  int d1 = p1[0] - p1[1];

  return d2 - d1;
}



// -----------------------------------------------------------------------
// Vergleichskriterien (Zahl gew. Spiele, Punkte, Spiele, Saetze, Baelle)
// positiv, wenn p2 besser ist

int  TbSort::MaxMatchPoints(TbItem *p1, TbItem *p2)
{
  if (p1 == p2)
	  return 0;

  TbItem *pS1 = (TbItem *) p1;
  TbItem *pS2 = (TbItem *) p2;

  // Absteigend nach Zahl gewonnener Spiele
  return  (pS2)->result.matchPoints - (pS1)->result.matchPoints;
}


int  TbSort::MaxPoints(TbItem *p1, TbItem *p2)
{
  if (p1 == p2)
    return 0;

  TbItem *pS1 = (TbItem *) p1;
  TbItem *pS2 = (TbItem *) p2;

  int res;
  // Absteigend nach Zahl gewonnener Spiele
  res =  (short) (pS2)->result.points[0] -
			   (short) (pS1)->result.points[0];
  if (res)
	  return res;

  // Aufsteigend nach Zahl verlorener Spiele
  return (pS1)->result.points[1] - (pS2)->result.points[1];
}


// Die folgenden Funktionen extrahieren nur den Pointer und rufen Comp auf
int  TbSort::CompPoints(TbItem *p1, TbItem *p2)
{
  if (p1 == p2)
	  return 0;

  TbItem *pS1 = (TbItem *) p1;
  TbItem *pS2 = (TbItem *) p2;

  TbEntry::Result *pE1 = &pS1->result;
  TbEntry::Result *pE2 = &pS2->result;

  return (*Comp)(pE1->points, pE2->points);
}


int  TbSort::CompMatches(TbItem *p1, TbItem *p2)
{
  TbItem *pS1 = (TbItem *) p1;
  TbItem *pS2 = (TbItem *) p2;

  TbEntry::Result *pE1 = &pS1->result;
  TbEntry::Result *pE2 = &pS2->result;

  return (*Comp)(pE1->matches, pE2->matches);
}


int  TbSort::CompSets(TbItem *p1, TbItem *p2)
{
  TbItem *pS1 = (TbItem *) p1;
  TbItem *pS2 = (TbItem *) p2;

  TbEntry::Result *pE1 = &pS1->result;
  TbEntry::Result *pE2 = &pS2->result;

  return (*Comp)(pE1->sets, pE2->sets);
}


int  TbSort::CompBalls(TbItem *p1, TbItem *p2)
{
  TbItem *pS1 = (TbItem *) p1;
  TbItem *pS2 = (TbItem *) p2;

  TbEntry::Result *pE1 = &pS1->result;
  TbEntry::Result *pE2 = &pS2->result;

  return (*Comp)(pE1->balls, pE2->balls);
}



// -----------------------------------------------------------------------
bool  IttfSort::SumUp(short from, short to, const GrRec &gr, short cpType,
                    std::vector<TbItem *> &tbList, std::vector<MtRec> &mtList)
{
  int size = tbList.size();
  std::vector<TbItem *>::iterator tmpIt = tbList.begin();

  TbItemMap  tbMap;
  for (int i = from; i <= to; i++)
    tbMap.insert(TbItemMap::value_type(tbList[i]->st.stID, tbList[i]));

  std::vector<MtRec>::iterator it;
  for (it = mtList.begin(); it != mtList.end(); it++)
  {
    // Nur fertige Spiele zaehlen
    if ( !(*it).IsFinished() )
      continue;

    TbItemMap::iterator tbItA = tbMap.find((*it).stA);
    TbItemMap::iterator tbItX = tbMap.find((*it).stX);

    if ( (tbItA == tbMap.end()) || (tbItX == tbMap.end()) )
      continue;

    MtRec  *mtPtr = &(*it);
    TbItem *itemPtrA = tbItA->second;
    TbItem *itemPtrX = tbItX->second;

    // Baelle aufsummieren
    itemPtrA->result.balls[0] += mtPtr->mtBallsA;
    itemPtrA->result.balls[1] += mtPtr->mtBallsX;

    itemPtrX->result.balls[0] += mtPtr->mtBallsX;
    itemPtrX->result.balls[1] += mtPtr->mtBallsA;

    // Saetze aufsummieren
    itemPtrA->result.sets[0] += mtPtr->mtSetsA;
    itemPtrA->result.sets[1] += mtPtr->mtSetsX;

    itemPtrX->result.sets[0] += mtPtr->mtSetsX;
    itemPtrX->result.sets[1] += mtPtr->mtSetsA;

    if (cpType == CP_TEAM)
    {
      // mtRes ist die Zahl der Einzelspiele
      itemPtrA->result.matches[0] += mtPtr->mtResA;
      itemPtrA->result.matches[1] += mtPtr->mtResX;

      itemPtrX->result.matches[0] += mtPtr->mtResX;
      itemPtrX->result.matches[1] += mtPtr->mtResA;

      if (2 * mtPtr->mtResA > mtPtr->mtMatches)
      {
        itemPtrA->result.points[0]++;
        itemPtrX->result.points[1]++;

        itemPtrA->result.matchPoints += 2;
        if (!mtPtr->mtWalkOverX && !mtPtr->mtInjuredX && !mtPtr->mtDisqualifiedX)
          itemPtrX->result.matchPoints++;
      }
      else if (2 * mtPtr->mtResX > mtPtr->mtMatches)
      {
        itemPtrX->result.points[0]++;
        itemPtrA->result.points[1]++;

        itemPtrX->result.matchPoints += 2;
        if (!mtPtr->mtWalkOverA && !mtPtr->mtInjuredA && !mtPtr->mtDisqualifiedA)
          itemPtrA->result.matchPoints++;
      }
    }
    else
    {
      if (2 * mtPtr->mtResA > mtPtr->mtBestOf)
      {
        itemPtrA->result.matches[0]++;
        itemPtrX->result.matches[1]++;

        itemPtrA->result.points[0]++;
        itemPtrX->result.points[1]++;

        itemPtrA->result.matchPoints += 2;
        if (!mtPtr->mtWalkOverX && !mtPtr->mtInjuredX && !mtPtr->mtDisqualifiedX)
          itemPtrX->result.matchPoints++;
      }
      else if (2 * mtPtr->mtResX > mtPtr->mtBestOf)
      {
        itemPtrX->result.matches[0]++;
        itemPtrA->result.matches[1]++;

        itemPtrX->result.points[0]++;
        itemPtrA->result.points[1]++;

        itemPtrX->result.matchPoints += 2;
        if (!mtPtr->mtWalkOverA && !mtPtr->mtInjuredA && !mtPtr->mtDisqualifiedA)
          itemPtrA->result.matchPoints++;
      }
    }

    // Das Spiel ist auf jeden Fall fertig, also kann man die 
    // Punkte immer hochzaehlen
    itemPtrA->result.nrMatches++;
    itemPtrX->result.nrMatches++;    
  } // for mt

  return true;
}


bool  IttfSort::DoSort(short from, short to, const GrRec &gr, short cpType, 
                       std::vector<TbItem *> &tbList, std::vector<MtRec> &mtList)
{
  SumUp(from, to, gr, cpType, tbList, mtList);

  TbSort::CompQuot();
  SortRange(from, to, tbList, TbSort::MaxMatchPoints);

  // Wurde gespielt? dann hat Sieger matchPoints
  if (!tbList[from]->result.matchPoints)
    return true;

  // Direkter Vergleich?
  for (int first = from, next = first; first < to; )
  {
    while (next < to && tbList[first]->result.pos == tbList[next+1]->result.pos)
      next++;

    if (next > from)
      DirectComp(first, next, gr, cpType, tbList, mtList);

    first = next+1;
    next = first;
  }

  return true;
}


bool  IttfSort::DirectComp(short from, short to, const GrRec &gr, short cpType,
                           std::vector<TbItem *> &tbList, std::vector<MtRec> &mtList)
{
  if (from == to)
    return true;

  wxASSERT(from < (short) tbList.size());
  wxASSERT(to <= (short) tbList.size());

  std::vector<TbItem *>::iterator it;

  // Ergebnisse sichern
  for (it = tbList.begin() + from; it <= tbList.begin() + to; it++)
    (*it)->PushResult();

  // Aufaddieren der Ergebnisse im direkten Vergleich
  SumUp(from, to, gr, cpType, tbList, mtList);

  // Vergleich der Quotienten
  TbSort::CompQuot();

  // Sukzessive Vergleichen (matchPoints faellt aus) (doch nicht)
  SortRange(from, to, tbList, MaxMatchPoints);

  // Vergleich von Spielen nur bei Mannschaften
  if (tbList[from]->result.pos == tbList[to]->result.pos)
  {
    if (cpType == CP_TEAM)
      SortRange(from, to, tbList, CompMatches);
  }

  if (tbList[from]->result.pos == tbList[to]->result.pos)
    SortRange(from, to, tbList, CompSets);

  if (tbList[from]->result.pos == tbList[to]->result.pos)
    SortRange(from, to, tbList, CompBalls);

  // Ein weiterer Direkter Vergleich nur,
  // wenn sich Spieler trennen liessen
  if (tbList[from]->result.pos != tbList[to]->result.pos)
  {
    for (int first = from, next = first; first < to; )
    {
      while (next < to && tbList[first]->result.pos == tbList[next+1]->result.pos)
        next++;

      if (next > from)
        DirectComp(first, next, gr, cpType, tbList, mtList);

      first = next+1;
      next = first;
    }
  } // end if (weitere direkter Vergleich)

  // Alte Ergebnisse wieder eintragen (ausser Pos)
  for (it = tbList.begin() + from; it <= tbList.begin() + to; it++)
  {
    int pos = (*it)->result.pos;
    (*it)->PopResult();
    (*it)->result.pos = pos;
  }

  return true;
}


// -----------------------------------------------------------------------
bool  DttbSort::SumUp(short from, short to, const GrRec &gr, short cpType,
                    std::vector<TbItem *> &tbList, std::vector<MtRec> &mtList)
{
  int size = tbList.size();
  std::vector<TbItem *>::iterator tmpIt = tbList.begin();

  TbItemMap  tbMap;
  for (int i = from; i <= to; i++)
    tbMap.insert(TbItemMap::value_type(tbList[i]->st.stID, tbList[i]));

  std::vector<MtRec>::iterator it;
  for (it = mtList.begin(); it != mtList.end(); it++)
  {
    // Nur fertige Spiele zaehlen
    if ( !(*it).IsFinished() )
      continue;

    TbItemMap::iterator tbItA = tbMap.find((*it).stA);
    TbItemMap::iterator tbItX = tbMap.find((*it).stX);

    if ( (tbItA == tbMap.end()) || (tbItX == tbMap.end()) )
      continue;

    MtRec  *mtPtr = &(*it);
    TbItem *itemPtrA = tbItA->second;
    TbItem *itemPtrX = tbItX->second;

    // Baelle aufsummieren
    itemPtrA->result.balls[0] += mtPtr->mtBallsA;
    itemPtrA->result.balls[1] += mtPtr->mtBallsX;

    itemPtrX->result.balls[0] += mtPtr->mtBallsX;
    itemPtrX->result.balls[1] += mtPtr->mtBallsA;

    // Saetze aufsummieren
    itemPtrA->result.sets[0] += mtPtr->mtSetsA;
    itemPtrA->result.sets[1] += mtPtr->mtSetsX;

    itemPtrX->result.sets[0] += mtPtr->mtSetsX;
    itemPtrX->result.sets[1] += mtPtr->mtSetsA;

    if (cpType == CP_TEAM)
    {
      // mtRes ist die Zahl der Einzelspiele
      itemPtrA->result.matches[0] += mtPtr->mtResA;
      itemPtrA->result.matches[1] += mtPtr->mtResX;

      itemPtrX->result.matches[0] += mtPtr->mtResX;
      itemPtrX->result.matches[1] += mtPtr->mtResA;

      if (2 * mtPtr->mtResA > mtPtr->mtMatches)
      {
        itemPtrA->result.points[0] += 2;
        itemPtrX->result.points[1] += 2;
      }
      else if (2 * mtPtr->mtResX > mtPtr->mtMatches)
      {
        itemPtrA->result.points[1] += 2;
        itemPtrX->result.points[0] += 2;
      }
      else
      {
        itemPtrA->result.points[0]++;
        itemPtrA->result.points[1]++;
        
        itemPtrX->result.points[0]++;
        itemPtrX->result.points[1]++;
      }
    }
    else
    {
      if (2 * mtPtr->mtResA > mtPtr->mtBestOf)
      {
        itemPtrA->result.matches[0]++;
        itemPtrX->result.matches[1]++;

        itemPtrA->result.points[0]++;
        itemPtrX->result.points[1]++;
      }
      else if (2 * mtPtr->mtResX > mtPtr->mtBestOf)
      {
        itemPtrX->result.matches[0]++;
        itemPtrA->result.matches[1]++;

        itemPtrX->result.points[0]++;
        itemPtrA->result.points[1]++;
      }
    }

    // Das Spiel ist auf jeden Fall fertig, also kann man die 
    // Punkte immer hochzaehlen
    itemPtrA->result.nrMatches++;
    itemPtrX->result.nrMatches++;    
  } // for mt

  return true;
}


bool  DttbSort::DoSort(short from, short to, const GrRec &gr, short cpType, 
                       std::vector<TbItem *> &tbList, std::vector<MtRec> &mtList)
{
  SumUp(from, to, gr, cpType, tbList, mtList);
  
	// Mannschaften: Punktverhaeltnis, Differenz gew. Spiele, Saetze, Baelle
	if (cpType == CP_TEAM)
	{
		// Punktverhaeltnis
		TbSort::CompQuot();
		SortRange(from, to, tbList, CompPoints);

		// Spieldifferenz
		TbSort::CompDiff();
		SortRange(from, to, tbList, CompMatches);

		// Satzdifferenz
		TbSort::CompDiff();
		SortRange(from, to, tbList, CompSets);

		// Balldifferenz
		TbSort::CompDiff();
		SortRange(from, to, tbList, CompBalls);
	} // Mannschaften
	else
	{
		// Spieldifferenz
		TbSort::CompDiff();
		SortRange(from, to, tbList, CompPoints);

		// Satzdifferenz
		TbSort::CompDiff();
		SortRange(from, to, tbList, CompSets);

		// Direkter Vergleich?
		for (int first = from, next = first; first < to; )
		{
			while (next < to && tbList[first]->result.pos == tbList[next+1]->result.pos)
				next++;

			if (next > first)
        DirectComp(first, next, gr, cpType, tbList, mtList);

			first = next+1;
			next = first;
		}		
	} // Einzel / Doppel

  return true;
}


// Direkter Vergleich
bool  DttbSort::DirectComp(short from, short to, const GrRec &gr, short cpType,
                           std::vector<TbItem *> &tbList, std::vector<MtRec> &mtList)
{
	// Kein direkter Vergleich in Mannschaften
	if (cpType == CP_TEAM)
		return true;

  if (from == to)
    return true;

  wxASSERT(from < (short) tbList.size());
  wxASSERT(to <= (short) tbList.size());

  std::vector<TbItem *>::iterator it;

  // Ergebnisse sichern
  for (it = tbList.begin() + from; it <= tbList.begin() + to; it++)
    (*it)->PushResult();

	// Aufaddieren der Ergebnisse im direkten Vergleich
  SumUp(from, to, gr, cpType, tbList, mtList);

	// Vergleich der Differenz Punkte, Saetze, Baelle
	TbSort::CompDiff();
	SortRange(from, to, tbList, CompPoints);

	// Differenz Saetze
	TbSort::CompDiff();
	SortRange(from, to, tbList, CompSets);

	// Differenz Baelle
	TbSort::CompDiff();
	SortRange(from, to, tbList, CompBalls);

  // Alte Ergebnisse wieder eintragen (ausser Pos)
  for (it = tbList.begin() + from; it <= tbList.begin() + to; it++)
  {
    int pos = (*it)->result.pos;
    (*it)->PopResult();
    (*it)->result.pos = pos;
  }

	return true;
}


