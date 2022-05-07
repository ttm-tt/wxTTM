/* Copyright (C) 2020 Christoph Theis */

// DB-Tabelle der Spielmodi

#include  "stdafx.h"
#include  "MdStore.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "ResultSet.h"
#include  "SQLException.h"
#include  "InfoSystem.h"
#include  "Request.h"

#include  "IdStore.h"

#include  <stdio.h>
#include  <fstream>
#include  <stdlib.h>

#define  CP_SINGLE  1
#define  CP_DOUBLE  2


// Abspeichern eines einzelnen Spieles
struct  MdMatchRec
{
  long   mdID;      // Foreign key
  short  mdRound;   // Laufende Nr
  short  mdMatch;   // Single / Double
  long   mdPlayerA; // Verweis auf Setzung A
  long   mdPlayerX; // Verweis auf Setzung X  

  MdMatchRec()  {Init();}
  
  void  Init()  {memset(this, 0, sizeof(MdMatchRec));}
};


class  MdMatchStore : public StoreObj, public MdMatchRec
{
  public:
    static  bool  CreateTable();
    static  bool  UpdateTable(long version);

    static  bool  CreateConstraints();
    static  bool  UpdateConstraints(long version);

    MdMatchStore(Connection *connPtr = 0) : StoreObj(connPtr) {}

    virtual void Init();

    bool  Select(long id);
    bool  Insert(MdRec *);
    bool  Remove(long id);
};


void  MdMatchStore::Init()
{
  MdMatchRec::Init();
}


bool  MdMatchStore::CreateTable()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  wxASSERT(connPtr);

  Statement *tmp = connPtr->CreateStatement();

  wxString  INTEGER  = connPtr->GetDataType(SQL_INTEGER);
  wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);

  wxString  sql = 
    "CREATE TABLE MdMatch (         "
    "mdID        "+INTEGER+"      NOT NULL,  "
    "mdRound     "+SMALLINT+"     NOT NULL,  "
	  "mdMatch     "+SMALLINT+"     NOT NULL,  "
	  "mdPlayerA   "+INTEGER+"      NOT NULL,  "
	  "mdPlayerX   "+INTEGER+"      NOT NULL,  "
    "CONSTRAINT mdMatchKey PRIMARY KEY (mdID, mdRound, mdMatch) "

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
    tmp->ExecuteUpdate(sql = "CREATE UNIQUE INDEX mdMatchIdKey ON MdMatch (mdID, mdRound, mdMatch)");
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(sql, e);
  };

  tmp->Close();
  delete tmp;
  
  return true;
}


bool  MdMatchStore::UpdateTable(long version)
{
  if (version == 0)
    return CreateTable();
  return true;
}


bool  MdMatchStore::CreateConstraints()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str;
  
  try
  {
    tmp->ExecuteUpdate("ALTER TABLE MdMatch DROP CONSTRAINT mdMatchIdRef");
  }
  catch (SQLException &)
  {
  }

  try
  {
    tmp->ExecuteUpdate(str = 
      "ALTER TABLE MdMatch ADD CONSTRAINT mdMatchIdRef "
      "FOREIGN KEY (mdID) REFERENCES MdRec (mdID) ON DELETE CASCADE");
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
  };

  delete tmp;
  return true;
}


bool  MdMatchStore::UpdateConstraints(long version)
{
  if (version <= 50)
    return CreateConstraints();
    
  return true;
}



// -----------------------------------------------------------------------
bool  MdMatchStore::Select(long id)
{
  wxString  str = 
      "SELECT mdID, mdRound, mdMatch, mdPlayerA, mdPlayerX "
      "FROM   mdMatch "
      "WHERE  mdID = ";
  str += ltostr(id);
  str += "";

  if (!ExecuteQuery(str))
    return false;

  BindCol(1, &mdID);
  BindCol(2, &mdRound);
  BindCol(3, &mdMatch);
  BindCol(4, &mdPlayerA);
  BindCol(5, &mdPlayerX);

  return true;
}


