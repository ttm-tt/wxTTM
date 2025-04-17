/* Copyright (C) Christoph Theis 2022 */

#include "stdafx.h"

#include "MpStore.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "ResultSet.h"
#include  "SQLException.h"
#include  "InfoSystem.h"
#include  "Request.h"

#include  "IdStore.h"


void MpRec::ChangeCount(int newCount)
{
  if (newCount == 0)
  {
    delete[] mpList;
    mpList = nullptr;

    mpCount = newCount;
  }

  MpItem *tmp = new MpItem[newCount];
  if (newCount < mpCount)  
    memcpy(tmp, mpList, newCount * sizeof(MpItem));
  else
  {
    if (mpList)
      memcpy(tmp, mpList, newCount * sizeof(MpItem));
      
    for (int i = mpCount; i < newCount; i++)
    {
      tmp[i].mpResA = 0;
      tmp[i].mpResX = 0;
      tmp[i].mpPts = 0;
    }
  }
  
  delete[] mpList;
  mpList = tmp;
  
  mpCount = newCount;
}


// -----------------------------------------------------------------------
// Persistence of MpItem
struct MpItemRec 
{
  long  mpID = 0;   // Foreign key
  short mpNr = 0;   // Index
  short mpResA = 0;
  short mpResX = 0;
  short mpPts = 0;

  MpItemRec() { Init(); }
  void Init() { memset(this, 0, sizeof(MpItemRec)); }
};


class MpItemStore : public StoreObj, public MpItemRec
{
  public:
    static bool CreateTable();
    static bool UpdateTable(long version);

    static bool CreateConstraints();
    static bool UpdateConstraints(long version);

    MpItemStore(Connection *connPtr = nullptr) : StoreObj(connPtr) {}

    virtual void Init();

    bool Select(long mpID);
    bool Insert(MpRec *);
    bool Remove(long mpID);
};


void MpItemStore::Init()
{
  MpItemRec::Init();
}


bool MpItemStore::CreateTable()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  wxASSERT(connPtr);

  Statement *tmp = connPtr->CreateStatement();

  wxString  INTEGER  = connPtr->GetDataType(SQL_INTEGER);
  wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);

  wxString  sql = 
    "CREATE TABLE MpItem (         "
    "mpID        "+INTEGER+"      NOT NULL,  "
    "mpNr        "+SMALLINT+"     NOT NULL,  "
	  "mpResA      "+SMALLINT+"     NOT NULL,  "
    "mpResX      "+SMALLINT+"     NOT NULL, "
    "mpPts       "+SMALLINT+"     NOT NULL, "
    "CONSTRAINT mpItemKey PRIMARY KEY (mpID, mpNr) "
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
    tmp->ExecuteUpdate(sql = "CREATE UNIQUE INDEX mpItemIdKey ON MpItem (mpID, MpNr)");
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(sql, e);
  };

  tmp->Close();
  delete tmp;
  
  return true;
}


bool MpItemStore::UpdateTable(long version)
{
  if (version < 164)
    return CreateTable();

  return true;
}


bool MpItemStore::CreateConstraints()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str;
  
  try
  {
    tmp->ExecuteUpdate("ALTER TABLE MpItem DROP CONSTRAINT mpItemIdRef");
  }
  catch (SQLException &)
  {
  }

  try
  {
    tmp->ExecuteUpdate(str = 
      "ALTER TABLE MpItem ADD CONSTRAINT mpItemIdRef "
      "FOREIGN KEY (mpID) REFERENCES MpRec (mpID) ON DELETE CASCADE");
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
  };

  delete tmp;

  return true;
}


bool MpItemStore::UpdateConstraints(long version)
{
  if (version < 164)
    return CreateConstraints();

  return true;
}


bool MpItemStore::Select(long id)
{
  wxString str = 
    "SELECT mpID, mpNr, mpResA, mpResX, mpPts "
    "  FROM MpItem "
    " WHERE mpID = " + ltostr(id);

  if (!ExecuteQuery(str))
    return false;

  BindCol(1, &mpID);
  BindCol(2, &mpNr);
  BindCol(3, &mpResA);
  BindCol(4, &mpResX);
  BindCol(5, &mpPts);

  return true;
}


