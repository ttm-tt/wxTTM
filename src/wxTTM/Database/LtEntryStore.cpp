/* Copyright (C) 2020 Christoph Theis */

// Meldungen
#include  "stdafx.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "ResultSet.h"
#include  "SQLException.h"
#include  "InfoSystem.h"

#include  "LtEntryStore.h"
#include  "TmEntryStore.h"

#include  "CpListStore.h"

#include  "LtStore.h"
#include  "TmStore.h"

#include  "Rec.h"


// -----------------------------------------------------------------------
bool  LtEntryStore::CreateView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str;

  try
  {
    // Alle Spielermeldungen
    str = "CREATE VIEW LtEntryList AS                                             "
          "  SELECT plNr, psLast, psFirst, psSex, psBirthday, plExtID, plRankPts, "
          "         ltRankPts, naName, naDesc, naRegion, ltID, lt.cpID,           "
          "         pl.plID, pl.naID, ltTimestamp                                 "
          "    FROM LtList lt INNER JOIN PlList pl ON lt.plID = pl.plID           ";
    tmp->ExecuteUpdate(str);
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  delete tmp;
  
  return true;
}


bool  LtEntryStore::RemoveView()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  try
  {
    tmp->ExecuteUpdate("DROP VIEW LtEntryList");
  }
  catch(SQLException &)
  {
  }

  delete tmp;
  
  return true;
}


// -----------------------------------------------------------------------
LtEntryStore::LtEntryStore(Connection *ptr)
            : StoreObj(ptr)
{
}


void  LtEntryStore::Init()
{
  LtEntry::Init();
}


bool  LtEntryStore::Next()
{
  if (!StoreObj::Next())
    return false;

  LtRec::plID = PlRec::plID;
  return true;
}

// -----------------------------------------------------------------------
// Auswahl per DB

bool  LtEntryStore::SelectById(long id)
{
  wxString str = SelectString();
  str += " WHERE ltID = " + ltostr(id);
  
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

bool  LtEntryStore::SelectByCpPl(long cpID, long plID, const timestamp *when)
{
  wxString str = SelectString(when);
  str += " WHERE lt.cpID = " + ltostr(cpID) + " AND plID = " + ltostr(plID);
  
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


bool  LtEntryStore::SelectBuddy(const LtRec &lt, const timestamp *when)
{
  wxString select;
  wxString ts;
  if (when == NULL || when->year == 9999)
  {
    select = "SELECT ";
  }
  else if (when->year == 0)
  {
    select = "SELECT DISTINCT ";
    ts = " FOR SYSTEM_TIME ALL ";
  }
  else
  {
    select = "SELECT DISTINCT ";
    ts = " FOR SYSTEM_TIME AS OF '" + tstostr(*when) + "' ";
  }

  wxString str = SelectString(when);
  str += " WHERE ltID IN ( "
         "    " + select + " ntbd.ltID "
         "      FROM NtRec " + ts + " ntpl INNER JOIN NtRec " + ts + " ntbd ON ntpl.tmID = ntbd.tmID "
         "     WHERE ntpl.ltID = " + ltostr(lt.ltID) + " AND ntpl.ntNr <> ntbd.ntNr "
         " ) ";

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


bool  LtEntryStore::SelectForDouble(const PlRec &pl, const CpRec &cp)
{
  wxString  str = SelectString();
  str += " WHERE  psSex ";
  if (cp.cpType == CP_DOUBLE)
    str += " = ";
  else
    str += " <> ";
  str += ltostr(pl.psSex);
  str += " AND ltID IN (SELECT ltID FROM LtRec "
         "                 WHERE LtRec.cpID = ";
  str += ltostr(cp.cpID);
  str += ")";
  str += " AND ltID NOT IN (SELECT NtRec.ltID FROM NtRec) ";
  
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




bool  LtEntryStore::SelectOpenEntriesByCp(const CpRec &cp)
{
  wxString  str = SelectString();
  str += " WHERE lt.cpID = ";
  str += ltostr(cp.cpID);
  str += " AND ltID NOT IN (SELECT ltID FROM NtRec) ";
  str += " ORDER BY plNr";

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


bool  LtEntryStore::SelectOpenEntriesByCpNa(const CpRec &cp, const NaRec &na)
{
  wxString  str = SelectString();
  str += " WHERE lt.cpID = ";
  str += ltostr(cp.cpID);
  
  if (na.naID != 0)
  {
    str += " AND naID = ";
    str += ltostr(na.naID);
  }
  
  str += " AND ltID NOT IN (SELECT ltID FROM NtRec) ";
  str += " ORDER BY plNr";

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


bool  LtEntryStore::SelectPlayerByTm(const TmRec &tm)
{
  wxString  str = SelectString();
  str += " WHERE ltID IN (SELECT ltID FROM NtRec WHERE tmID = ";
  str += ltostr(tm.tmID);
  str += ")";

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
short  LtEntryStore::CountFemaleEntries(const CpRec &cp)
{
  Connection * connPtr = GetConnectionPtr();
  Statement  * stmtPtr = 0;
  ResultSet  * resPtr  = 0;

  wxString  str = 
      "SELECT COUNT(plNr) FROM LtEntryList lt "
      " WHERE   psSex = 2 AND lt.cpID = ";
  str += ltostr(cp.cpID);

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


short  LtEntryStore::CountMaleEntries(const CpRec &cp)
{
  Connection * connPtr = GetConnectionPtr();
  Statement  * stmtPtr = 0;
  ResultSet  * resPtr  = 0;

  wxString  str = 
      "SELECT COUNT(plNr) FROM LtEntryList lt "
      " WHERE psSex = 1 AND lt.cpID = ";
  str += ltostr(cp.cpID);

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
wxString  LtEntryStore::SelectString(const timestamp *when) const
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
     " plID, plNr, psLast, psFirst, psSex, psBirthday, plExtID, plRankPts, ltRankPts, "
     "       naName, naDesc, naRegion, ltID, lt.cpID, naID, ltRankPts "
     "  FROM LtEntryList " + ts + " lt INNER JOIN CpList " + ts + " cp ON lt.cpID = cp.cpID "
   ;

  return str;
}


bool  LtEntryStore::BindRec()
{
  int  col = 1;

  BindCol(col++, &(PlRec::plID));
  BindCol(col++, &plNr);
  BindCol(col++, psName.psLast, sizeof(psName.psLast));
  BindCol(col++, psName.psFirst, sizeof(psName.psFirst));
  BindCol(col++, &psSex);
  BindCol(col++, &psBirthday);
  BindCol(col++, plExtID, sizeof(plExtID));
  BindCol(col++, &plRankPts);
  BindCol(col++, &ltRankPts);
  BindCol(col++, naName, sizeof(naName));
  BindCol(col++, naDesc, sizeof(naDesc));
  BindCol(col++, naRegion, sizeof(naRegion));
  BindCol(col++, &ltID);
  BindCol(col++, &cpID);
  BindCol(col++, &naID);

  return true;
}