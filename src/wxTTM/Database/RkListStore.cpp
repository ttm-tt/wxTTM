/* Copyright (C) 2020 Christoph Theis */

// View auf TmRec

#include  "stdafx.h"
#include  "RkListStore.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "ResultSet.h"
#include  "SQLException.h"
#include  "InfoSystem.h"

#include  "CpListStore.h"
#include  "NaListStore.h"
#include  "TmListStore.h"


#include  <stdio.h>
#include  <stdlib.h>


// -----------------------------------------------------------------------
// Neue Tabelle in DB erzeugen
bool  RkListStore::CreateView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str = "CREATE VIEW RkList AS "
                     "SELECT rk.tmID, tm.cpID, rk.naID, "
                     "       rkNatlRank, rkIntlRank, rkDirectEntry "
                     "FROM RkRec rk INNER JOIN TmRec tm ON rk.tmID = tm.tmID";

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


bool  RkListStore::RemoveView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  try
  {
    tmp->ExecuteUpdate("DROP VIEW RkList");
  }
  catch(SQLException &)
  {
  }

  delete tmp;
  
  return true;
}


// -----------------------------------------------------------------------
// Konstruktor, Destruktor
RkListStore::RkListStore(Connection *connPtr)
           : StoreObj(connPtr)
{
}


RkListStore::~RkListStore()
{
}


void  RkListStore::Init()
{
  RkListRec::Init();
}


// -----------------------------------------------------------------------
bool  RkListStore::SelectByTm(const TmRec &tm)
{
  wxString str = SelectString();
  str += " WHERE tmID = " + ltostr(tm.tmID);
  
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


bool  RkListStore::SelectByCp(const CpRec &cp)
{
  wxString str = SelectString();
  str += " WHERE cpID = " + ltostr(cp.cpID);
  
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


// -----------------------------------------------------------------------
short RkListStore::CountDirectEntries(const CpRec &cp, const NaRec &na)
{
  wxString str = "SELECT COUNT(*) FROM RkList "
                    "WHERE cpID = " + ltostr(cp.cpID) +
                    " AND rkDirectEntry <> 0 ";
  if (na.naID != 0)
    str += " AND naId = " + ltostr(na.naID);                    
                    
  Connection * connPtr = GetConnectionPtr();
  Statement  * stmtPtr = 0;
  ResultSet  * resPtr  = 0;

  short res;

  try
  {
    stmtPtr = connPtr->CreateStatement();
    resPtr = stmtPtr->ExecuteQuery(str);
    resPtr->BindCol(1, &res);
    if ( !resPtr->Next() || resPtr->WasNull(1) )
      res = 0;
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    res = 0;
  }  
  
  delete resPtr;
  delete stmtPtr;

  return res;
}


short RkListStore::CountQualifiers(const CpRec &cp, const NaRec &na)
{
  wxString str = "SELECT COUNT(*) FROM RkList "
                    "WHERE cpID = " + ltostr(cp.cpID) +
                    " AND rkDirectEntry = 0 ";
  if (na.naID != 0)
    str += " AND naID = " + ltostr(na.naID);                    
                    
  Connection * connPtr = GetConnectionPtr();
  Statement  * stmtPtr = 0;
  ResultSet  * resPtr  = 0;

  short res;

  try
  {
    stmtPtr = connPtr->CreateStatement();
    resPtr = stmtPtr->ExecuteQuery(str);
    resPtr->BindCol(1, &res);
    if ( !resPtr->Next() || resPtr->WasNull(1) )
      res = 0;
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    res = 0;
  }  
  
  delete resPtr;
  delete stmtPtr;

  return res;
}


// -----------------------------------------------------------------------
wxString  RkListStore::SelectString() const
{
  wxString  str = 
    "SELECT tmID, naID, cpID, rkNatlRank, rkIntlRank, rkDirectEntry "
    "FROM RkList ";

  return str;
}


bool  RkListStore::BindRec()
{
  BindCol(1, &tmID);
  BindCol(2, &naID);
  BindCol(3, &cpID);
  BindCol(4, &rkNatlRank);
  BindCol(5, &rkIntlRank);
  BindCol(6, &rkDirectEntry);

  return true;
}
