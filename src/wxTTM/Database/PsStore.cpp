/* Copyright (C) 2020 Christoph Theis */

// Tabelle der personenbezogenen Daten

#include  "stdafx.h"
#include  "PsStore.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "ResultSet.h"
#include  "SQLException.h"
#include  "InfoSystem.h"
#include  "Request.h"

#include  "IdStore.h"

#include  "wxStringTokenizerEx.h"

#include  <stdio.h>
#include  <fstream>
#include  <stdlib.h>


// -----------------------------------------------------------------------
// Neue Tabelle in DB erzeugen
bool  PsStore::CreateTable()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  WVARCHAR = connPtr->GetDataType(SQL_WVARCHAR);
  wxString  INTEGER  = connPtr->GetDataType(SQL_INTEGER);
  wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);
  wxString  TIMESTAMP = connPtr->GetDataType(SQL_TIMESTAMP);

  wxString  sql = 
    "CREATE TABLE PsRec (         "
    "psID        "+INTEGER+"      NOT NULL, "
    "psLast      "+WVARCHAR+"(64) NOT NULL, "
	  "psFirst     "+WVARCHAR+"(64) NOT NULL, "
	  "psSex       "+SMALLINT+"     NOT NULL, "
	  "psBirthday  "+INTEGER+",     "
	  "psArrived   "+SMALLINT+"     DEFAULT 0, "
    "psEmail     "+WVARCHAR+ "(64) DEFAULT NULL, "
    "psPhone     "+WVARCHAR+"(64) DEFAULT NULL, "
    "psTimestamp "+TIMESTAMP+"    NOT NULL DEFAULT GETUTCDATE(), "
    "psNote      "+WVARCHAR+"(MAX) DEFAULT NULL, "
    "CONSTRAINT  psIdKey PRIMARY KEY (psID), "
    "psCreatedBy " + WVARCHAR + "(64) NOT NULL DEFAULT (SUSER_SNAME()), "
    "psModifiedBy AS (SUSER_SNAME()), "
    "psStartTime datetime2 GENERATED ALWAYS AS ROW START NOT NULL DEFAULT SYSUTCDATETIME(), "
    "psEndTime datetime2 GENERATED ALWAYS AS ROW END NOT NULL DEFAULT CAST('9999-12-31 23:59:59.9999999' AS datetime2), "
    "PERIOD FOR SYSTEM_TIME (psStartTime, psEndTime) "
    ") WITH (SYSTEM_VERSIONING = ON (HISTORY_TABLE = dbo.PsHist))";

  try
  {
    tmp->ExecuteUpdate(sql);
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(sql, e);
    delete tmp;

    return false;
  }

  try
  {
    tmp->ExecuteUpdate(sql = "CREATE INDEX psNameKey ON PsRec (psLast, psFirst)");
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(sql, e);
  };

  delete tmp;
  
  return true;
}


