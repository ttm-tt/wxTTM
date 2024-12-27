/* Copyright (C) 2020 Christoph Theis */

#include  "stdafx.h"
#include  "ReportStore.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "SQLException.h"
#include  "InfoSystem.h"

#include  <stdio.h>
#include  <stdlib.h>


// -----------------------------------------------------------------------
// Neue Tabelle in DB erzeugen
bool  ReportStore::CreateView()
{
  CreateListK();
  
  return true;
}


bool ReportStore::CreateListK()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str = 
    "CREATE VIEW List_K (cpName, cpDesc, cpType, cpID, "
    "             plplNr, plpsLast, plpsFirst, plnaName, plnaDesc, plltTimestamp, "
    "             bdplNr, bdpsLast, bdpsFirst, bdnaName, bdnaDesc, bdltTimestamp, "
    "             ntNr, tmName, tmDesc, tmID) "
    "AS SELECT "
    "    cp.cpName, cp.cpDesc, cp.cpType, cp.cpID, "
    "    ltpl.plNr, ltpl.psLast, ltpl.psFirst, napl.naName, napl.naDesc, ltpl.ltTimestamp, "
    "    ltbd.plNr, ltbd.psLast, ltbd.psFirst, nabd.naName, nabd.naDesc, ltbd.ltTimestamp, "
    "    ntpl.ntNr, tm.tmName, tm.tmDesc, tm.tmID "
    "FROM "
    "    CpList cp "
    "      INNER JOIN LtEntryList ltpl ON cp.cpID = ltpl.cpID "
    "      INNER JOIN NaList napl ON ltpl.naID = napl.naID "
    "      LEFT OUTER JOIN (NtRec ntpl "
    "                         INNER JOIN TmList tm ON ntpl.tmID = tm.tmID "
    "                         INNER JOIN NtRec ntbd ON tm.tmID = ntbd.tmID "
    "                         INNER JOIN LtEntryList ltbd ON ntbd.ltID = ltbd.ltID "
    "                         INNER JOIN NaList nabd ON ltbd.naID = nabd.naID) "
    "      ON ltpl.ltID = ntpl.ltID "
    "         AND ((cp.cpType <> 4 AND ltpl.ltID <> ltbd.ltID) OR "
    "              (cp.cpType = 4 AND ltpl.ltID = ltbd.ltID)) ";

  try
  {
    tmp->ExecuteUpdate(str);
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  delete tmp;
  
  return true;
}


bool  ReportStore::RemoveView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  try
  {
    tmp->ExecuteUpdate("DROP VIEW IF EXISTS List_K");
  }
  catch(SQLException &)
  {
  }

  delete tmp;
  
  return true;
}


