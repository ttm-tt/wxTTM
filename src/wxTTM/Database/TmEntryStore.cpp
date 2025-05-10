/* Copyright (C) 2020 Christoph Theis */

// Teams
#include  "stdafx.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "ResultSet.h"
#include  "SQLException.h"
#include  "InfoSystem.h"

#include  "TmEntryStore.h"

#include  "CpStore.h"
#include  "GrStore.h"
#include  "StStore.h"
#include  "TmStore.h"

#include  "Rec.h"


// -----------------------------------------------------------------------
void  TmPlayer::SetPlayer(const LtEntry &pl)
{
  LtEntry::operator=(pl);
}


void  TmTeam::SetTeam(const TmRec &tm)
{
  TmRec::operator=(tm);
}


void  TmGroup::SetGroup(const GrRec &gr, short pos)
{
  GrRec::operator=(gr);
  grPos = pos;
}


void  TmEntry::SetSingle(const LtEntry &pl)
{
  team.pl.SetPlayer(pl);
  team.cpType = CP_SINGLE;
}


void  TmEntry::SetDouble(const LtEntry &pl, const LtEntry &bd)
{
  team.pl.SetPlayer(pl);
  team.bd.SetPlayer(bd);
  team.cpType = CP_DOUBLE;
}


void  TmEntry::SetMixed(const LtEntry &pl, const LtEntry &bd)
{
  team.pl.SetPlayer(pl);
  team.bd.SetPlayer(bd);
  team.cpType = CP_MIXED;
}


void  TmEntry::SetTeam(const TmRec &tm)
{
  team.tm.SetTeam(tm);
  team.cpType = CP_TEAM;
}


void  TmEntry::SetGroup(const GrRec &gr, short pos)
{
  team.gr.SetGroup(gr, pos);
  team.cpType = CP_GROUP;
}


bool  TmEntry::HasString(const wxString &str) const
{
  switch (team.cpType)
  {
    case CP_SINGLE :
    {
      wxChar tmp[10];
      _itot(team.pl.plNr, tmp, 10);
      if (wxStrnicmp(tmp, str, str.length()) == 0)
        return true;

      if (wxStrnicmp(team.pl.psName.psLast, str, str.length()) == 0)
        return true;
        
      return wxStrnicmp(team.pl.naName, str, str.length()) == 0;
    }

    case CP_DOUBLE :
    case CP_MIXED :
    {
      wxChar tmpA[10], tmpB[10];
      _itot(team.pl.plNr, tmpA, 10);
      _itot(team.bd.plNr, tmpB, 10);
      if (wxStrnicmp(tmpA, str, wxStrlen(str)) == 0 ||
          wxStrnicmp(tmpB, str, wxStrlen(str)) == 0)
        return true;

      if ( wxStrnicmp(team.pl.psName.psLast, str, wxStrlen(str)) == 0 ||
           wxStrnicmp(team.bd.psName.psLast, str, wxStrlen(str)) == 0 )
        return true;
        
      return wxStrnicmp(team.pl.naName, str, wxStrlen(str)) == 0 ||
             wxStrnicmp(team.bd.naName, str, wxStrlen(str)) == 0;
    }

    case CP_TEAM :
    {
      return wxStrnicmp(team.tm.tmName, str, wxStrlen(str)) == 0;
    }

    case CP_GROUP :
      return wxStrnicmp(team.gr.grName, str, wxStrlen(str)) == 0;
    
    default :
      return false;
  }
}


