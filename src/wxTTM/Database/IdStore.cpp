/* Copyright (C) 2020 Christoph Theis */

// DB-Tabelle der internen IDs

#include  "stdafx.h"
#include  "TT32App.h"
#include  "IdStore.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "ResultSet.h"
#include  "SQLException.h"

#include  "InfoSystem.h"

#include  <string>
#include  <stdio.h>

// -----------------------------------------------------------------------
// Neue Tabelle in DB erzeugen
bool  IdStore::CreateTable(long dbVersion)
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  Statement *stmtPtr  = connPtr->CreateStatement();
  if (!stmtPtr)
    return false;

  wxString  WVARCHAR = connPtr->GetDataType(SQL_WVARCHAR);
  wxString  INTEGER  = connPtr->GetDataType(SQL_INTEGER);
  wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);
  wxString  IMAGE    = connPtr->GetDataType(SQL_LONGVARBINARY);

  wxString str = 
  // Tabelle anlegen
      "CREATE TABLE IdRec ("
	    "idLast     "+INTEGER+"  NOT NULL, "
      "idVersion  "+INTEGER+"  NOT NULL, "
      "idType     "+SMALLINT+" DEFAULT 1 NOT NULL,  "
      "idTable    "+SMALLINT+" DEFAULT 0 NOT NULL,  "
      "idTitle    "+WVARCHAR+"(64) DEFAULT '' NOT NULL, "
      "idSubTitle "+WVARCHAR+"(64) DEFAULT '' NOT NULL, "
      "idScoreExtras "+SMALLINT+" DEFAULT 0 NOT NULL, "
      "idPlayersSignature "+SMALLINT+" DEFAULT 1 NOT NULL, "
      "idScoreStartEnd " + SMALLINT + " DEFAULT 0 NOT NULL, "
      "idScoreRemarks " + SMALLINT + " DEFAULT 0 NOT NULL, "
      "idScoreCoaches " + SMALLINT + " DEFAULT 1 NOT NULL, "
      "idScoreUmpires " + SMALLINT + " DEFAULT 1 NOT NULL, "
      "idScoreUmpireName " + SMALLINT + " DEFAULT 0 NOT NULL, "
      "idPrintScoreServiceTimeout " + SMALLINT + " DEFAULT 0 NOT NULL, "
      "idLogo     "+IMAGE+", "
      "idSponsor  "+IMAGE+", "
      "idBanner   "+IMAGE+", "
      "CONSTRAINT idVersionKey PRIMARY KEY (idVersion) "

	    ")";

  try
  {
    stmtPtr->ExecuteUpdate(str);
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
    delete stmtPtr;

    return false;
  }

  // Tabelle initialisieren
  wxString sql = 
    wxString::Format("INSERT INTO IdRec (idLast, idVersion, idLogo, idSponsor) "
                     "VALUES (0, %d, NULL, NULL)", dbVersion);

  try
  {
    stmtPtr->ExecuteUpdate(sql);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(sql, e);
  };


  delete stmtPtr;
  return true;
}