bool  MdMatchStore::Remove(long id)
{
  wxString  str = 
      "DELETE FROM mdMatch WHERE mdID = ";
  str += ltostr(id);
  str += "";

  return ExecuteUpdate(str);
}


bool  MdMatchStore::Insert(MdRec *mdPtr)
{
  wxASSERT(mdPtr);
  wxASSERT(mdPtr->mdList);

  wxString str = "INSERT INTO MdMatch "
                    "(mdID, mdRound, mdMatch, mdPlayerA, mdPlayerX) "
                    "VALUES(?, ?, ?, ?, ?)";

  PreparedStatement *stmtPtr = GetConnectionPtr()->PrepareStatement(str);
  wxASSERT(stmtPtr);

  mdID = mdPtr->mdID;
  for (int rd = 0; rd < mdPtr->Rounds(); rd++)
  {
    for (int mt = 0; mt < mdPtr->Matches(); mt++)
    {
      mdRound = rd+1;
      mdMatch = mt+1;

      mdPlayerA = mdPtr->GetPlayerA(rd+1, mt+1);
      mdPlayerX = mdPtr->GetPlayerX(rd+1, mt+1);

      stmtPtr->SetData(1, &mdID);
      stmtPtr->SetData(2, &mdRound);
      stmtPtr->SetData(3, &mdMatch);
      stmtPtr->SetData(4, &mdPlayerA);
      stmtPtr->SetData(5, &mdPlayerX);

      stmtPtr->Execute();
    }
  }

  delete stmtPtr;
  return true;
}


// -----------------------------------------------------------------------
// Neue Tabelle in DB erzeugen
bool  MdStore::CreateTable()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  wxASSERT(connPtr);

  Statement *tmp = connPtr->CreateStatement();

  wxString  WVARCHAR = connPtr->GetDataType(SQL_WVARCHAR);
  wxString  INTEGER  = connPtr->GetDataType(SQL_INTEGER);
  wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);

  wxString  sql = 
    "CREATE TABLE MdRec (         "
    "mdID        "+INTEGER+"      NOT NULL,  "
    "mdName      "+WVARCHAR+"(8)  NOT NULL,  "
	  "mdDesc      "+WVARCHAR+"(64),           "
	  "mdSize      "+SMALLINT+"     NOT NULL,  "
    "mdMtPtsWin  "+SMALLINT+"     NOT NULL DEFAULT 2, "
    "mdMtPtsTie  "+SMALLINT+"     NOT NULL DEFAULT 1, "
    "mdMtPtsLoss "+SMALLINT+"     NOT NULL DEFAULT 1, "
    "mpID        "+INTEGER+"      DEFAULT NULL, "
    "CONSTRAINT mdIdKey PRIMARY KEY (mdID)   "
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
    tmp->ExecuteUpdate(sql = "CREATE UNIQUE INDEX mdNameKey ON MdRec (mdName)");
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(sql, e);
  };

  MdMatchStore::CreateTable();

  tmp->Close();
  delete tmp;
  
  return true;
}


bool  MdStore::UpdateTable(long version)
{
  if (version == 0)
    return CreateTable();

  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  Statement *stmtPtr = nullptr;

  wxString  INTEGER  = connPtr->GetDataType(SQL_INTEGER);
  wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);
  wxString  WVARCHAR = connPtr->GetDataType(SQL_WVARCHAR);

  wxString str;

  try
  {
    stmtPtr = connPtr->CreateStatement();

    if (version < 156)
    {
      str = "ALTER TABLE MdRec ADD "
          "mdMtPtsWin  "+SMALLINT+"     NOT NULL DEFAULT 2, "
          "mdMtPtsTie  "+SMALLINT+"     NOT NULL DEFAULT 1, "
          "mdMtPtsLoss "+SMALLINT+"     NOT NULL DEFAULT 1  "
      ;

      stmtPtr->ExecuteUpdate(str);
    }

    if (version < 164)
    {
      str = "ALTER TABLE MdRec ADD "
          " mpID "+INTEGER+" DEFAULT NULL"
      ;

      stmtPtr->ExecuteUpdate(str);
    }
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    delete stmtPtr;
    return false;
  }

  delete stmtPtr;

  return true;
}


