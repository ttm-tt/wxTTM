/* Copyright (C) 2020 Christoph Theis */

#include  "stdafx.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "SQLException.h"
#include  "InfoSystem.h"

#include "TbEntryStore.h"


// -----------------------------------------------------------------------
bool  TbEntryStore::CreateView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str;

  // TODO: naDesc beruecksichtigen

  try
  {
    str = "CREATE VIEW TbSingleList (                                         "
          " stID, grID, stNr, stPos, stSeeded, stGaveup, stDisqu, stNocons, stTimestamp, "
          " plNr, psLast, psFirst, naID, naName, naDesc, plExtID, tmID, tmnaID,     "
          " tbMatchCount, tbMatchPoints, tbPointsA, tbPointsX, tbMatchesA, tbMatchesX, "
          " tbSetsA, tbSetsX, tbBallsA, tbBallsX, tbPos, tbID) "
          "AS                                                                 "
          "  SELECT st.stID, st.grID, st.stNr, st.stPos, stSeeded, stGaveup, stDisqu,     "
          "         stNocons, stTimestamp, "
          "         plNr, psLast, psFirst, st.naID, naName, naDesc, plExtID, st.tmID, st.tmnaID,   "
          "         tb.mtMatchCount, tb.mtMatchPoints, tb.mtPointsA, tb.mtPointsX, "
          "         tb.mtMatchesA, tb.mtMatchesX, tb.mtSetsA, tb.mtSetsX, tb.mtBallsA, tb.mtBallsX, "
          "         tb.stPos, tb.stID "
          "    FROM StSingleList st CROSS APPLY TbSortFunc(grID) tb "
          "   WHERE st.stID = tb.stID "
    ;
    tmp->ExecuteUpdate(str);

    str = "CREATE VIEW TbDoubleList (                                         "
          "  stID, grID, stNr, stPos, stSeeded, stGaveup, stDisqu, stNocons,  "
          "  stTimestamp, "
          "  plplNr, plpsLast, plpsFirst, plnaID, plnaName, plnaDesc, plplExtID,      "
          "  bdplNr, bdpsLast, bdpsFirst, bdnaID, bdnaName, bdnaDesc, bdplExtID,      "
          "  tmID, tmnaID,                                                     "
          "  tbMatchCount, tbMatchPoints, tbPointsA, tbPointsX, tbMatchesA, tbMatchesX, "
          "  tbSetsA, tbSetsX, tbBallsA, tbBallsX, tbPos, tbID) "
          "AS                                                                 "
          "  SELECT st.stID, st.grID, st.stNr, st.stPos, stSeeded, stGaveup, stDisqu,     "
          "         stNocons, stTimestamp, "
          "         plplNr, plpsLast, plpsFirst, st.plnaID, plnaName, plnaDesc, plplExtID, "
          "         bdplNr, bdpsLast, bdpsFirst, st.bdnaID, bdnaName, bdnaDesc, bdplExtID, "
          "         st.tmID, st.tmnaID,  "
          "         tb.mtMatchCount, tb.mtMatchPoints, tb.mtPointsA, tb.mtPointsX, "
          "         tb.mtMatchesA, tb.mtMatchesX, tb.mtSetsA, tb.mtSetsX, tb.mtBallsA, tb.mtBallsX, "
          "         tb.stPos, tb.stID "
          "    FROM StDoubleList st CROSS APPLY TbSortFunc(grID) tb "
          "   WHERE st.stID = tb.stID "
      ;
    tmp->ExecuteUpdate(str);

    str = "CREATE VIEW TbTeamList (                                           "
          "  stID, grID, stNr, stPos, stSeeded, stGaveup, stDisqu, stNocons,  "
          "  stTimestamp, "
          "  tmName, tmDesc, naName, naDesc, tmID, tmnaID,                    "
          "  tbMatchCount, tbMatchPoints, tbPointsA, tbPointsX, tbMatchesA, tbMatchesX, "
          "  tbSetsA, tbSetsX, tbBallsA, tbBallsX, tbPos, tbID) "
          "AS                                                                 "
          "  SELECT st.stID, st.grID, st.stNr, st.stPos, stSeeded, stGaveup, stDisqu,     "
          "         stNocons, stTimestamp, "
          "         tmName, tmDesc, naName, naDesc, st.tmID, st.tmnaId,        "
          "         tb.mtMatchCount, tb.mtMatchPoints, tb.mtPointsA, tb.mtPointsX, "
          "         tb.mtMatchesA, tb.mtMatchesX, tb.mtSetsA, tb.mtSetsX, tb.mtBallsA, tb.mtBallsX, "
          "         tb.stPos, tb.stID "
          "    FROM StTeamList st CROSS APPLY TbSortFunc(grID) tb "
          "   WHERE st.stID = tb.stID "
      ;
    tmp->ExecuteUpdate(str);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  delete tmp;

  return true;
}


bool  TbEntryStore::RemoveView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  try
  {
    tmp->ExecuteUpdate("DROP VIEW IF EXISTS TbSingleList");
    tmp->ExecuteUpdate("DROP VIEW IF EXISTS TbDoubleList");
    tmp->ExecuteUpdate("DROP VIEW IF EXISTS TbTeamList");
  }
  catch (SQLException &)
  {
  }

  delete tmp;

  return true;
}