bool  IdStore::UpdateTable(long version, long dbVersion)
{
  if (version == 0)
    return CreateTable(dbVersion);

  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  if (!connPtr)
    return false;

  Statement *stmtPtr = connPtr->CreateStatement();
  if (!stmtPtr)
    return false;

  if (version <= 56)
  {
    wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);
    wxString  WVARCHAR = connPtr->GetDataType(SQL_WVARCHAR);
    
    wxString str;
    
    str = "ALTER TABLE IdRec DROP COLUMN idType, idTitle, idSubTitle";
    try
    {
      stmtPtr->ExecuteUpdate(str);
    }
    catch (SQLException &)
    {
    }
    
    str = "ALTER TABLE IdRec ADD "
      "idType     "+SMALLINT+" DEFAULT 2 NOT NULL,  "
      "idTable    "+SMALLINT+" DEFAULT 1 NOT NULL,  "
      "idTitle    "+WVARCHAR+"(64), "
      "idSubTitle "+WVARCHAR+"(64)  ";      

    try 
    {      
      stmtPtr->ExecuteUpdate(str);        
    }
    catch (SQLException &)
    {
    }
  }

  if (version <= 92)
  {
    wxString  IMAGE    = connPtr->GetDataType(SQL_LONGVARBINARY);

    wxString str;
    
    str = "ALTER TABLE IdRec DROP COLUMN idLogo, idSponsor";
    try
    {
      stmtPtr->ExecuteUpdate(str);
    }
    catch (SQLException &)
    {
    }
    
    str = "ALTER TABLE IdRec ADD "
      "idLogo     "+IMAGE+", "
      "idSponsor  "+IMAGE+"  ";

    try 
    {      
      stmtPtr->ExecuteUpdate(str);        
    }
    catch (SQLException &)
    {
    }
  }

  if (version <= 101)
  {
    wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);
    wxString str = "ALTER TABLE IdRec ADD idScoreExtras "+SMALLINT+" DEFAULT 0 NOT NULL";

    try 
    {      
      stmtPtr->ExecuteUpdate(str);        
    }
    catch (SQLException &)
    {
    }
  }

  if (version <= 121)
  {
    wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);
    wxString str = "ALTER TABLE IdRec ADD idPlayersSignature "+SMALLINT+" DEFAULT 1 NOT NULL";

    try 
    {      
      stmtPtr->ExecuteUpdate(str);        
    }
    catch (SQLException &)
    {
    }
  }

  if (version <= 128)
  {
    wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);
    wxString str = "ALTER TABLE IdRec ADD "
        "idScoreRemarks " + SMALLINT + " DEFAULT 0 NOT NULL, "
        "idScoreCoaches " + SMALLINT + " DEFAULT 1 NOT NULL, "
        "idScoreUmpires " + SMALLINT + " DEFAULT 1 NOT NULL, "
        "idPrintScoreServiceTimeout " + SMALLINT + " DEFAULT 0 NOT NULL"
    ;

    try
    {
      stmtPtr->ExecuteUpdate(str);
    }
    catch (SQLException &)
    {
    }
  }

  if (version <= 131)
  {
    wxString  IMAGE = connPtr->GetDataType(SQL_LONGVARBINARY);

    wxString str;

    str = "ALTER TABLE IdRec ADD idBanner  " + IMAGE + "  ";

    try
    {
      stmtPtr->ExecuteUpdate(str);
    }
    catch (SQLException &)
    {
    }
  }

  if (version <= 158)
  {
    wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);

    wxString str;

    str = "ALTER TABLE IdRec ADD idScoreUmpireName " + SMALLINT + " DEFAULT 0 NOT NULL ";

    try
    {
      stmtPtr->ExecuteUpdate(str);
    }
    catch (SQLException &)
    {
    }
  }

  if (version <= 159)
  {
    wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);
    wxString str = "ALTER TABLE IdRec ADD "
        "idScoreStartEnd " + SMALLINT + " DEFAULT 0 NOT NULL "
    ;

    try
    {
      stmtPtr->ExecuteUpdate(str);
    }
    catch (SQLException &)
    {
    }
  }

  char  strUpdate[64];
  sprintf(strUpdate, "UPDATE IdRec SET idVersion = %d", dbVersion);
  stmtPtr->ExecuteUpdate(strUpdate);
  
  delete stmtPtr;

  return true;
}


// -----------------------------------------------------------------------
// Neue id erzeugen
long  IdStore::ID(Connection *connPtr)
{
  wxASSERT(connPtr);

  long id;
  Statement *stmtPtr   = connPtr->CreateStatement();
  stmtPtr->ExecuteUpdate("UPDATE IdRec SET idLast = idLast+1");
  
  ResultSet *resultPtr = stmtPtr->ExecuteQuery("SELECT idLast FROM IdRec");
  resultPtr->BindCol(1, &id);
  resultPtr->Next();

  delete resultPtr;
  delete stmtPtr;

  return id;
}


