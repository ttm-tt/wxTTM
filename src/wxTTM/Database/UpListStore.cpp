/* Copyright (C) 2020 Christoph Theis */

// DB-Liste der Spieler

#include  "stdafx.h"
#include  "UpListStore.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "SQLException.h"
#include  "InfoSystem.h"

#include  "UpStore.h"

#include  "Rec.h"

#include  <stdio.h>
#include  <stdlib.h>


// -----------------------------------------------------------------------
// Neue Tabelle in DB erzeugen
bool  UpListStore::CreateView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str = 
      "CREATE VIEW UpList AS "
      " SELECT upID, upNr, UpRec.psID, psLast, psFirst, psSex, "
      "        UpRec.naID, naName, naDesc, naRegion, psEmail, psPhone, "
      "        psTimestamp "
      " FROM (PsRec INNER JOIN UpRec ON PsRec.psID = UpRec.psID) "
      "       LEFT OUTER JOIN NaRec ON UpRec.naID = NaRec.naID";

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


bool  UpListStore::RemoveView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  try
  {
    tmp->ExecuteUpdate("DROP VIEW IF EXISTS UpList");
  }
  catch(SQLException &)
  {
  }

  delete tmp;
  
  return true;
}


// -----------------------------------------------------------------------
// Konstruktor, Destruktor
UpListStore::UpListStore(Connection *connPtr)
           : StoreObj(connPtr)
{
}


UpListStore::~UpListStore()
{
}


void  UpListStore::Init()
{
  UpListRec::Init();
}


// -----------------------------------------------------------------------
// Select-Statements
bool  UpListStore::SelectAll()
{
  wxString  str = SelectString();
  str += " ORDER BY upNr";

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


bool  UpListStore::SelectById(long id)
{
  wxString  stmt = SelectString();
  stmt += "WHERE upID = ";
  stmt += ltostr(id);

  try
  {
    if (!ExecuteQuery(stmt))
      return false;

    BindRec();
     
    return true;
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(stmt, e);
    return false;
  }

  return true;
}


bool  UpListStore::SelectByNr(long nr)
{
  wxString  stmt = SelectString();
  stmt += "WHERE upNr = ";
  stmt += ltostr(nr);

  try
  {
    if (!ExecuteQuery(stmt))
      return false;

    BindRec();
     
    return true;
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(stmt, e);
    return false;
  }

  return true;
}


long UpListStore::Count()
{
  Statement *stmtPtr = NULL;
  ResultSet *resPtr = NULL;

  wxString str = "SELECT COUNT(*) FROM UpList";

  long count = 0;

  try
  {
    stmtPtr = GetConnectionPtr()->CreateStatement();

    ResultSet *resPtr = stmtPtr->ExecuteQuery(str);
    if (!resPtr || !resPtr->Next())
      count = 0;
    else if (!resPtr->GetData(1, count) || resPtr->WasNull())
      count = 0;
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    count = 0;
  }

  delete resPtr;
  delete stmtPtr;

  return count;
}

// -----------------------------------------------------------------------
wxString  UpListStore::SelectString() const
{
  wxString  str = 
      "SELECT upID, upNr, psID, psLast, psFirst, psSex, "
      "       naID, naName, naDesc, naRegion, psEmail, psPhone "
      "FROM UpList ";

  return str;  
}


bool  UpListStore::BindRec()
{
  BindCol(1, &upID);
  BindCol(2, &upNr);
  BindCol(3, &psID);
  BindCol(4, psName.psLast, sizeof(psName.psLast));
  BindCol(5, psName.psFirst, sizeof(psName.psFirst));
  BindCol(6, &psSex);
  BindCol(7, &naID);
  BindCol(8, naName, sizeof(naName));
  BindCol(9, naDesc, sizeof(naDesc));
  BindCol(10, naRegion, sizeof(naRegion));
  BindCol(12, psEmail, sizeof(psEmail));
  BindCol(13, psPhone, sizeof(psPhone));

  return true;
}