/* Copyright (C) 2020 Christoph Theis */

// SEtzung
#include  "stdafx.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "SQLException.h"
#include  "InfoSystem.h"

#include  "MtEntryStore.h"

#include  "MtStore.h"
#include  "GrListStore.h"

#include  "Rec.h"



// -----------------------------------------------------------------------
bool  MtEntryStore::CreateView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str;
  
  // TODO: mtWalkOverA, mtWalkOverX, naDesc beruecksichtigen

  try
  {
    str = "CREATE VIEW MtSingleList                                             "
          " (mtID, mtNr, mtRound, mtMatch, mtChance, grID,                      "
          "  mtDateTime, mtTable, mtResA, mtResX, mtWalkOverA, mtWalkOverX,     "
          "  mtInjuredA, mtInjuredX, mtDisqualifiedA, mtDisqualifiedX,          "
          "  plAplID, plAplNr, plApsLast, plApsFirst, plAnaID, plAnaName, plAnaDesc, plAnaRegion, plAplExtID,  "
          "  stA, tmAtmID, tmAnaID,                                             "
          "  plXplID, plXplNr, plXpsLast, plXpsFirst, plXnaID, plXnaName, plXnaDesc, plXnaRegion, plXplExtID,  "
          "  stX, tmXtmID, tmXnaID,                                             "
          "  mtMatches, mtBestOf, mtUmpire, mtUmpire2,                          "
          "  mtPrinted, mtChecked, mtTimestamp)  "
          "AS SELECT mt.mtID, mt.mtNr,                                          "
          "          mt.mtRound, mt.mtMatch, mt.mtChance, mt.grID,              "
          "          mt.mtDateTime, mt.mtTable, mt.mtResA, mt.mtResX,           "
          "          mt.mtWalkOverA, mt.mtWalkOverX,                            "
          "          mt.mtInjuredA, mt.mtInjuredX,                              "
          "          mt.mtDisqualifiedA, mt.mtDisqualifiedX,                    "
          "          sta.plID, sta.plNr, sta.psLast, sta.psFirst, sta.naID, sta.naName, sta.naDesc, sta.naRegion, sta.plExtID, "
          "          mt.stA, sta.tmID, sta.tmnaID,                              "
          "          stx.plID, stx.plNr, stx.psLast, stx.psFirst, stx.naID, stx.naName, stx.naDesc, stx.naRegion, stx.plExtID, "
          "          mt.stX, stx.tmID, stx.tmnaID,                              "
          "          mt.mtMatches, mt.mtBestOf, mt.mtUmpire, mt.mtUmpire2,      "
          "          mt.mtPrinted, mtChecked, "
          "          mt.mtTimestamp                                             "
          "     FROM MtRec mt                                                   "
          "             LEFT OUTER JOIN StSingleList sta ON mt.sta = sta.stID   "
          "             LEFT OUTER JOIN StSingleList stx ON mt.stx = stx.stID   ";
    tmp->ExecuteUpdate(str);

    str = "CREATE VIEW MtDoubleList                             "
          " (mtID, mtNr, mtRound, mtMatch, mtChance, grID,      "
          "  mtDateTime, mtTable, mtResA, mtResX,               "
          "  mtWalkOverA, mtWalkOverX,                          "
          "  mtInjuredA, mtInjuredX, mtDisqualifiedA, mtDisqualifiedX,          "
          "  plAplID, plAplNr, plApsLast, plApsFirst, plAnaID, plAnaName, plAnaDesc, plAnaRegion, plAplExtID, "
          "  plBplID, plBplNr, plBpsLast, plBpsFirst, plBnaID, plBnaName, plBnaDesc, plBnaRegion, plBplExtID, "
          "  stA, tmAtmID, tmAnaID,                             "
          "  plXplID, plXplNr, plXpsLast, plXpsFirst, plXnaID, plXnaName, plXnaDesc, plXnaRegion, plXplExtID, "
          "  plYplID, plYplNr, plYpsLast, plYpsFirst, plYnaID, plYnaName, plYnaDesc, plYnaRegion, plYplExtID, "
          "  stX, tmXtmID, tmXnaID,                             "
          "  mtMatches, mtBestOf, mtUmpire, mtUmpire2, mtPrinted, mtChecked, mtTimestamp)  "
          "AS SELECT mt.mtID, mt.mtNr,                                          "
          "          mt.mtRound, mt.mtMatch, mt.mtChance, mt.grID,              "
          "          mt.mtDateTime, mt.mtTable, mt.mtResA, mt.mtResX,           "
          "          mt.mtWalkOverA, mt.mtWalkOverX,                            "
          "          mt.mtInjuredA, mt.mtInjuredX,                              "
          "          mt.mtDisqualifiedA, mt.mtDisqualifiedX,                    "
          "          sta.plplID, sta.plplNr, sta.plpsLast, sta.plpsFirst,                   "
          "          sta.plnaID, sta.plnaName, sta.plnaDesc, sta.plnaRegion, sta.plplExtID,     "
          "          sta.bdplID, sta.bdplNr, sta.bdpsLast, sta.bdpsFirst,                   "
          "          sta.bdnaID, sta.bdnaName, sta.bdnaDesc, sta.bdnaRegion, sta.bdplExtID,     "
          "          mt.stA, sta.tmID, sta.tmnaID,                              "
          "          stx.plplID, stx.plplNr, stx.plpsLast, stx.plpsFirst,                   "
          "          stx.plnaID, stx.plnaName, stx.plnaDesc, stx.plnaRegion, stx.plplExtID,     "
          "          stx.bdplID, stx.bdplNr, stx.bdpsLast, stx.bdpsFirst,                   "
          "          stx.bdnaID, stx.bdnaName, stx.bdnaDesc, stx.bdnaRegion, stx.bdplExtID,      "
          "          mt.stX, stx.tmID, stx.tmnaID,                              "
          "          mt.mtMatches, mt.mtBestOf, mt.mtUmpire, mt.mtUmpire2, mt.mtPrinted, mtChecked, "
          "          mt.mtTimestamp                                             "
          "     FROM MtRec mt                                                   "
          "             LEFT OUTER JOIN StDoubleList sta ON mt.sta = sta.stID   "
          "             LEFT OUTER JOIN StDoubleList stx ON mt.stx = stx.stID   ";

    tmp->ExecuteUpdate(str);

    str = "CREATE VIEW MtTeamList                                                   "
          " (mtID, mtNr, mtRound, mtMatch, mtChance, grID,                          "
          "  mtDateTime, mtTable, mtResA, mtResX, mtReverse,                        "
          "  mtWalkOverA, mtWalkOverX,                                              "
          "  mtInjuredA, mtInjuredX, mtDisqualifiedA, mtDisqualifiedX,          "
          "  tmAtmName, tmAtmDesc, tmAnaName, tmAnaDesc, tmAnaRegion, stA, tmAtmID, tmAnaID, "
          "  tmXtmName, tmXtmDesc, tmXnaName, tmXnaDesc, tmXnaRegion, stX, tmXtmID, tmXnaID, "
          "  mtMatches, mtBestOf, mtUmpire, mtUmpire2, mtPrinted, mtChecked, mtTimestamp)      "
          "AS SELECT mt.mtID, mt.mtNr,                                              "
          "          mt.mtRound, mt.mtMatch, mt.mtChance, mt.grID,                  "
          "          mt.mtDateTime, mt.mtTable, mt.mtResA, mt.mtResX, mt.mtReverse, "
          "          mt.mtWalkOverA, mt.mtWalkOverX,                                "
          "          mt.mtInjuredA, mt.mtInjuredX,                              "
          "          mt.mtDisqualifiedA, mt.mtDisqualifiedX,                    "
          "          sta.tmName, sta.tmDesc, sta.naName, sta.naDesc, sta.naRegion, mt.stA, sta.tmID, sta.tmnaID,"
          "          stx.tmName, stx.tmDesc, stx.naName, stx.naDesc, stx.naRegion, mt.stX, stx.tmID, stx.tmnaID,"
          "          mt.mtMatches, mt.mtBestOf, mt.mtUmpire, mt.mtUmpire2, mt.mtPrinted, mtChecked, "
          "          mt.mtTimestamp                                                 "
          "     FROM MtRec mt                                                       "
          "             LEFT OUTER JOIN StTeamList sta ON mt.sta = sta.stID         "
          "             LEFT OUTER JOIN StTeamList stx ON mt.stx = stx.stID         ";

    tmp->ExecuteUpdate(str);

    str = "CREATE VIEW MtIndividualList                                         "
          " (mtID, mtNr, mtRound, mtMatch, mtChance, mtMS, grID,                "
          "  mtDateTime, mtTable, mtResA, mtResX, mtReverse, nmType,            "
          "  mtWalkOverA, mtWalkOverX,                                          "
          "  mtInjuredA, mtInjuredX, mtDisqualifiedA, mtDisqualifiedX,          "
          "  plAplID, plAplNr, plApsLast, plApsFirst, plAnaID, plAnaName, plAnaDesc, plAnaRegion, plAplExtID,  "
          "  plBplID, plBplNr, plBpsLast, plBpsFirst, plBnaID, plBnaName, plBnaDesc, plBnaRegion, plBplExtID,  "
          "  stA, tmAtmID, nmAnmNr,                                             "
          "  plXplID, plXplNr, plXpsLast, plXpsFirst, plXnaID, plXnaName, plXnaDesc, plXnaRegion, plXplExtID,  "
          "  plYplID, plYplNr, plYpsLast, plYpsFirst, plYnaID, plYnaName, plYnaDesc, plYnaRegion, plYplExtID,  "
          "  stX, tmXtmID, nmXnmNr,                                             "
          "  mtMatches, mtBestOf, mttmResA, mttmResX, mtUmpire, mtUmpire2, mtPrinted, mtChecked, mtTimestamp)  "
          "AS SELECT mt.mtID, mt.mtNr,                                          "
          "          mt.mtRound, mt.mtMatch, mt.mtChance, syMatch.syNr, mt.grID,"
          "          mt.mtDateTime, mt.mtTable, mtMatch.mtResA, mtMatch.mtResX, "
          "          mt.mtReverse, syMatch.syType,                              "
          "          mtMatch.mtWalkOverA, mtMatch.mtWalkOverX,                  "
          "          mtMatch.mtInjuredA, mtMatch.mtInjuredX,                    "
          "          mtMatch.mtDisqualifiedA, mtMatch.mtDisqualifiedX,          "
          "          nma.plAplID, nma.plAplNr, nma.plApsLast, nma.plApsFirst,                "
          "          nma.plAnaID, nma.plAnaName, nma.plAnaDesc, nma.plAnaRegion, nma.plAplExtID, "
          "          nma.plBplID, nma.plBplNr, nma.plBpsLast, nma.plBpsFirst,                "
          "          nma.plBnaID, nma.plBnaName, nma.plBnaDesc, nma.plBnaRegion, nma.plBplExtID, "
          "          mt.stA, sta.tmID, syMatch.syPlayerA,                       "
          "          nmx.plAplID, nmx.plAplNr, nmx.plApsLast, nmx.plApsFirst,                "
          "          nmx.plAnaID, nmx.plAnaName, nmx.plAnaDesc, nmx.plAnaRegion, nmx.plAplExtID, "
          "          nmx.plBplID, nmx.plBplNr, nmx.plBpsLast, nmx.plBpsFirst,                "
          "          nmx.plBnaID, nmx.plBnaName, nmx.plBnaDesc, nmx.plBnaRegion, nmx.plBplExtID, "
          "          mt.stX, stx.tmID, syMatch.syPlayerX,                       "
          "          mt.mtMatches, mt.mtBestOf, mt.mtResA, mt.mtResX, mt.mtUmpire, mt.mtUmpire2, mt.mtPrinted, mtChecked, "
          "          mt.mtTimestamp                                             "
          " FROM MtRec mt                                                       "
          "      INNER JOIN GrRec gr ON mt.grID = gr.grID                       "
          "      INNER JOIN SyMatch syMatch ON gr.syID = syMatch.syID           "
          "      LEFT OUTER JOIN MtMatch mtMatch ON                             "
          "             mtMatch.mtID = mt.mtID AND mtMatch.mtMS = syMatch.syNr  "
          "      LEFT OUTER JOIN StRec sta ON mt.stA = sta.stID                 "
          "      LEFT OUTER JOIN StRec stx ON mt.stX = stx.stID                 "
          "      LEFT OUTER JOIN NmEntryList nma ON                             "
          "             mt.mtID = nma.mtID AND                                  "
          // "             nma.nmNr = syMatch.syPlayerA AND                        "
          "             (nma.nmNr = syMatch.syPlayerA AND mt.mtReverse <> 1 OR  "
          "              nma.nmNr = syMatch.syPlayerX AND mt.mtReverse = 1) AND "
          "             nma.nmType = syMatch.syType  AND                        "
          // "             (nma.tmID = sta.tmID AND mt.mtReverse <> 1 OR           "
          // "              nma.tmID = stx.tmID AND mt.mtReverse = 1)              "
          "             nma.tmID = sta.tmID                                     "
          "      LEFT OUTER JOIN NmEntryList nmx ON                             "
          "             mt.mtID = nmx.mtID AND                                  "
          // "             nmx.nmNr = syMatch.syPlayerX AND                        "
          "             (nmx.nmNr = syMatch.syPlayerX AND mt.mtReverse <> 1 OR  "
          "              nmx.nmNr = syMatch.syPlayerA AND mt.mtReverse = 1) AND "
          "             nmx.nmType = syMatch.syType  AND                        "
          // "             (nmx.tmID = stx.tmID AND mt.mtReverse <> 1 OR           "
          // "              nmx.tmID = sta.tmID AND mt.mtReverse = 1)              ";
          "             nmx.tmID = stx.tmID                                     ";

    tmp->ExecuteUpdate(str);
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  delete tmp;
  
  return true;
}