// -----------------------------------------------------------------------
// Versionsabfrage
long  IdStore::IdVersion()
{
  long version = 0;
  Connection *connPtr  = TTDbse::instance()->GetDefaultConnection();
  Statement *stmtPtr   = connPtr->CreateStatement();
  ResultSet *resultPtr = 0;

  try
  {
    resultPtr = stmtPtr->ExecuteQuery("SELECT idVersion FROM IdRec");
    resultPtr->BindCol(1, &version);
    if (!resultPtr->Next() || resultPtr->WasNull(1))
      version = 0;
  }
  catch(SQLException &)
  {
    version = 0;
  }

  delete stmtPtr;
  delete resultPtr;

  return version;
}


void IdStore::SetType(short type)
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  Statement  *stmtPtr = connPtr->CreateStatement();
  wxString str = "UPDATE IdRec SET idType = " + ltostr(type);
  
  try
  {
    stmtPtr->ExecuteUpdate(str);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
  }  
}


short IdStore::GetType()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  Statement  *stmtPtr = connPtr->CreateStatement();
  ResultSet  *resultPtr = NULL;
  
  short type;
  
  try
  {
    resultPtr = stmtPtr->ExecuteQuery("SELECT idType FROM IdRec");
    resultPtr->BindCol(1, &type);
    if (!resultPtr->Next() || resultPtr->WasNull(1))
      type = 1;
      
    stmtPtr->Close();
  }
  catch(SQLException &)
  {
    type = 1;
  }
  
  delete resultPtr;
  delete stmtPtr;
  
  return type;
}


void IdStore::SetTable(short table)
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  Statement  *stmtPtr = connPtr->CreateStatement();
  wxString str = "UPDATE IdRec SET idTable = " + ltostr(table);
  
  try
  {
    stmtPtr->ExecuteUpdate(str);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
  }  
}


short IdStore::GetTable()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  Statement  *stmtPtr = connPtr->CreateStatement();
  ResultSet  *resultPtr = NULL;
  
  short table;
  
  try
  {
    resultPtr = stmtPtr->ExecuteQuery("SELECT idTable FROM IdRec");
    resultPtr->BindCol(1, &table);
    if (!resultPtr->Next() || resultPtr->WasNull(1))
      table = 0;
      
    stmtPtr->Close();
  }
  catch(SQLException &)
  {
    table = 0;
  }
  
  delete resultPtr;
  delete stmtPtr;
  
  return table;
}


void IdStore::SetReportTitle(const wxString &title)
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  Statement  *stmtPtr = connPtr->CreateStatement();
  
  wxString str = "UPDATE IdRec SET idTitle = '" + 
                    TransformString(title) + "'";
  
  try
  {
    stmtPtr->ExecuteUpdate(str);
  }
  catch (SQLException &)
  {
  }
  
  delete stmtPtr;
}


wxString IdStore::GetReportTitle()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  Statement  *stmtPtr = connPtr->CreateStatement();
  ResultSet  *resultPtr = NULL;
  
  wxChar title[64];
  
  try
  {
    resultPtr = stmtPtr->ExecuteQuery("SELECT idTitle FROM IdRec");
    resultPtr->BindCol(1, title, sizeof(title));
    if (!resultPtr->Next() || resultPtr->WasNull(1))
      *title = 0;
  }
  catch(SQLException &)
  {
    *title = 0;
  }
  
  delete resultPtr;
  delete stmtPtr;
  
  return wxString(title);
}


void IdStore::SetReportSubtitle(const wxString &subtitle)
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  Statement  *stmtPtr = connPtr->CreateStatement();
  
  wxString str = "UPDATE IdRec SET idSubTitle = '" + 
                    TransformString(subtitle) + "'";
  
  try
  {
    stmtPtr->ExecuteUpdate(str);
  }
  catch (SQLException &)
  {
  }
  
  delete stmtPtr;
}


