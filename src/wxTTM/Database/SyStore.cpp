/* Copyright (C) 2020 Christoph Theis */

// DB-Tabelle der Spielsysteme

#include  "stdafx.h"
#include  "SyStore.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "ResultSet.h"
#include  "SQLException.h"
#include  "InfoSystem.h"
#include  "Request.h"

#include  "IdStore.h"

#define  CP_SINGLE  1
#define  CP_DOUBLE  2


// -----------------------------------------------------------------------
SyRec& SyRec::operator=(const SyRec& sy)
{
  syID = sy.syID;
  wxStrcpy(syName, sy.syName);
  wxStrcpy(syDesc, sy.syDesc);
  sySingles = sy.sySingles;
  syDoubles = sy.syDoubles;
  syMatches = sy.syMatches;
  syComplete = sy.syComplete;
  syList = nullptr;

  if (sy.syList)
  {
    syList = new SyList[sy.syMatches];
    for (int idx = 0; idx < sy.syMatches; ++idx)
      syList[idx] = sy.syList[idx];
  }

  return *this;
}

void SyRec::ChangeSize(int mt)
{
  if (mt == 0)
  {
    delete[] syList;
    syList = 0;
    
    syMatches = mt;
    
    return;
  }
  
  SyList *tmp = new SyList[mt];
  if (mt < syMatches)  
    memcpy(tmp, syList, mt * sizeof(SyList));
  else
  {
    if (syList)
      memcpy(tmp, syList, syMatches * sizeof(SyList));
      
    for (int i = syMatches; i < mt; i++)
    {
      tmp[i].syType = CP_SINGLE;
      tmp[i].syPlayerA = tmp[i].syPlayerX = 0;
    }
  }
  
  delete[] syList;
  syList = tmp;
  
  syMatches = mt;
}


void  SyMatchStore::Init()
{
  SyMatchRec::Init();
}


bool  SyMatchStore::CreateTable()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  wxASSERT(connPtr);

  Statement *tmp = connPtr->CreateStatement();

  wxString  INTEGER  = connPtr->GetDataType(SQL_INTEGER);
  wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);

  wxString  sql = 
    "CREATE TABLE SyMatch (         "
    "syID        "+INTEGER+"      NOT NULL,  "
    "syNr        "+SMALLINT+"     NOT NULL,  "
	  "syType      "+SMALLINT+"     NOT NULL,  "
	  "syPlayerA   "+INTEGER+"      NOT NULL,  "
	  "syPlayerX   "+INTEGER+"      NOT NULL,  "
    "CONSTRAINT syMatchKey PRIMARY KEY (syID, syNr),  "    
    "CONSTRAINT syMatchIdRef FOREIGN KEY (syID) REFERENCES SyRec (syID) "
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
    tmp->ExecuteUpdate(sql = "CREATE UNIQUE INDEX syMatchIdKey ON SyMatch (syID, syNr)");
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(sql, e);
  };

  tmp->Close();
  delete tmp;
  
  return true;
}


bool  SyMatchStore::UpdateTable(long version)
{
  if (version == 0)
    return CreateTable();
  return true;
}


bool  SyMatchStore::CreateConstraints()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str;
  
  try
  {
    tmp->ExecuteUpdate("ALTER TABLE SyMatch DROP CONSTRAINT syMatchIdRef");
  }
  catch (SQLException &)
  {
  }

  try
  {
    tmp->ExecuteUpdate(str = 
      "ALTER TABLE SyMatch ADD CONSTRAINT syMatchIdRef "
      "FOREIGN KEY (syID) REFERENCES SyRec (syID) ON DELETE CASCADE");
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
  };

  delete tmp;
  return true;
}


bool  SyMatchStore::UpdateConstraints(long version)
{
  if (version <= 50)
    return CreateConstraints();
    
  return true;
}


bool  SyMatchStore::Select(long id)
{
  wxString  str = 
      "SELECT syID, syNr, syType, syPlayerA, syPlayerX "
      "FROM   syMatch "
      "WHERE  syID = ";
  str += ltostr(id);
  str += "";

  if (!ExecuteQuery(str))
    return false;

  BindCol(1, &syID);
  BindCol(2, &syNr);
  BindCol(3, &syType);
  BindCol(4, &syPlayerA);
  BindCol(5, &syPlayerX);

  return true;
}


