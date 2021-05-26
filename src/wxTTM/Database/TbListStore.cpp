/* Copyright (C) 2021 Christoph Theis */

#include  "stdafx.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "SQLException.h"
#include  "InfoSystem.h"

#include  "TbListStore.h"


// -----------------------------------------------------------------------
TbListStore::TbListStore(Connection *connPtr) : StoreObj(connPtr)
{
}


void TbListStore::Init()
{
  TbListRec::Init();
}


bool  TbListStore::SelectByGr(const GrRec &gr)
{
  wxString  str = SelectString(gr.grID);
  str += " ORDER BY stID";

  try
  {
    if (!ExecuteQuery(str))
      return false;

    BindRec();
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


bool  TbListStore::SelectByGr(const GrRec &gr, const std::set<long> &ids)
{
  wxString  str = SelectString(gr.grID, ids);
  str += " WHERE stID != 0 ORDER BY stID";

  try
  {
    if (!ExecuteQuery(str))
      return false;

    BindRec();
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


wxString TbListStore::SelectString(long grID) const
{
  return 
    "SELECT mtMatchCount, mtMatchPoints, mtPointsA, mtPointsX, "
    "       mtMatchesA, mtMatchesX, mtSetsA, mtSetsX, mtBallsA, mtBallsX, "
    "       stPos, stID "
    "  FROM TbSortFunc(" + ltostr(grID) + ") "
  ;
}


wxString TbListStore::SelectString(long grID, const std::set<long> &ids) const
{
  return 
    "SELECT mtMatchCount, mtMatchPoints, mtPointsA, mtPointsX, "
    "       mtMatchesA, mtMatchesX, mtSetsA, mtSetsX, mtBallsA, mtBallsX, "
    "       stPos, stID "
    "  FROM TbSortSubsetFunc(" + ltostr(grID) + ", '" + ltostr(ids) + ",') "
  ;
}


bool  TbListStore::BindRec()
{
  int col = 0;

  BindCol(++col, &result.nrMatches);
  BindCol(++col, &result.matchPoints);
  BindCol(++col, &result.points[0]);
  BindCol(++col, &result.points[1]);
  BindCol(++col, &result.matches[0]);
  BindCol(++col, &result.matches[1]);
  BindCol(++col, &result.sets[0]);
  BindCol(++col, &result.sets[1]);
  BindCol(++col, &result.balls[0]);
  BindCol(++col, &result.balls[1]);
  BindCol(++col, &result.pos);
  BindCol(++col, &tbID);

  return true;
}