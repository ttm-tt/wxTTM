/* Copyright (C) 2020 Christoph Theis */

// View auf TmRec

#include  "stdafx.h"
#include  "NtListStore.h"

#include  "LtStore.h"
#include  "TmStore.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "SQLException.h"
#include  "InfoSystem.h"

#include  "Rec.h"


#include  <stdio.h>
#include  <stdlib.h>


// -----------------------------------------------------------------------
// Neue Tabelle in DB erzeugen
bool  NtListStore::CreateView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str = "CREATE VIEW NtList AS "
                     "SELECT ltID, tmID, ntNr "
                     "FROM NtRec ";

  try
  {
    tmp->ExecuteUpdate(str);

    connPtr->Commit();
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  connPtr->Commit();

  delete tmp;
  
  return true;
}


bool  NtListStore::RemoveView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  try
  {
    tmp->ExecuteUpdate("DROP VIEW IF EXISTS NtList");
  }
  catch(SQLException &)
  {
  }

  delete tmp;
  
  return true;
}


// -----------------------------------------------------------------------
// Konstruktor, Destruktor
NtListStore::NtListStore(Connection *connPtr)
           : StoreObj(connPtr)
{
}


NtListStore::~NtListStore()
{
}


void  NtListStore::Init()
{
  NtListRec::Init();
}


// -----------------------------------------------------------------------
bool NtListStore::SelectByLt(const LtRec &lt, const timestamp *when)
{
  wxString str = SelectString(when) + " WHERE ltID = " + ltostr(lt.ltID);
  
  try
  {
    ExecuteQuery(str);
    BindRec();
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }
  
  return true;
}



short NtListStore::Count(const TmRec &tm)
{
  Connection * connPtr = GetConnectionPtr();
  Statement  * stmtPtr = 0;
  ResultSet  * resPtr  = 0;

  wxString  str = 
      "SELECT COUNT(*) FROM NtList WHERE tmID = " + ltostr(tm.tmID);

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
std::vector<std::pair<timestamp, long>> NtListStore::GetTimestamps(long id)
{
  std::vector<std::pair<timestamp, long>> ret;

  if (id == 0)
    id = ltID;

  if (id == 0)
    return ret;
    
  wxString sql = "SELECT DISTINCT ntStartTime, ntEndTime, tmID FROM NtRec FOR SYSTEM_TIME ALL WHERE ltID = " + ltostr(id) + " ORDER BY ntEndTime DESC";

  Statement *stmtPtr = 0;
  ResultSet *resPtr = 0;

  wxString cpTypeQuery = 
      "SELECT DISTINCT cpType FROM LtRec FOR SYSTEM_TIME ALL lt INNER JOIN CpRec FOR SYSTEM_TIME ALL cp ON lt.cpID = cp.cpID WHERE lt.ltID = " + ltostr(id);

  try
  {
    stmtPtr = GetConnectionPtr()->CreateStatement();

    // Get cpType
    short cpType = 0;

    resPtr = stmtPtr->ExecuteQuery(cpTypeQuery);
    if (resPtr->Next())
    {
      if (!resPtr->WasNull(1))
        resPtr->GetData(1, cpType);
    }
    delete resPtr;

    resPtr = stmtPtr->ExecuteQuery(sql);

    timestamp lastTs = {};

    while (resPtr->Next())
    {
      long tmID;
      timestamp start, end;      

      resPtr->GetData(1, start);
      if (resPtr->WasNull(1))
        continue;

      resPtr->GetData(2, end);
      if (resPtr->WasNull(2))
        continue;

      resPtr->GetData(3, tmID);
      if (resPtr->WasNull(3))
        continue;
       
      // Something else has changed, maybe sequence in doubles or teams
      if (cpType == CP_TEAM && end == lastTs)
      {
        lastTs = start;
        continue;
      }

      lastTs = start;

      ret.push_back({end, tmID});
      ret.push_back({start, tmID});
    }
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(sql, e);
  }

  delete resPtr;
  delete stmtPtr;

  return ret;
}


// -----------------------------------------------------------------------
wxString  NtListStore::SelectString(const timestamp *when) const
{
  wxString  str;
  wxString  ts;
  if (when == NULL || when->year == 9999)
  {
    str = "SELECT ";
  }
  else
  {
    str = "SELECT DISTINCT ";
    if (when->year == 0)
      ts = " FOR SYSTEM_TIME ALL ";
    else
      ts = " FOR SYSTEM_TIME AS OF '" + tstostr(*when) + "' ";
  }

  str += " ltID, tmID, ntNr FROM NtList " + ts + " ";

  return str;
}


bool  NtListStore::BindRec()
{
  BindCol(1, &ltID);
  BindCol(2, &tmID);
  BindCol(3, &ntNr);

  return true;
}
