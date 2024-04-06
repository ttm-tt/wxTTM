/* Copyright (C) 2020 Christoph Theis */

// Tabellendefinition der Nationen

#include  "stdafx.h"
#include  "NaStore.h"

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


bool NaRec::Read(const wxString &line)
{
  wxStringTokenizerEx tokens(line, ",;\t");
  wxString strName = tokens.GetNextToken().Strip(wxString::both);
  wxString strDesc = tokens.GetNextToken().Strip(wxString::both);
  wxString strRegion = (tokens.HasMoreTokens() ? tokens.GetNextToken().Strip(wxString::both).c_str() : wxEmptyString);

  if (!strName.IsEmpty() && !strDesc.IsEmpty())
  {
    Init();

    wxStrncpy((wxChar *) naName, strName, sizeof(naName) / sizeof(wxChar) -1);
    wxStrncpy((wxChar *) naDesc, strDesc, sizeof(naDesc) / sizeof(wxChar) -1);
    wxStrncpy((wxChar *) naRegion, strRegion, sizeof(naRegion) / sizeof(wxChar) - 1);
  }

  return true;
}


bool NaRec::Write(wxString &str) const
{
  str << naName << ";" 
      << naDesc << ";"
      << naRegion << "";

  return true;
}



// -----------------------------------------------------------------------
// Neue Tabelle in DB erzeugen
bool  NaStore::CreateTable()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  Statement *tmp = connPtr->CreateStatement();

  wxString  WVARCHAR = connPtr->GetDataType(SQL_WVARCHAR);
  wxString  INTEGER  = connPtr->GetDataType(SQL_INTEGER);
  wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);

  wxString  sql = 
    "CREATE TABLE NaRec (         "
    "naID        "+INTEGER+"      NOT NULL,  "
    "naName      "+WVARCHAR+"(8)  NOT NULL,  "
	  "naDesc      "+WVARCHAR+"(64) NOT NULL,  "
    "naRegion    "+WVARCHAR+"(64),           "
    "CONSTRAINT naIdKey PRIMARY KEY (naID),  "
    "naCreatedBy " + WVARCHAR + "(64) NOT NULL DEFAULT (SUSER_SNAME()), "
    "naModifiedBy AS (SUSER_SNAME()), "
    "naStartTime datetime2 GENERATED ALWAYS AS ROW START NOT NULL DEFAULT SYSUTCDATETIME(), "
    "naEndTime datetime2 GENERATED ALWAYS AS ROW END NOT NULL DEFAULT CAST('9999-12-31 23:59:59.9999999' AS datetime2), "
    "PERIOD FOR SYSTEM_TIME (naStartTime, naEndTime) "
    ") WITH (SYSTEM_VERSIONING = ON (HISTORY_TABLE = dbo.NaHist))";

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
    tmp->ExecuteUpdate(sql = "CREATE UNIQUE INDEX naNameKey ON NaRec (naName)");
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(sql, e);
  };

  delete tmp;
  
  return true;
}