bool  SyMatchStore::Remove(long id)
{
  wxString  str = 
      "DELETE FROM syMatch WHERE syID = ";
  str += ltostr(id);
  str += "";

  return ExecuteUpdate(str);
}


bool  SyMatchStore::Insert(SyRec *syPtr)
{
  wxASSERT(syPtr);
  wxASSERT(syPtr->syList);

  wxString str = "INSERT INTO SyMatch "
                 "(syID, syNr, syType, syPlayerA, syPlayerX) "
                 "VALUES(?, ?, ?, ?, ?)";

  PreparedStatement *stmtPtr = GetConnectionPtr()->PrepareStatement(str);
  wxASSERT(stmtPtr);

  syID = syPtr->syID;
  for (int nr = 0; nr < syPtr->syMatches; nr++)
  {
    syNr = nr+1;
    syType    = syPtr->syList[nr].syType;
    syPlayerA = syPtr->syList[nr].syPlayerA;
    syPlayerX = syPtr->syList[nr].syPlayerX;

    stmtPtr->SetData(1, &syID);
    stmtPtr->SetData(2, &syNr);
    stmtPtr->SetData(3, &syType);
    stmtPtr->SetData(4, &syPlayerA);
    stmtPtr->SetData(5, &syPlayerX);

    stmtPtr->Execute();
  }

  delete stmtPtr;
  return true;
}


// -----------------------------------------------------------------------
// Neue Tabelle in DB erzeugen
bool  SyStore::CreateTable()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  wxASSERT(connPtr);

  Statement *tmp = connPtr->CreateStatement();

  wxString  WVARCHAR = connPtr->GetDataType(SQL_WVARCHAR);
  wxString  INTEGER  = connPtr->GetDataType(SQL_INTEGER);
  wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);

  wxString  sql = 
    "CREATE TABLE SyRec (         "
    "syID        "+INTEGER+"      NOT NULL,  "
    "syName      "+WVARCHAR+"(8)  NOT NULL,  "
	  "syDesc      "+WVARCHAR+"(64),           "
	  "sySingles   "+SMALLINT+"     NOT NULL,  "
	  "syDoubles   "+SMALLINT+"     NOT NULL,  "
	  "syMatches   "+SMALLINT+"     NOT NULL,  "
    "syComplete  "+SMALLINT+",    "
    "CONSTRAINT syIdKey PRIMARY KEY (syID)   "
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
    tmp->ExecuteUpdate(sql = "CREATE UNIQUE INDEX syNameKey ON SyRec (syName)");
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(sql, e);
  };

  SyMatchStore::CreateTable();

  tmp->Close();
  delete tmp;
  
  // Defaultspielsysteme
  connPtr->StartTransaction();

  AddCorbillonCup();
  AddModifiedCorbillonCup();
  AddModifiedSwaythlingCup();
  AddOlympicTeamSystem();
  // AddOlympicTeamSystem2Players();
  AddECTeamSystem();
  AddMixedTeamSystem();
  AddMixedTeamSystemA();

  connPtr->Commit();

  return true;
}


bool  SyStore::UpdateTable(long version)
{
  if (version == 0)
    return CreateTable();

  return true;
}


bool  SyStore::CreateConstraints()
{
  return SyMatchStore::CreateConstraints();
}


bool  SyStore::UpdateConstraints(long version)
{
  return SyMatchStore::UpdateConstraints(version);
}


// -----------------------------------------------------------------------
// Konstruktor
SyStore::SyStore(Connection *connPtr)
       : StoreObj(connPtr)
{
}


SyStore::~SyStore()
{
}


void  SyStore::Init()
{
}