// -----------------------------------------------------------------------
bool  TmEntryStore::CreateView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str;
  
  try
  {
    // Einzel
    str = "CREATE VIEW TmSingleList "
          "           (plID, plNr, psLast, psFirst, psSex,                  "
          "            naID, naName, naDesc, naRegion, plExtID, plRankPts, ltRankPts, ntNr, "
          "            tmID, cpID, cpType, cpName, tmnaID, tmTimestamp)                     "
          "  AS SELECT PlRec.plID, plNr, psLast, psFirst, psSex,            "
          "            NaRec.naID, naName, naDesc, naRegion, plExtID, plRankPts,      "
          "            ISNULL((SELECT TOP 1 rpRankPts FROM RpRec rp WHERE rp.plID = PlRec.plID AND rp.rpYear <= CpRec.cpYear ORDER BY rp.rpYear DESC ), PlRec.plRankPts) AS ltRankPts, "
          "            NtRec.ntNr, TmRec.tmID, TmRec.cpID, CpRec.cpType, CpRec.cpName, RkRec.naID, tmTimestamp  "
          "    FROM TmRec INNER JOIN NtRec ON TmRec.tmID = NtRec.tmID       "
          "               INNER JOIN LtRec ON NtRec.ltID = LtRec.ltID       "
          "               INNER JOIN PlRec ON LtRec.plID = PlRec.plID       "
          "               INNER JOIN PsRec ON PlRec.psID = PsRec.psID       "
          "               INNER JOIN CpRec ON CpRec.cpID = TmRec.cpID       "
          "               LEFT OUTER JOIN NaRec ON PlRec.naID = NaRec.naID  "
          "               LEFT OUTER JOIN RkRec ON TmRec.tmID = RkRec.tmID  "
    ;
    tmp->ExecuteUpdate(str);

    str = "CREATE VIEW TmDoubleList "
          "           (plplID, plplNr,  plpsLast,  plpsFirst,  plpsSex,           "
          "            plNaID, plnaName,  plnaDesc, plnaRegion, plplExtID, plRankPts, plltRankPts, plntNr, "
          "            bdplID, bdplNr,  bdpsLast,  bdpsFirst,  bdpsSex,           "
          "            bdnaID, bdnaName, bdnaDesc,  bdnaRegion, bdplExtID, bdRankPts, bdltRankPts, bdntNr, "
          "            tmID, cpID, cpType, cpName, tmnaID, tmTimestamp) "
          "  AS SELECT pl.plID, pl.plNr, pl.psLast, pl.psFirst, pl.psSex,           "
          "            pl.naID, pl.naName, pl.naDesc, pl.naRegion, pl.plExtID, pl.plRankPts, pl.ltRankPts, "
          "            pl.ntNr, "
          "            bd.plID, bd.plNr, bd.psLast, bd.psFirst, bd.psSex,           "
          "            bd.naID, bd.naName, bd.naDesc, bd.naRegion, bd.plExtID, bd.plRankPts, bd.ltRankPts, "
          "            bd.ntNr, "
          "            pl.tmID, pl.cpID, CpRec.cpType, CpRec.cpName, RkRec.naID, pl.tmTimestamp "  // View pl.tmTimestamp und bd.tmTimestamp muessen gleich sein weil gleiches Team
          "       FROM TmSingleList pl INNER JOIN TmSingleList bd          "
          "            ON pl.tmID = bd.tmID AND pl.ntNr = 1 AND bd.ntNr = 2 "
          "            INNER JOIN CpRec ON pl.cpID = CpRec.cpID            "
          "            LEFT OUTER JOIN RkRec ON pl.tmID = RkRec.tmID       "
    ;
    tmp->ExecuteUpdate(str);

    str = "CREATE VIEW TmTeamList                                           "
          "           (tmName, tmDesc, naID, naName, naDesc, naRegion,      "
          "            tmID, cpID, cpType, cpName, tmnaID, tmTimestamp)                     "
          "  AS SELECT tmName, tmDesc, NaRec.naID, naName, naDesc, naRegion, "
          "            TmRec.tmID, TmRec.cpID, CpRec.cpType, CpRec.cpName, RkRec.naID, tmTimestamp "
          "       FROM TmRec INNER JOIN CpRec ON CpRec.cpID = TmRec.cpID    "
          "                  LEFT OUTER JOIN("
          "                  RkRec INNER JOIN NaRec ON RkRec.naID = NaRec.naID) "
          "                  ON TmRec.tmID = RkRec.tmID                     "
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


bool  TmEntryStore::RemoveView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  try
  {
    tmp->ExecuteUpdate("DROP VIEW IF EXISTS TmSingleList");
    tmp->ExecuteUpdate("DROP VIEW IF EXISTS TmDoubleList");
    tmp->ExecuteUpdate("DROP VIEW IF EXISTS TmTeamList");
  }
  catch(SQLException &)
  {
  }

  delete tmp;
  
  return true;
}


