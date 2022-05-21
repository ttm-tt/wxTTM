/* Copyright (C) 2020 Christoph Theis */

// Drawinf.cpp
// Verwalten der Information fuer Draw

#include "stdafx.h"

#include  "DrawInf.h"

#include  <stdlib.h>
#include  <time.h>

# ifndef TURBO
#   define  random(x)  (rand() % x)
# endif


void CalculateByes(int stg, int sec, int lastStage, 
                   int dbc, int &dbu, int &dbl)
{
  dbu = dbl = 0;
  if (dbc == 0)
    return;
    
  if (stg == 0)
  {
    if (dbc <= 2)
    {
      // oben, unten
      dbu = 1;
      dbl = dbc - dbu;
    }
    else
    {
      dbu = dbc / 2;
      dbl = dbu;
    }
  }
  else if (stg < lastStage)
  {
    if (dbc == 1)
    {
      if ( (sec % 2) )
        dbu = 1;
      else 
        dbu = 0;

      dbl = dbc - dbu;
    }
    else
    {
      dbu = dbc / 2;
      dbl = dbu;
    }
  }
  else
  {
    if (dbc == 1)
    {
      if (sec % 2)
        dbu = 0;
      else
        dbu = 1;    

      dbl = dbc - dbu;
    }
    else
    {
      dbu = dbc / 2;
      if ( (dbc % 2) && !(sec % 2) )
        dbu++;    

      dbl = dbc - dbu;
    }
  }  
}



DrawItem::~DrawItem()
{
}


DrawList::~DrawList()
{
  for (DrawList::iterator it = begin(); it != end(); it++)
    delete (*it);
}


void  DrawList::Shuffle()
{
  int  count = Count();

  // randomize();

  for (DrawList::iterator it = begin(); it != end(); it++)
    (*it)->key = random(count);

  Sort();
}


DrawItem * DrawList::CutFirst()
{
  if (begin() == end())
    return 0;
    
  DrawItem *itemPtr = *begin();
  remove(itemPtr);
  return itemPtr;
}


DrawItem* DrawList::CutLast()
{
  if (begin() == end())
    return 0;
    
  DrawItem *itemPtr = *rbegin();
  remove(itemPtr);
  return itemPtr;
}


// DrawItemPack

DrawItemPack::DrawItemPack()
{
  teams[0] = teams[1] = 0;
}


DrawItemPack::DrawItemPack(DrawItemTeam *item1, DrawItemTeam *item2)
{
  teams[0] = teams[1] = 0;

  Add(item1);
  Add(item2);
}


DrawItemPack::~DrawItemPack()
{
  if (teams[0])
    teams[0]->pack = 0;

  if (teams[1])
    teams[1]->pack = 0;
}


int  DrawItemPack::Add(DrawItemTeam *item)
{
  if (!item)
    return 0;

  if (teams[0] == 0)
    teams[0] = item;
  else if (teams[1] == 0)
    teams[1] = item;
  else
    return -1;

  item->pack = this;
  return 0;
}


// DRAWLIST_TEAM

DrawItemTeam * DrawListTeam::Add(const TmListRec &tm)
{
  if (!tm.tmID)
    return 0;

  DrawItemTeam *item = new DrawItemTeam(tm);
  push_back(item);
  return item;
}


DrawItemTeam * DrawListTeam::Get(const TmListRec &tm)
{
  return GetTeam(tm.tmID);
}


DrawItemTeam * DrawListTeam::GetTeam(long tmID)
{
  if (!tmID)
    return 0;

  for (DrawList::iterator it = begin(); it != end(); it++)
  {
    if ( ((DrawItemTeam *) (*it))->tm.tmID == tmID)
      return (DrawItemTeam *) (*it);
  }

  return 0;
}


DrawItemTeam * DrawListTeam::GetFirst(int stg, int sec)
{
  for (DrawList::iterator it = begin(); it != end(); it++)
  {
    if ( ((DrawItemTeam *) (*it))->pos[stg] == sec)
      return (DrawItemTeam *) (*it);
  }

  return 0;
}


int DrawListTeam::Count(int stage)
{
  if (stage == 0)
    return Count();

  int  count = 0;

  for (DrawList::iterator it = begin(); it != end(); it++)
  {
    if ( ((DrawItemTeam *) (*it))->pos[stage])
      count++;
  }

  return count;
}


int  DrawListTeam::Count(int stage, int sec)
{
  if (stage == 0 && sec == 0)
    return Count();

  int  count = 0;

  for (DrawList::iterator it = begin(); it != end(); it++)
  {
    if ( ((DrawItemTeam *) (*it))->pos[stage] == sec)
      count++;
  }

  return count;
}



// DRAWITEM_TEAM

DrawItemTeam::DrawItemTeam()
{
  lastGroup = 0;
  lastPos = 0;
  rkDirectEntry = 0;
  rkNatlRank = 0;
  rkIntlRank = 0;
  rankPts = 0;
  mask = false;
  pack = 0;
  
  memset(pos, 0, MAX_POS * sizeof(pos[0]));
}


DrawItemTeam::DrawItemTeam(const TmListRec &tm_)
{
  tm = tm_;

  lastGroup = 0;
  lastPos = 0;
  rkDirectEntry = 0;
  rkNatlRank = 0;
  rkIntlRank = 0;
  rankPts = 0;
  mask = false;
  pack = 0;
  
  memset(pos, 0, MAX_POS * sizeof(pos[0]));
}


DrawItemTeam::DrawItemTeam(const StEntry & st_)
{
  tm = st_;

  lastGroup = st_.st.grID;
  lastPos = st_.st.stPos;

  rkDirectEntry = 0;
  rkNatlRank = 0;
  rkIntlRank = 0;
  rankPts = 0;
  mask = false;
  pack = 0;
  
  memset(pos, 0, MAX_POS * sizeof(pos[0]));

}



