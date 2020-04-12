/* Copyright (C) 2020 Christoph Theis */

// Tabelle der Teams
// Relation PlRec, CpRec, TmRec

#include  "stdafx.h"
#include  "TmStore.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "SQLException.h"
#include  "InfoSystem.h"
#include  "Request.h"

#include  "IdStore.h"
#include  "CpStore.h"
#include  "LtStore.h"
#include  "RkStore.h"
#include  "NtStore.h"
#include  "StStore.h"


bool  TmStore::CreateTable()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  wxASSERT(connPtr);

  Statement *tmp = connPtr->CreateStatement();

  wxString  WVARCHAR = connPtr->GetDataType(SQL_WVARCHAR);
  wxString  INTEGER  = connPtr->GetDataType(SQL_INTEGER);
  wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);
  wxString  TIMESTAMP = connPtr->GetDataType(SQL_TIMESTAMP);

  wxString  sql = 
    "CREATE TABLE TmRec (         "
    "tmID        "+INTEGER+"      NOT NULL,  "
    "cpID        "+INTEGER+"      NOT NULL,  "
    "tmDisqu     "+SMALLINT+"     DEFAULT 0, "
    "tmGaveup    "+SMALLINT+"     DEFAULT 0, "
    "tmName      "+WVARCHAR+"(8),  "
    "tmDesc      "+WVARCHAR+"(64), "
    "tmTimestamp " + TIMESTAMP + "    NOT NULL DEFAULT GETUTCDATE(), "
    "CONSTRAINT tmIdKey PRIMARY KEY (tmID),  "
    "tmCreatedBy " + WVARCHAR + "(64) NOT NULL DEFAULT (SUSER_SNAME()), "
    "tmModifiedBy AS (SUSER_SNAME()), "
    "tmStartTime datetime2 GENERATED ALWAYS AS ROW START NOT NULL DEFAULT SYSUTCDATETIME(), "
    "tmEndTime datetime2 GENERATED ALWAYS AS ROW END NOT NULL DEFAULT CAST('9999-12-31 23:59:59.9999999' AS datetime2), "
    "PERIOD FOR SYSTEM_TIME (tmStartTime, tmEndTime) "
    ") WITH (SYSTEM_VERSIONING = ON (HISTORY_TABLE = dbo.TmHist))";

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
    tmp->ExecuteUpdate(sql = "CREATE INDEX tmCpIdKey ON TmRec (cpID)");
    tmp->ExecuteUpdate(sql = "CREATE INDEX tmNameKey ON TmRec (tmName)");
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(sql, e);
  };

  tmp->Close();
  delete tmp;
  
  return true;
}