// -----------------------------------------------------------------------
TbEntryStore::TbEntryStore(Connection *connPtr) : StoreObj(connPtr)
{
  cpType = 0;
}


void TbEntryStore::Init()
{
  TbEntry::Init();
}


bool  TbEntryStore::SelectByGr(const GrRec &gr, short type)
{
  team.cpType = cpType = type;

  wxString  str = SelectString(gr.grID);
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


wxString TbEntryStore::SelectString(long grID) const
{
  wxString  str;

  switch (team.cpType)
  {
    case CP_SINGLE:
      str = 
        "SELECT st.stID AS \"stID\", st.grID AS \"grID\",           "
        "       stNr, st.stPos, stSeeded, stGaveup, stDisqu, stNocons, "
        "       plNr, psLast, psFirst, naName, naDesc, tmID, tmnaID,  "
        "       grQual.grName, grQual.grDesc, xx.stID, xx.grPos, xx.grID,   "
        "       tbMatchCount, tbMatchPoints, tbPointsA, tbPointsX, "
        "       tbMatchesA, tbMatchesX, tbSetsA, tbSetsX, tbBallsA, tbBallsX, "
        "       tbPos "
        "  FROM TbSingleList st "
        "       LEFT OUTER JOIN (XxRec xx LEFT OUTER JOIN GrRec grQual ON xx.grID = grQual.grID) ON st.stID = xx.stID "
        ;

      break;

    case CP_DOUBLE:
    case CP_MIXED:
      str = 
        "SELECT st.stID AS \"stID\", st.grID AS \"grID\",           "
        "       stNr, st.stPos, stSeeded, stGaveup, stDisqu, stNocons, "
        "       plplNr, plpsLast, plpsFirst, plnaName, plnaDesc,    "
        "       bdplNr, bdpsLast, bdpsFirst, bdnaName, bdnaDesc,    "
        "       tmID, tmnaID, "
        "       grQual.grName, grQual.grDesc, xx.stID, xx.grPos, xx.grID,     "
        "       tbMatchCount, tbMatchPoints, tbPointsA, tbPointsX, "
        "       tbMatchesA, tbMatchesX, tbSetsA, tbSetsX, tbBallsA, tbBallsX, "
        "       tbPos "
        "  FROM TbDoubleList st "
        "       LEFT OUTER JOIN (XxRec xx LEFT OUTER JOIN GrRec grQual ON xx.grID = grQual.grID) ON st.stID = xx.stID "
        ;

      break;

    case CP_TEAM:
      str = 
        "SELECT st.stID AS \"stID\", st.grID AS \"grID\",           "
        "       stNr, st.stPos, stSeeded, stGaveup, stDisqu, stNocons, "
        "       tmName, tmDesc, naName, naDesc, tmID, tmnaID,       "
        "       grQual.grName, grQual.grDesc, xx.stID, xx.grPos, xx.grID,     "
        "       tbMatchCount, tbMatchPoints, tbPointsA, tbPointsX, "
        "       tbMatchesA, tbMatchesX, tbSetsA, tbSetsX, tbBallsA, tbBallsX, "
        "       tbPos "
        "  FROM TbTeamList st "
        "       LEFT OUTER JOIN (XxRec xx LEFT OUTER JOIN GrRec grQual ON xx.grID = grQual.grID) ON st.stID = xx.stID "
        ;

      break;

    default:
      str = "";
      break;
  }

  return str;
}


bool  TbEntryStore::Next()
{
  if (!StoreObj::Next())
    return false;

  team.cpType = cpType;

  st.tmID = tmID;

  if (tmID || !team.gr.xxStID)
    return true;

  team.cpType = CP_GROUP;

  tmID = team.gr.grID;

  return true;
}


bool  TbEntryStore::BindRec()
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

      BindCol(++col, &tmID);
      BindCol(++col, &naID);

      BindCol(++col, team.gr.grName, sizeof(team.gr.grName));
      BindCol(++col, team.gr.grDesc, sizeof(team.gr.grDesc));
      BindCol(++col, &team.gr.xxStID);
      BindCol(++col, &team.gr.grPos);
      BindCol(++col, &team.gr.grID);

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

      BindCol(++col, &team.bd.plNr);
      BindCol(++col, team.bd.psName.psLast, sizeof(team.bd.psName.psLast));
      BindCol(++col, team.bd.psName.psFirst, sizeof(team.bd.psName.psFirst));
      BindCol(++col, team.bd.naName, sizeof(team.bd.naName));
      BindCol(++col, team.bd.naDesc, sizeof(team.bd.naDesc));

      BindCol(++col, &tmID);
      BindCol(++col, &naID);

      BindCol(++col, team.gr.grName, sizeof(team.gr.grName));
      BindCol(++col, team.gr.grDesc, sizeof(team.gr.grDesc));
      BindCol(++col, &team.gr.xxStID);
      BindCol(++col, &team.gr.grPos);
      BindCol(++col, &team.gr.grID);

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

      BindCol(++col, &tmID);
      BindCol(++col, &naID);

      BindCol(++col, team.gr.grName, sizeof(team.gr.grName));
      BindCol(++col, team.gr.grDesc, sizeof(team.gr.grDesc));
      BindCol(++col, &team.gr.xxStID);
      BindCol(++col, &team.gr.grPos);
      BindCol(++col, &team.gr.grID);

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

      break;

    default:
      return false;
  }

  return true;
}