bool  MtEntryStore::RemoveView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  try
  {
    tmp->ExecuteUpdate("DROP VIEW MtSingleList");
    tmp->ExecuteUpdate("DROP VIEW MtDoubleList");
    tmp->ExecuteUpdate("DROP VIEW MtTeamList");
    tmp->ExecuteUpdate("DROP VIEW MtIndividualList");
  }
  catch(SQLException &)
  {
  }

  delete tmp;
  
  return true;
}


// -----------------------------------------------------------------------
MtEntryStore::MtEntryStore(Connection *ptr)
            : StoreObj(ptr)
{
  mt.cpType = 0;
  Init();
}


void  MtEntryStore::Init()
{
  short type = mt.cpType;

  MtEntry::Init();

  tmA.team.cpType = tmX.team.cpType = mt.cpType = type;
  
  xxAID = 0;
  xxAPos = 0;
  memset(xxAName, 0, sizeof(xxAName));
  memset(xxXDesc, 0, sizeof(xxXDesc));
  
  xxXID = 0;
  xxXPos = 0;
  memset(xxXName, 0, sizeof(xxXName));
  memset(xxXDesc, 0, sizeof(xxXDesc));
}


bool  MtEntryStore::Next()
{
  if (!StoreObj::Next())
    return false;

  // TODO: tmID nicht doppelt halten. Unsauber :(
  mt.tmA = tmA.tmID;
  mt.tmX = tmX.tmID;

  if (!tmA.tmID && mt.xxAstID)
  {
    // tmA.tmID = xxAID;
    // mt.grA = xxAID;
    tmA.team.cpType = CP_GROUP;

    wxStrcpy(tmA.team.gr.grName, xxAName);
    wxStrcpy(tmA.team.gr.grDesc, xxADesc);
    tmA.team.gr.grID = xxAID;
    tmA.team.gr.grPos = xxAPos;
    tmA.team.gr.xxStID = mt.xxAstID;
  }

  if (!tmX.tmID && mt.xxXstID)
  {
    // tmX.tmID = xxXID;
    // mt.grX = xxXID;
    tmX.team.cpType = CP_GROUP;

    wxStrcpy(tmX.team.gr.grName, xxXName);
    wxStrcpy(tmX.team.gr.grDesc, xxXDesc);
    tmX.team.gr.grID = xxXID;
    tmX.team.gr.grPos = xxXPos;
    tmX.team.gr.xxStID=mt.xxXstID;
  }

  return true;
}