DrawItemTeam::DrawItemTeam(const RkEntry &rk)
{
  tm = rk;

  lastGroup = 0;
  lastPos = 0;
  rkDirectEntry = rk.rk.rkDirectEntry;
  rkNatlRank = rk.rk.rkNatlRank;
  rkIntlRank = rk.rk.rkIntlRank;
  rankPts = rk.rankPts;
  mask = false;
  pack = 0;

  memset(pos, 0, MAX_POS * sizeof(pos[0]));

}


DrawItemTeam::~DrawItemTeam()
{
  if (pack)
  {
    if (pack->teams[0] == this)
      pack->teams[0] = 0;
    else if (pack->teams[1] == this)
      pack->teams[1] = 0;
  }
}


// DrawListNation

DrawItemNation * DrawListNation::Add(const NaRec &na)
{
  DrawItemNation *item = new DrawItemNation(na);
  push_back(item);

  return item;
}


DrawItemNation * DrawListNation::Get(const NaRec &na)
{
  return GetNation(na.naID);
}


DrawItemNation * DrawListNation::GetNation(long naID)
{
  for (DrawList::iterator it = begin(); it != end(); it++)
  {
    if ( ((DrawItemNation *) (*it))->na.naID == naID)
      return (DrawItemNation *) (*it);
  }

  return 0;
}

DrawItemTeam * DrawListNation::AddTeam(const TmListRec &tm)
{
  if (!tm.tmID)
    return 0;

  DrawItemNation *item = GetNation(tm.naID);
  if (item == 0)
  {
    NaRec na;
    na.naID = tm.naID;
    item = Add(na);
  }

  if (item)
    return item->Add(tm);

  return 0;
}


DrawItemTeam * DrawListNation::AddTeam(const StListRec &st)
{
  TmListRec tmp;
  tmp.tmID = st.tmID;
  tmp.naID = st.naID;

  return AddTeam(tmp);
}


DrawItemTeam * DrawListNation::AddTeam(const RkListRec &rk)
{
  TmListRec tmp;
  tmp.tmID = rk.tmID;
  tmp.naID = rk.naID;
  
  DrawItemTeam *item = AddTeam(tmp);
  
  item->rkDirectEntry = rk.rkDirectEntry != 0;
  item->rkNatlRank    = rk.rkNatlRank;
  item->rkIntlRank    = rk.rkIntlRank;
  item->rankPts       = 0;
  
  return item;
}


DrawItemTeam * DrawListNation::AddTeam(const RkEntry &rk)
{
  DrawItemTeam *item = AddTeam(rk.rk);
  item->rankPts = rk.rankPts;

  return item;
}

DrawItemTeam * DrawListNation::GetTeam(const TmListRec &tm)
{
  if (!tm.tmID)
    return 0;

  DrawItemNation *item = GetNation(tm.naID);

  if (item)
    return item->Get(tm);
  else
    return NULL;
}

DrawItemTeam * DrawListNation::GetTeam(const RkListRec &rk)
{
  if (!rk.tmID)
    return NULL;

  DrawItemNation *item = GetNation(rk.naID);
  if (item)
    return item->GetTeam(rk.tmID);
  else
    return NULL;
}

DrawItemTeam * DrawListNation::GetTeam(long tmId)
{
  DrawItemTeam *ptr;

  for (DrawListNation::iterator it = begin(); it != end(); it++)
  {
    if ( (ptr = ((DrawItemNation *) (*it))->GetTeam(tmId)) )
      return ptr;  
  }

  return 0;
}


DrawItemTeam * DrawListNation::GetTeam(long grID, short pos)
{
  DrawItemTeam *ptr;
  
  for (DrawListNation::iterator it = begin(); it != end(); it++)
  {
    if ( (ptr = ((DrawItemNation *) (*it))->GetTeam(grID, pos)) )
      return ptr;
  }
  
  return 0;
}


DrawItemTeam * DrawListNation::Add(DrawItemTeam *itemTM)
{
  if (!itemTM)
    return NULL;

  DrawItemNation *itemNA = GetNation(itemTM->tm.naID);
  if (itemNA == NULL)
  {
    NaRec na;
    na.naID = itemTM->tm.naID;
    itemNA = Add(na);
  }

  itemNA->Add(itemTM);

  return itemTM;
}


DrawItemTeam * DrawListNation::Subtract(DrawItemTeam *itemTM)
{
  if (!itemTM)
    return NULL;

  DrawItemNation *itemNA = GetNation(itemTM->tm.naID);
  if (itemNA)
    itemNA->Subtract(itemTM);

  return itemTM;
}


int  DrawListNation::Count(int stage, int sec)
{
  int count = 0;

  for (DrawList::iterator it = begin(); it != end(); it++)
  {
    count += ((DrawItemNation *) (*it))->Count(stage, sec);
  }

  return count;
}


// DrawListRegion
DrawListRegion::~DrawListRegion()
{
  for (DrawList::iterator it = begin(); it != end(); it++)
    ((DrawItemNation *) (*it))->teams.clear();
}


// DrawItemNation

DrawItemNation::DrawItemNation(const NaRec &na_)
{
  na = na_;
}


DrawItemTeam * DrawItemNation::GetTeam(long grID, short pos)
{
  for (DrawListTeam::iterator it = teams.begin(); it != teams.end(); ++it)
  {
    if ( ((DrawItemTeam *) (*it))->lastGroup == grID && 
         ((DrawItemTeam *) (*it))->lastPos == pos )
      return (DrawItemTeam *) (*it);         
  }
  
  return 0;
}


