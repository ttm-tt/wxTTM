/* Copyright (C) 2020 Christoph Theis */

// Setzung
#include  "stdafx.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "SQLException.h"
#include  "InfoSystem.h"

#include  "StEntryStore.h"

#include  "GrStore.h"

#include  "Rec.h"



// -----------------------------------------------------------------------
bool  StEntryStore::CreateView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str;
  
  // TODO: naDesc beruecksichtigen

  try
  {
    str = "CREATE VIEW StSingleList (                                         "
          " stID, grID, stNr, stPos, stSeeded, stGaveup, stDisqu, stNocons,   "
          " stTimestamp, "
          " plID, plNr, psLast, psFirst, naID, naName, naDesc, naRegion, plExtID, tmID, tmnaID)     "
          "AS                                                                 "
          "  SELECT stID, grID, stNr, stPos, stSeeded, stGaveup, stDisqu,     "
          "         stNocons, stTimestamp, "
          "         plID, plNr, psLast, psFirst, tm.naID, naName, naDesc, naRegion, plExtID, st.tmID, tm.tmnaID   "
          "    FROM StList st LEFT OUTER JOIN TmSingleList tm                 "
          "      ON st.tmID = tm.tmID ";
    tmp->ExecuteUpdate(str);

    str = "CREATE VIEW StDoubleList (                                         "
          "  stID, grID, stNr, stPos, stSeeded, stGaveup, stDisqu, stNocons,  "
          "  stTimestamp, "
          "  plplID, plplNr, plpsLast, plpsFirst, plnaID, plnaName, plnaDesc, plnaRegion, plplExtID,      "
          "  bdplID, bdplNr, bdpsLast, bdpsFirst, bdnaID, bdnaName, bdnaDesc, bdnaRegion, bdplExtID,      "
          "  tmID, tmnaID)                                                    "
          "AS                                                                 "
          "  SELECT stID, grID, stNr, stPos, stSeeded, stGaveup, stDisqu,     "
          "         stNocons, stTimestamp, "
          "         plplID, plplNr, plpsLast, plpsFirst, tm.plnaID, plnaName, plnaDesc, plnaRegion, plplExtID, "
          "         bdplID, bdplNr, bdpsLast, bdpsFirst, tm.bdnaID, bdnaName, bdnaDesc, bdnaRegion, bdplExtID, "
          "         st.tmID, tm.tmnaID  "
          "    FROM StList st LEFT OUTER JOIN TmDoubleList tm                 "
          "      ON st.tmID = tm.tmID ";
    tmp->ExecuteUpdate(str);

    str = "CREATE VIEW StTeamList (                                           "
          "  stID, grID, stNr, stPos, stSeeded, stGaveup, stDisqu, stNocons,  "
          "  stTimestamp, "
          "  tmID, tmName, tmDesc, naName, naDesc, naRegion, tmnaID)          "
          "AS                                                                 "
          "  SELECT stID, grID, stNr, stPos, stSeeded, stGaveup, stDisqu,     "
          "         stNocons, stTimestamp, "
          "         st.tmID, tmName, tmDesc, naName, naDesc, naRegion, tm.tmnaId        "
          "    FROM StList st LEFT OUTER JOIN TmTeamList tm                   "
          "      ON st.tmID = tm.tmID ";
    tmp->ExecuteUpdate(str);
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  delete tmp;
  
  return true;
}


bool  StEntryStore::RemoveView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  try
  {
    tmp->ExecuteUpdate("DROP VIEW StSingleList");
    tmp->ExecuteUpdate("DROP VIEW StDoubleList");
    tmp->ExecuteUpdate("DROP VIEW StTeamList");
  }
  catch(SQLException &)
  {
  }

  delete tmp;
  
  return true;
}


// -----------------------------------------------------------------------
StEntryStore::StEntryStore(Connection *ptr)
            : StoreObj(ptr)
{
  cpType = 0;
}


void  StEntryStore::Init()
{
  StEntry::Init();
  
  team.cpType = cpType;
}


bool  StEntryStore::Next()
{
  if (!StoreObj::Next())
    return false;
    
  st.tmID = tmID;

  if (tmID || !team.gr.xxStID)
    return true;

  team.cpType = CP_GROUP;

  tmID = team.gr.grID;

  return true;
}


