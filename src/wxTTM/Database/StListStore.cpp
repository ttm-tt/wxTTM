/* Copyright (C) 2020 Christoph Theis */

// DB-Tabelle der Wettbewerbe

#include  "stdafx.h"
#include  "StListStore.h"
#include  "StStore.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "SQLException.h"
#include  "InfoSystem.h"

#include  "GrStore.h"
#include  "StEntryStore.h"

#include  <stdio.h>
#include  <stdlib.h>


// -----------------------------------------------------------------------
StListRec::StListRec(const StEntry &rec)
{
  StRec::operator=(rec.st);
}


// -----------------------------------------------------------------------
// Neue Tabelle in DB erzeugen
bool  StListStore::CreateView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str = "CREATE VIEW StList AS "
                     "SELECT stID, grID, st.tmID, stNr, stPos, stSeeded, "
                     "       stGaveup, stDisqu, stNocons, stTimestamp, naID "
                     "FROM StRec st LEFT OUTER JOIN RkRec rk ON st.tmID = rk.tmID ";
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


bool  StListStore::RemoveView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  try
  {
    tmp->ExecuteUpdate("DROP VIEW StList");
  }
  catch(SQLException &)
  {
  }

  delete tmp;
  
  return true;
}


// -----------------------------------------------------------------------
// Konstruktor, Destruktor
StListStore::StListStore(Connection *connPtr)
           : StoreObj(connPtr)
{
}


StListStore::~StListStore()
{
}


void  StListStore::Init()
{
  StListRec::Init();
}


// -----------------------------------------------------------------------
// Select-Statements
bool  StListStore::SelectAll(const GrRec &gr)
{
  wxString  str = SelectString();
  str += " WHERE grID = " + ltostr(gr.grID) + " ORDER BY stNr";

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


bool  StListStore::SelectById(long id)
{
  wxString str = SelectString();
  str += " WHERE stID = " + ltostr(id);

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


bool  StListStore::SelectById(const std::set<long> &ids)
{
  wxString str = SelectString();
  str += " WHERE stID IN (" + ltostr(ids) + ")";

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


// ----------------------------------------------------------------------
bool  StListStore::SelectAll(const CpRec &cp, const char *stage)
{
  wxString str = SelectString();
  str += " WHERE grID IN ";
  str += "(SELECT grID FROM GrRec WHERE cpID = ";
  str += ltostr(cp.cpID);
  str += " AND grStage = '";
  str += stage;
  str += "') ORDER BY grID, stPos";

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


// ----------------------------------------------------------------------
bool  StListStore::SelectByCpNa(const CpRec &cp, const NaRec &na)
{
  wxString str = SelectString();
  str += " WHERE grID IN ";
  str += "(SELECT grID FROM GrRec WHERE cpID = ";
  str += ltostr(cp.cpID);
  str += ") AND tmID IN ";
  str += "(SELECT tmID FROM RkRec WHERE naID = ";
  str += ltostr(na.naID);
  str += ") ORDER BY stNr";

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
bool StListStore::SelectByCpTm(const CpRec &cp, const TmRec &tm, const timestamp * when)
{
  wxString  sel;
  wxString  ts;
  if (when == NULL || when->year == 9999)
  {
    sel = "SELECT ";
  }
  else
  {
    sel = "SELECT DISTINCT ";
    if (when->year == 0)
      ts = " FOR SYSTEM_TIME ALL ";
    else
      ts = " FOR SYSTEM_TIME AS OF '" + tstostr(*when) + "' ";
  }

  wxString str = SelectString(when);
  str += " WHERE grID IN (" + sel + " grID FROM GrList " + ts + " WHERE cpID = " + ltostr(cp.cpID) + ") AND tmID = " + ltostr(tm.tmID);
  
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
bool StListStore::SelectByGrTm(const GrRec &gr, const TmRec &tm)
{
  wxString str = SelectString();
  str += " WHERE grID = " + ltostr(gr.grID) + " AND tmID = " + ltostr(tm.tmID);
  
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
long StListStore::Count()
{
  Statement *stmtPtr = NULL;
  ResultSet *resPtr = NULL;

  wxString str = "SELECT COUNT(*) FROM StList";

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

short StListStore::CountNoCons(const CpRec &cp, const char *stage)
{
  wxString str = "SELECT COUNT(*) FROM StList st " 
                    "INNER JOIN GrList gr ON st.grID = gr.grID "
                    "INNER JOIN CpList cp ON gr.cpID = cp.cpID "
                    "WHERE cp.cpID = " + ltostr(cp.cpID) +
                    " AND stNoCons <> 0 ";

  if (stage != NULL)
  {
    str += " AND grStage = '";
    str += stage;
    str +=+ "'";
  }
                    
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


std::vector<std::pair<timestamp, long>> StListStore::GetTimestamps(long id)
{
  std::vector<std::pair<timestamp, long>> ret;

  if (id == 0)
    id = tmID;

  if (id == 0)
    return ret;
    
  wxString sql = "SELECT DISTINCT stStartTime, stEndTime, stID FROM StRec FOR SYSTEM_TIME ALL WHERE tmID = " + ltostr(id) + " ORDER BY stEndTime DESC ";

  Statement *stmtPtr = 0;
  ResultSet *resPtr = 0;

  try
  {
    stmtPtr = GetConnectionPtr()->CreateStatement();
    resPtr = stmtPtr->ExecuteQuery(sql);

    timestamp lastTs = {};

    while (resPtr->Next())
    {
      long stID;
      timestamp start, end;      

      resPtr->GetData(1, start);
      if (resPtr->WasNull(1))
        continue;

      resPtr->GetData(2, end);
      if (resPtr->WasNull(2))
        continue;

      resPtr->GetData(3, stID);
      if (resPtr->WasNull(3))
        continue;

      // Something else has changed, e.g. final standing
      if (end == lastTs)
      {
        lastTs = start;
        continue;
      }

      lastTs = start;

      ret.push_back({end, stID});
      ret.push_back({start, stID});
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
wxString  StListStore::SelectString(const timestamp * when) const
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

  str += " stID, grID, tmID, stNr, stPos, stSeeded, stGaveup, "
    "       stDisqu, stNocons, naID "
    "FROM StList " + ts + " ";

  return str;
}


bool  StListStore::BindRec()
{
  BindCol(1, &stID);
  BindCol(2, &grID);
  BindCol(3, &tmID);
  BindCol(4, &stNr);
  BindCol(5, &stPos);
  BindCol(6, &stSeeded);
  BindCol(7, &stGaveup);
  BindCol(8, &stDisqu);
  BindCol(9, &stNocons);
  BindCol(10, &naID);

  return true;
}
