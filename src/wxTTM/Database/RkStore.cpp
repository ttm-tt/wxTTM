/* Copyright (C) 2020 Christoph Theis */

// Ranking eines "Teams"

#include  "stdafx.h"

#include  "RkStore.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "ResultSet.h"
#include  "SQLException.h"
#include  "InfoSystem.h"
#include  "Request.h"

#include  "IdStore.h"
#include  "TmStore.h"
#include  "NaStore.h"
#include  "CpStore.h"

#include  "NaListStore.h"
#include  "CpListStore.h"


bool  RkStore::CreateTable()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  wxASSERT(connPtr);

  Statement *tmp = connPtr->CreateStatement();

  wxString  INTEGER  = connPtr->GetDataType(SQL_INTEGER);
  wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);
  wxString  WVARCHAR = connPtr->GetDataType(SQL_WVARCHAR);

  wxString  sql = 
    "CREATE TABLE RkRec ( "
    "tmID           "+INTEGER+"      NOT NULL, "
    "naID           "+INTEGER+"      NOT NULL, "
    "rkNatlRank     "+SMALLINT+"     NOT NULL, "
    "rkIntlRank     "+SMALLINT+",    "
    "rkDirectEntry  "+SMALLINT+"     NOT NULL, "
    "CONSTRAINT rkKey PRIMARY KEY (tmID),      "    
    "rkCreatedBy " + WVARCHAR + "(64) NOT NULL DEFAULT (SUSER_SNAME()), "
    "rkModifiedBy AS (SUSER_SNAME()), "
    "rkStartTime datetime2 GENERATED ALWAYS AS ROW START NOT NULL DEFAULT SYSUTCDATETIME(), "
    "rkEndTime datetime2 GENERATED ALWAYS AS ROW END NOT NULL DEFAULT CAST('9999-12-31 23:59:59.9999999' AS datetime2), "
    "PERIOD FOR SYSTEM_TIME (rkStartTime, rkEndTime) "
    ") WITH (SYSTEM_VERSIONING = ON (HISTORY_TABLE = dbo.RkHist))";
	  ")";

  try
  {
    tmp->ExecuteUpdate(sql);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(sql, e);
    delete tmp;

    return false;
  }

  try
  {
    // tmp->ExecuteUpdate(sql = "CREATE UNIQUE INDEX rkNatlRankKey ON RkRec (naID, rkNatlRank)");
    tmp->ExecuteUpdate(sql = "CREATE UNIQUE INDEX rkTmKey ON RkRec (tmID)");
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(sql, e);
  };

  tmp->Close();
  delete tmp;
  
  return true;
}


bool  RkStore::UpdateTable(long version)
{
  if (version == 0)
    return CreateTable();


  if (version < 149)
  {
    // History
    Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
    Statement *stmtPtr = connPtr->CreateStatement();

    wxString  WVARCHAR = connPtr->GetDataType(SQL_WVARCHAR);
    wxString str;

    try
    {
      str = "ALTER TABLE RkRec ADD "
        "rkCreatedBy " + WVARCHAR + "(64) NOT NULL DEFAULT (SUSER_SNAME()), "
        "rkModifiedBy AS (SUSER_SNAME()), "
        "rkStartTime datetime2 GENERATED ALWAYS AS ROW START NOT NULL DEFAULT SYSUTCDATETIME(), "
        "rkEndTime datetime2 GENERATED ALWAYS AS ROW END NOT NULL DEFAULT CAST('9999-12-31 23:59:59.9999999' AS datetime2), "
        "PERIOD FOR SYSTEM_TIME (rkStartTime, rkEndTime)"
        ;
      stmtPtr->ExecuteUpdate(str);

      str = "ALTER TABLE RkRec SET (SYSTEM_VERSIONING = ON(HISTORY_TABLE = dbo.RkHist))";
      stmtPtr->ExecuteUpdate(str);
    }
    catch (SQLException &e)
    {
      infoSystem.Exception(str, e);
      delete stmtPtr;
      return false;
    }

    delete stmtPtr;
  }
  return true;
}


bool  RkStore::CreateConstraints()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  INTEGER  = connPtr->GetDataType(SQL_INTEGER);
  wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);
  
  wxString  str;

  // Siehe unten
#if 0    
  try
  {
    tmp->ExecuteUpdate("DROP TRIGGER rkDeleteTrigger");
  }
  catch (SQLException &)
  {
  }