wxString IdStore::GetReportSubtitle()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  Statement  *stmtPtr = connPtr->CreateStatement();
  ResultSet  *resultPtr = NULL;
  
  wxChar subTitle[64];
  
  try
  {
    resultPtr = stmtPtr->ExecuteQuery("SELECT idSubTitle FROM IdRec");
    resultPtr->BindCol(1, subTitle, sizeof(subTitle));
    if (!resultPtr->Next() || resultPtr->WasNull(1))
      *subTitle = 0;
  }
  catch(SQLException &)
  {
    *subTitle = 0;
  }
  
  delete resultPtr;
  delete stmtPtr;
  
  return wxString(subTitle);
}


void IdStore::SetBannerImage(const wxString &file)
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  Statement *stmtPtr = NULL;

  // Max 8000 Byte, sonst muss man ein XML format file angeben
  const wxString str = file.IsEmpty() ?
    wxString("UPDATE IdRec SET idBanner = NULL") :
    wxString("UPDATE IdRec SET idBanner = (SELECT * FROM OPENROWSET(BULK N'" + file + "', SINGLE_BLOB) as idBanner)");

  try
  {
    stmtPtr = connPtr->CreateStatement();

    stmtPtr->ExecuteUpdate(str);
  }
  catch (SQLException &)
  {
  }

  delete stmtPtr;
}


bool IdStore::GetBannerImage(wxImage &img)
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  Statement  *stmtPtr = connPtr->CreateStatement();
  ResultSet  *resultPtr = NULL;

  void *data = NULL;
  size_t len = 0;

  try
  {
    resultPtr = stmtPtr->ExecuteQuery("SELECT idBanner FROM IdRec");
    if (!resultPtr->Next() || !resultPtr->GetData(1, data, len) || resultPtr->WasNull())
      data = NULL;
  }
  catch (SQLException &)
  {
    data = NULL;
  }

  delete resultPtr;
  delete stmtPtr;

  if (data == NULL)
    return false;

  wxMemoryInputStream is((const char *)data, len);
  img.LoadFile(is, wxBITMAP_TYPE_ANY);

  delete[](char *) data;

  return true;
}


void IdStore::SetLogoImage(const wxString &file)
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  Statement *stmtPtr = NULL; 

  // Max 8000 Byte, sonst muss man ein XML format file angeben
  const wxString str = file.IsEmpty() ?
    wxString("UPDATE IdRec SET idLogo = NULL") :
    wxString("UPDATE IdRec SET idLogo = (SELECT * FROM OPENROWSET(BULK N'" + file + "', SINGLE_BLOB) as idLogo)"); 

  try
  {
    stmtPtr = connPtr->CreateStatement();
    
    stmtPtr->ExecuteUpdate(str);        
  }
  catch (SQLException &)
  {
  }
  
  delete stmtPtr;
}


bool IdStore::GetLogoImage(wxImage &img)
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  Statement  *stmtPtr = connPtr->CreateStatement();
  ResultSet  *resultPtr = NULL;
  
  void *data = NULL;
  size_t len = 0;
  
  try
  {
    resultPtr = stmtPtr->ExecuteQuery("SELECT idLogo FROM IdRec");
    if (!resultPtr->Next() || !resultPtr->GetData(1, data, len) || resultPtr->WasNull())
      data = NULL;      
  }
  catch(SQLException &)
  {
    data = NULL;
  }

  delete resultPtr;
  delete stmtPtr;

  if (data == NULL)
    return false;

  wxMemoryInputStream is( (const char *) data, len);
  img.LoadFile(is, wxBITMAP_TYPE_ANY);

  delete[] (char *) data;

  return true;
}


void IdStore::SetSponsorImage(const wxString &file)
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  Statement *stmtPtr = NULL; 

  // Max 8000 Byte, sonst muss man ein XML format file angeben
  const wxString str = file.IsEmpty() ?  
    wxString("UPDATE IdRec SET idSponsor = NULL") :
    wxString("UPDATE IdRec SET idSponsor = (SELECT * FROM OPENROWSET(BULK N'" + file + "', SINGLE_BLOB) as idSponsor)"); 

  try
  {
    stmtPtr = connPtr->CreateStatement();

    stmtPtr->ExecuteUpdate(str);        
  }
  catch (SQLException &)
  {
  }
  
  delete stmtPtr;
}


