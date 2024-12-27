/* Copyright (C) 2020 Christoph Theis */

// DB-Tabelle der Nationen

#include  "stdafx.h"
#include  "NaListStore.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "SQLException.h"
#include  "InfoSystem.h"

#include  <stdio.h>
#include  <stdlib.h>



// -----------------------------------------------------------------------
// Neue Tabelle in DB erzeugen
bool  NaListStore::CreateView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str =  "CREATE VIEW NaList AS "
                      "SELECT naID, naName, naDesc, naRegion FROM naRec";

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


bool  NaListStore::RemoveView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  try
  {
    tmp->ExecuteUpdate("DROP VIEW IF EXISTS NaList");
  }
  catch(SQLException &)
  {
  }

  delete tmp;
  
  return true;
}


// -----------------------------------------------------------------------
// Konstruktor, Destruktor
NaListStore::NaListStore(Connection *connPtr)
           : StoreObj(connPtr)
{
}


NaListStore::~NaListStore()
{
}


void NaListStore::Init()
{
  NaListRec::Init();
}


// -----------------------------------------------------------------------
// Select-Statements
bool  NaListStore::SelectAll()
{
  wxString  str = "SELECT naID, naName, naDesc, naRegion FROM NaList "
                     "ORDER BY naName";

  try
  {
    if (!ExecuteQuery(str))
      return false;
 
    BindCol(1, &naID);
    BindCol(2, naName, sizeof(naName));
    BindCol(3, naDesc, sizeof(naDesc));
    BindCol(4, naRegion, sizeof(naRegion));
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


bool  NaListStore::SelectById(long id)
{
  wxString str = "SELECT naId, naName, naDesc, naRegion FROM NaList WHERE naID = " + ltostr(id);

  try
  {
    if (!ExecuteQuery(str))
      return false;
 
    BindCol(1, &naID);
    BindCol(2, naName, sizeof(naName));
    BindCol(3, naDesc, sizeof(naDesc));
    BindCol(4, naRegion, sizeof(naRegion));
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


bool  NaListStore::SelectByName(const wxChar *name)
{
  wxString str = "SELECT naID, naName, naDesc,naRegion FROM NaList WHERE naName = '" + TransformString(name) + "'";

  try
  {
    if (!ExecuteQuery(str))
      return false;
 
    BindCol(1, &naID);
    BindCol(2, naName, sizeof(naName));
    BindCol(3, naDesc, sizeof(naDesc));
    BindCol(4, naRegion, sizeof(naRegion));
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}