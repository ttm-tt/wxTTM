/* Copyright (C) 2020 Christoph Theis */

// DB-Tabelle der Wettbewerbe

#include  "stdafx.h"
#include  "SyListStore.h"

#include  "GrStore.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "SQLException.h"
#include  "InfoSystem.h"

#include  <stdio.h>
#include  <stdlib.h>



// -----------------------------------------------------------------------
// Create new view in database
bool  SyListStore::CreateView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str = "CREATE VIEW SyList AS "
                     "SELECT syID, syName, syDesc, syMatches, sySingles, syDoubles, syComplete "
                     "  FROM SyRec";

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


bool  SyListStore::RemoveView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  try
  {
    tmp->ExecuteUpdate("DROP VIEW SyList");
  }
  catch(SQLException &)
  {
  }

  delete tmp;
  
  return true;
}


// -----------------------------------------------------------------------
// Konstruktor, Destruktor
SyListStore::SyListStore(Connection *connPtr)
           : StoreObj(connPtr)
{
}


SyListStore::~SyListStore()
{
}


void  SyListStore::Init()
{
  SyListRec::Init();
}


bool  SyListStore::Next()
{
  if (!StoreObj::Next())
    return false;

  delete syList;
  syList = new SyList[syMatches];

  Connection *connPtr = TTDbse::instance()->GetNewConnection();

  try
  {
    // System einlesen
    SyMatchStore  syMatch(connPtr);
    syMatch.Select(syID);
    while (syMatch.Next())
    {
      syList[syMatch.syNr-1].syType    = syMatch.syType;
      syList[syMatch.syNr-1].syPlayerA = syMatch.syPlayerA;
      syList[syMatch.syNr-1].syPlayerX = syMatch.syPlayerX;
    }
  }
  catch (SQLException &e)
  {
    infoSystem.Exception("SELECT ... FROM SyMatch WHERE ...", e);
    delete connPtr;
    return false;
  }

  delete connPtr;

  return true;
}


// -----------------------------------------------------------------------
// Select-Statements
bool  SyListStore::SelectAll()
{
  wxString stmt = SelectString();
  stmt += " ORDER BY syName";

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


bool  SyListStore::SelectById(long id)
{
  wxString stmt = SelectString();
  stmt += " WHERE syID = ";
  stmt += ltostr(id);

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


bool  SyListStore::SelectByGr(const GrRec &gr)
{
  wxString stmt = SelectString();
  stmt += " WHERE syID = (SELECT syID FROM GrRec WHERE grID = " + ltostr(gr.grID) + ")";

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


wxString SyListStore::SelectString() const
{
  return 
    "SELECT syID, syName, syDesc, syMatches, sySingles, syDoubles, syComplete FROM SyList ";
}


bool SyListStore::BindRec() 
{
  BindCol(1, &syID);
  BindCol(2, syName, sizeof(syName));
  BindCol(3, syDesc, sizeof(syDesc));
  BindCol(4, &syMatches);
  BindCol(5, &sySingles);
  BindCol(6, &syDoubles);
  BindCol(7, &syComplete);
  
  return true;
}  