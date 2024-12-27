/* Copyright (C) 2020 Christoph Theis */

// DB-Tabelle der Wettbewerbe

#include  "stdafx.h"
#include  "CpListStore.h"
#include  "CpStore.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "SQLException.h"
#include  "InfoSystem.h"

#include  <stdio.h>
#include  <stdlib.h>


// -----------------------------------------------------------------------
CpListRec::CpListRec(const CpRec &rec)
{
  cpID = rec.cpID;
  cpType = rec.cpType;
  cpSex  = rec.cpSex;
  wxStrcpy(cpName, rec.cpName);
  wxStrcpy(cpDesc, rec.cpDesc);
}


// -----------------------------------------------------------------------
// Neue Tabelle in DB erzeugen
bool  CpListStore::CreateView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str = "CREATE VIEW CpList AS "
                     "SELECT cpID, cpName, cpDesc, cpCategory, cpType, cpSex, cpYear, syID, "
                     "       cpBestOf, cpPtsToWin, cpPtsAhead, cpPtsToWinLast, cpPtsAheadLast "
                     "FROM cpRec";

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


bool  CpListStore::RemoveView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  try
  {
    tmp->ExecuteUpdate("DROP VIEW IF EXISTS CpList");
  }
  catch(SQLException &)
  {
  }

  delete tmp;
  
  return true;
}


// -----------------------------------------------------------------------
// Konstruktor, Destruktor
CpListStore::CpListStore(Connection *connPtr)
           : StoreObj(connPtr)
{
}


CpListStore::~CpListStore()
{
}


void  CpListStore::Init()
{
  CpListRec::Init();
}


// -----------------------------------------------------------------------
// Select-Statements
bool  CpListStore::SelectAll()
{
  wxString  str = SelectString();
  str += " ORDER BY cpName";

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


bool  CpListStore::SelectById(long id)
{
  wxString stmt = SelectString();
  stmt += " WHERE cpID = ";
  stmt += ltostr(id);
  stmt += "";

  try
  {
    if (!ExecuteQuery(stmt))
      return false;

    BindRec(); 
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(stmt, e);
    return false;
  }

  return true;
}


bool  CpListStore::SelectById(const std::set<long> &ids)
{
  wxString stmt = SelectString();
  stmt += " WHERE cpID IN (";
  stmt += ltostr(ids);
  stmt += ")";

  try
  {
    if (!ExecuteQuery(stmt))
      return false;

    BindRec();
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(stmt, e);
    return false;
  }

  return true;
}


bool  CpListStore::SelectByName(const wxString &name)
{
  wxString str = SelectString();
  str += " WHERE cpName = '";
  str += name;
  str += "'";

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
wxString  CpListStore::SelectString() const
{
  wxString  str = 
    "SELECT cpID, cpName, cpDesc, cpCategory, cpType, cpSex, cpYear, syID, "
    "       cpBestOf, cpPtsToWin, cpPtsToWinLast, cpPtsAhead, cpPtsAheadLast FROM CpList ";

  return str;
}


bool  CpListStore::BindRec()
{
  int idx = 0;

  BindCol(++idx, &cpID);
  BindCol(++idx, cpName, sizeof(cpName));
  BindCol(++idx, cpDesc, sizeof(cpDesc));
  BindCol(++idx, cpCategory, sizeof(cpCategory));
  BindCol(++idx, &cpType);
  BindCol(++idx, &cpSex);
  BindCol(++idx, &cpYear);
  BindCol(++idx, &syID);
  BindCol(++idx, &cpBestOf);
  BindCol(++idx, &cpPtsToWin);
  BindCol(++idx, &cpPtsToWinLast);
  BindCol(++idx, &cpPtsAhead);
  BindCol(++idx, &cpPtsAheadLast);

  return true;
}