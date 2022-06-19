/* Copyright (C) 2020 Christoph Theis */

// Liste der Spiele
#include  "stdafx.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "SQLException.h"
#include  "InfoSystem.h"

#include  "MtListStore.h"
#include  "GrListStore.h"
#include  "TmListStore.h"


// ------------------------------------------------------------------------
bool  MtListStore::CreateView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str;
  
  try
  {
    str = "CREATE VIEW MtList                                                "
          " (mtID, mtNr, mtRound, mtMatch, mtChance, grID, mtMS, mtSet,      "
          "  mtDateTime, mtTable, stA, stX, tmA, tmX,                        "
          "  mtMatches, mtBestOf, mtUmpire, mtUmpire2,                       "
          "  mtPrinted, mtChecked, mtTimestamp,                              "
          "  cpID, cpName, cpType, grName, grModus, grSize, grWinner,        "
          "  grQualRounds, grNofRounds, grNofMatches,                        "
          "  syComplete,                                                     "
          "  mtResA, mtResX, mtSetsA, mtSetsX, mtBallsA, mtBallsX, mtReverse, "
          "  mtWalkOverA, mtWalkOverX,                                       "
          "  mtInjuredA, mtInjuredX,                                         "
          "  mtDisqualifiedA, mtDisqualifiedX)                               "
          "AS SELECT mt.mtID, mt.mtNr,                                       "
          "          mt.mtRound, mt.mtMatch, mt.mtChance, mt.grID,           "
          "          mtMatch.mtMS, mtSet.mtSet,                              "
          "          mt.mtDateTime, mt.mtTable,                              "
          "          mt.stA, mt.stX, stA.tmID, stX.tmID,                     "
          "          mt.mtMatches, mt.mtBestOf, mt.mtUmpire, mtUmpire2,      "
          "          mt.mtPrinted, mt.mtChecked, mt.mtTimestamp,             "
          "          cp.cpID, cp.cpName, cp.cpType,                          "
          "          gr.grName, gr.grModus, gr.grSize, gr.grWinner,          "
          "          gr.grQualRounds, gr.grNofRounds, gr.grNofMatches,       "
          "          sy.syComplete,                                          "
          "          mt.mtResA, mt.mtResX, mtMatch.mtResA, MtMatch.mtResX,   "
          "          MtSet.mtResA, MtSet.mtResX, mt.mtReverse,               "
          "          mt.mtWalkOverA, mt.mtWalkOverX,                         "
          "          mt.mtInjuredA, mt.mtInjuredX,                           "
          "          mt.mtDisqualifiedA, mt.mtDisqualifiedX                  "
          "     FROM MtRec mt                                                "
          "          INNER JOIN GrRec gr ON mt.grID = gr.grID                "
          "          INNER JOIN CpRec cp ON gr.cpID = cp.cpID                "
          "          LEFT OUTER JOIN SyRec sy ON gr.syID = sy.syID           "
          "          LEFT OUTER JOIN StRec stA ON mt.stA = stA.stID          "
          "          LEFT OUTER JOIN StRec stX ON mt.stX = stX.stID          "
          "          LEFT OUTER JOIN MtMatch ON mt.mtID = MtMatch.mtID       "
//          "                      AND MtMatch.mtMS = 0                      "
          "          LEFT OUTER JOIN MtSet ON MtMatch.mtID = MtSet.mtID      "
          "                               AND MtMatch.mtMS = MtSet.mtMS      "
//          "                      AND MtSet.mtMS = 0 AND MtSet.mtSet = 0      "
         ;

    tmp->ExecuteUpdate(str);
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  delete tmp;
  
  return true;
}


bool  MtListStore::RemoveView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  try
  {
    tmp->ExecuteUpdate("DROP VIEW MtList");
  }
  catch(SQLException &)
  {
  }

  delete tmp;
  
  return true;
}


// -----------------------------------------------------------------------
MtListStore::MtListStore(Connection *ptr)
            : StoreObj(ptr)
{
}


void  MtListStore::Init()
{
  MtListRec::Init();
}


bool  MtListStore::Next()
{
  if (!StoreObj::Next())
    return false;

  // When all team matches have to be played and we are in a RR group, set the flag in MtRec
  mtComplete = syComplete && cpType == 4 && grModus == 1;
  return true;
}


