/* Copyright (C) 2020 Christoph Theis */

// DB-Liste der Spieler

#include  "stdafx.h"
#include  "PlListStore.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "SQLException.h"
#include  "InfoSystem.h"

#include  "PlStore.h"

#include  "Rec.h"

#include  <stdio.h>
#include  <stdlib.h>


// -----------------------------------------------------------------------
// Neue Tabelle in DB erzeugen
bool  PlListStore::CreateView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str = 
      "CREATE VIEW PlList AS "
      " SELECT plID, plExtID, plNr, PlRec.psID, psLast, psFirst, psSex, "
      "        psBirthday, PlRec.naID, naName, naDesc, naRegion, plRankPts, "
      "        psTimestamp, psPhone, CASE WHEN psNote IS NULL THEN 0 ELSE 1 END AS psHasNote, "
      "        plDeleted "
      " FROM (PsRec INNER JOIN PlRec ON PsRec.psID = PlRec.psID) "
      "       LEFT OUTER JOIN NaRec ON PlRec.naID = NaRec.naID";

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


bool  PlListStore::RemoveView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  try
  {
    tmp->ExecuteUpdate("DROP VIEW PlList");
  }
  catch(SQLException &)
  {
  }

  delete tmp;
  
  return true;
}


// -----------------------------------------------------------------------
// Konstruktor, Destruktor
PlListStore::PlListStore(Connection *connPtr)
           : StoreObj(connPtr)
{
}


PlListStore::~PlListStore()
{
}


void  PlListStore::Init()
{
  PlListRec::Init();
}


// -----------------------------------------------------------------------
// Select-Statements
bool  PlListStore::SelectAll()
{
  wxString  str = SelectString();
  str += " ORDER BY plNr";

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


bool  PlListStore::SelectById(long id, const timestamp *when)
{
  wxString  stmt = SelectString(when);
  stmt += " WHERE plID = ";
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


bool  PlListStore::SelectByNr(long nr)
{
  wxString  stmt = SelectString();
  stmt += "WHERE plNr = ";
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


bool  PlListStore::SelectForDouble(const PlRec &pl, const CpRec &cp)
{
  wxString  str = SelectString();
  str += " WHERE  psSex ";
  if (cp.cpType == CP_DOUBLE)
    str += " = ";
  else
    str += " <> ";
  str += ltostr(pl.psSex);
  str += " AND plID = ANY (SELECT LtRec.plID FROM LtRec "
         "                 WHERE LtRec.cpID = ";
  str += ltostr(cp.cpID);
  str += " AND NOT EXISTS (SELECT NtRec.* FROM NtRec "
         "                 WHERE NtRec.ltID = LtRec.ltID) ";
  str += " ) ORDER BY psLast, psFirst, naName";

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


bool PlListStore::SelectForCp(const CpRec &cp)
{
  wxString str = SelectString() + 
      "WHERE plID NOT IN "
      "    (SELECT plID FROM LtRec WHERE cpID = " + ltostr(cp.cpID) + ") ";
      
  if (cp.cpYear != 0)
  {
    if (CTT32App::instance()->GetType() == TT_SCI)
      str += " AND (psBirthday = 0 OR psBirthday <= " + ltostr(cp.cpYear) + ") ";
    else // if (CTT32App::instance()->GetType() == TT_YOUTH)
      str += " AND (psBirthday = 0 OR psBirthday >= " + ltostr(cp.cpYear) + ") ";
  }
  
  if (cp.cpType != CP_MIXED)
    str += " AND psSex = " + ltostr(cp.cpSex);
    
  str += " ORDER BY plNr";
    
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


long PlListStore::Count()
{
  Statement *stmtPtr = NULL;
  ResultSet *resPtr = NULL;

  wxString str = "SELECT COUNT(*) FROM PlList";

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


wxString PlListStore::GetNote()
{
  return PlStore(*this, GetConnectionPtr()).GetNote();
}


// -----------------------------------------------------------------------
std::vector<timestamp> PlListStore::GetTimestamps(long id)
{
  std::vector<timestamp> ret;

  if (id == 0)
    id = psID;

  if (id == 0)
    return ret;
    
  wxString sql = "SELECT DISTINCT psEndTime FROM PsRec FOR SYSTEM_TIME ALL WHERE psID = " + ltostr(id);

  Statement *stmtPtr = 0;
  ResultSet *resPtr = 0;

  try
  {
    stmtPtr = GetConnectionPtr()->CreateStatement();
    resPtr = stmtPtr->ExecuteQuery(sql);

    while (resPtr->Next())
    {
      timestamp ts;      
      resPtr->GetData(1, ts);
      if (!resPtr->WasNull(1))
        ret.push_back(ts);
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
wxString  PlListStore::SelectString(const timestamp *when) const
{
  wxString  str;
  wxString  ts;

  if (when == NULL || when->year == 9999)
  {
    str = "SELECT ";
  }
  else if (when->year == 0)
  {
    str = "SELECT DISTINCT ";
    ts = " FOR SYSTEM_TIME ALL ";
  }
  else
  {
    str = "SELECT DISTINCT ";
    ts = " FOR SYSTEM_TIME AS OF '" + tstostr(*when) + "' ";
  }

  str += 
      "       plID, plExtID, plNr, psID, psLast, psFirst, psSex, psBirthday, "
      "       naID, naName, naDesc, naRegion, plRankPts, psPhone, psHasNote, "
      "      plDeleted, psTimestamp "
      " FROM PlList " + ts;

  return str;  
}


bool  PlListStore::BindRec()
{
  BindCol(1, &plID);
  BindCol(2, plExtID, sizeof(plExtID));
  BindCol(3, &plNr);
  BindCol(4, &psID);
  BindCol(5, psName.psLast, sizeof(psName.psLast));
  BindCol(6, psName.psFirst, sizeof(psName.psFirst));
  BindCol(7, &psSex);
  BindCol(8, &psBirthday);
  BindCol(9, &naID);
  BindCol(10, naName, sizeof(naName));
  BindCol(11, naDesc, sizeof(naDesc));
  BindCol(12, naRegion, sizeof(naRegion));
  BindCol(13, &plRankPts);
  BindCol(14, psPhone, sizeof(psPhone));
  BindCol(15, &psHasNote);
  BindCol(16, &plDeleted);
  BindCol(17, &psTimestamp);
  
  return true;
}