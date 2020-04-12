/* Copyright (C) 2020 Christoph Theis */

// Relation GrRec <--> StRec

#include  "stdafx.h"
#include  "XxStore.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "SQLException.h"
#include  "InfoSystem.h"
#include  "Request.h"

#include  "StStore.h"
#include  "GrStore.h"

#include  "Rec.h"


// -----------------------------------------------------------------------
bool  XxStore::CreateTable()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  wxASSERT(connPtr);

  Statement *tmp = connPtr->CreateStatement();

  wxString  INTEGER  = connPtr->GetDataType(SQL_INTEGER);
  wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);
  wxString  WVARCHAR = connPtr->GetDataType(SQL_WVARCHAR);

  wxString  sql = 
    "CREATE TABLE XxRec (         "
    "stID        "+INTEGER+"      NOT NULL,  "
    "grID        "+INTEGER+",     "
    "grPos       "+SMALLINT+",    "
    "CONSTRAINT xxIdKey PRIMARY KEY (stID),  "    
    "xxCreatedBy " + WVARCHAR + "(64) NOT NULL DEFAULT (SUSER_SNAME()), "
    "xxModifiedBy AS (SUSER_SNAME()), "
    "xxStartTime datetime2 GENERATED ALWAYS AS ROW START NOT NULL DEFAULT SYSUTCDATETIME(), "
    "xxEndTime datetime2 GENERATED ALWAYS AS ROW END NOT NULL DEFAULT CAST('9999-12-31 23:59:59.9999999' AS datetime2), "
    "PERIOD FOR SYSTEM_TIME (xxStartTime, xxEndTime) "
    ") WITH (SYSTEM_VERSIONING = ON (HISTORY_TABLE = dbo.XxHist))";

  try
  {
    tmp->ExecuteUpdate("DROP TABLE XxRec");
  }
  catch(SQLException &)
  {
  }

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
    tmp->ExecuteUpdate(sql = "CREATE INDEX xxGrKey ON XxRec (grID, grPos)");
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(sql, e);
  };

  tmp->Close();
  delete tmp;
  
  return true;
}


bool  XxStore::UpdateTable(long version)
{
  if (version <= 16)
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
      str = "ALTER TABLE XxRec ADD "
        "xxCreatedBy " + WVARCHAR + "(64) NOT NULL DEFAULT (SUSER_SNAME()), "
        "xxModifiedBy AS (SUSER_SNAME()), "
        "xxStartTime datetime2 GENERATED ALWAYS AS ROW START NOT NULL DEFAULT SYSUTCDATETIME(), "
        "xxEndTime datetime2 GENERATED ALWAYS AS ROW END NOT NULL DEFAULT CAST('9999-12-31 23:59:59.9999999' AS datetime2), "
        "PERIOD FOR SYSTEM_TIME (xxStartTime, xxEndTime)"
        ;
      stmtPtr->ExecuteUpdate(str);

      str = "ALTER TABLE XxRec SET (SYSTEM_VERSIONING = ON(HISTORY_TABLE = dbo.XxHist))";
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


bool  XxStore::CreateConstraints()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();
  
  try
  {
    tmp->ExecuteUpdate("ALTER TABLE XxRec DROP CONSTRAINT xxGrRec");
  }
  catch (SQLException &)
  {
  }
  try
  {
    tmp->ExecuteUpdate("ALTER TABLE XxRec DROP CONSTRAINT xxGrRef");
  }
  catch (SQLException &)
  {
  }
  try
  {
    tmp->ExecuteUpdate("ALTER TABLE XxRec DROP CONSTRAINT xxStRef");
  }
  catch (SQLException &)
  {
  }
  wxString  str;

  try
  {
    tmp->ExecuteUpdate(str = 
      "ALTER TABLE XxRec ADD CONSTRAINT xxGrRef "
      "FOREIGN KEY (grID) REFERENCES GrRec (grID) ON DELETE CASCADE");

    tmp->ExecuteUpdate(str = 
      "ALTER TABLE XxRec ADD CONSTRAINT xxStRef "
      "FOREIGN KEY (stID) REFERENCES StRec (stID) ON DELETE CASCADE");
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
  };

  delete tmp;
  return true;
}


bool  XxStore::UpdateConstraints(long version)
{
  if (version <= 50)
    return CreateConstraints();
  return true;
}


// -----------------------------------------------------------------------
XxStore::XxStore(Connection *ptr)
       : StoreObj(ptr)
{
}


XxStore::~XxStore()
{
}


// -----------------------------------------------------------------------
void  XxStore::Init()
{
  XxRec::Init();
}


// -----------------------------------------------------------------------
bool  XxStore::Select(const StRec &st)
{
  wxString str = SelectString();
  str += "WHERE stID = ";
  str += ltostr(st.stID);

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


bool  XxStore::Select(const GrRec &gr, short pos)
{
  wxString str = SelectString();
  str += " WHERE grID = " + ltostr(gr.grID) +
         "   AND grPos = " + ltostr(pos);

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


bool  XxStore::SelectAll()
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



// -----------------------------------------------------------------------
bool  XxStore::Insert(const StRec &st, const GrRec &gr, short pos)
{
  grID = gr.grID;
  stID = st.stID;
  grPos = pos;

  PreparedStatement *stmtPtr = 0;

  wxString str = "INSERT INTO XxRec (stID, grID, grPos) "
                    "            VALUES(?,    ?,    ?   ) ";

  try
  {
    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    stmtPtr->SetData(1, &stID);
    stmtPtr->SetData(2, grID ? &grID : (long *) NULL);
    stmtPtr->SetData(3, grPos ? &grPos : (short *) NULL);

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
  update.rec  = CRequest::STREC;
  update.id   = stID;

  CTT32App::NotifyChange(update);

  return true;
}


bool  XxStore::Remove(const StRec &st)
{
  wxString  str = 
    "DELETE FROM XxRec WHERE stID = " + ltostr(st.stID);
  
  try
  {
    ExecuteUpdate(str);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  // Notify Views
  CRequest update;
  update.type = CRequest::UPDATE;
  update.rec  = CRequest::STREC;
  update.id   = st.stID;

  CTT32App::NotifyChange(update);

  return true;
}


bool  XxStore::Remove(const GrRec &gr, short pos)
{
  wxString  str;
    "DELETE FROM XxRec WHERE grID = " + ltostr(gr.grID);
  if (pos == 0)
    str += " AND grPos = " + ltostr(pos);
  
  try
  {
    // Die loeschen, die gr als Quelle haben
    str = "DELETE FROM XxRec WHERE grID = " + ltostr(gr.grID);
    if (pos != 0)
      str += " AND grPos = " + ltostr(pos);
    ExecuteUpdate(str);

    // Wenn pos == 0 ist, wird die ganze Gruppe geloescht.
    // Also auch die loeschen, die StRec in dieser Gruppe referenziert
    if (pos == 0)
    {
      str = "DELETE FROM XxRec WHERE stID In "
            "(SELECT stID FROM StRec WHERE grID = " + ltostr(gr.grID) + ")";
      ExecuteUpdate(str);
    }
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  // XXX TODO: Update Views
  return true;
}


// -----------------------------------------------------------------------
wxString  XxStore::SelectString() const
{
  wxString  str = 
      "SELECT stID, grID, grPos FROM XxRec ";

  return str;
}


bool  XxStore::BindRec()
{
  BindCol(1, &stID);
  BindCol(2, &grID);
  BindCol(3, &grPos);

  return true;
}


