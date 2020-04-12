/* Copyright (C) 2020 Christoph Theis */

// View auf TmRec

#include  "stdafx.h"
#include  "LtListStore.h"
#include  "LtStore.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "SQLException.h"
#include  "InfoSystem.h"


#include  <stdio.h>
#include  <stdlib.h>


// -----------------------------------------------------------------------
// Neue Tabelle in DB erzeugen
bool  LtListStore::CreateView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str = "CREATE VIEW LtList AS "
                     "SELECT LtRec.ltID, LtRec.plID, LtRec.cpID, LtRec.ltTimestamp,  "
                     "       ISNULL((SELECT rpRankPts FROM RpRec rp WHERE rp.plID = LtRec.plID AND rpYear = CpRec.cpYear), PlRec.plRankPts) AS ltRankPts "
                     "FROM LtRec INNER JOIN CpRec ON LtRec.cpID = CpRec.cpID INNER JOIN PlRec ON LtRec.plID = PlRec.plID ";

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


bool  LtListStore::RemoveView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  try
  {
    tmp->ExecuteUpdate("DROP VIEW LtList");
  }
  catch(SQLException &)
  {
  }

  delete tmp;
  
  return true;
}


// -----------------------------------------------------------------------
// Konstruktor, Destruktor
LtListStore::LtListStore(Connection *connPtr)
           : StoreObj(connPtr)
{
}


LtListStore::~LtListStore()
{
}


void  LtListStore::Init()
{
  LtRec::Init();
}


// -----------------------------------------------------------------------
bool LtListStore::SelectById(long id)
{
  wxString str = SelectString() + " WHERE ltID = " + ltostr(id);
  
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

// -----------------------------------------------------------------------
bool LtListStore::SelectByPl(long plID, const timestamp *when)
{
  wxString str = SelectString(when) + " WHERE lt.plID = " + ltostr(plID);
  
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

// -----------------------------------------------------------------------
std::vector<std::pair<timestamp, long>> LtListStore::GetTimestamps(long id)
{
  std::vector<std::pair<timestamp, long>> ret;

  if (id == 0)
    id = plID;

  if (id == 0)
    return ret;
    
  wxString sql = "SELECT DISTINCT ltStartTime, ltEndTime, ltID FROM LtRec FOR SYSTEM_TIME ALL WHERE plID = " + ltostr(id) + " ORDER BY ltEndTime DESC";

  Statement *stmtPtr = 0;
  ResultSet *resPtr = 0;

  timestamp lastTs = {};

  try
  {
    stmtPtr = GetConnectionPtr()->CreateStatement();
    resPtr = stmtPtr->ExecuteQuery(sql);

    while (resPtr->Next())
    {
      long ltID;
      timestamp start, end;      

      resPtr->GetData(1, start);
      if (resPtr->WasNull(1))
        continue;

      resPtr->GetData(2, end);
      if (resPtr->WasNull(2))
        continue;

      resPtr->GetData(3, ltID);
      if (resPtr->WasNull(3))
        continue;
       
      // Something else has changed, but what?
      if (end == lastTs)
      {
        lastTs = start;
        continue;
      }

      lastTs = start;

      ret.push_back({end, ltID});
      ret.push_back({start, ltID});
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
wxString  LtListStore::SelectString(const timestamp *when) const
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

  str += " ltID, lt.plID, lt.cpID, ltRankPts ";
  str += "  FROM LtList " + ts + " lt INNER JOIN PlList " + ts + " pl ON lt.plID = pl.plID INNER JOIN CpList " + ts + " cp ON lt.cpID = cp.cpID";

  return str;
}


bool  LtListStore::BindRec()
{
  BindCol(1, &ltID);
  BindCol(2, &plID);
  BindCol(3, &cpID);
  BindCol(4, &ltRankPts);

  return true;
}
