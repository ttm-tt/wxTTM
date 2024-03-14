/* Copyright (C) 2020 Christoph Theis */

// DB-Tabelle der Spielmodi

#include  "stdafx.h"
#include  "MdListStore.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "SQLException.h"
#include  "InfoSystem.h"

#include  <stdio.h>
#include  <stdlib.h>



// -----------------------------------------------------------------------
// Neue Tabelle in DB erzeugen
bool  MdListStore::CreateView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str = "CREATE VIEW MdList AS "
                     "SELECT mdID, mdName, mdDesc, mdSize, mpID FROM MdRec";

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


bool  MdListStore::RemoveView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  try
  {
    tmp->ExecuteUpdate("DROP VIEW MdList");
  }
  catch(SQLException &)
  {
  }

  delete tmp;
  
  return true;
}


// -----------------------------------------------------------------------
// Konstruktor, Destruktor
MdListStore::MdListStore(Connection *connPtr)
           : StoreObj(connPtr)
{
}


MdListStore::~MdListStore()
{
}


void  MdListStore::Init()
{
  MdListRec::Init();
}


// -----------------------------------------------------------------------
// Select-Statements
bool  MdListStore::SelectAll()
{
  wxString  str = SelectString(); 
  str += " ORDER BY mdName";

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


bool  MdListStore::SelectById(long id)
{
  wxString  stmt = SelectString();
  stmt += "WHERE mdID = ";
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


bool  MdListStore::SelectById(const std::set<long> mdIdList)
{
  wxString  stmt = SelectString();
  stmt += "WHERE mdID IN (";
  stmt += ltostr(mdIdList);
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


// -----------------------------------------------------------------------
wxString  MdListStore::SelectString() const
{
  wxString  str = 
    "SELECT mdID, mdName, mdDesc, mdSize FROM MdList ";

  return str;
}


bool  MdListStore::BindRec()
{
  BindCol(1, &mdID);
  BindCol(2, mdName, sizeof(mdName));
  BindCol(3, mdDesc, sizeof(mdDesc));
  BindCol(4, &mdSize);

  return true;  
}