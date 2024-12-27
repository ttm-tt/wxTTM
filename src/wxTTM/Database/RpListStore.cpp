/* Copyright (C) 2020 Christoph Theis */

// View fuer Rankingpunkte per Alterskategorie

#include  "stdafx.h"
#include  "RpListStore.h"
#include  "RpStore.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "SQLException.h"
#include  "InfoSystem.h"

#include  <stdio.h>
#include  <stdlib.h>


// -----------------------------------------------------------------------
RpListRec::RpListRec(const RpRec &rec)
{
  rpID = rec.rpID;
  plID = rec.plID;
  rpYear = rec.rpYear;
  rpRankPts = rec.rpRankPts;
}


// -----------------------------------------------------------------------
// Neue Tabelle in DB erzeugen
bool  RpListStore::CreateView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str = "CREATE VIEW RpList AS "
                     "SELECT rpID, plID, rpYear, rpRankPts "
                     "FROM RpRec";

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


bool  RpListStore::RemoveView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  try
  {
    tmp->ExecuteUpdate("DROP VIEW IF EXISTS RpList");
  }
  catch(SQLException &)
  {
  }

  delete tmp;
  
  return true;
}


// -----------------------------------------------------------------------
// Konstruktor, Destruktor
RpListStore::RpListStore(Connection *connPtr)
           : StoreObj(connPtr)
{
}


RpListStore::~RpListStore()
{
}


void  RpListStore::Init()
{
  RpListRec::Init();
}


// -----------------------------------------------------------------------
// Select-Statements
bool  RpListStore::SelectAll()
{
  wxString  str = SelectString();
  str += " ORDER BY RpYear DESC";

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


bool  RpListStore::SelectById(long id)
{
  wxString stmt = SelectString();
  stmt += " WHERE rpID = ";
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


// -----------------------------------------------------------------------
wxString  RpListStore::SelectString() const
{
  wxString  str = 
    "SELECT rpID, plID, rpYear, rpRankPts FROM RpList ";

  return str;
}


bool  RpListStore::BindRec()
{
  int col = 0;
  BindCol(++col, &rpID);
  BindCol(++col, &plID);
  BindCol(++col, &rpYear);
  BindCol(++col, &rpRankPts);

  return true;
}