// -----------------------------------------------------------------------
bool  StEntryStore::SelectBySt(const StRec &st, short type)
{
  cpType = team.cpType = type;

  wxString  str = SelectString();
  str += "WHERE stID = " + ltostr(st.stID);

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


bool  StEntryStore::SelectByGr(const GrRec &gr, short type)
{
  // TODO: PreparedStatement
  cpType = team.cpType = type;

  wxString  str = SelectString();
  str += " WHERE st.grID = " + ltostr(gr.grID) + " ORDER BY stNr";

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


bool  StEntryStore::SelectById(long id, short type)
{
  cpType = team.cpType = type;

  wxString  str = SelectString();
  str += " WHERE st.stID = " + ltostr(id) + " ORDER BY st.stNr";

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


bool  StEntryStore::SelectById(const std::set<long> &ids, short type)
{
  // TODO: PreparedStatement
  cpType = team.cpType = type;

  wxString  str = SelectString();
  str += " WHERE st.stID IN (" + ltostr(ids) + ") ORDER BY st.grID, st.stNr";

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


bool  StEntryStore::SelectByNr(const GrRec &gr, short nr, short type)
{
  cpType = team.cpType = type;

  wxString  str = SelectString();
  str += " WHERE st.grID = " + ltostr(gr.grID) + " AND st.stNr = " + ltostr(nr);

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


// ----------------------------------------------------------------------
bool  StEntryStore::SelectAll(const CpRec &cp, const wxString &stage)
{
  cpType = team.cpType = cp.cpType;

  wxString str = SelectString();
  str += " WHERE st.grID IN ";
  str += "(SELECT grID FROM GrRec WHERE cpID = ";
  str += ltostr(cp.cpID);
  str += " AND grStage = '";
  str += stage;
  str += "') ORDER BY st.grID, stPos";

  try
  {
    if (!ExecuteQuery(str))
      return false;

    BindRec();
    return true;
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


// -----------------------------------------------------------------------
wxString  StEntryStore::SelectString() const
{
  wxString  str;

  switch (cpType)
  {
    case CP_SINGLE :
      str = "SELECT st.stID AS \"stID\", st.grID AS \"grID\",           "
            "       stNr, stPos, stSeeded, stGaveup, stDisqu, stNocons, "
            "       plNr, psLast, psFirst, naName, naDesc, naRegion, tmID, tmnaID,  "
            "       grQual.grName, grQual.grDesc, xx.stID, xx.grPos, xx.grID     "
            "FROM StSingleList st "
            "LEFT OUTER JOIN (XxRec xx LEFT OUTER JOIN GrRec grQual ON xx.grID = grQual.grID) "
            "             ON st.stID = xx.stID ";

      break;

    case CP_DOUBLE :
    case CP_MIXED :
      str = "SELECT st.stID AS \"stID\", st.grID AS \"grID\",           "
            "       stNr, stPos, stSeeded, stGaveup, stDisqu, stNocons, "
            "       plplNr, plpsLast, plpsFirst, plnaName, plnaDesc, plnaRegion,   "
            "       bdplNr, bdpsLast, bdpsFirst, bdnaName, bdnaDesc, bdnaRegion,   "
            "       tmID, tmnaID, "
            "       grQual.grName, grQual.grDesc, xx.stID, xx.grPos, xx.grID     "
            "  FROM StDoubleList st "
            "LEFT OUTER JOIN (XxRec xx LEFT OUTER JOIN GrRec grQual ON xx.grID = grQual.grID) "
            "             ON st.stID = xx.stID ";

      break;

    case CP_TEAM :
      str = "SELECT st.stID AS \"stID\", st.grID AS \"grID\",           "
            "       stNr, stPos, stSeeded, stGaveup, stDisqu, stNocons, "
            "       tmName, tmDesc, naName, naDesc, naRegion, tmID, tmnaID,       "
            "       grQual.grName, grQual.grDesc, xx.stID, xx.grPos, xx.grID     "
            "FROM StTeamList st "
            "LEFT OUTER JOIN (XxRec xx LEFT OUTER JOIN GrRec grQual ON xx.grID = grQual.grID) "
            "             ON st.stID = xx.stID ";

      break;

    default :
      str = "";
      break;
  }

  return str;
}


bool  StEntryStore::BindRec()
{
  int col = 0;

  switch (cpType)
  {
    case CP_SINGLE:
      BindCol(++col, &st.stID);
      BindCol(++col, &st.grID);
      BindCol(++col, &st.stNr);
      BindCol(++col, &st.stPos);
      BindCol(++col, &st.stSeeded);
      BindCol(++col, &st.stGaveup);
      BindCol(++col, &st.stDisqu);
      BindCol(++col, &st.stNocons);

      BindCol(++col, &team.pl.plNr);
      BindCol(++col, team.pl.psName.psLast, sizeof(team.pl.psName.psLast));
      BindCol(++col, team.pl.psName.psFirst, sizeof(team.pl.psName.psFirst));
      BindCol(++col, team.pl.naName, sizeof(team.pl.naName));
      BindCol(++col, team.pl.naDesc, sizeof(team.pl.naDesc));
      BindCol(++col, team.pl.naRegion, sizeof(team.pl.naRegion));

      BindCol(++col, &tmID);
      BindCol(++col, &naID);

      BindCol(++col, team.gr.grName, sizeof(team.gr.grName));
      BindCol(++col, team.gr.grDesc, sizeof(team.gr.grDesc));
      BindCol(++col, &team.gr.xxStID);
      BindCol(++col, &team.gr.grPos);
      BindCol(++col, &team.gr.grID);

      break;

    case CP_DOUBLE:
    case CP_MIXED:
      BindCol(++col, &st.stID);
      BindCol(++col, &st.grID);
      BindCol(++col, &st.stNr);
      BindCol(++col, &st.stPos);
      BindCol(++col, &st.stSeeded);
      BindCol(++col, &st.stGaveup);
      BindCol(++col, &st.stDisqu);
      BindCol(++col, &st.stNocons);

      BindCol(++col, &team.pl.plNr);
      BindCol(++col, team.pl.psName.psLast, sizeof(team.pl.psName.psLast));
      BindCol(++col, team.pl.psName.psFirst, sizeof(team.pl.psName.psFirst));
      BindCol(++col, team.pl.naName, sizeof(team.pl.naName));
      BindCol(++col, team.pl.naDesc, sizeof(team.pl.naDesc));
      BindCol(++col, team.pl.naRegion, sizeof(team.pl.naRegion));

      BindCol(++col, &team.bd.plNr);
      BindCol(++col, team.bd.psName.psLast, sizeof(team.bd.psName.psLast));
      BindCol(++col, team.bd.psName.psFirst, sizeof(team.bd.psName.psFirst));
      BindCol(++col, team.bd.naName, sizeof(team.bd.naName));
      BindCol(++col, team.bd.naDesc, sizeof(team.bd.naDesc));
      BindCol(++col, team.bd.naRegion, sizeof(team.bd.naRegion));

      BindCol(++col, &tmID);
      BindCol(++col, &naID);

      BindCol(++col, team.gr.grName, sizeof(team.gr.grName));
      BindCol(++col, team.gr.grDesc, sizeof(team.gr.grDesc));
      BindCol(++col, &team.gr.xxStID);
      BindCol(++col, &team.gr.grPos);
      BindCol(++col, &team.gr.grID);

      break;

    case CP_TEAM:
      BindCol(++col, &st.stID);
      BindCol(++col, &st.grID);
      BindCol(++col, &st.stNr);
      BindCol(++col, &st.stPos);
      BindCol(++col, &st.stSeeded);
      BindCol(++col, &st.stGaveup);
      BindCol(++col, &st.stDisqu);
      BindCol(++col, &st.stNocons);

      BindCol(++col, team.tm.tmName, sizeof(team.tm.tmName));
      BindCol(++col, team.tm.tmDesc, sizeof(team.tm.tmDesc));
      BindCol(++col, team.tm.naName, sizeof(team.tm.naName));
      BindCol(++col, team.tm.naDesc, sizeof(team.tm.naDesc));
      BindCol(++col, team.tm.naRegion, sizeof(team.tm.naRegion));

      BindCol(++col, &tmID);
      BindCol(++col, &naID);

      BindCol(++col, team.gr.grName, sizeof(team.gr.grName));
      BindCol(++col, team.gr.grDesc, sizeof(team.gr.grDesc));
      BindCol(++col, &team.gr.xxStID);
      BindCol(++col, &team.gr.grPos);
      BindCol(++col, &team.gr.grID);

      break;

    default:
      return false;
  }

  return true;
}