#endif  
  
  try
  {
    tmp->ExecuteUpdate("ALTER TABLE RkRec DROP CONSTRAINT rkTmRef");
    tmp->ExecuteUpdate("ALTER TABLE RkRec DROP CONSTRAINT rkNaRef");
  }
  catch (SQLException &)
  {
  }

  try
  {
    // Funktioniert nicht, da TmRec schon geloescht wurde,
    // wenn der Trigger feuert
#if 0  
    tmp->ExecuteUpdate(str = 
      "CREATE TRIGGER rkDeleteTrigger ON RkRec INSTEAD OF DELETE NOT FOR REPLICATION \n"
      "AS \n"
      "BEGIN \n"
      // Die deleted-Tabelle kann mehrere Zeilen enthalten
      "  DECLARE @naID "+INTEGER+" \n"
      "  DECLARE @rkNatlRank "+SMALLINT+" \n"
      "  DECLARE @cpID "+INTEGER+" \n"
      "  DECLARE rkCursor CURSOR LOCAL FOR \n"
      "    SELECT naID, rkNatlRank, cpID \n"
      "      FROM deleted INNER JOIN TmRec tm ON deleted.tmID = tm.tmID \n"
      "  OPEN rkCursor \n"
      "  FETCH NEXT FROM rkCursor INTO @naID, @rkNatlRank, @cpID \n"
      "  WHILE (@@FETCH_STATUS = 0) \n"
      "  BEGIN \n"
      "    UPDATE RkRec SET rkNatlRank = rkNatlRank - 1  \n"
      "     WHERE naID = @naID AND rkNatlRank > @rkNatlRank AND \n"
      "           tmID IN (SELECT tmID FROM TmRec WHERE cpID = @cpID) \n"
      " \n"
      "    FETCH NEXT FROM rkCursor INTO @naID, @rkNatlRank, @cpID \n"
      "  END \n"
      "  CLOSE rkCursor \n"
      "  DEALLOCATE rkCursor \n"
      "  DELETE FROM RkRec WHERE tmID IN (SELECT tmID FROM deleted) \n"
      "END \n"
    );
#endif
    
    tmp->ExecuteUpdate(str = 
      "ALTER TABLE RkRec ADD CONSTRAINT rkTmRef "
      "FOREIGN KEY (tmID) REFERENCES TmRec (tmID) ON DELETE CASCADE");

    tmp->ExecuteUpdate(str = 
      "ALTER TABLE RkRec ADD CONSTRAINT rkNaRef "
      "FOREIGN KEY (naID) REFERENCES NaRec (naID) ON DELETE CASCADE");
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
  };

  delete tmp;
  return true;
}


bool  RkStore::UpdateConstraints(long version)
{
  if (version <= 75)
    return CreateConstraints();

  return true;
}


// -----------------------------------------------------------------------
RkStore::RkStore(Connection *ptr)
       : StoreObj(ptr)
{
}


RkStore::RkStore(const RkRec &rec, Connection *ptr)
       : StoreObj(ptr), RkRec(rec)
{
}


RkStore::~RkStore()
{
}


void  RkStore::Init()
{
  RkRec::Init();
}


// -----------------------------------------------------------------------
bool  RkStore::Insert(const TmRec &tm, const NaRec &na, short natlRank, short intlRank)
{
  // Team und Assoc sind Pflicht, ausserdem brauch ich den WB
  if (!tm.tmID || !na.naID || !tm.cpID)
    return false;

  tmID = tm.tmID;
  naID = na.naID;
  
  if (natlRank == 0)
    natlRank = GetNextNatlRank(na, tm, intlRank);

  if (natlRank && ExistsNatlRank(na, tm, natlRank))
  {
    RkStore rk(GetConnectionPtr());
    rk.SelectByRanking(tm, na, natlRank);
    rk.Next();
    rk.Close();

    wxString str = "UPDATE RkRec SET rkNatlRank = rkNatlRank + 1 "
      "WHERE naID = " + ltostr(na.naID) + 
      "  AND rkNatlRank >= " + ltostr(natlRank) +
      "  AND tmID IN (SELECT tmID FROM TmRec WHERE cpID = " + ltostr(tm.cpID) + ")";
      
    try
    {
      ExecuteUpdate(str);
    }
    catch (SQLException &e)
    {
      infoSystem.Exception(str, e);
      return false;
    }

    // Wenn es ein DE ist, so erhalten.
    // Wenn es ein QU ist und der naechste ein DE, dann auf DE.
    // Wenn es ein QU ist und der naechste ein QU, dann auf QU.
    if (!rkDirectEntry)
      rkDirectEntry = rk.rkDirectEntry;
  }
  
  rkNatlRank = natlRank;
  rkIntlRank = intlRank;

  return Insert();
}