bool MpItemStore::Insert(MpRec* mpPtr)
{
  wxASSERT(mpPtr);
  wxASSERT(mpPtr->mpList);

  wxString str = "INSERT INTO MpItem "
                 "(mpID, mpNr, mpResA, mpResX, mpPts) "
                 "VALUES(?, ?, ?, ?, ?)";

  PreparedStatement *stmtPtr = GetConnectionPtr()->PrepareStatement(str);
  wxASSERT(stmtPtr);

  mpID = mpPtr->mpID;
  for (int nr = 0; nr < mpPtr->mpCount; nr++)
  {
    mpNr = nr+1;
    mpResA = mpPtr->mpList[nr].mpResA;
    mpResX = mpPtr->mpList[nr].mpResX;
    mpPts  = mpPtr->mpList[nr].mpPts;

    stmtPtr->SetData(1, &mpID);
    stmtPtr->SetData(2, &mpNr);
    stmtPtr->SetData(3, &mpResA);
    stmtPtr->SetData(4, &mpResX);
    stmtPtr->SetData(5, &mpPts);

    stmtPtr->Execute();
  }

  delete stmtPtr;
  return true;
}


bool MpItemStore::Remove(long mpID)
{
  wxString  str = 
      "DELETE FROM MpItem WHERE mpID = " + ltostr(mpID);

  return ExecuteUpdate(str);
}


// -----------------------------------------------------------------------
bool MpStore::CreateTable()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  wxASSERT(connPtr);

  Statement *tmp = connPtr->CreateStatement();

  wxString  WVARCHAR = connPtr->GetDataType(SQL_WVARCHAR);
  wxString  INTEGER  = connPtr->GetDataType(SQL_INTEGER);
  wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);

  wxString  sql = 
    "CREATE TABLE MpRec (         "
    "mpID        "+INTEGER+"      NOT NULL,  "
    "mpName      "+WVARCHAR+"(8)  NOT NULL,  "
	  "mpDesc      "+WVARCHAR+"(64),           "
    "mpCount     "+SMALLINT+" NOT NULL,      "
    "CONSTRAINT mpIdKey PRIMARY KEY (mpID)   "
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
    tmp->ExecuteUpdate(sql = "CREATE UNIQUE INDEX mpNameKey ON MpRec (mpName)");
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(sql, e);
  };

  MpItemStore::CreateTable();

  tmp->Close();
  delete tmp;
  
  return true;
}


bool MpStore::UpdateTable(long version)
{
  if (version < 164)
    return CreateTable();

  return true;
}


bool MpStore::CreateConstraints()
{
  return MpItemStore::CreateConstraints();
}


bool MpStore::UpdateConstraints(long version)
{
  if (version < 164)
    return CreateConstraints();

  return MpItemStore::UpdateConstraints(version);
}


MpStore::MpStore(Connection* connPtr) : StoreObj(connPtr)
{
}

MpStore::~MpStore()
{
}


void MpStore::Init()
{
  // MpRec::Init();
}