bool  MdStore::CreateConstraints()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str;

  try
  {
    tmp->ExecuteQuery("ALTER TABLE GrRec DROP CONSTRAINT mdMpRef");
  }
  catch (SQLException &)
  {
  }

  try
  {
    tmp->ExecuteUpdate(str = 
      "ALTER TABLE MdRec ADD CONSTRAINT mdMpRef "
      "FOREIGN KEY (mpID) REFERENCES MpRec (mpID) ON DELETE NO ACTION");
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
  };

  delete tmp;

  return MdMatchStore::CreateConstraints();
}


bool  MdStore::UpdateConstraints(long version)
{
  if (version < 164)
    return MdStore::CreateConstraints();

  return true;
}


// -----------------------------------------------------------------------
// Konstruktor
MdStore::MdStore(Connection *connPtr)
       : StoreObj(connPtr)
{
}


MdStore::~MdStore()
{
}


// -----------------------------------------------------------------------
// Select
bool  MdStore::SelectAll()
{
  wxString str = "SELECT mdID, mdName, mdDesc, mdSize, mdMtPtsWin, mdMtPtsTie, mdMtPtsLoss, mpID "
                    "FROM MdRec";

  try
  {
    if (!ExecuteQuery(str))
      return false;

    BindCol(1, &mdID);
    BindCol(2, mdName, sizeof(mdName));
    BindCol(3, mdDesc, sizeof(mdDesc));
    BindCol(4, &mdSize);
    BindCol(5, &mdMtPtsWin);
    BindCol(6, &mdMtPtsTie);
    BindCol(7, &mdMtPtsLoss);
    BindCol(8, &mpID);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  return true;
}


bool  MdStore::SelectById(long id)
{
  wxString str = "SELECT mdID, mdName, mdDesc, mdSize, mdMtPtsWin, mdMtPtsTie, mdMtPtsLoss, mpID "
                    "FROM MdRec "
                    "WHERE mdID = ";
  str += ltostr(id);
  str += "";

  try
  {
    if (!ExecuteQuery(str))
      return false;

    BindCol(1, &mdID);
    BindCol(2, mdName, sizeof(mdName));
    BindCol(3, mdDesc, sizeof(mdDesc));
    BindCol(4, &mdSize);
    BindCol(5, &mdMtPtsWin);
    BindCol(6, &mdMtPtsTie);
    BindCol(7, &mdMtPtsLoss);
    BindCol(8, &mpID);

    return true;
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


bool  MdStore::SelectByName(const wxString &name)
{
  wxString str = 
    "SELECT mdID, mdName, mdDesc, mdSize, mdMtPtsWin, mdMtPtsTie, mdMtPtsLoss, mpID "
    "  FROM MdRec "
    " WHERE mdName = '";
  str += name;
  str += "'";

  try
  {
    if (!ExecuteQuery(str))
      return false;

    BindCol(1, &mdID);
    BindCol(2, mdName, sizeof(mdName));
    BindCol(3, mdDesc, sizeof(mdDesc));
    BindCol(4, &mdSize);
    BindCol(5, &mdMtPtsWin);
    BindCol(6, &mdMtPtsTie);
    BindCol(7, &mdMtPtsLoss);
    BindCol(8, &mpID);

    return true;
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


bool  MdStore::Insert()
{
  PreparedStatement *stmtPtr = 0;

  wxString str = "INSERT INTO MdRec "
                    "(mdID, mdName, mdDesc, mdSize, mdMtPtsWin, mdMtPtsTie, mdMtPtsLoss, mpID) "
                    "VALUES(?, ?, ?, ?, ?, ?, ?, ?)";

  try
  {
    mdID = IdStore::ID(GetConnectionPtr());

    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    stmtPtr->SetData(1, &mdID);
    stmtPtr->SetData(2, mdName);
    stmtPtr->SetData(3, mdDesc);
    stmtPtr->SetData(4, &mdSize);
    stmtPtr->SetData(5, &mdMtPtsWin);
    stmtPtr->SetData(6, &mdMtPtsTie);
    stmtPtr->SetData(7, &mdMtPtsLoss);
    stmtPtr->SetData(8, mpID ? &mpID : nullptr);

    stmtPtr->Execute();

    MdMatchStore(GetConnectionPtr()).Insert(this);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  delete stmtPtr;

  // Notify Views
  CRequest update;
  update.type = CRequest::INSERT;
  update.rec  = CRequest::MDREC;
  update.id   = mdID;

  CTT32App::NotifyChange(update);

  return true;
}


bool  MdStore::Update()
{
  PreparedStatement *stmtPtr = 0;

  wxString str = "UPDATE MdRec "
                    "SET mdName = ?, mdDesc = ?, mdMtPtsWin = ?, mdMtPtsTie = ?, mdMtPtsLoss = ?, mpID = ? "
                    "WHERE mdID = ?";

  try
  {
    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    stmtPtr->SetData(1, mdName);
    stmtPtr->SetData(2, mdDesc);
    stmtPtr->SetData(3, &mdMtPtsWin);
    stmtPtr->SetData(4, &mdMtPtsTie);
    stmtPtr->SetData(5, &mdMtPtsLoss);
    stmtPtr->SetData(6, mpID ? &mpID : nullptr);
    stmtPtr->SetData(7, &mdID);

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
  update.rec  = CRequest::MDREC;
  update.id   = mdID;

  CTT32App::NotifyChange(update);

  return true;
}


bool  MdStore::Remove(long id)
{
  // ID-Check
  if (!id)
    id = mdID;

  if (!id)
    return true;

  wxString str = "DELETE FROM MdRec WHERE mdID = ";
  str += ltostr(id);
  str += "";

  try
  {
    // MdMatchStore(GetConnectionPtr()).Remove(id);
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
  update.rec  = CRequest::MDREC;
  update.id   = id;

  CTT32App::NotifyChange(update);

  return true;
}


// Check auf mdName, ob Modus existiert
bool  MdStore::InsertOrUpdate()
{
  Statement *stmtPtr;
  ResultSet *resPtr;

  long  id;
  
  stmtPtr = GetConnectionPtr()->CreateStatement();

  wxString  sql = "SELECT mdID FROM MdRec WHERE mdName = '";
  sql += mdName;
  sql += "'";

  resPtr = stmtPtr->ExecuteQuery(sql);
  resPtr->BindCol(1, &id);
  bool  exist = (resPtr->Next() && !resPtr->WasNull(1));

  delete resPtr;
  delete stmtPtr;

  if (exist)
  {
    mdID = id;
    return Update();
  }
  else
    return Insert();
}


bool  MdStore::Next()
{
  if (!StoreObj::Next())
    return false;

  // Spiele auslesen
  ChangeSize(mdSize);

  Connection *connPtr = TTDbse::instance()->GetNewConnection();
  try
  {
    MdMatchStore  mdMatch(connPtr);
    mdMatch.Select(mdID);
    while (mdMatch.Next())
    {
      SetPlayerA(mdMatch.mdRound, mdMatch.mdMatch, mdMatch.mdPlayerA);
      SetPlayerX(mdMatch.mdRound, mdMatch.mdMatch, mdMatch.mdPlayerX);
    }
  }
  catch (SQLException &e)
  {
    infoSystem.Exception("SELECT ... FROM MdMatch WHERE ...", e);
    delete connPtr;
    return false;
  }

  delete connPtr;

  return true;
}


void  MdStore::Init()
{
  MdRec::Init();
}