// -----------------------------------------------------------------------
bool  MtListStore::SelectById(long id)
{
  wxString  str = SelectString();
  str += "WHERE (mtMS IS NULL OR mtMS = 0) "
         "  AND (mtSet IS NULL OR mtSet = 0) "  
         "  AND mt.mtID = " + ltostr(id);

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


// -----------------------------------------------------------------------
bool  MtListStore::SelectByNr(long nr)
{
  wxString  str = SelectString();
  str += "WHERE (mtMS IS NULL OR mtMS = 0) "
         "  AND (mtSet IS NULL OR mtSet = 0) "
         "  AND mt.mtNr = " + ltostr(nr);

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


// -----------------------------------------------------------------------
bool  MtListStore::SelectByGr(const GrRec &gr)
{
  wxString  str = SelectString();
  str += "WHERE (mtMS IS NULL OR mtMS = 0) "
         "  AND (mtSet IS NULL OR mtSet = 0) "
         "  AND mt.grID = " + ltostr(gr.grID);

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


bool  MtListStore::SelectByGrTm(const GrRec &gr, const TmRec &tm)
{
  wxString  str = SelectString();
  str += "WHERE (mtMS IS NULL OR mtMS = 0) "
         "  AND (mtSet IS NULL OR mtSet = 0) "
         "  AND mt.grID = " + ltostr(gr.grID) + " "
         "  AND (mt.tmA = " + ltostr(tm.tmID) + " OR mt.tmX = " + ltostr(tm.tmID) + ") "
         "ORDER BY mtDateTime, mtTable, mtRound, mtMatch";

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


bool  MtListStore::SelectByRound(const GrRec &gr, short round)
{
  wxString  str = SelectString();
  str += 
      " WHERE (mtMS IS NULL OR mtMS = 0) "
      " AND (mtSet IS NULL OR mtSet = 0) "
      " AND mt.grID = " + ltostr(gr.grID) +
      " AND mt.mtRound = " + ltostr(round) +
      " ORDER BY mtMatch ";

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


// -----------------------------------------------------------------------
bool  MtListStore::SelectByEvent(const MtEvent &event)
{
  // XXX: PreparedStatement
  wxString  str = SelectString();
  str += " WHERE mt.grID  = " + ltostr(event.grID) +
         "   AND mtRound  = " + ltostr(event.mtRound) + 
         "   AND mtMatch  = " + ltostr(event.mtMatch) +
         "   AND mtChance = " + ltostr(event.mtChance) +
         "   AND (mtMS IS NULL OR mtMS = 0) "+
         "   AND (mtSet IS NULL OR mtSet = 0)";

  try
  {
    ExecuteQuery(str);
    BindRec();
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


// -----------------------------------------------------------------------
bool  MtListStore::SelectByTime(const timestamp &fromTime, short fromTable,
                                const timestamp &toTime, short toTable, 
                                bool includeNoTable)
{
  wxString  str = SelectString();

  // Where-Bedingung
  str += " WHERE (mtMS IS NULL OR mtMS = 0) "
         "   AND (mtSet IS NULL OR mtSet = 0) ";
         
  if (fromTime.year == 0 && toTime.year == 0)
    str += " AND mtDateTime IS NULL ";
  else
  {
    if (fromTime.year > 0)
      str += " AND mtDateTime >= '" + tstostr(fromTime) + "'";
    
    if (toTime.year > 0)
      str += " AND mtDateTime <= '" + tstostr(toTime)   + "'";
  }

  if (includeNoTable)
    str += " AND (mtTable = 0 OR (mtTable > 0 ";
    
  if (fromTable > 0)
    str += "   AND mtTable >= " + ltostr(fromTable);
    
  if (toTable > 0)
    str += "   AND mtTable <= " + ltostr(toTable);

  if (includeNoTable)
    str += ") ) ";

  // Order by
  str += " ORDER BY mtDateTime, mtTable, mt.grID, mtRound, mtMatch";

  try
  {
    ExecuteQuery(str);
    BindRec();
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


bool MtListStore::SelectByTimestamp(const timestamp &ts)
{
  wxString  str = SelectString();

  str += " WHERE mtTimestamp > '" + tstostr(ts) + "'";

  // Order by
  str += " ORDER BY mtDateTime, mtTable, mt.grID, mtRound, mtMatch";

  try
  {
    ExecuteQuery(str);
    BindRec();
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


// -----------------------------------------------------------------------
std::list<timestamp> MtListStore::ListVenueDays()
{
  std::list<timestamp> tsList;
  timestamp ts;
  long year, month, day;
  
  memset(&ts, 0, sizeof(timestamp));
  
  wxString str = 
      "SELECT DISTINCT YEAR(mtDateTime), MONTH(mtDateTime), DAY(mtDateTime)"
      "  FROM MtList mt INNER JOIN GrList gr ON mt.grID = gr.grID "
	  " WHERE (gr.grNofRounds = 0 OR gr.grNofRounds >= mt.mtRound) AND "
	  "       (gr.grNofMatches = 0 OR gr.grNofMatches / POWER(2, mt.mtRound - 1) >= mt.mtMatch) AND "
	  "       (stA IS NULL OR tmA <> 0) AND (stX IS NULL OR tmX <> 0) "
	  "    OR DAY(mtDateTime) <> 0 "
      " ORDER BY 1, 2, 3";
      
  try
  {
    ExecuteQuery(str);
    BindCol(1, &year);
    BindCol(2, &month);
    BindCol(3, &day);
    
    while (Next())
    {
      if (WasNull(1))
      {
        ts.year = 0;
        ts.month = 0;
        ts.day = 0;
      }
      else
      {
        ts.year = year;
        ts.month = month;
        ts.day = day;
      }
      
      tsList.push_back(ts);
    }
    
    Close();
  }    
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
  }
    
  return tsList;
}


// -----------------------------------------------------------------------
timestamp MtListStore::GetLastUpdateTime()
{
  wxString str = "SELECT MAX(mtTimestamp) FROM MtList";
  timestamp ts;

  memset(&ts, 0, sizeof(ts));

  try
  {
    ExecuteQuery(str);
    BindCol(1, &ts);
    Next();
    Close();
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  return ts;
}


// -----------------------------------------------------------------------
std::list<std::tuple<long, short, timestamp>> MtListStore::GetFinishedRounds(Finished what)
{
  std::list<std::tuple<long, short, timestamp>> ret;
  PreparedStatement *stmtPtr = 0;

  wxString sql = 
    "SELECT DISTINCT cp.cpID, gr.grStage, gr.grID, mt.mtRound, mt.mtDateTime, gr.grPrinted "
    " FROM MtList mt INNER JOIN GrList gr on mt.grID = gr.grID "
    "      INNER JOIN CpList cp ON gr.cpID = cp.cpID ";

  // Select all checked (finished) matches where no corresponding match today is not checked (finished) and no bye involved
  sql += 
    "WHERE CAST(mtDateTime AS DATE) = CAST(CURRENT_TIMESTAMP AS DATE) "
    "  AND mt.mtChecked <> 0 "
    "AND NOT EXISTS ("
    "  SELECT * FROM MtList INNER JOIN GrList ON MtList.grID = GrList.grID INNER JOIN CpList ON GrList.cpID = CpList.cpID " 
    "   WHERE MtList.mtID IS NOT NULL "
    "     AND CAST(mtDateTime AS DATE) = CAST(mt.mtDateTime AS DATE) "
    "     AND MtList.mtChecked = 0 AND MtList.tmA IS NOT NULL AND MtList.tmX IS NOT NULL "
  ;

  // Fall through intended
  switch (what)
  {
    case Round :
      sql += " AND mt.mtRound = MtList.mtRound ";
    case Group:
      sql += " AND gr.grID = GrList.grID ";
    case Stage:
      sql += " AND gr.grStage = GrList.grStage ";
    case Event:
      sql += " AND cp.cpID = CpList.cpID ";
  }

  sql += ") ";

  sql += "ORDER BY cp.cpID, gr.grStage, gr.grID, mt.mtRound, mt.mtDateTime, gr.grPrinted "
  ;

  long cpID;
  wxChar grStage[65];
  long grID;
  short mtRound;
  timestamp mtDateTime;

  try
  {
    ExecuteQuery(sql);

    BindCol(1, &cpID);
    BindCol(2, grStage, sizeof(grStage));
    BindCol(3, &grID);
    BindCol(4, &mtRound);
    BindCol(5, &mtDateTime);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(sql, e);
  }

  std::tuple<long, short, timestamp> lastTuple;

  long lastCpID = 0;
  wxString lastGrStage;
  long lastGrID = 0;
  short lastMtRound = 0;

  while (Next())
  {
    std::tuple<long, short, timestamp> tuple(grID, mtRound, mtDateTime);

    bool changed = false;
    switch (what)
    {
      // Fall through intended
      case Round :
        // We show the last round played
        // changed |= lastMtRound != mtRound;
      case Group :
        changed |= lastGrID != grID;
      case Stage :
        changed |= lastGrStage != grStage;
      case Event :
        changed |= lastCpID != cpID;
    }

    if (!changed)
      ret.pop_back();

    ret.push_back(tuple);

    lastTuple = tuple;
    lastMtRound = mtRound;
    lastGrID = grID;
    lastGrStage = grStage;
    lastCpID = cpID;
  }

  return ret;
}

// -----------------------------------------------------------------------
wxString  MtListStore::SelectString() const
{
  wxString  str;

  str = "SELECT mtID, mtNr, mtRound, mtMatch, mtChance, mtMS, mtSet,  "
        "       mtDateTime, mtTable, stA, stX, mt.grID,               "
        "       mtResA, mtResX, mtSetsA, mtSetsX, mtBallsA, mtBallsX, "
        "       mtReverse, mtWalkOverA, mtWalkOverX,                  "
        "       mtInjuredA, mtInjuredX, mtDisqualifiedA, mtDisqualifiedX, "
        "       cpName, cpType, grName, grModus, grSize, grWinner,    "
        "       grQualRounds, grNofRounds, grNofMatches,              "
        "       syComplete,                                           "
        "       tmA, tmX, xxA.stID, xxX.stID,                         "
        "       mtMatches, mtBestOf, mtUmpire, mtUmpire2,             "
        "       mtPrinted, mtChecked   "
        "FROM   MtList mt "
        "       LEFT OUTER JOIN XxRec xxA ON stA = xxA.stID  "
        "       LEFT OUTER JOIN XxRec xxX ON stX = xxX.stID  ";  

  return str;
}


bool  MtListStore::BindRec()
{
  int col = 0;

  BindCol(++col, &mtID);
  BindCol(++col, &mtNr);
  BindCol(++col, &mtEvent.mtRound);
  BindCol(++col, &mtEvent.mtMatch);
  BindCol(++col, &mtEvent.mtChance);
  BindCol(++col, &mtEvent.mtMS);
  BindCol(++col, &mtSet);
  BindCol(++col, &mtPlace.mtDateTime);
  BindCol(++col, &mtPlace.mtTable);

  BindCol(++col, &stA);
  BindCol(++col, &stX);

  BindCol(++col, &mtEvent.grID);

  BindCol(++col, &mtResA);
  BindCol(++col, &mtResX);
  BindCol(++col, &mtSetsA);
  BindCol(++col, &mtSetsX);
  BindCol(++col, &mtBallsA);
  BindCol(++col, &mtBallsX);

  BindCol(++col, &mtReverse);
    
  BindCol(++col, &mtWalkOverA);
  BindCol(++col, &mtWalkOverX);
  BindCol(++col, &mtInjuredA);
  BindCol(++col, &mtInjuredX);
  BindCol(++col, &mtDisqualifiedA);
  BindCol(++col, &mtDisqualifiedX);

  BindCol(++col, cpName, sizeof(cpName));
  BindCol(++col, &cpType);
  BindCol(++col, grName, sizeof(grName));
  BindCol(++col, &grModus);
  BindCol(++col, &grSize);
  BindCol(++col, &grWinner);
  BindCol(++col, &grQualRounds);
  BindCol(++col, &grNofRounds);
  BindCol(++col, &grNofMatches);

  BindCol(++col, &syComplete);

  BindCol(++col, &tmA);
  BindCol(++col, &tmX);

  BindCol(++col, &xxAstID);
  BindCol(++col, &xxXstID);

  BindCol(++col, &mtMatches);
  BindCol(++col, &mtBestOf);
  BindCol(++col, &mtUmpire);
  BindCol(++col, &mtUmpire2);
  BindCol(++col, &mtScorePrinted);
  BindCol(++col, &mtScoreChecked);

  return true;
}