bool  TmStore::UpdateTable(long version)
{
  if (version == 0)
    return CreateTable();

  else if (version < 147)
  {
    Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
    Statement *stmtPtr = connPtr->CreateStatement();

    wxString  TIMESTAMP = connPtr->GetDataType(SQL_TIMESTAMP);
    wxString str;

    try
    {
      str = "ALTER TABLE TmRec ADD tmTimestamp " + TIMESTAMP + " NOT NULL DEFAULT GETUTCDATE()";
      stmtPtr->ExecuteUpdate(str);
    }
    catch (SQLException &e)
    {
      infoSystem.Exception(str, e);
      delete stmtPtr;
      return false;
    }

    if (version < 149)
    {
      // History
      Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
      Statement *stmtPtr = connPtr->CreateStatement();

      wxString  WVARCHAR = connPtr->GetDataType(SQL_WVARCHAR);
      wxString str;

      try
      {
        str = "ALTER TABLE TmRec ADD "
          "tmCreatedBy " + WVARCHAR + "(64) NOT NULL DEFAULT (SUSER_SNAME()), "
          "tmModifiedBy AS (SUSER_SNAME()), "
          "tmStartTime datetime2 GENERATED ALWAYS AS ROW START NOT NULL DEFAULT SYSUTCDATETIME(), "
          "tmEndTime datetime2 GENERATED ALWAYS AS ROW END NOT NULL DEFAULT CAST('9999-12-31 23:59:59.9999999' AS datetime2), "
          "PERIOD FOR SYSTEM_TIME (tmStartTime, tmEndTime)"
          ;
        stmtPtr->ExecuteUpdate(str);

        str = "ALTER TABLE TmRec SET (SYSTEM_VERSIONING = ON(HISTORY_TABLE = dbo.TmHist))";
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

    delete stmtPtr;
  }

  return true;
}


bool  TmStore::CreateConstraints()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str;
  
  try
  {
    tmp->ExecuteUpdate("DROP TRIGGER tmUpdateTrigger");
  }
  catch (SQLException &)
  {
  }

  try
  {
    tmp->ExecuteUpdate("ALTER TABLE TmRec DROP CONSTRAINT tmCpRef");
  }
  catch (SQLException &)
  {
  }

  try
  {
    tmp->ExecuteUpdate(str =
      "CREATE TRIGGER tmUpdateTrigger ON TmRec FOR UPDATE AS \n"
      " --- Update timestamp for last changed \n"
      "UPDATE TmRec SET tmTimestamp = GETUTCDATE() \n"
      " WHERE tmID IN (SELECT tmID FROM deleted) \n;");

    tmp->ExecuteUpdate(str =
      "ALTER TABLE TmRec ADD CONSTRAINT tmCpRef "
      "FOREIGN KEY (cpID) REFERENCES CpRec (cpID) ON DELETE NO ACTION");
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
  };

  delete tmp;
  return true;
}


bool  TmStore::UpdateConstraints(long version)
{
  if (version <= 50)
    return CreateConstraints();

  return true;
}


// -----------------------------------------------------------------------
TmStore::TmStore(Connection *ptr)
       : StoreObj(ptr)
{
}


TmStore::~TmStore()
{
}


void  TmStore::Init()
{
  TmRec::Init();
}

// -----------------------------------------------------------------------
// Selects
bool  TmStore::SelectAll()
{
  wxString str = SelectString();

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


bool  TmStore::SelectById(long id)
{
  wxString str = SelectString();
  str += "WHERE tmID = ";
  str += ltostr(id);

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


bool  TmStore::SelectByName(const wxString &name)
{
  wxString str = SelectString();
  str += "WHERE tmName = '";
  str += name;
  str += "'";

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


bool  TmStore::SelectByCp(long id)
{
  wxString str = SelectString();
  str += "WHERE cpID = ";
  str += ltostr(id);

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


bool  TmStore::SelectByLt(const LtRec &lt)
{
  wxString str = SelectString();
  str += "WHERE tm.tmID IN ( "
         "   SELECT tmID FROM NtRec nt "
         "    WHERE nt.ltID = " + ltostr(lt.ltID);
  str += " )";

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


bool  TmStore::SelectByCpPlNr(const CpRec &cp, long plNr)
{
  wxString  str = SelectString();
  str += " WHERE cpID = " + ltostr(cp.cpID) + " AND tmID IN "
         " (SELECT tmID FROM NtRec nt INNER JOIN LtRec lt ON nt.ltID = lt.ltID "
         "   INNER JOIN PlRec pl ON lt.plID = pl.plID "
         "   WHERE pl.plNr = " + ltostr(plNr) + ")";

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


bool  TmStore::SelectByCpTmName(const CpRec &cp, const wxString &name)
{
  wxString  str = SelectString();
  str += " WHERE cpID = " + ltostr(cp.cpID) + " AND tmName = '" + name + "'";

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
bool  TmStore::Insert(const CpRec &cp)
{
  // cpID ist Pflicht!
  if (!cp.cpID)
    return false;

  PreparedStatement *stmtPtr = 0;

  wxString str = "INSERT INTO TmRec (tmID, cpID, tmDisqu, tmGaveup, tmName, tmDesc) "
                    "            VALUES(?,    ?,    ?,       ?,        ?,      ?     ) ";

  try
  {
    tmID = IdStore::ID(GetConnectionPtr());
    cpID = cp.cpID;

    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    wxASSERT(stmtPtr);

    stmtPtr->SetData(1, &tmID);
    stmtPtr->SetData(2, &cpID);
    stmtPtr->SetData(3, &tmDisqu);
    stmtPtr->SetData(4, &tmGaveup);
    stmtPtr->SetData(5, *tmName ? tmName : (wxChar *) NULL);
    stmtPtr->SetData(6, *tmDesc ? tmDesc : (wxChar *) NULL);

    stmtPtr->Execute();
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    delete stmtPtr;

    return false;
  }

  delete stmtPtr;

  // Notify Views
  CRequest update;
  update.type = CRequest::INSERT;
  update.rec  = CRequest::TMREC;
  update.id   = tmID;

  CTT32App::NotifyChange(update);

  return true;
}


bool  TmStore::Update()
{
  // cpID ist Pflicht!
  if (!cpID)
    return false;

  PreparedStatement *stmtPtr = 0;

  wxString str = 
      "UPDATE TmRec SET tmDisqu = ?, tmGaveup = ?, tmName = ?, tmDesc = ? "
      "WHERE tmID = ?";
                    
  stmtPtr = GetConnectionPtr()->PrepareStatement(str);
  wxASSERT(stmtPtr);

  try
  {
    stmtPtr->SetData(1, &tmDisqu);
    stmtPtr->SetData(2, &tmGaveup);
    stmtPtr->SetData(3, *tmName ? tmName : (wxChar *) NULL);
    stmtPtr->SetData(4, *tmDesc ? tmDesc : (wxChar *) NULL);
    stmtPtr->SetData(5, &tmID);

    stmtPtr->Execute();
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  delete stmtPtr;

  // Notify Views
  CRequest update;
  update.type = CRequest::UPDATE;
  update.rec  = CRequest::TMREC;
  update.id   = tmID;

  CTT32App::NotifyChange(update);

  return true;
}


bool  TmStore::Remove(long id)
{
  // Ich brauch mehr als nur meine ID, RkRec haengt auch vom WB ab.
  if (id)
  {
    if (!SelectById(id) || !Next())
      return false;
  }
  else if (!tmID)
    return false;
  
  wxString  str;

  try
  {
    // Aus StRec loeschen (StRec kennt kein ON DELETE SET NULL)
    if ( !StStore(GetConnectionPtr()).RemoveTeam(tmID) )
      return false;
      
    // Aus RkRec loeschen (rkNatlRank korrigieren)
    if ( !RkStore(GetConnectionPtr()).Remove(*this) )
      return false;

    // NtRec explizit loeschen
    RemoveAllEntries();
      
    // Record endgueltig loeschen
    str = "DELETE FROM TmRec WHERE tmID = ";
    str += ltostr(tmID);
    ExecuteUpdate(str);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  // Notify Views
  CRequest update;
  update.type = CRequest::REMOVE;
  update.rec  = CRequest::TMREC;
  update.id   = tmID;

  CTT32App::NotifyChange(update);

  return true;  
}


// -----------------------------------------------------------------------
// Remove all entries
bool  TmStore::RemoveAllEntries()
{
  if (!tmID)
    return false;

  return NtStore(GetConnectionPtr()).Remove(*this);
}


bool  TmStore::RemoveEntry(const LtRec &lt)
{
  NtStore  nt(GetConnectionPtr());
  return nt.Remove(lt);
}


// Add one entry
bool  TmStore::AddEntry(const LtRec &lt, short ntNr)
{
  if (!tmID || !lt.ltID)
    return false;

  return NtStore(GetConnectionPtr()).Insert(lt, *this, ntNr);
}


long  TmStore::GetMaxNameLength()
{
  Statement *stmtPtr;
  ResultSet *resPtr;

  long  len = 0;

  stmtPtr = GetConnectionPtr()->CreateStatement();

  wxString  sql = "SELECT MAX(LEN(tmName)) FROM TmRec";

  resPtr = stmtPtr->ExecuteQuery(sql);
  resPtr->BindCol(1, &len);
  resPtr->Next();

  delete resPtr;
  delete stmtPtr;

  return len;
}





// -----------------------------------------------------------------------
wxString  TmStore::SelectString() const
{
  wxString str = 
    "SELECT tm.tmID, tm.cpID, tm.tmDisqu, tm.tmGaveup, tm.tmName, tm.tmDesc "
    "FROM TmRec tm ";

  return str;
}


bool  TmStore::BindRec()
{
  BindCol(1, &tmID);
  BindCol(2, &cpID);
  BindCol(3, &tmDisqu);
  BindCol(4, &tmGaveup);
  BindCol(5, tmName, sizeof(tmName));
  BindCol(6, tmDesc, sizeof(tmDesc));

  return true;
}