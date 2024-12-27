/* Copyright (C) 2020 Christoph Theis */

// Ranking
#include  "stdafx.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "SQLException.h"
#include  "InfoSystem.h"

#include  "RkEntryStore.h"
#include  "CpListStore.h"
#include  "NaListStore.h"

#include  "Rec.h"



// -----------------------------------------------------------------------
bool  RkEntryStore::CreateView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str;

  try
  {
    str = "CREATE VIEW RkSingleList (                            "
          " rkNatlRank, rkIntlRank, rkDirectEntry,               "
          " plID, plNr, psLast, psFirst, naName, plRankPts, ltRankPts, "
          " tmID, cpID, naID)                                    "
          "AS                                                    "
          "  SELECT rkNatlRank, rkIntlRank, rkDirectEntry,       "
          "         plID, plNr, psLast, psFirst, naName, plRankPts, ltRankPts, "
          "         rk.tmID, tm.cpID, rk.naID                    "
          "    FROM RkList rk INNER JOIN TmSingleList tm         "
          "      ON rk.tmID = tm.tmID ";
    tmp->ExecuteUpdate(str);

    str = "CREATE VIEW RkDoubleList (                            "
          "  rkNatlRank, rkIntlRank, rkDirectEntry,              "
          "  plplID, plplNr, plpsLast, plpsFirst, plnaName, plRankPts, plltRankPts,   "
          "  bdplID, bdplNr, bdpsLast, bdpsFirst, bdnaName, bdRankPts, bdltRankPts,   "
          "  tmID, cpID, naID)                                   "
          "AS                                                    "
          "  SELECT rkNatlRank, rkIntlRank, rkDirectEntry,       "
          "         plplID, plplNr, plpsLast, plpsFirst, plnaName, plRankPts, plltRankPts, "
          "         bdplId, bdplNr, bdpsLast, bdpsFirst, bdnaName, bdRankPts, bdltRankPts, "
          "         rk.tmID, tm.cpID, rk.naID                    "
          "    FROM RkList rk INNER JOIN TmDoubleList tm         "
          "      ON rk.tmID = tm.tmID ";
    tmp->ExecuteUpdate(str);

    str = "CREATE VIEW RkTeamList (                              "
          "  rkNatlRank, rkIntlRank, rkDirectEntry,              "
          "  tmName, tmDesc, naName, tmID, cpID, naID)           "
          "AS                                                    "
          "  SELECT rkNatlRank, rkIntlRank, rkDirectEntry,       "
          "         tmName, tmDesc, naName,                      "
          "         rk.tmID, tm.cpID, rk.naID                    "
          "    FROM RkList rk INNER JOIN TmTeamList tm           "
          "      ON rk.tmID = tm.tmID ";
    tmp->ExecuteUpdate(str);
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  delete tmp;
  
  return true;
}


bool  RkEntryStore::RemoveView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  try
  {
    tmp->ExecuteUpdate("DROP VIEW IF EXISTS RkSingleList");
    tmp->ExecuteUpdate("DROP VIEW IF EXISTS RkDoubleList");
    tmp->ExecuteUpdate("DROP VIEW IF EXISTS RkTeamList");
  }
  catch(SQLException &)
  {
  }

  delete tmp;
  
  return true;
}


// -----------------------------------------------------------------------
RkEntryStore::RkEntryStore(Connection *ptr)
            : StoreObj(ptr)
{
}


void  RkEntryStore::Init()
{
  short type = team.cpType;

  RkEntry::Init();

  team.cpType = type;
}