// -----------------------------------------------------------------------
// Select
bool  SyStore::SelectAll()
{
  wxString str = 
    "SELECT syID, syName, syDesc, syComplete, "
    "       sySingles, syDoubles, syMatches "
    "  FROM SyRec";

  try
  {
    if (!ExecuteQuery(str))
      return false;

    BindCol(1, &syID);
    BindCol(2, syName, sizeof(syName));
    BindCol(3, syDesc, sizeof(syDesc));
    BindCol(4, &syComplete);
    BindCol(5, &sySingles);
    BindCol(6, &syDoubles);
    BindCol(7, &syMatches);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  return true;
}


bool  SyStore::SelectById(long id)
{
  wxString str = 
    "SELECT syID, syName, syDesc, syComplete, "
    "             sySingles, syDoubles, syMatches "
    "  FROM SyRec WHERE syID = ";
  str += ltostr(id);
  str += "";

  try
  {
    if (!ExecuteQuery(str))
      return false;

    BindCol(1, &syID);
    BindCol(2, syName, sizeof(syName));
    BindCol(3, syDesc, sizeof(syDesc));
    BindCol(4, &syComplete);
    BindCol(5, &sySingles);
    BindCol(6, &syDoubles);
    BindCol(7, &syMatches);

    return true;
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


bool  SyStore::SelectByName(const wxString &name)
{
  wxString str = 
    "SELECT syID, syName, syDesc, syComplete, "
    "             sySingles, syDoubles, syMatches "
    " WHERE syName = '";
  str += name;
  str += "'";

  try
  {
    if (!ExecuteQuery(str))
      return false;

    BindCol(1, &syID);
    BindCol(2, syName, sizeof(syName));
    BindCol(3, syDesc, sizeof(syDesc));
    BindCol(4, &syComplete);
    BindCol(5, &sySingles);
    BindCol(6, &syDoubles);
    BindCol(7, &syMatches);

    return true;
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


bool  SyStore::Insert()
{
  PreparedStatement *stmtPtr = 0;

  wxString str = "INSERT INTO SyRec "
                    "(syID, syName, syDesc, "
                    " sySingles, syDoubles, syMatches, syComplete) "
                    "VALUES(?, ?, ?, ?, ?, ?, ?)";

  try
  {
    syID = IdStore::ID(GetConnectionPtr());

    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    stmtPtr->SetData(1, &syID);
    stmtPtr->SetData(2, syName);
    stmtPtr->SetData(3, syDesc);
    stmtPtr->SetData(4, &sySingles);
    stmtPtr->SetData(5, &syDoubles);
    stmtPtr->SetData(6, &syMatches);
    stmtPtr->SetData(7, &syComplete);

    stmtPtr->Execute();

    SyMatchStore(GetConnectionPtr()).Insert(this);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  delete stmtPtr;

  // Notify Views
  CRequest update;
  update.type = CRequest::INSERT;
  update.rec  = CRequest::SYREC;
  update.id   = syID;

  CTT32App::NotifyChange(update);

  return true;
}


bool  SyStore::Update()
{
  PreparedStatement *stmtPtr = 0;

  wxString str = "UPDATE SyRec "
                    "SET syName = ?, syDesc = ?, syComplete = ? "
                    "WHERE syID = ?";

  try
  {
    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    stmtPtr->SetData(1, syName);
    stmtPtr->SetData(2, syDesc);
    stmtPtr->SetData(3, &syComplete);
    stmtPtr->SetData(4, &syID);

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
  update.rec  = CRequest::SYREC;
  update.id   = syID;

  CTT32App::NotifyChange(update);

  return true;
}


bool  SyStore::Remove(long id)
{
  // ID-Check
  if (!id)
    id = syID;

  if (!id)
    return true;
  
  wxString str = "DELETE FROM SyRec WHERE syID = ";
  str += ltostr(id);
  str += "";

  try
  {
    // SyMatchStore(GetConnectionPtr()).Remove(syID);
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
  update.rec  = CRequest::SYREC;
  update.id   = id;

  CTT32App::NotifyChange(update);

  return true;
}


// Check auf syName, ob System existiert
bool  SyStore::InsertOrUpdate()
{
  Statement *stmtPtr;
  ResultSet *resPtr;

  long  id;
  
  stmtPtr = GetConnectionPtr()->CreateStatement();

  wxString  sql = "SELECT syID FROM SyRec WHERE syName = '";
  sql += syName;
  sql += "'";

  resPtr = stmtPtr->ExecuteQuery(sql);
  resPtr->BindCol(1, &id);
  bool  exist = (resPtr->Next() && !resPtr->WasNull(1));

  delete resPtr;
  delete stmtPtr;

  if (exist)
  {
    syID = id;
    return Update();
  }
  else
    return Insert();
}


bool  SyStore::Next()
{
  if (!StoreObj::Next())
    return false;

  delete syList;
  syList = new SyList[syMatches];

  Connection *connPtr = TTDbse::instance()->GetNewConnection();

  try
  {
    // System einlesen
    SyMatchStore  syMatch(connPtr);
    syMatch.Select(syID);
    while (syMatch.Next())
    {
      syList[syMatch.syNr-1].syType    = syMatch.syType;
      syList[syMatch.syNr-1].syPlayerA = syMatch.syPlayerA;
      syList[syMatch.syNr-1].syPlayerX = syMatch.syPlayerX;
    }
  }
  catch (SQLException &e)
  {
    infoSystem.Exception("SELECT ... FROM SyMatch WHERE ...", e);
    delete connPtr;
    return false;
  }

  delete connPtr;

  return true;
}


// -----------------------------------------------------------------------
// + Swaythling Cup
bool  SyStore::AddModifiedSwaythlingCup()
{
	SyStore  sy;

	wxStrcpy((wxChar *) sy.syName, "MSC");
	wxStrcpy((wxChar *) sy.syDesc, "Modified Swaythling Cup");
	sy.syMatches = 7;
	sy.sySingles = 3;
	sy.syDoubles = 1;
  sy.syComplete = 0;

  sy.syList = new SyList[7];
	sy.syList[0].syType = CP_SINGLE;
	sy.syList[0].syPlayerA = 1;
	sy.syList[0].syPlayerX = 2;

	sy.syList[1].syType = CP_SINGLE;
	sy.syList[1].syPlayerA = 2;
	sy.syList[1].syPlayerX = 1;

	sy.syList[2].syType = CP_SINGLE;
	sy.syList[2].syPlayerA = 3;
	sy.syList[2].syPlayerX = 3;

	sy.syList[3].syType = CP_DOUBLE;
	sy.syList[3].syPlayerA = 1;
	sy.syList[3].syPlayerX = 1;

	sy.syList[4].syType = CP_SINGLE;
	sy.syList[4].syPlayerA = 1;
	sy.syList[4].syPlayerX = 1;

	sy.syList[5].syType = CP_SINGLE;
	sy.syList[5].syPlayerA = 3;
	sy.syList[5].syPlayerX = 2;

	sy.syList[6].syType = CP_SINGLE;
	sy.syList[6].syPlayerA = 2;
	sy.syList[6].syPlayerX = 3;

  return sy.Insert();
}


// + Corbillion Cup
bool  SyStore::AddCorbillonCup()
{
  SyStore sy;

	wxStrcpy((wxChar *) sy.syName, "COR");
	wxStrcpy((wxChar *) sy.syDesc, "Corbillon Cup");
	sy.syMatches = 5;
	sy.sySingles = 2;
	sy.syDoubles = 1;
  sy.syComplete = 0;

  sy.syList = new SyList[5];

	sy.syList[0].syType = CP_SINGLE;
	sy.syList[0].syPlayerA = 1;
	sy.syList[0].syPlayerX = 1;

	sy.syList[1].syType = CP_SINGLE;
	sy.syList[1].syPlayerA = 2;
	sy.syList[1].syPlayerX = 2;

	sy.syList[2].syType = CP_DOUBLE;
	sy.syList[2].syPlayerA = 1;
	sy.syList[2].syPlayerX = 1;

	sy.syList[3].syType = CP_SINGLE;
	sy.syList[3].syPlayerA = 1;
	sy.syList[3].syPlayerX = 2;

	sy.syList[4].syType = CP_SINGLE;
	sy.syList[4].syPlayerA = 2;
	sy.syList[4].syPlayerX = 1;

  return sy.InsertOrUpdate();
}


bool SyStore::AddModifiedCorbillonCup()
{
  SyStore sy;

	wxStrcpy((wxChar *) sy.syName, "MCC");
	wxStrcpy((wxChar *) sy.syDesc, "Modified Corbillon Cup");
	sy.syMatches = 5;
	sy.sySingles = 3;
	sy.syDoubles = 0;
  sy.syComplete = 0;

  sy.syList = new SyList[5];

	sy.syList[0].syType = CP_SINGLE;
	sy.syList[0].syPlayerA = 1;
	sy.syList[0].syPlayerX = 1;

	sy.syList[1].syType = CP_SINGLE;
	sy.syList[1].syPlayerA = 2;
	sy.syList[1].syPlayerX = 2;

	sy.syList[2].syType = CP_SINGLE;
	sy.syList[2].syPlayerA = 3;
	sy.syList[2].syPlayerX = 3;

	sy.syList[3].syType = CP_SINGLE;
	sy.syList[3].syPlayerA = 1;
	sy.syList[3].syPlayerX = 2;

	sy.syList[4].syType = CP_SINGLE;
	sy.syList[4].syPlayerA = 2;
	sy.syList[4].syPlayerX = 1;

  return sy.InsertOrUpdate();
}
 
bool SyStore::AddOlympicTeamSystem()
{
  SyStore sy;

	wxStrcpy((wxChar *) sy.syName, "OTS");
	wxStrcpy((wxChar *) sy.syDesc, "Olympic Team System");
	sy.syMatches = 5;
	sy.sySingles = 4;
	sy.syDoubles = 1;
  sy.syComplete = 0;

  sy.syList = new SyList[5];

	sy.syList[0].syType = CP_SINGLE;
	sy.syList[0].syPlayerA = 1;
	sy.syList[0].syPlayerX = 1;

	sy.syList[1].syType = CP_SINGLE;
	sy.syList[1].syPlayerA = 2;
	sy.syList[1].syPlayerX = 2;

	sy.syList[2].syType = CP_DOUBLE;
	sy.syList[2].syPlayerA = 1;
	sy.syList[2].syPlayerX = 1;

	sy.syList[3].syType = CP_SINGLE;
	sy.syList[3].syPlayerA = 4;
	sy.syList[3].syPlayerX = 3;

	sy.syList[4].syType = CP_SINGLE;
	sy.syList[4].syPlayerA = 3;
	sy.syList[4].syPlayerX = 4;

  return sy.InsertOrUpdate();
}
 
bool SyStore::AddOlympicTeamSystem2Players()
{
  SyStore sy;

	wxStrcpy((wxChar *) sy.syName, "OTS2");
	wxStrcpy((wxChar *) sy.syDesc, "Olympic Team System 2 Players");
	sy.syMatches = 3;
	sy.sySingles = 2;
	sy.syDoubles = 1;
  sy.syComplete = 0;

  sy.syList = new SyList[3];

	sy.syList[0].syType = CP_SINGLE;
	sy.syList[0].syPlayerA = 1;
	sy.syList[0].syPlayerX = 1;

	sy.syList[1].syType = CP_DOUBLE;
	sy.syList[1].syPlayerA = 1;
	sy.syList[1].syPlayerX = 1;

	sy.syList[2].syType = CP_SINGLE;
	sy.syList[2].syPlayerA = 2;
	sy.syList[2].syPlayerX = 2;

  return sy.InsertOrUpdate();
}
 

bool SyStore::AddECTeamSystem()
{
  SyStore sy;

	wxStrcpy((wxChar *) sy.syName, "ETS");
	wxStrcpy((wxChar *) sy.syDesc, "EC Team System");
	sy.syMatches = 5;
	sy.sySingles = 6;
	sy.syDoubles = 0;
  sy.syComplete = 0;

  sy.syList = new SyList[5];

	sy.syList[0].syType = CP_SINGLE;
	sy.syList[0].syPlayerA = 1;
	sy.syList[0].syPlayerX = 2;

	sy.syList[1].syType = CP_SINGLE;
	sy.syList[1].syPlayerA = 2;
	sy.syList[1].syPlayerX = 1;

	sy.syList[2].syType = CP_SINGLE;
	sy.syList[2].syPlayerA = 3;
	sy.syList[2].syPlayerX = 3;

	sy.syList[3].syType = CP_SINGLE;
	sy.syList[3].syPlayerA = 5;
	sy.syList[3].syPlayerX = 5;

	sy.syList[4].syType = CP_SINGLE;
	sy.syList[4].syPlayerA = 6;
	sy.syList[4].syPlayerX = 6;

  return sy.InsertOrUpdate();
}


bool SyStore::AddMixedTeamSystem()
{
  SyStore sy;

	wxStrcpy((wxChar *) sy.syName, "XTS");
	wxStrcpy((wxChar *) sy.syDesc, "Mixed Team System");
	sy.syMatches = 5;
	sy.sySingles = 4;
	sy.syDoubles = 1;
  sy.syComplete = 1;

  sy.syList = new SyList[5];

	sy.syList[0].syType = CP_DOUBLE;
	sy.syList[0].syPlayerA = 1;
	sy.syList[0].syPlayerX = 1;

	sy.syList[1].syType = CP_SINGLE;
	sy.syList[1].syPlayerA = 1;
	sy.syList[1].syPlayerX = 1;

	sy.syList[2].syType = CP_SINGLE;
	sy.syList[2].syPlayerA = 2;
	sy.syList[2].syPlayerX = 2;

	sy.syList[3].syType = CP_SINGLE;
	sy.syList[3].syPlayerA = 3;
	sy.syList[3].syPlayerX = 3;

	sy.syList[4].syType = CP_SINGLE;
	sy.syList[4].syPlayerA = 4;
	sy.syList[4].syPlayerX = 4;

  return sy.InsertOrUpdate();
}

 
bool SyStore::AddMixedTeamSystemA()
{
  SyStore sy;

	wxStrcpy((wxChar *) sy.syName, "XTSA");
	wxStrcpy((wxChar *) sy.syDesc, "Mixed Team System A-Final");
	sy.syMatches = 9;
	sy.sySingles = 4;
	sy.syDoubles = 1;
  sy.syComplete = 0;

  sy.syList = new SyList[9];

	sy.syList[0].syType = CP_DOUBLE;
	sy.syList[0].syPlayerA = 1;
	sy.syList[0].syPlayerX = 1;

	sy.syList[1].syType = CP_SINGLE;
	sy.syList[1].syPlayerA = 1;
	sy.syList[1].syPlayerX = 3;

	sy.syList[2].syType = CP_SINGLE;
	sy.syList[2].syPlayerA = 2;
	sy.syList[2].syPlayerX = 4;

	sy.syList[3].syType = CP_SINGLE;
	sy.syList[3].syPlayerA = 3;
	sy.syList[3].syPlayerX = 1;

	sy.syList[4].syType = CP_SINGLE;
	sy.syList[4].syPlayerA = 4;
	sy.syList[4].syPlayerX = 2;

	sy.syList[5].syType = CP_SINGLE;
	sy.syList[5].syPlayerA = 1;
	sy.syList[5].syPlayerX = 1;

	sy.syList[6].syType = CP_SINGLE;
	sy.syList[6].syPlayerA = 2;
	sy.syList[6].syPlayerX = 2;

	sy.syList[7].syType = CP_SINGLE;
	sy.syList[7].syPlayerA = 3;
	sy.syList[7].syPlayerX = 3;

	sy.syList[8].syType = CP_SINGLE;
	sy.syList[8].syPlayerA = 4;
	sy.syList[8].syPlayerX = 4;

  return sy.InsertOrUpdate();
}
 
bool SyStore::AddYouthSeriesTeamA()
{
  SyStore sy;

  wxStrcpy((wxChar *) sy.syName, "YSTA");
  wxStrcpy((wxChar *) sy.syDesc, "Youth Series Team A-Final");
  sy.syMatches = 5;
  sy.sySingles = 6;
  sy.syDoubles = 1;
  sy.syComplete = 0;

  sy.syList = new SyList[9];

  sy.syList[1].syType = CP_SINGLE;
  sy.syList[1].syPlayerA = 1;
  sy.syList[1].syPlayerX = 2;

  sy.syList[2].syType = CP_SINGLE;
  sy.syList[2].syPlayerA = 2;
  sy.syList[2].syPlayerX = 1;

  sy.syList[0].syType = CP_DOUBLE;
  sy.syList[0].syPlayerA = 1;
  sy.syList[0].syPlayerX = 1;

  sy.syList[3].syType = CP_SINGLE;
  sy.syList[3].syPlayerA = 5;
  sy.syList[3].syPlayerX = 5;

  sy.syList[4].syType = CP_SINGLE;
  sy.syList[4].syPlayerA = 6;
  sy.syList[4].syPlayerX = 6;

  return sy.InsertOrUpdate();
}

bool SyStore::AddYouthSeriesTeamQualification()
{
  SyStore sy;

  wxStrcpy((wxChar *) sy.syName, "YSTQ");
  wxStrcpy((wxChar *) sy.syDesc, "Youth Series Team Qualification");
  sy.syMatches = 3;
  sy.sySingles = 2;
  sy.syDoubles = 1;
  sy.syComplete = 1;

  sy.syList = new SyList[3];

  sy.syList[1].syType = CP_SINGLE;
  sy.syList[1].syPlayerA = 1;
  sy.syList[1].syPlayerX = 2;

  sy.syList[2].syType = CP_SINGLE;
  sy.syList[2].syPlayerA = 2;
  sy.syList[2].syPlayerX = 1;

  sy.syList[0].syType = CP_DOUBLE;
  sy.syList[0].syPlayerA = 1;
  sy.syList[0].syPlayerX = 1;

  return sy.InsertOrUpdate();
}