bool  NaStore::UpdateTable(long version)
{
  if (version == 0)
    return CreateTable();

  if (version < 94)
  {
    Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
    Statement *tmp = connPtr->CreateStatement();

    wxString  WVARCHAR = connPtr->GetDataType(SQL_WVARCHAR);

    try
    {
      tmp->ExecuteUpdate("ALTER TABLE NaRec ADD "
                          "naRegion " + WVARCHAR + "(64) DEFAULT NULL");
    }
    catch (SQLException &)
    {
      delete tmp;
      return false;
    }
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
      str = "ALTER TABLE NaRec ADD "
        "naCreatedBy " + WVARCHAR + "(64) NOT NULL DEFAULT (SUSER_SNAME()), "
        "naModifiedBy AS (SUSER_SNAME()), "
        "naStartTime datetime2 GENERATED ALWAYS AS ROW START NOT NULL DEFAULT SYSUTCDATETIME(), "
        "naEndTime datetime2 GENERATED ALWAYS AS ROW END NOT NULL DEFAULT CAST('9999-12-31 23:59:59.9999999' AS datetime2), "
        "PERIOD FOR SYSTEM_TIME (naStartTime, naEndTime)"
        ;
      stmtPtr->ExecuteUpdate(str);

      str = "ALTER TABLE NaRec SET (SYSTEM_VERSIONING = ON(HISTORY_TABLE = dbo.NaHist))";
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


// -----------------------------------------------------------------------
// Konstruktor
NaStore::NaStore(Connection * connPtr)
       : StoreObj(connPtr)
{
}


NaStore::NaStore(const NaRec &rec, Connection *connPtr) 
       : StoreObj(connPtr), NaRec(rec)
{
}


NaStore::~NaStore()
{
}


void  NaStore::Init()
{
  NaRec::Init();
}


// -----------------------------------------------------------------------
// Select
bool  NaStore::SelectAll()
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


bool  NaStore::SelectById(long id)
{
  wxString str = SelectString();
  str += " WHERE naID = ";
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


bool  NaStore::SelectByName(const wxString &name)
{
  wxString str = SelectString();
  str += " WHERE naName = '";
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


bool  NaStore::Insert()
{
  PreparedStatement *stmtPtr = 0;

  wxString str = "INSERT INTO NaRec (naID, naName, naDesc, naRegion) "
                    "VALUES(?, ?, ?, ?)";

  try
  {
    naID = IdStore::ID(GetConnectionPtr());

    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    stmtPtr->SetData(1, &naID);
    stmtPtr->SetData(2, naName);
    stmtPtr->SetData(3, naDesc);
    stmtPtr->SetData(4, naRegion);

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
  update.rec  = CRequest::NAREC;
  update.id   = naID;

  CTT32App::NotifyChange(update);

  return true;
}


bool  NaStore::Update()
{
  PreparedStatement *stmtPtr = 0;

  wxString str = "UPDATE NaRec "
                    "SET naName = ?, naDesc = ?, naRegion = ? "
                    "WHERE naID = ?";

  try
  {
    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    stmtPtr->SetData(1, naName);
    stmtPtr->SetData(2, naDesc);
    stmtPtr->SetData(3, naRegion);
    stmtPtr->SetData(4, &naID);



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
  update.rec  = CRequest::NAREC;
  update.id   = naID;

  CTT32App::NotifyChange(update);

  return true;
}


bool  NaStore::Remove(long id)
{
  // ID-Check
  if (!id)
    id = naID;

  if (!id)
    return true;
  
  wxString str = "DELETE FROM NaRec WHERE naID = ";
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
  update.rec  = CRequest::NAREC;
  update.id   = id;

  CTT32App::NotifyChange(update);

  return true;
}


// Check auf cpName, ob WB existiert
bool  NaStore::InsertOrUpdate()
{
  if (*naName == 0)
    return false;

  Statement *stmtPtr;
  ResultSet *resPtr;

  long  id;
  
  stmtPtr = GetConnectionPtr()->CreateStatement();

  wxString  sql = "SELECT naID FROM NaRec WHERE naName = '";
  sql += naName;
  sql += "'";

  resPtr = stmtPtr->ExecuteQuery(sql);
  resPtr->BindCol(1, &id);

  bool  exist = (resPtr->Next() && !resPtr->WasNull(1));

  delete resPtr;
  delete stmtPtr;

  if (exist)
  {
    naID = id;
    return Update();
  }
  else
    return Insert();
}


// -----------------------------------------------------------------------
long  NaStore::NameToID(const wxString &name)
{
  Statement *stmtPtr;
  ResultSet *resPtr;

  long  id = 0;
  
  stmtPtr = GetConnectionPtr()->CreateStatement();

  wxString  sql = "SELECT naID FROM NaRec WHERE naName = '";
  sql += name;
  sql += "'";

  resPtr = stmtPtr->ExecuteQuery(sql);
  resPtr->BindCol(1, &id);
  resPtr->Next();

  delete resPtr;
  delete stmtPtr;

  return id;
}


long  NaStore::GetMaxNameLength()
{
  Statement *stmtPtr;
  ResultSet *resPtr;

  long  len = 0;

  stmtPtr = GetConnectionPtr()->CreateStatement();

  wxString  sql = "SELECT MAX(LEN(naName)) FROM NaRec";

  resPtr = stmtPtr->ExecuteQuery(sql);
  resPtr->BindCol(1, &len);
  resPtr->Next();

  delete resPtr;
  delete stmtPtr;

  return len;
}

long  NaStore::GetMaxDescLength()
{
  Statement *stmtPtr;
  ResultSet *resPtr;

  long  len = 0;

  stmtPtr = GetConnectionPtr()->CreateStatement();

  wxString  sql = "SELECT MAX(LEN(naDesc)) FROM NaRec";

  resPtr = stmtPtr->ExecuteQuery(sql);
  resPtr->BindCol(1, &len);
  resPtr->Next();

  delete resPtr;
  delete stmtPtr;

  return len;
}

long  NaStore::GetMaxRegionLength()
{
  Statement *stmtPtr;
  ResultSet *resPtr;

  long  len = 0;

  stmtPtr = GetConnectionPtr()->CreateStatement();

  wxString  sql = "SELECT MAX(LEN(naRegion)) FROM NaRec";

  resPtr = stmtPtr->ExecuteQuery(sql);
  resPtr->BindCol(1, &len);
  resPtr->Next();

  delete resPtr;
  delete stmtPtr;

  return len;
}


// -----------------------------------------------------------------------
// Import / Export
bool  NaStore::Import(wxTextBuffer &is)
{
  long version = 1;

  wxString line = is.GetFirstLine();

  // Check header
  if (!CheckImportHeader(line, "#NATIONS", version))
  {
    if (!infoSystem.Question(_("First comment is not %s but \"%s\". Continue anyway?"), wxT("#NATIONS"), line.wx_str()))
      return false;
  }

  if (version > 1)
  {
    infoSystem.Error(_("Version %d of import file is not supported"), version);
    return false;
  }

  Connection *connPtr = TTDbse::instance()->GetNewConnection();

  for (; !is.Eof(); line = is.GetNextLine())
  {
    CTT32App::ProgressBarStep();

    if (line.GetChar(0) == '#')
      continue;

    NaStore  na(connPtr);
    if (na.Read(line))
    {
      // Skip empty records
      if (*na.naName == '\0')
        continue;

      connPtr->StartTransaction();

      if (na.InsertOrUpdate())
        connPtr->Commit();
      else
        connPtr->Rollback();
    }
    else
      connPtr->Rollback();
  }
    
  delete connPtr;

  return true;
}


bool  NaStore::Export(wxTextBuffer &os, long version)
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  NaStore  na(connPtr);
  if (!na.SelectAll())
    return false;

  os.AddLine(wxString::Format("#NATIONS %d", version));
  os.AddLine("# Name; Description; Region");

  while (na.Next())
  {
    wxString line;
    if (na.Write(line))
      os.AddLine(line);
  }

  return true;
}


// -----------------------------------------------------------------------
wxString  NaStore::SelectString() const
{
  wxString  str = "SELECT naID, naName, naDesc, naRegion FROM NaRec ";
  return str;
}


void  NaStore::BindRec()
{
  BindCol(1, &naID);
  BindCol(2, naName, sizeof(naName));
  BindCol(3, naDesc, sizeof(naDesc));
  BindCol(4, naRegion, sizeof(naRegion));
}