// -----------------------------------------------------------------------
bool  RkEntryStore::SelectByTm(const TmRec &tm, short type)
{
  team.cpType = type;

  wxString  str = SelectString();
  str += "WHERE tm = " + ltostr(tm.tmID);

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


bool  RkEntryStore::SelectByCp(const CpRec &cp)
{
  team.cpType = cp.cpType;

  wxString  str = SelectString();
  str += " WHERE rk.cpID = " + ltostr(cp.cpID);
  str += " ORDER BY na.naName, rkNatlRank";

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


bool  RkEntryStore::SelectByCpNa(const CpRec &cp, const NaRec &na)
{
  team.cpType = cp.cpType;

  wxString  str = SelectString();
  str += " WHERE rk.cpID = " + ltostr(cp.cpID) + " AND rk.naID = " + ltostr(na.naID);
  str += " ORDER BY rkNatlRank";

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
wxString  RkEntryStore::SelectString() const
{
  wxString  str;

  switch (team.cpType)
  {
    case CP_SINGLE :
      str = "SELECT rkNatlRank, rkIntlRank, rkDirectEntry,    "
            "       plNr, psLast, psFirst, rk.naName, plRankPts, ltRankPts, "
            "       ltRankPts AS rankPts, tmID, cpID, rk.naID  "
            "FROM RkSingleList rk INNER JOIN NaList na ON rk.naID = na.naID ";

      break;

    case CP_DOUBLE :
    case CP_MIXED :
      str = "SELECT rkNatlRank, rkIntlRank, rkDirectEntry, "
            "       plplNr, plpsLast, plpsFirst, plnaName, plRankPts, plltRankPts, "
            "       bdplNr, bdpsLast, bdpsFirst, bdnaName, bdRankPts, bdltRankPts, "
            "       (plltRankPts + bdltRankPts) AS rankPts, tmID, cpID, rk.naID "
            "  FROM RkDoubleList rk INNER JOIN NaList na ON rk.naID = na.naID ";

      break;

    case CP_TEAM :
      str = "SELECT rkNatlRank, rkIntlRank, rkDirectEntry, "
            "       tmName, tmDesc, rk.naName,                "
            "       (SELECT SUM(ltRankPts) FROM NtEntryList nt "
            "         WHERE nt.tmID = rk.tmID "
            "           AND nt.ntNr <= ISNULL(sy.sySingles, 3) "
            "       ) AS rankPts, "
            "       rk.tmID, cp.cpID, rk.naID "
            "  FROM RkTeamList rk INNER JOIN NaList na ON rk.naID = na.naID "
            "       INNER JOIN CpList cp ON rk.cpID = cp.cpID INNER JOIN SyList sy ON cp.syID = sy.syID ";
      break;

    default :
      str = "";
      break;
  }

  return str;
}


bool  RkEntryStore::Next()
{
  if (!StoreObj::Next())
    return false;

  // Doppelte IDs kopieren
  rk.tmID = tmID;
  rk.cpID = cpID;
  rk.naID = naID;

  return true;
}


bool  RkEntryStore::BindRec()
{
  int col = 0;

  switch (team.cpType)
  {
    case CP_SINGLE :
      BindCol(++col, &rk.rkNatlRank);
      BindCol(++col, &rk.rkIntlRank);
      BindCol(++col, &rk.rkDirectEntry);

      BindCol(++col, &team.pl.plNr);
      BindCol(++col, team.pl.psName.psLast, sizeof(team.pl.psName.psLast));
      BindCol(++col, team.pl.psName.psFirst, sizeof(team.pl.psName.psFirst));
      BindCol(++col, team.pl.naName, sizeof(team.pl.naName));
      BindCol(++col, &team.pl.plRankPts);
      BindCol(++col, &team.pl.ltRankPts);
      
      BindCol(++col, &rankPts);

      BindCol(++col, &tmID);
      BindCol(++col, &cpID);
      BindCol(++col, &naID);

      break;

    case CP_DOUBLE :
    case CP_MIXED  :
      BindCol(++col, &rk.rkNatlRank);
      BindCol(++col, &rk.rkIntlRank);
      BindCol(++col, &rk.rkDirectEntry);

      BindCol(++col, &team.pl.plNr);
      BindCol(++col, team.pl.psName.psLast, sizeof(team.pl.psName.psLast));
      BindCol(++col, team.pl.psName.psFirst, sizeof(team.pl.psName.psFirst));
      BindCol(++col, team.pl.naName, sizeof(team.pl.naName));
      BindCol(++col, &team.pl.plRankPts);
      BindCol(++col, &team.pl.ltRankPts);

      BindCol(++col, &team.bd.plNr);
      BindCol(++col, team.bd.psName.psLast, sizeof(team.bd.psName.psLast));
      BindCol(++col, team.bd.psName.psFirst, sizeof(team.bd.psName.psFirst));
      BindCol(++col, team.bd.naName, sizeof(team.bd.naName));
      BindCol(++col, &team.bd.plRankPts);
      BindCol(++col, &team.bd.ltRankPts);

      BindCol(++col, &rankPts);

      BindCol(++col, &tmID);
      BindCol(++col, &cpID);
      BindCol(++col, &naID);

      break;

    case CP_TEAM :
      BindCol(++col, &rk.rkNatlRank);
      BindCol(++col, &rk.rkIntlRank);
      BindCol(++col, &rk.rkDirectEntry);
      
      BindCol(++col, team.tm.tmName, sizeof(team.tm.tmName));
      BindCol(++col, team.tm.tmDesc, sizeof(team.tm.tmDesc));
      BindCol(++col, team.tm.naName, sizeof(team.tm.naName));

      BindCol(++col, &rankPts);

      BindCol(++col, &tmID);
      BindCol(++col, &cpID);
      BindCol(++col, &naID);

      break;

    default :
      return false;
  }

  return true;
}