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
                     "SELECT cpID, cpName, cpDesc, cpType, cpSex, cpYear, syID, cpBestOf "
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
    tmp->ExecuteUpdate("DROP VIEW CpList");
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
    "SELECT cpID, cpName, cpDesc, cpType, cpSex, cpYear, syID, cpBestOf FROM CpList ";

  return str;
}


bool  CpListStore::BindRec()
{
  BindCol(1, &cpID);
  BindCol(2, cpName, sizeof(cpName));
  BindCol(3, cpDesc, sizeof(cpDesc));
  BindCol(4, &cpType);
  BindCol(5, &cpSex);
  BindCol(6, &cpYear);
  BindCol(7, &syID);
  BindCol(8, &cpBestOf);

  return true;
}