// -----------------------------------------------------------------------
bool MpStore::SelectAll()
{
  wxString str =
    "SELECT mpID, mpName, mpDesc, mpCount "
    "  FROM MpRec";

  try
  {
    if (!ExecuteQuery(str))
      return false;

    BindCol(1, &mpID);
    BindCol(2, mpName, sizeof(mpName));
    BindCol(3, mpDesc, sizeof(mpDesc));
    BindCol(4, &mpCount);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  return true;
}


bool MpStore::SelectById(long id)
{
  wxString str =
    "SELECT mpID, mpName, mpDesc, mpCount "
    "  FROM MpRec "
    " WHERE mpID = " + ltostr(id);

  try
  {
    if (!ExecuteQuery(str))
      return false;

    BindCol(1, &mpID);
    BindCol(2, mpName, sizeof(mpName));
    BindCol(3, mpDesc, sizeof(mpDesc));
    BindCol(4, &mpCount);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  return true;
}


bool MpStore::SelectByName(const wxString& name)
{
  wxString str =
    "SELECT mpID, mpName, mpDesc, mpCount "
    "  FROM MpRec "
    " WHERE mpName = '" + name + "'";

  try
  {
    if (!ExecuteQuery(str))
      return false;

    BindCol(1, &mpID);
    BindCol(2, mpName, sizeof(mpName));
    BindCol(3, mpDesc, sizeof(mpDesc));
    BindCol(4, &mpCount);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  return true;
}


// -----------------------------------------------------------------------
bool MpStore::Insert()
{
  PreparedStatement *stmtPtr = 0;

  wxString str = "INSERT INTO MpRec "
                    "(mpID, mpName, mpDesc, mpCount)"
                    "VALUES(?, ?, ?, ?)";

  try
  {
    mpID = IdStore::ID(GetConnectionPtr());

    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    stmtPtr->SetData(1, &mpID);
    stmtPtr->SetData(2, mpName);
    stmtPtr->SetData(3, mpDesc);
    stmtPtr->SetData(4, &mpCount);

    stmtPtr->Execute();

    MpItemStore(GetConnectionPtr()).Insert(this);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  delete stmtPtr;

  // Notify Views
  CRequest update;
  update.type = CRequest::INSERT;
  update.rec  = CRequest::MPREC;
  update.id   = mpID;

  CTT32App::NotifyChange(update);

  return true;
}


bool MpStore::Update()
{
  PreparedStatement *stmtPtr = 0;

  wxString str = "UPDATE MpRec "
                    "SET mpName = ?, mpDesc = ? "
                    "WHERE mpID = ?";

  try
  {
    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    stmtPtr->SetData(1, mpName);
    stmtPtr->SetData(2, mpDesc);
    stmtPtr->SetData(3, &mpID);

    stmtPtr->Execute();   
    
    MpItemStore().Remove(mpID);
    MpItemStore().Insert(this);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  delete stmtPtr;

  // Notify Views
  CRequest update;
  update.type = CRequest::UPDATE;
  update.rec  = CRequest::MPREC;
  update.id   = mpID;

  CTT32App::NotifyChange(update);

  return true;
}


bool MpStore::InsertOrUpdate()
{
  Statement *stmtPtr;
  ResultSet *resPtr;

  long  id;
  
  stmtPtr = GetConnectionPtr()->CreateStatement();

  wxString  sql = "SELECT mpID FROM MpRec WHERE syName = '";
  sql += mpName;
  sql += "'";

  resPtr = stmtPtr->ExecuteQuery(sql);
  resPtr->BindCol(1, &id);
  bool  exist = (resPtr->Next() && !resPtr->WasNull(1));

  delete resPtr;
  delete stmtPtr;

  if (exist)
  {
    mpID = id;
    return Update();
  }
  else
    return Insert();
}


bool MpStore::Remove(long id)
{
  // ID-Check
  if (!id)
    id = mpID;

  if (!id)
    return true;
  
  wxString str = "DELETE FROM MpRec WHERE mpID = ";
  str += ltostr(id);
  str += "";

  try
  {
    // MpItemStore(GetConnectionPtr()).Remove(mpID);
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
  update.rec  = CRequest::MPREC;
  update.id   = id;

  CTT32App::NotifyChange(update);

  return true;
}


// -----------------------------------------------------------------------
bool MpStore::Next()
{
  if (!StoreObj::Next())
    return false;

  delete[] mpList;
  mpList = new MpItem[mpCount];
  
  Connection *connPtr = TTDbse::instance()->GetNewConnection();

  try
  {
    // Match points einlesen
    MpItemStore mpItem(connPtr);
    mpItem.Select(mpID);
    while (mpItem.Next())
    {
      mpList[mpItem.mpNr - 1].mpResA = mpItem.mpResA;
      mpList[mpItem.mpNr - 1].mpResX = mpItem.mpResX;
      mpList[mpItem.mpNr - 1].mpPts  = mpItem.mpPts;
    }
  }
  catch (SQLException &e)
  {
    infoSystem.Exception("SELECT ... FROM MpItem WHERE ...", e);
    delete connPtr;
    return false;
  }

  delete connPtr;

  return true;
}
