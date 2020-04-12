/* Copyright (C) 2020 Christoph Theis */

// Tabelle der Mitglieder eines Teams

#include  "stdafx.h"
#include  "NtStore.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "ResultSet.h"
#include  "SQLException.h"
#include  "InfoSystem.h"
#include  "Request.h"

#include  "TmStore.h"
#include  "LtStore.h"



bool  NtStore::CreateTable()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  wxASSERT(connPtr);

  Statement *tmp = connPtr->CreateStatement();

  wxString  INTEGER  = connPtr->GetDataType(SQL_INTEGER);
  wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);
  wxString  WVARCHAR = connPtr->GetDataType(SQL_WVARCHAR);

  wxString  sql = 
    "CREATE TABLE NtRec (         "
    "tmID        "+INTEGER+"      NOT NULL,  "
    "ltID        "+INTEGER+"      NOT NULL,  "
    "ntNr        "+SMALLINT+"     NOT NULL,  "
    "CONSTRAINT ntKey PRIMARY KEY (tmID, ntNr), "    
    "ntCreatedBy " + WVARCHAR + "(64) NOT NULL DEFAULT (SUSER_SNAME()), "
    "ntModifiedBy AS (SUSER_SNAME()), "
    "ntStartTime datetime2 GENERATED ALWAYS AS ROW START NOT NULL DEFAULT SYSUTCDATETIME(), "
    "ntEndTime datetime2 GENERATED ALWAYS AS ROW END NOT NULL DEFAULT CAST('9999-12-31 23:59:59.9999999' AS datetime2), "
    "PERIOD FOR SYSTEM_TIME (ntStartTime, ntEndTime) "
    ") WITH (SYSTEM_VERSIONING = ON (HISTORY_TABLE = dbo.NtHist))";

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
    tmp->ExecuteUpdate(sql = "CREATE UNIQUE INDEX ntTmKey ON NtRec (tmID, ntNr)");
    tmp->ExecuteUpdate(sql = "CREATE INDEX ntLtKey ON NtRec (ltID)");
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(sql, e);
  };

  tmp->Close();
  delete tmp;
  
  return true;
}


bool  NtStore::UpdateTable(long version)
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
      str = "ALTER TABLE NtRec ADD "
        "ntCreatedBy " + WVARCHAR + "(64) NOT NULL DEFAULT (SUSER_SNAME()), "
        "ntModifiedBy AS (SUSER_SNAME()), "
        "ntStartTime datetime2 GENERATED ALWAYS AS ROW START NOT NULL DEFAULT SYSUTCDATETIME(), "
        "ntEndTime datetime2 GENERATED ALWAYS AS ROW END NOT NULL DEFAULT CAST('9999-12-31 23:59:59.9999999' AS datetime2), "
        "PERIOD FOR SYSTEM_TIME (ntStartTime, ntEndTime)"
        ;
      stmtPtr->ExecuteUpdate(str);

      str = "ALTER TABLE NtRec SET (SYSTEM_VERSIONING = ON(HISTORY_TABLE = dbo.NtHist))";
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


bool  NtStore::CreateConstraints()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str;
  
  try
  {
    tmp->ExecuteUpdate("DROP TRIGGER ntInsertDeleteTrigger");
  }
  catch (SQLException &)
  {
  }
  
  try
  {
    tmp->ExecuteUpdate("ALTER TABLE NtRec DROP CONSTRAINT ntTmRec");
  }
  catch (SQLException &)
  {
  }

  try
  {
    tmp->ExecuteUpdate("ALTER TABLE NtRec DROP CONSTRAINT ntTmRef");
  }
  catch (SQLException &)
  {
  }
    
  try
  {
    tmp->ExecuteUpdate("ALTER TABLE NtRec DROP CONSTRAINT ntLtRec");
  }
  catch (SQLException &)
  {
  }
  try
  {
    tmp->ExecuteUpdate("ALTER TABLE NtRec DROP CONSTRAINT ntLtRef");
  }
  catch (SQLException &)
  {
  }

  try
  {
    tmp->ExecuteUpdate(str = 
        "CREATE TRIGGER ntInsertDeleteTrigger ON NtRec FOR INSERT, DELETE AS \n"
        " --- Update timestamp for last changed \n"
        "UPDATE LtRec SET ltTimestamp = GETUTCDATE() \n"
        " WHERE ltID IN (SELECT ltID FROM deleted) OR ltID IN (SELECT ltID FROM inserted) \n;");
        
    // ON DELETE CASCADE geht hier leider nicht, weil es mehrere 
    // Pfade gibt
    tmp->ExecuteUpdate(str = 
      "ALTER TABLE NtRec ADD CONSTRAINT ntTmRef "
      "FOREIGN KEY (tmID) REFERENCES TmRec (tmID) ON DELETE CASCADE");

    tmp->ExecuteUpdate(str = 
      "ALTER TABLE NtRec ADD CONSTRAINT ntLtRef "
      "FOREIGN KEY (ltID) REFERENCES LtRec (ltID) ON DELETE NO ACTION");
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
  };

  delete tmp;
  return true;
}


bool  NtStore::UpdateConstraints(long version)
{
  if (version < 87)
    return CreateConstraints();

  return true;
}


// -----------------------------------------------------------------------
NtStore::NtStore(Connection *ptr)
       : StoreObj(ptr)
{
}


NtStore::~NtStore()
{
}


void  NtStore::Init()
{
  NtRec::Init();
}