// -----------------------------------------------------------------------
bool  MtEntryStore::SelectByGr(const GrRec &gr, short round, short type)
{
  // XXX: PreparedStatement
  tmA.team.cpType = type;
  tmX.team.cpType = type;
  mt.cpType  = type;

  wxString  str = SelectString();
  str += " WHERE mt.grID = " + ltostr(gr.grID);
  if (round != 0)
    str += " AND mtRound = " + ltostr(round);
  str += " ORDER BY mtChance, mtRound, mtMatch";

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


bool  MtEntryStore::SelectByMS(const MtRec &rec, short ms)
{
  mt.cpType = CP_INDIVIDUAL;

  wxString  str = SelectString();
  str += "WHERE mtID = " + ltostr(rec.mtID);

  if (ms != 0)
    str += "  AND mtMS = " + ltostr(ms);

  else
    str += " ORDER BY mtMS";

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


bool  MtEntryStore::SelectById(long id, short type)
{
  tmA.team.cpType = type;
  tmX.team.cpType = type;
  mt.cpType  = type;

  wxString  str = SelectString();
  str += "WHERE mtID = ";
  str += ltostr(id);

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
bool  MtEntryStore::SelectByTime(const timestamp &fromTime, short fromTable,
                                const timestamp &toTime, short toTable)
{
  wxString  str = SelectString();

  // Erweitertes JOIN
  str += 

  // Where-Bedingung
  str += " WHERE mtDateTime >= '" + tstostr(fromTime) + "'"
         "   AND mtDateTime <= '" + tstostr(toTime)   + "'";
  if (fromTable > 0)
    str += "   AND mtTable >= " + ltostr(fromTable);
  if (toTable > 0)
    str += "   AND mtTable <= " + ltostr(toTable);

  // Order by
  str += " ORDER BY mtDateTime, mtTable";

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
bool MtEntryStore::SelectUnscheduled(short cpType, long cpID, long grID)
{
  mt.cpType = cpType;

  wxString sql = SelectString();

  sql += 
    " WHERE tmAtmID IS NOT NULL AND tmXtmID IS NOT NULL "
    "   AND mtResA = 0 AND mtResX = 0 "
    // "   AND mtDateTime IS NOT NULL "
    "   AND (mtTable IS NULL OR mtTable = 0) "
    "   AND (gr.grNofRounds = 0 OR mt.mtRound <= gr.grNofRounds) "
    "   AND cp.cpType = " + ltostr(cpType)
  ;

  if (cpID)
    sql += " AND cp.cpID = " + ltostr(cpID);

  if (grID)
    sql += " AND gr.grID = " + ltostr(grID);

  try
  {
    ExecuteQuery(sql);
    BindRec();
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(sql, e);
    return false;
  }

  return true;
}


// -----------------------------------------------------------------------
wxString  MtEntryStore::SelectString() const
{
  wxString  str;

  switch (mt.cpType)
  {
    case CP_SINGLE :
      str = "SELECT cp.cpName, cp.cpType, "
            "       gr.grName, gr.grModus, gr.grSize, gr.grWinner, gr.grQualRounds, "
            "       mtID, mtNr, mtRound, mtMatch, mtChance, "
            "       mtDateTime, mtTable, "
            "       mtMatches, mtBestOf, mtUmpire, mtUmpire2, mtPrinted, mtChecked,  "
            "       plAplNr, plApsLast, plApsFirst, plAnaName, plAnaDesc, "
            "       stA, tmAtmID, tmAnaID, "
            "       plXplNr, plXpsLast, plXpsFirst, plXnaName, plXnaDesc, "
            "       stX, tmXtmID, tmXnaID, "
            "       mt.grID, mtResA, mtResX, mtWalkOverA, mtWalkOverX, "
            "       mtInjuredA, mtInjuredX, mtDisqualifiedA, mtDisqualifiedX, "
            "       xxA.grName, xxA.grDesc, xxA.grPos, xxA.grID, xxA.stID, "
            "       xxX.grName, xxX.grDesc, xxX.grPos, xxX.grID, xxX.stID  "
            "FROM   MtSingleList mt "
            "       INNER JOIN GrList gr ON mt.grID = gr.grID "
            "       INNER JOIN CpList cp ON gr.cpID = cp.cpID "
            "LEFT OUTER JOIN XxList xxA ON mt.stA = xxA.stID "
            "LEFT OUTER JOIN XxList xxX ON mt.stX = xxX.stID ";

      break;

    case CP_DOUBLE :
    case CP_MIXED :
      // HACK: Nur mit den CAST(... AS char) bekomm ich Werte von MS-SQL
      // Ohne gibt es einen "protocol error in TDS stream"
      str =  "SELECT cp.cpName, cp.cpType, "
             "       gr.grName, gr.grModus, gr.grSize, gr.grWinner, gr.grQualRounds, "
             "       mtID, mtNr, mtRound, mtMatch, mtChance,            "
             "       mtDateTime, mtTable,                               "
             "       mtMatches, mtBestOf, mtUmpire, mtUmpire2, mtPrinted, mtChecked, "
             "       CAST(plAplNr AS char), plApsLast, plApsFirst, plAnaName, plAnaDesc, "
             "       CAST(plBplNr AS char), plBpsLast, plBpsFirst, plBnaName, plBnaDesc, "
             "       stA, tmAtmID, tmAnaID,                                   "
             "       CAST(plXplNr AS char), plXpsLast, plXpsFirst, plXnaName, plXnaDesc, "
             "       CAST(plYplNr AS char), plYpsLast, plYpsFirst, plYnaName, plYnaDesc, "
             "       stX, tmXtmID, tmXnaID,                                   " 
             "       mt.grID, mtResA, mtResX, mtWalkOverA, mtWalkOverX,       "
             "       mtInjuredA, mtInjuredX, mtDisqualifiedA, mtDisqualifiedX, "
             "       xxA.grName, xxA.grDesc, CAST(xxA.grPos AS char), CAST(xxA.grID AS char), CAST(xxA.stID AS CHAR), "
             "       xxX.grName, xxX.grDesc, CAST(xxX.grPos AS char), CAST(xxX.grID AS char), CAST(xxX.stID AS CHAR)  "
             "FROM   MtDoubleList mt "
             "       INNER JOIN GrList gr ON mt.grID = gr.grID "
             "       INNER JOIN CpList cp ON gr.cpID = cp.cpID "
             "LEFT OUTER JOIN XxList xxA ON mt.stA = xxA.stID "
             "LEFT OUTER JOIN XxList xxX ON mt.stX = xxX.stID ";

      break;

    case CP_TEAM :
      str =  "SELECT cp.cpName, cp.cpType, "
             "       gr.grName, gr.grModus, gr.grSize, gr.grWinner, gr.grQualRounds, "
             "       mtID, mtNr, mtRound, mtMatch, mtChance,          "
             "       mtDateTime, mtTable,                             "
             "       mtMatches, mtBestOf, mtUmpire, mtUmpire2, mtPrinted, mtChecked, "
             "       tmAtmName, tmAtmDesc, tmAnaName, tmAnaDesc,      "
             "       stA, tmAtmID, tmAnaID,                           "
             "       tmXtmName, tmXtmDesc, tmXnaName, tmAnaDesc,      "
             "       stX, tmXtmID, tmXnaID,                           "
             "       mt.grID, mtResA, mtResX, mtReverse,              "
             "       mtWalkOverA, mtWalkOverX,                        "
             "       mtInjuredA, mtInjuredX, mtDisqualifiedA, mtDisqualifiedX, "
             "       xxA.grName, xxA.grDesc, xxA.grPos, xxA.grID, xxA.stID, "
             "       xxX.grName, xxX.grDesc, xxX.grPos, xxX.grID, xxX.stID  "
             "FROM   MtTeamList mt                                    "
             "       INNER JOIN GrList gr ON mt.grID = gr.grID "
             "       INNER JOIN CpList cp ON gr.cpID = cp.cpID "
             "LEFT OUTER JOIN XxList xxA ON mt.stA = xxA.stID "
             "LEFT OUTER JOIN XxList xxX ON mt.stX = xxX.stID ";
      break;

    case CP_INDIVIDUAL :
    default :
      str =  "SELECT cp.cpName, " // Not cpType, always CP_INDIVIDUAL cp.cpType,
             "       gr.grName, gr.grModus, gr.grSize, gr.grWinner, gr.grQualRounds, "
             "       mtID, mtNr, mtRound, mtMatch, mtChance,             "
             "       mtDateTime, mtTable,                                "
             "       mtMatches, mtBestOf, mtUmpire, mtUmpire2, mtPrinted, mtChecked,  "
             "       plAplNr, plApsLast, plApsFirst, plAnaName, plAnaDesc, "
             "       plBplNr, plBpsLast, plBpsFirst, plBnaName, plBnaDesc, "
             "       stA, tmAtmID, nmAnmNr,                              "
             "       plXplNr, plXpsLast, plXpsFirst, plXnaName, plXnaDesc, "
             "       plYplNr, plYpsLast, plYpsFirst, plYnaName, plYnaDesc, "
             "       stX, tmXtmID, nmXnmNr,                              "
             "       mt.grID, mtResA, mtResX,                            "
             "       nmType, nmType, mtMS, mtReverse,                    "
             "       mtWalkOverA, mtWalkoverX                            "
             "       mtInjuredA, mtInjuredX, mtDisqualifiedA, mtDisqualifiedX "
             "FROM   MtIndividualList mt                                 "
             "       INNER JOIN GrList gr ON mt.grID = gr.grID           "
             "       INNER JOIN CpList cp ON gr.cpID = cp.cpID           ";
      break;
  }

  return str;
}


bool  MtEntryStore::BindRec()
{
  int col = 0;
  switch (mt.cpType)
  {
    case CP_SINGLE :
      BindCol(++col, mt.cpName, sizeof(mt.cpName));
      BindCol(++col, &mt.cpType);
      BindCol(++col, mt.grName, sizeof(mt.grName));
      BindCol(++col, &mt.grModus);
      BindCol(++col, &mt.grSize);
      BindCol(++col, &mt.grWinner);
      BindCol(++col, &mt.grQualRounds);
      BindCol(++col, &mt.mtID);
      BindCol(++col, &mt.mtNr);
      BindCol(++col, &mt.mtEvent.mtRound);
      BindCol(++col, &mt.mtEvent.mtMatch);
      BindCol(++col, &mt.mtEvent.mtChance);
      BindCol(++col, &mt.mtPlace.mtDateTime);
      BindCol(++col, &mt.mtPlace.mtTable);
      BindCol(++col, &mt.mtMatches);
      BindCol(++col, &mt.mtBestOf);
      BindCol(++col, &mt.mtUmpire);
      BindCol(++col, &mt.mtUmpire2);
      BindCol(++col, &mt.mtScorePrinted);
      BindCol(++col, &mt.mtScoreChecked);

      BindCol(++col, &tmA.team.pl.plNr);
      BindCol(++col, tmA.team.pl.psName.psLast, sizeof(tmA.team.pl.psName.psLast));
      BindCol(++col, tmA.team.pl.psName.psFirst, sizeof(tmA.team.pl.psName.psFirst));
      BindCol(++col, tmA.team.pl.naName, sizeof(tmA.team.pl.naName));
      BindCol(++col, tmA.team.pl.naDesc, sizeof(tmA.team.pl.naDesc));
      
      BindCol(++col, &mt.stA);
      BindCol(++col, &tmA.tmID);
      BindCol(++col, &tmA.naID);
 
      BindCol(++col, &tmX.team.pl.plNr);
      BindCol(++col, tmX.team.pl.psName.psLast, sizeof(tmX.team.pl.psName.psLast));
      BindCol(++col, tmX.team.pl.psName.psFirst, sizeof(tmX.team.pl.psName.psFirst));
      BindCol(++col, tmX.team.pl.naName, sizeof(tmX.team.pl.naName));
      BindCol(++col, tmX.team.pl.naDesc, sizeof(tmX.team.pl.naDesc));
      
      BindCol(++col, &mt.stX);
      BindCol(++col, &tmX.tmID);
      BindCol(++col, &tmX.naID);

      BindCol(++col, &mt.mtEvent.grID);

      BindCol(++col, &mt.mtResA);
      BindCol(++col, &mt.mtResX);
      
      BindCol(++col, &mt.mtWalkOverA);
      BindCol(++col, &mt.mtWalkOverX);
      BindCol(++col, &mt.mtInjuredA);
      BindCol(++col, &mt.mtInjuredX);
      BindCol(++col, &mt.mtDisqualifiedA);
      BindCol(++col, &mt.mtDisqualifiedX);

      BindCol(++col, xxAName, sizeof(xxAName));
      BindCol(++col, xxADesc, sizeof(xxADesc));
      BindCol(++col, &xxAPos);
      BindCol(++col, &xxAID);
      BindCol(++col, &mt.xxAstID);

      BindCol(++col, xxXName, sizeof(xxXName));
      BindCol(++col, xxXDesc, sizeof(xxXDesc));
      BindCol(++col, &xxXPos);
      BindCol(++col, &xxXID);
      BindCol(++col, &mt.xxXstID);

      break;

    case CP_DOUBLE :
    case CP_MIXED  :
      BindCol(++col, mt.cpName, sizeof(mt.cpName));
      BindCol(++col, &mt.cpType);
      BindCol(++col, mt.grName, sizeof(mt.grName));
      BindCol(++col, &mt.grModus);
      BindCol(++col, &mt.grSize);
      BindCol(++col, &mt.grWinner);
      BindCol(++col, &mt.grQualRounds);
      BindCol(++col, &mt.mtID);
      BindCol(++col, &mt.mtNr);
      BindCol(++col, &mt.mtEvent.mtRound);
      BindCol(++col, &mt.mtEvent.mtMatch);
      BindCol(++col, &mt.mtEvent.mtChance);
      BindCol(++col, &mt.mtPlace.mtDateTime);
      BindCol(++col, &mt.mtPlace.mtTable);
      BindCol(++col, &mt.mtMatches);
      BindCol(++col, &mt.mtBestOf);
      BindCol(++col, &mt.mtUmpire);
      BindCol(++col, &mt.mtUmpire2);
      BindCol(++col, &mt.mtScorePrinted);
      BindCol(++col, &mt.mtScoreChecked);

      BindCol(++col, &tmA.team.pl.plNr);
      BindCol(++col, tmA.team.pl.psName.psLast, sizeof(tmA.team.pl.psName.psLast));
      BindCol(++col, tmA.team.pl.psName.psFirst, sizeof(tmA.team.pl.psName.psFirst));
      BindCol(++col, tmA.team.pl.naName, sizeof(tmA.team.pl.naName));
      BindCol(++col, tmA.team.pl.naDesc, sizeof(tmA.team.pl.naDesc));

      BindCol(++col, &tmA.team.bd.plNr);
      BindCol(++col, tmA.team.bd.psName.psLast, sizeof(tmA.team.bd.psName.psLast));
      BindCol(++col, tmA.team.bd.psName.psFirst, sizeof(tmA.team.bd.psName.psFirst));
      BindCol(++col, tmA.team.bd.naName, sizeof(tmA.team.bd.naName));
      BindCol(++col, tmA.team.bd.naDesc, sizeof(tmA.team.bd.naDesc));

      BindCol(++col, &mt.stA);
      BindCol(++col, &tmA.tmID);
      BindCol(++col, &tmA.naID);

      BindCol(++col, &tmX.team.pl.plNr);
      BindCol(++col, tmX.team.pl.psName.psLast, sizeof(tmX.team.pl.psName.psLast));
      BindCol(++col, tmX.team.pl.psName.psFirst, sizeof(tmX.team.pl.psName.psFirst));
      BindCol(++col, tmX.team.pl.naName, sizeof(tmX.team.pl.naName));
      BindCol(++col, tmX.team.pl.naDesc, sizeof(tmX.team.pl.naDesc));

      BindCol(++col, &tmX.team.bd.plNr);
      BindCol(++col, tmX.team.bd.psName.psLast, sizeof(tmX.team.bd.psName.psLast));
      BindCol(++col, tmX.team.bd.psName.psFirst, sizeof(tmX.team.bd.psName.psFirst));
      BindCol(++col, tmX.team.bd.naName, sizeof(tmX.team.bd.naName));
      BindCol(++col, tmX.team.bd.naDesc, sizeof(tmX.team.bd.naDesc));

      BindCol(++col, &mt.stX);
      BindCol(++col, &tmX.tmID);
      BindCol(++col, &tmX.naID);

      BindCol(++col, &mt.mtEvent.grID);

      BindCol(++col, &mt.mtResA);
      BindCol(++col, &mt.mtResX); 
      
      BindCol(++col, &mt.mtWalkOverA);
      BindCol(++col, &mt.mtWalkOverX);
      BindCol(++col, &mt.mtInjuredA);
      BindCol(++col, &mt.mtInjuredX);
      BindCol(++col, &mt.mtDisqualifiedA);
      BindCol(++col, &mt.mtDisqualifiedX);

      BindCol(++col, xxAName, sizeof(xxAName));
      BindCol(++col, xxADesc, sizeof(xxADesc));
      BindCol(++col, &xxAPos);
      BindCol(++col, &xxAID);
      BindCol(++col, &mt.xxAstID);

      BindCol(++col, xxXName, sizeof(xxXName));
      BindCol(++col, xxXDesc, sizeof(xxXDesc));
      BindCol(++col, &xxXPos);
      BindCol(++col, &xxXID);
      BindCol(++col, &mt.xxXstID);

      break;

    case CP_TEAM :
      BindCol(++col, mt.cpName, sizeof(mt.cpName));
      BindCol(++col, &mt.cpType);
      BindCol(++col, mt.grName, sizeof(mt.grName));
      BindCol(++col, &mt.grModus);
      BindCol(++col, &mt.grSize);
      BindCol(++col, &mt.grWinner);
      BindCol(++col, &mt.grQualRounds);
      BindCol(++col, &mt.mtID);
      BindCol(++col, &mt.mtNr);
      BindCol(++col, &mt.mtEvent.mtRound);
      BindCol(++col, &mt.mtEvent.mtMatch);
      BindCol(++col, &mt.mtEvent.mtChance);
      BindCol(++col, &mt.mtPlace.mtDateTime);
      BindCol(++col, &mt.mtPlace.mtTable);
      BindCol(++col, &mt.mtMatches);
      BindCol(++col, &mt.mtBestOf);
      BindCol(++col, &mt.mtUmpire);
      BindCol(++col, &mt.mtUmpire2);
      BindCol(++col, &mt.mtScorePrinted);
      BindCol(++col, &mt.mtScoreChecked);
      
      BindCol(++col, tmA.team.tm.tmName, sizeof(tmA.team.tm.tmName));
      BindCol(++col, tmA.team.tm.tmDesc, sizeof(tmA.team.tm.tmDesc));
      BindCol(++col, tmA.team.tm.naName, sizeof(tmA.team.tm.naName));
      BindCol(++col, tmA.team.tm.naDesc, sizeof(tmA.team.tm.naDesc));
      
      BindCol(++col, &mt.stA);
      BindCol(++col, &tmA.tmID);
      BindCol(++col, &tmA.naID);

      BindCol(++col, tmX.team.tm.tmName, sizeof(tmX.team.tm.tmName));
      BindCol(++col, tmX.team.tm.tmDesc, sizeof(tmX.team.tm.tmDesc));
      BindCol(++col, tmX.team.tm.naName, sizeof(tmX.team.tm.naName));
      BindCol(++col, tmX.team.tm.naDesc, sizeof(tmX.team.tm.naDesc));
      
      BindCol(++col, &mt.stX);
      BindCol(++col, &tmX.tmID);
      BindCol(++col, &tmX.naID);

      BindCol(++col, &mt.mtEvent.grID);

      BindCol(++col, &mt.mtResA);
      BindCol(++col, &mt.mtResX);
      BindCol(++col, &mt.mtReverse);
      
      BindCol(++col, &mt.mtWalkOverA);
      BindCol(++col, &mt.mtWalkOverX);
      BindCol(++col, &mt.mtInjuredA);
      BindCol(++col, &mt.mtInjuredX);
      BindCol(++col, &mt.mtDisqualifiedA);
      BindCol(++col, &mt.mtDisqualifiedX);

      BindCol(++col, xxAName, sizeof(xxAName));
      BindCol(++col, xxADesc, sizeof(xxADesc));
      BindCol(++col, &xxAPos);
      BindCol(++col, &xxAID);
      BindCol(++col, &mt.xxAstID);

      BindCol(++col, xxXName, sizeof(xxXName));
      BindCol(++col, xxXDesc, sizeof(xxXDesc));
      BindCol(++col, &xxXPos);
      BindCol(++col, &xxXID);
      BindCol(++col, &mt.xxXstID);

      break;

    case CP_INDIVIDUAL :
      BindCol(++col, mt.cpName, sizeof(mt.cpName));
      // Not cpType, always CP_INDIVIDUAL
      // BindCol(++col, &mt.cpType);
      BindCol(++col, mt.grName, sizeof(mt.grName));
      BindCol(++col, &mt.grModus);
      BindCol(++col, &mt.grSize);
      BindCol(++col, &mt.grWinner);
      BindCol(++col, &mt.grQualRounds);
      BindCol(++col, &mt.mtID);
      BindCol(++col, &mt.mtNr);
      BindCol(++col, &mt.mtEvent.mtRound);
      BindCol(++col, &mt.mtEvent.mtMatch);
      BindCol(++col, &mt.mtEvent.mtChance);
      BindCol(++col, &mt.mtPlace.mtDateTime);
      BindCol(++col, &mt.mtPlace.mtTable);
      BindCol(++col, &mt.mtMatches);
      BindCol(++col, &mt.mtBestOf);
      BindCol(++col, &mt.mtUmpire);
      BindCol(++col, &mt.mtUmpire2);
      BindCol(++col, &mt.mtScorePrinted);
      BindCol(++col, &mt.mtScoreChecked);

      BindCol(++col, &tmA.team.pl.plNr);
      BindCol(++col, tmA.team.pl.psName.psLast, sizeof(tmA.team.pl.psName.psLast));
      BindCol(++col, tmA.team.pl.psName.psFirst, sizeof(tmA.team.pl.psName.psFirst));
      BindCol(++col, tmA.team.pl.naName, sizeof(tmA.team.pl.naName));
      BindCol(++col, tmA.team.pl.naDesc, sizeof(tmA.team.pl.naDesc));

      BindCol(++col, &tmA.team.bd.plNr);
      BindCol(++col, tmA.team.bd.psName.psLast, sizeof(tmA.team.bd.psName.psLast));
      BindCol(++col, tmA.team.bd.psName.psFirst, sizeof(tmA.team.bd.psName.psFirst));
      BindCol(++col, tmA.team.bd.naName, sizeof(tmA.team.bd.naName));
      BindCol(++col, tmA.team.bd.naDesc, sizeof(tmA.team.bd.naDesc));

      BindCol(++col, &mt.stA);
      BindCol(++col, &tmA.tmID);
      BindCol(++col, &nmAnmNr);

      BindCol(++col, &tmX.team.pl.plNr);
      BindCol(++col, tmX.team.pl.psName.psLast, sizeof(tmX.team.pl.psName.psLast));
      BindCol(++col, tmX.team.pl.psName.psFirst, sizeof(tmX.team.pl.psName.psFirst));
      BindCol(++col, tmX.team.pl.naName, sizeof(tmX.team.pl.naName));
      BindCol(++col, tmX.team.pl.naDesc, sizeof(tmX.team.pl.naDesc));

      BindCol(++col, &tmX.team.bd.plNr);
      BindCol(++col, tmX.team.bd.psName.psLast, sizeof(tmX.team.bd.psName.psLast));
      BindCol(++col, tmX.team.bd.psName.psFirst, sizeof(tmX.team.bd.psName.psFirst));
      BindCol(++col, tmX.team.bd.naName, sizeof(tmX.team.bd.naName));
      BindCol(++col, tmX.team.bd.naDesc, sizeof(tmX.team.bd.naDesc));

      BindCol(++col, &mt.stX);
      BindCol(++col, &tmX.tmID);
      BindCol(++col, &nmXnmNr);

      BindCol(++col, &mt.mtEvent.grID);

      BindCol(++col, &mt.mtResA);
      BindCol(++col, &mt.mtResX);      

      BindCol(++col, &tmA.team.cpType);
      BindCol(++col, &tmX.team.cpType);

      BindCol(++col, &mt.mtEvent.mtMS);
      
      BindCol(++col, &mt.mtReverse);

      BindCol(++col, &mt.mtWalkOverA);
      BindCol(++col, &mt.mtWalkOverX);
      BindCol(++col, &mt.mtInjuredA);
      BindCol(++col, &mt.mtInjuredX);
      BindCol(++col, &mt.mtDisqualifiedA);
      BindCol(++col, &mt.mtDisqualifiedX);

      break;

    default :
      return false;
  }

  return true;
}