bool  PsStore::UpdateTable(long version)
{
  if (version == 0)
    return CreateTable();
  
  if (version < 86)
  {
    Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
    Statement *stmtPtr = connPtr->CreateStatement();

    wxString  TIMESTAMP = connPtr->GetDataType(SQL_TIMESTAMP);
    wxString str;
    
    try 
    {      
      str = "ALTER TABLE PsRec ADD psTimestamp "+TIMESTAMP+" NOT NULL DEFAULT GETUTCDATE()";
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

  if (version < 115)
  {
    Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
    Statement *stmtPtr = connPtr->CreateStatement();

    wxString  WVARCHAR = connPtr->GetDataType(SQL_WVARCHAR);
    wxString str;
    
    try 
    {      
      str = "ALTER TABLE PsRec ADD psPhone "+WVARCHAR+"(64) DEFAULT NULL";
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

  if (version < 126)
  {
    Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
    Statement *stmtPtr = connPtr->CreateStatement();

    wxString  WVARCHAR = connPtr->GetDataType(SQL_WVARCHAR);
    wxString str;
    
    try 
    {      
      str = "ALTER TABLE PsRec ADD psNote "+WVARCHAR+"(MAX) DEFAULT NULL";
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

  if (version < 134)
  {
    Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
    Statement *stmtPtr = connPtr->CreateStatement();

    wxString  WVARCHAR = connPtr->GetDataType(SQL_WVARCHAR);
    wxString str;

    try
    {
      str = "ALTER TABLE PsRec ADD psEmail " + WVARCHAR + "(64) DEFAULT NULL";
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

  if (version < 149)
  {
    // History
    Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
    Statement *stmtPtr = connPtr->CreateStatement();

    wxString  WVARCHAR = connPtr->GetDataType(SQL_WVARCHAR);
    wxString str;

    try
    {
      str = "ALTER TABLE PsRec ADD "
        "psCreatedBy " + WVARCHAR + "(64) NOT NULL DEFAULT (SUSER_SNAME()), "
        "psModifiedBy AS (SUSER_SNAME()), "
        "psStartTime datetime2 GENERATED ALWAYS AS ROW START NOT NULL DEFAULT SYSUTCDATETIME(), "
        "psEndTime datetime2 GENERATED ALWAYS AS ROW END NOT NULL DEFAULT CAST('9999-12-31 23:59:59.9999999' AS datetime2), "
        "PERIOD FOR SYSTEM_TIME (psStartTime, psEndTime)"
      ;
      stmtPtr->ExecuteUpdate(str);

      str = "ALTER TABLE PsRec SET (SYSTEM_VERSIONING = ON (HISTORY_TABLE = dbo.PsHist))";
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


bool  PsStore::CreateConstraints()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str;

  try
  {
    tmp->ExecuteUpdate("DROP TRIGGER psUpdateTrigger");
  }
  catch (SQLException &)
  {
  }
  
  try
  {
    tmp->ExecuteUpdate(str = 
        "CREATE TRIGGER psUpdateTrigger ON PsRec AFTER UPDATE AS \n"
        " --- Update timestamp for last changed \n"
        "UPDATE PsRec SET psTimestamp = SYSUTCDATETIME() \n"
        " WHERE psID IN (SELECT psID FROM deleted) \n;");
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
  };

  delete tmp;
  return true;
}


bool  PsStore::UpdateConstraints(long version)
{
  if (version < 86)
    return CreateConstraints();

  return true;
}


// -----------------------------------------------------------------------
// Konstruktor
PsStore::PsStore(Connection *connPtr)
       : StoreObj(connPtr)
{
}


PsStore::PsStore(const PsRec &rec, Connection *connPtr)
       : StoreObj(connPtr), PsRec(rec)
{
}


PsStore::~PsStore()
{
}


void  PsStore::Init()
{
  PsRec::Init();
}


// -----------------------------------------------------------------------
// Select
bool  PsStore::SelectAll()
{
  wxString str = "SELECT psID, psLast, psFirst, psSex, psBirthday, psArrived, psPhone, "
                 "       CASE WHEN psNote IS NULL THEN 0 ELSE 1 END AS psHasNote "
                 "  FROM PsRec";

  try
  {
    if (!ExecuteQuery(str))
      return false;

    BindCol(1, &psID);
    BindCol(2, psName.psLast, sizeof(psName.psLast));
    BindCol(3, psName.psFirst, sizeof(psName.psFirst));
    BindCol(4, &psSex);
    BindCol(5, &psBirthday);
    BindCol(6, &psArrived);
    BindCol(7, psPhone, sizeof(psPhone));
    BindCol(8, &psHasNote);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  return true;
}


bool  PsStore::SelectById(long id)
{
  wxString str = "SELECT psID, psLast, psFirst, psSex, psBirthday, psArrived, psPhone, CASE WHEN psNote IS NULL THEN 0 ELSE 1 END AS psHasNote "
                    "FROM PsRec "
                    "WHERE psID = ";
  str += ltostr(id);
  str += "";

  try
  {
    if (!ExecuteQuery(str))
      return false;

    BindCol(1, &psID);
    BindCol(2, psName.psLast, sizeof(psName.psLast));
    BindCol(3, psName.psFirst, sizeof(psName.psFirst));
    BindCol(4, &psSex);
    BindCol(5, &psBirthday);
    BindCol(6, &psArrived);
    BindCol(7, psPhone, sizeof(psPhone));
    BindCol(8, &psHasNote);

    return true;
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


bool  PsStore::Insert()
{
  PreparedStatement *stmtPtr = 0;

  wxString str = "INSERT INTO PsRec "
                    "(psID, psLast, psFirst, psSex, psBirthday, psArrived, psEmail, psPhone) "
                    "VALUES(?, ?, ?, ?, ?, ?, ?, ?)";

  try
  {
    psID = IdStore::ID(GetConnectionPtr());

    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    stmtPtr->SetData(1, &psID);
    stmtPtr->SetData(2, psName.psLast);
    stmtPtr->SetData(3, psName.psFirst);
    stmtPtr->SetData(4, &psSex);
    stmtPtr->SetData(5, &psBirthday);
    stmtPtr->SetData(6, &psArrived);
    stmtPtr->SetData(7, wxStrlen(psEmail) ? psEmail : NULL);
    stmtPtr->SetData(8, wxStrlen(psPhone) ? psPhone : NULL);


    stmtPtr->Execute();
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  delete stmtPtr;

  // Notify Views
  CRequest update;
  update.type = CRequest::INSERT;
  update.rec  = CRequest::PSREC;
  update.id   = psID;

  CTT32App::NotifyChange(update);

  return true;
}


bool  PsStore::Update()
{
  PreparedStatement *stmtPtr = 0;

  wxString str = "UPDATE PsRec "
                    "SET psLast = ?, psFirst = ?, psSex = ?, "
                     "   psBirthday = ?, psArrived = ?, psEmail = ?, psPhone = ? "
                    "WHERE psID = ?";
                    
  try
  {
    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    stmtPtr->SetData(1, psName.psLast);
    stmtPtr->SetData(2, psName.psFirst);
    stmtPtr->SetData(3, &psSex);
    stmtPtr->SetData(4, &psBirthday);
    stmtPtr->SetData(5, &psArrived);
    stmtPtr->SetData(6, wxStrlen(psEmail) ? psEmail : NULL);
    stmtPtr->SetData(7, wxStrlen(psPhone) ? psPhone : NULL);
    stmtPtr->SetData(8, &psID);

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
  update.rec  = CRequest::PSREC;
  update.id   = psID;

  CTT32App::NotifyChange(update);

  return true;
}


bool  PsStore::Remove(long id)
{
  // ID-Check
  if (!id)
    id = psID;

  if (!id)
    return true;

  wxString str = "DELETE FROM PsRec WHERE psID = ";
  str += ltostr(id);
  str += "";

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
  update.type = CRequest::REMOVE;
  update.rec  = CRequest::PSREC;
  update.id   = id;

  CTT32App::NotifyChange(update);

  return true;
}


// Check auf psName, ob WB existiert
bool  PsStore::InsertOrUpdate()
{
  Statement *stmtPtr;
  ResultSet *resPtr;

  long  id;
  
  stmtPtr = TTDbse::instance()->GetDefaultConnection()->CreateStatement();

  wxString  sql = "SELECT psID FROM PsRec "
                     "WHERE psLast = '";
  sql += TransformString(psName.psLast);
  sql += "' AND psFirst = '";
  sql += TransformString(psName.psFirst);
  sql += "'";

  resPtr = stmtPtr->ExecuteQuery(sql);
  resPtr->BindCol(1, &id);

  bool exist = (resPtr->Next() && !resPtr->WasNull(1));
 
  delete resPtr;
  delete stmtPtr;

  if (exist)
  {
    psID = id;
    return Update();
  }
  else
    return Insert();
}


// Insert note
bool PsStore::InsertNote(const wxString &note)
{
  PreparedStatement *stmtPtr = 0;

  wxString str = "UPDATE PsRec "
                    "SET psNote = ? "
                    "WHERE psID = ?";
                    
  try
  {
    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    stmtPtr->SetData(1, note.IsEmpty() ? NULL : (wxChar *) note.wx_str());
    stmtPtr->SetData(2, &psID);

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
  update.rec  = CRequest::PSREC;
  update.id   = psID;

  CTT32App::NotifyChange(update);

  return true;
}


// Get Note
wxString PsStore::GetNote()
{
  wxString str = "SELECT psNote FROM PsRec WHERE psID = " + ltostr(psID);
  wxString note;

  Statement *stmtPtr = 0;
  ResultSet *resPtr = 0;

  wxChar *tmp = new wxChar[4096];
  memset(tmp, 0, 4096 * sizeof(wxChar));

  try
  {
    stmtPtr = GetConnectionPtr()->CreateStatement();
    resPtr = stmtPtr->ExecuteQuery(str);
    bool ret = resPtr->Next() && resPtr->GetData(1, tmp, 4095);
    if (!ret || resPtr->WasNull(1))
      *tmp = 0;
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  delete resPtr;
  delete stmtPtr;

  note = tmp;
  delete[] tmp;

  return note;
}