// -----------------------------------------------------------------------
bool  NtStore::Insert(const LtRec &lt, const TmRec &tm, short nr)
{
  if (!lt.ltID || !tm.tmID)
    return false;

  PreparedStatement *stmtPtr = 0;

  wxString  str;

  tmID = tm.tmID;
  ltID = lt.ltID;
  ntNr = (nr ? nr : GetNextNumber(tm.tmID));

  try
  {
    if (nr)
    {
      str = "UPDATE NtRec SET ntNr = ntNr + 1 WHERE tmID = ";
      str += ltostr(tmID);
      str += " AND ntNr >= ";
      str += ltostr(nr);

      if (!ExecuteUpdate(str))
        return false;
    }

    str = "INSERT INTO NtRec (tmID, ltID, ntNr) VALUES (?, ?, ?)";

    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    stmtPtr->SetData(1, &tmID);
    stmtPtr->SetData(2, &ltID);
    stmtPtr->SetData(3, &ntNr);

    stmtPtr->Execute();
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str,e);
    delete stmtPtr;

    return false;
  }
  
  delete stmtPtr;

  // Notify Views (absichtlich LTREC)
  CRequest update;
  update.type = CRequest::UPDATE;
  update.rec  = CRequest::LTREC;
  update.id   = ltID;

  CTT32App::NotifyChange(update);

  return true;
}


bool  NtStore::Remove(const LtRec &lt)
{
  if (!lt.ltID)
    return false;

  short  nr = 0;
  TmRec  tm;
  wxString  str;

  Statement *stmtPtr = 0;
  ResultSet *resPtr  = 0;

  try
  {
    // tmID und ntNr ermitteln, damit nachfolgende Nummern nachrutschen
    str = "SELECT ntNr, tmID FROM NtRec WHERE ltID = ";
    str += ltostr(lt.ltID);

    stmtPtr = GetConnectionPtr()->CreateStatement();
    resPtr  = stmtPtr->ExecuteQuery(str);
    if ( !resPtr->Next() || 
         !resPtr->GetData(1, nr) || resPtr->WasNull() ||
         !resPtr->GetData(2, tm.tmID) || resPtr->WasNull() )
    {
      delete resPtr;
      delete stmtPtr;

      return false;
    }
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);

    delete resPtr;
    delete stmtPtr;

    return false;
  }

  delete resPtr;
  delete stmtPtr;
  
  ltID = lt.ltID;

  if (!Remove(tm, nr))
    return false;

  return true;
}


bool  NtStore::Remove(const TmRec &tm, short nr)
{
  if (!tm.tmID)
    return false;

  if (nr == 0)
  {
    std::vector<short> nrList;
    if (!SelectByTmNr(tm, 0))
      return false;

    while (Next())
      nrList.push_back(ntNr);

    Close();

    for (std::vector<short>::reverse_iterator it = nrList.rbegin(); it != nrList.rend(); it++)
      Remove(tm, *it);

    return true;
  }

  if (!SelectByTmNr(tm, nr) || !Next())
    return false;

  wxString  str;

  try
  {  
    // Record loeschen
    str = "DELETE FROM NtRec WHERE tmID = ";
    str += ltostr(tm.tmID);

    if (nr)
    {
      str += " AND ntNr = ";
      str += ltostr(nr);
    }

    if (!ExecuteUpdate(str))
      return false;
      
    // Nachfolgende Nummern nachrutschen lassen
    if (nr)
    {
      str = "UPDATE NtRec SET ntNr = ntNr - 1 WHERE tmID = ";
      str += ltostr(tm.tmID);
      str += " AND ntNr > ";
      str += ltostr(nr);

      if (!ExecuteUpdate(str))
        return false;
    }
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  // Notify Views
  CRequest update;
  update.type = CRequest::UPDATE;
  update.rec  = CRequest::LTREC;
  update.id   = ltID;

  CTT32App::NotifyChange(update);

  return true;
}


// -----------------------------------------------------------------------
bool  NtStore::SelectByLt(const LtRec &lt)
{
  wxString  str = SelectString();
  str += "WHERE ltID = ";
  str += ltostr(lt.ltID);

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


bool  NtStore::SelectByTmNr(const TmRec &tm, short nr)
{
  wxString  str = SelectString();
  str += "WHERE tmID = ";
  str += ltostr(tm.tmID);

  if (nr)
  {
    str += " AND ntNr = ";
    str += ltostr(nr);
  }

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
short  NtStore::GetNextNumber(long tmID)
{
  Statement *stmtPtr;
  ResultSet *resPtr;

  wxString  str = "SELECT MAX(ntNr) FROM NtRec WHERE tmID = ";
  str += ltostr(tmID);

  try
  {
    stmtPtr = GetConnectionPtr()->CreateStatement();
    wxASSERT(stmtPtr);

    resPtr = stmtPtr->ExecuteQuery(str);
    wxASSERT(resPtr);
  }
  catch (SQLException e)
  {
    infoSystem.Exception(str, e);
    return 1;
  }

  short nr;
  if (!resPtr->Next() || !resPtr->GetData(1, nr) || resPtr->WasNull())
  {
    delete stmtPtr;
    delete resPtr;

    return 1;
  }
  
  delete resPtr;
  delete stmtPtr;  

  return nr+1;
}




// -----------------------------------------------------------------------
wxString  NtStore::SelectString() const
{
  wxString str = 
      "SELECT tmID, ltID, ntNr FROM NtRec ";

  return str;
}


bool  NtStore::BindRec()
{
  BindCol(1, &tmID);
  BindCol(2, &ltID);
  BindCol(3, &ntNr);

  return true;
}