bool  RkStore::Insert()
{
  if (!tmID || !naID)
    return false;

  PreparedStatement *stmtPtr = 0;

  wxString  str = "INSERT INTO RkRec (tmID, naID, rkNatlRank, rkDirectEntry, rkIntlRank) "
                     "           VALUES (?,    ?,    ?,          ?,             ?         ) ";

  try
  {
    // TODO: Keine doppelten Rankings. Evtl. per Trigger?
    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    stmtPtr->SetData(1, &tmID);
    stmtPtr->SetData(2, &naID);
    stmtPtr->SetData(3, &rkNatlRank);
    stmtPtr->SetData(4, &rkDirectEntry);
    stmtPtr->SetData(5, &rkIntlRank);

    stmtPtr->Execute();
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
    delete stmtPtr;

    return false;
  }

  delete stmtPtr;

  // Notify Views (absichtlich TMREC)
  CRequest update;
  update.type = CRequest::UPDATE;
  update.rec  = CRequest::TMREC;
  update.id   = tmID;

  CTT32App::NotifyChange(update);

  return true;
}


bool  RkStore::Remove(const TmRec &tm)
{
  RkStore  tmp(GetConnectionPtr());

  wxString  str;

  if (!tm.tmID || !tm.cpID)
    return false;

  try
  {
    // aktuelles Ranking etc. ermitteln
    tmp.SelectByTm(tm);
    
    // Wenn es kein Ranking gibt, sind wir hier fertig
    if (!tmp.Next())
      return true;

    // Sich selbst jetzt loeschen
    str = "DELETE FROM RkRec WHERE tmID = ";
    str += ltostr(tm.tmID);

    ExecuteUpdate(str);

#if 1
    // Jetzt die nachfolgenden Rankings korrigieren
    str =  "UPDATE RkRec SET rkNatlRank = rkNatlRank - 1 ";
    str += "WHERE naID = ";
    str += ltostr(tmp.naID);
    str += " AND rkNatlRank > ";
    str += ltostr(tmp.rkNatlRank);
    str += " AND tmID IN (SELECT tmID FROM TmRec WHERE cpID = ";
    str += ltostr(tm.cpID);
    str += ")";

    ExecuteUpdate(str);
#endif    
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  // Notify Views (absichtlich TMREC)
  CRequest update;
  update.type = CRequest::UPDATE;
  update.rec  = CRequest::TMREC;
  update.id   = tm.tmID;

  CTT32App::NotifyChange(update);

  return true;
}


bool  RkStore::Remove(const CpRec &cp)
{
  wxString  str = 
      "DELETE FROM RkRec "
      " WHERE tmID IN (SELECT tmID FROM TmRec WHERE cpID = " + ltostr(cp.cpID) + ")";

  try
  {
    ExecuteUpdate(str);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  // Notify Views (absichtlich 0) 
  CRequest update;
  update.type = CRequest::REMOVE;
  update.rec  = CRequest::RKREC;
  update.id   = 0;

  CTT32App::NotifyChange(update);

  return true;
}



bool  RkStore::Remove(const CpRec &cp, const NaRec &na)
{
  wxString  str = 
      "DELETE FROM RkRec "
      " WHERE naID = " + ltostr(na.naID) +
      "   AND tmID IN (SELECT tmID FROM TmRec WHERE cpID = " + ltostr(cp.cpID) + ")";

  try
  {
    ExecuteUpdate(str);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  // Notify Views (absichtlich 0) 
  CRequest update;
  update.type = CRequest::REMOVE;
  update.rec  = CRequest::RKREC;
  update.id   = 0;

  CTT32App::NotifyChange(update);

  return true;
}



bool  RkStore::Remove(const CpListRec &cp, const NaListRec &na)
{
  wxString  str = 
      "DELETE FROM RkRec "
      " WHERE naID = " + ltostr(na.naID) +
      "   AND tmID IN (SELECT tmID FROM TmRec WHERE cpID = " + ltostr(cp.cpID) + ")";

  try
  {
    ExecuteUpdate(str);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  // Notify Views (absichtlich 0) 
  CRequest update;
  update.type = CRequest::REMOVE;
  update.rec  = CRequest::RKREC;
  update.id   = 0;

  CTT32App::NotifyChange(update);

  return true;
}



// -----------------------------------------------------------------------
bool  RkStore::SelectByTm(const TmRec &tm)
{
  wxString  str = SelectString();
  str += "WHERE tmID = ";
  str += ltostr(tm.tmID);

  try
  {
    ExecuteQuery(str);
    BindRec();
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


// -----------------------------------------------------------------------
bool  RkStore::SelectByRanking(const CpRec &cp, const NaRec &na, int rank)
{
  wxString  str = SelectString();
  str += "INNER JOIN TmRec ON RkRec.tmID = TmRec.tmID "
         "WHERE TmRec.cpID = " + ltostr(cp.cpID) + 
         "  AND naID = " + ltostr(na.naID) + 
         "  AND rkNatlRank = " + ltostr(rank);

  try
  {
    ExecuteQuery(str);
    BindRec();
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


// -----------------------------------------------------------------------
bool  RkStore::SelectByRanking(const TmRec &tm, const NaRec &na, int rank)
{
  wxString  str = SelectString();
  str += "INNER JOIN TmRec ON RkRec.tmID = TmRec.tmID "
         "WHERE TmRec.cpID = " + ltostr(tm.cpID) + 
         "  AND naID = " + ltostr(na.naID) + 
         "  AND rkNatlRank = " + ltostr(rank);

  try
  {
    ExecuteQuery(str);
    BindRec();
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


// -----------------------------------------------------------------------
short RkStore::GetNextNatlRank(const NaRec &na, const TmRec &tm, short intlRank)
{
/*
select max(rknatlrank) from rkrec inner join tmrec on rkrec.tmid = tmrec.tmid 
where rkrec.naid = 1620 and tmrec.cpid = 11 and rkrec.tmid in 
(select tmid from plrec pl inner join ltrec lt on pl.plid = lt.plid inner join ntrec nt on lt.ltid = nt.ltid
 where ntnr <= 3 group by nt.tmid having sum(pl.plrankpts) >= 100)
*/

  Statement  *stmtPtr = 0;
  ResultSet  *resPtr  = 0;

  wxString  str;
  
  if (intlRank > 0)
  {
    str =  "SELECT MAX(rkNatlRank) FROM RkRec WHERE naID = " + ltostr(na.naID);
    str += " AND tmID IN (SELECT tmID FROM TmRec WHERE cpID = " + ltostr(tm.cpID) + ")";
    str += " AND rkIntlRank > 0 AND rkIntlRank <= " + ltostr(intlRank);
  }
  else
  {
    str =  "SELECT MAX(rkNatlRank) FROM RkRec INNER JOIN TmRec ON RkRec.tmID = TmRec.tmID";
    str += " WHERE RkRec.naID = " + ltostr(na.naID) + " AND TmRec.cpID = " + ltostr(tm.cpID) + " AND RkRec.tmID IN (";
    str += "   SELECT nt.tmID FROM LtList lt INNER JOIN NtRec nt ON lt.ltID = nt.ltID INNER JOIN TmRec tm ON nt.tmID = tm.tmID ";
    str += "    WHERE nt.ntNr <= 3 AND tm.cpID = " + ltostr(tm.cpID) + " GROUP BY nt.tmID HAVING SUM(lt.ltRankPts) >= (";
    str += "       SELECT SUM(ltRankPts) FROM LtList INNER JOIN NtRec ON LtList.ltID = NtRec.ltID";
    str += "        WHERE NtRec.ntNr <= 3 AND NtRec.tmID = " + ltostr(tm.tmID) + " GROUP BY NtRec.tmID";
    str += "       )";
    str += "   )";
  }

  short nr = 0;

  try
  {
    stmtPtr = GetConnectionPtr()->CreateStatement();
    resPtr  = stmtPtr->ExecuteQuery(str);
    
    if (!resPtr->Next() || !resPtr->GetData(1, nr) || resPtr->WasNull())
      nr = 0;        
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  delete resPtr;
  delete stmtPtr;

  return (++nr);
}


// -----------------------------------------------------------------------
bool RkStore::ExistsNatlRank(const NaRec &na, const TmRec &tm, short natlRank)
{
  Statement  *stmtPtr = 0;
  ResultSet  *resPtr  = 0;

  wxString  str = "SELECT COUNT(*) FROM RkRec WHERE naID = ";
  str += ltostr(na.naID);
  str += " AND tmID IN (SELECT tmID FROM TmRec WHERE cpID = ";
  str += ltostr(tm.cpID);
  str += ") AND rkNatlRank = ";
  str += ltostr(natlRank);
  
  short count = 0;

  try
  {
    stmtPtr = GetConnectionPtr()->CreateStatement();
    resPtr  = stmtPtr->ExecuteQuery(str);
    
    if (!resPtr->Next() || !resPtr->GetData(1, count) || resPtr->WasNull())
      count = 0;        
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  delete resPtr;
  delete stmtPtr;

  return (count > 0);
}


// -----------------------------------------------------------------------
wxString  RkStore::SelectString() const
{
  wxString  str = 
    "SELECT RkRec.tmID, RkRec.naID, rkNatlRank, rkIntlRank, rkDirectEntry FROM RkRec ";

  return str;
}


bool  RkStore::BindRec()
{
  BindCol(1, &tmID);
  BindCol(2, &naID);
  BindCol(3, &rkNatlRank);
  BindCol(4, &rkIntlRank);
  BindCol(5, &rkDirectEntry);

  return true;
}