// -----------------------------------------------------------------------
TmEntryStore::TmEntryStore(Connection *ptr)
            : StoreObj(ptr)
{
}


void  TmEntryStore::Init()
{
  short type = team.cpType;

  TmEntry::Init();

  team.cpType = type;
}


// -----------------------------------------------------------------------
// Auswahl per DB

bool  TmEntryStore::SelectTeamById(long id, short type, const timestamp *when)
{
  team.cpType = type;
  wxString  str = SelectString(when);

  str += " WHERE tmID = " + ltostr(id);

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


bool  TmEntryStore::SelectTeamById(const std::set<long> &ids, short type)
{
  team.cpType = type;
  wxString  str = SelectString();

  str += " WHERE tmID IN (" + ltostr(ids) + ")";

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


bool  TmEntryStore::SelectTeamByCp(const CpRec &cp, long naID)
{
  team.cpType = cp.cpType;

  wxString  str = SelectString();

  switch (team.cpType)
  {
    case CP_SINGLE :
      str += " WHERE cpType = " + ltostr(CP_SINGLE);
      if (cp.cpID)
        str += " AND cpID = " + ltostr(cp.cpID);
      if (naID)
        str += " AND tmnaID = " + ltostr(naID);
      str += " ORDER BY plNr, cpName";
      break;

    case CP_DOUBLE :
      str += " WHERE cpType = " + ltostr(CP_DOUBLE);
      if (cp.cpID)
        str += " AND cpID = " + ltostr(cp.cpID);
      if (naID)
        str += " AND tmnaID = " + ltostr(naID);
      str += " ORDER BY plplNr, cpName";
      break;

    case CP_MIXED  :
      str += " WHERE cpType = " + ltostr(CP_MIXED);
      if (cp.cpID)
        str += " AND cpID = " + ltostr(cp.cpID);
      if (naID)
        str += " AND tmnaID = " + ltostr(naID);
      str += " ORDER BY plplNr, cpName";
      break;

    case CP_TEAM  :
      str += " WHERE cpType = " + ltostr(CP_TEAM);
      if (cp.cpID)
        str += " AND cpID = " + ltostr(cp.cpID);
      if (naID)
        str += " AND tmnaID = " + ltostr(naID);
      str += " ORDER BY tmName, cpName";
      break;

    default :
      return false;
  }

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


bool  TmEntryStore::SelectTeamForSeeding(const GrRec &gr, short type)
{
  team.cpType = type;

  wxString  str = SelectString();

  str += " WHERE cpID = " + ltostr(gr.cpID) +
         "   AND tmID NOT IN ("
         "      SELECT ISNULL(tmID, 0) FROM StRec WHERE StRec.grID IN ("
         "            SELECT grID FROM GrRec "
         "             WHERE GrRec.cpID = " + ltostr(gr.cpID) + 
         "               AND GrRec.grStage = '" + TransformString(gr.grStage) + "')"
         ") ";


  switch (team.cpType)
  {
    case CP_SINGLE :
      str += " ORDER BY plNr";
      break;

    case CP_DOUBLE :
    case CP_MIXED  :
      str += " ORDER BY plplNr";
      break;

    case CP_TEAM  :
      str += " ORDER BY tmName";
      break;

    default :
      return false;
  }

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
short  TmEntryStore::CountTeams(const CpRec &cp)
{
  Connection * connPtr = GetConnectionPtr();
  Statement  * stmtPtr = 0;
  ResultSet  * resPtr  = 0;

  wxString  str = 
      "SELECT COUNT(tmID) FROM TmRec "
      "WHERE tmID IS NOT NULL AND cpID = ";
  str += ltostr(cp.cpID);

  short res;

  try
  {
    stmtPtr = connPtr->CreateStatement();
    resPtr = stmtPtr->ExecuteQuery(str);
    resPtr->BindCol(1, &res);
    if ( !resPtr->Next() || resPtr->WasNull(1) )
      res = 0;
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    res = 0;
  }  
  
  delete resPtr;
  delete stmtPtr;

  return res;
}


// -----------------------------------------------------------------------
wxString  TmEntryStore::SelectString(const timestamp *when) const
{
  wxString  str;
  wxString  ts;
  if (when == NULL || when->year == 9999)
  {
    str = "SELECT ";
  }
  else
  {
    str = "SELECT DISTINCT ";
    if (when->year == 0)
      ts = " FOR SYSTEM_TIME ALL ";
    else
      ts = " FOR SYSTEM_TIME AS OF '" + tstostr(*when) + "' ";
  }

  switch (team.cpType)
  {
    case CP_SINGLE :
      str += " plNr, psLast, psFirst, psSex, naID, naName, naDesc, ltRankPts,"
             " tmID, cpID, cpName, tmnaID "
             "  FROM TmSingleList " + ts + " ";

      break;

    case CP_DOUBLE :
    case CP_MIXED :
      str += " plplNr, plpsLast, plpsFirst, plpsSex, plnaID, plnaName, plnaDesc, plltRankPts, "
             "       bdplNr, bdpsLast, bdpsFirst, bdpsSex, bdnaID, bdnaName, bdnaDesc, bdltRankpts, "
             "       tmID, cpID, cpName, tmnaID  "
             "  FROM TmDoubleList " + ts + " ";

      break;

    case CP_TEAM :
      str += "tmName, tmDesc, naID, naName, naDesc, tmID, cpID, cpName, tmnaID "
             "FROM TmTeamList " + ts + " ";
      break;

    default :
      str = "";
      break;
  }

  return str;
}


bool  TmEntryStore::BindRec()
{
  int  col = 1;

  switch (team.cpType)
  {
    case CP_SINGLE :
      BindCol(col++, &team.pl.plNr);
      BindCol(col++, team.pl.psName.psLast, sizeof(team.pl.psName.psLast));
      BindCol(col++, team.pl.psName.psFirst, sizeof(team.pl.psName.psFirst));
      BindCol(col++, &team.pl.psSex);
      BindCol(col++, &team.pl.naID);
      BindCol(col++, team.pl.naName, sizeof(team.pl.naName));
      BindCol(col++, team.pl.naDesc, sizeof(team.pl.naDesc));
      BindCol(col++, &team.pl.plRankPts);
      BindCol(col++, &tmID);
      BindCol(col++, &cpID);
      BindCol(col++, cpName, sizeof(cpName));
      BindCol(col++, &naID);
      break;

    case CP_DOUBLE :
    case CP_MIXED  :
      BindCol(col++, &team.pl.plNr);
      BindCol(col++, team.pl.psName.psLast, sizeof(team.pl.psName.psLast));
      BindCol(col++, team.pl.psName.psFirst, sizeof(team.pl.psName.psFirst));
      BindCol(col++, &team.pl.psSex);
      BindCol(col++, &team.pl.naID);
      BindCol(col++, team.pl.naName, sizeof(team.pl.naName));
      BindCol(col++, team.pl.naDesc, sizeof(team.pl.naDesc));
      BindCol(col++, &team.pl.plRankPts);
  
      BindCol(col++, &team.bd.plNr);
      BindCol(col++, team.bd.psName.psLast, sizeof(team.bd.psName.psLast));
      BindCol(col++, team.bd.psName.psFirst, sizeof(team.bd.psName.psFirst));
      BindCol(col++, &team.bd.psSex);
      BindCol(col++, &team.bd.naID);
      BindCol(col++, team.bd.naName, sizeof(team.bd.naName));
      BindCol(col++, team.bd.naDesc, sizeof(team.bd.naDesc));
      BindCol(col++, &team.bd.plRankPts);

      BindCol(col++, &tmID);
      BindCol(col++, &cpID);
      BindCol(col++, cpName, sizeof(cpName));
      BindCol(col++, &naID);

      break;

    case CP_TEAM :
      BindCol(col++, team.tm.tmName, sizeof(team.tm.tmName));
      BindCol(col++, team.tm.tmDesc, sizeof(team.tm.tmDesc));
      BindCol(col++, &team.tm.naID);
      BindCol(col++, team.tm.naName, sizeof(team.tm.naName));
      BindCol(col++, team.tm.naDesc, sizeof(team.tm.naDesc));
      BindCol(col++, &tmID);
      BindCol(col++, &cpID);
      BindCol(col++, cpName, sizeof(cpName));
      BindCol(col++, &naID);

      break;

    default :
      return false;
  }

  return true;
}