bool IdStore::GetSponsorImage(wxImage &img)
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  Statement  *stmtPtr = connPtr->CreateStatement();
  ResultSet  *resultPtr = NULL;
  
  void *data = NULL;
  size_t len = 0;
  
  try
  {
    resultPtr = stmtPtr->ExecuteQuery("SELECT idSponsor FROM IdRec");
    if (!resultPtr->Next() || !resultPtr->GetData(1, data, len) || resultPtr->WasNull())
      data = NULL;      
  }
  catch(SQLException &)
  {
    data = NULL;
  }

  delete resultPtr;
  delete stmtPtr;

  if (data == NULL)
    return false;

  wxMemoryInputStream is( (const char *) data, len);
  img.LoadFile(is, wxBITMAP_TYPE_ANY);

  delete[] (char *) data;

  return true;
}


void IdStore::SetPrintPlayersSignature(bool f)
{
  SetFlag("idPlayersSignature", f);
}


bool IdStore::GetPrintPlayersSignature()
{
  return GetFlag("idPlayersSignature");
}


void IdStore::SetPrintScoreCoaches(bool f)
{
  SetFlag("idScoreCoaches", f);
}


bool IdStore::GetPrintScoreCoaches()
{
  return GetFlag("idScoreCoaches");
}


void IdStore::SetPrintScoreUmpires(bool f)
{
  SetFlag("idScoreUmpires", f);
}


bool IdStore::GetPrintScoreUmpires()
{
  return GetFlag("idScoreUmpires");
}


void IdStore::SetPrintScoreUmpireName(bool f)
{
  SetFlag("idScoreUmpireName", f);
}


bool IdStore::GetPrintScoreUmpireName()
{
  return GetFlag("idScoreUmpireName");
}


void IdStore::SetPrintScoreExtras(bool f)
{
  return SetFlag("idScoreExtras", f);
}


bool IdStore::GetPrintScoreExtras()
{
  return GetFlag("idScoreExtras");
}


void IdStore::SetPrintScoreStartEnd(bool f)
{
  SetFlag("idScoreStartEnd", f);
}


bool IdStore::GetPrintScoreStartEnd()
{
  return GetFlag("idScoreStartEnd");
}


void IdStore::SetPrintScoreRemarks(bool f)
{
  SetFlag("idScoreRemarks", f);
}


bool IdStore::GetPrintScoreRemarks()
{
  return GetFlag("idScoreRemarks");
}


void IdStore::SetPrintScoreServiceTimeout(bool f)
{
  SetFlag("idPrintScoreServiceTimeout", f);
}


bool IdStore::GetPrintScoreServiceTimeout()
{
  return GetFlag("idPrintScoreServiceTimeout");
}


void IdStore::SetFlag(const wxString &name, bool sig)
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  Statement  *stmtPtr = connPtr->CreateStatement();
  wxString str = "UPDATE IdRec SET " + name + " = " + ltostr(sig ? 1 : 0);

  try
  {
    stmtPtr->ExecuteUpdate(str);
  }
  catch (SQLException &)
  {
  }
}


bool IdStore::GetFlag(const wxString &name)
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  Statement  *stmtPtr = connPtr->CreateStatement();
  ResultSet  *resultPtr = NULL;

  short value;

  try
  {
    resultPtr = stmtPtr->ExecuteQuery("SELECT " + name + " FROM IdRec");
    resultPtr->BindCol(1, &value);
    if (!resultPtr->Next() || resultPtr->WasNull(1))
      value = 0;

    stmtPtr->Close();
  }
  catch (SQLException &)
  {
    value = 0;
  }

  delete resultPtr;
  delete stmtPtr;

  return value ? true : false;
}


// -----------------------------------------------------------------------
// Konstruktor
IdStore::IdStore()
{
}



void IdStore::Init()
{
}