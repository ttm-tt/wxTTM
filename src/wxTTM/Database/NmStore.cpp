/* Copyright (C) 2020 Christoph Theis */

// DB-Tabelle der Wettbewerbe

#include  "stdafx.h"
#include  "NmStore.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "ResultSet.h"
#include  "SQLException.h"
#include  "InfoSystem.h"
#include  "Request.h"
#include  "Rec.h"

#include  "IdStore.h"
#include  "MtStore.h"
#include  "TmStore.h"
#include  "SyStore.h"
#include  "TmEntryStore.h"


// -----------------------------------------------------------------------
NmRec & NmRec::operator=(const NmRec &rec)
{
  Init();

  nmID = rec.nmID;
  mtID = rec.mtID;
  tmID = rec.tmID;

  nofSingles = rec.nofSingles;
  nofDoubles = rec.nofDoubles;

  if (nofSingles)
  {
    nmSingle = new NmSingle[nofSingles];
    for (short i = nofSingles; i--; )
      nmSingle[i] = rec.nmSingle[i];
  }

  if (nofDoubles)
  {
    nmDouble = new NmDouble;
    for (short i = nofDoubles; i--; )
      nmDouble[i] = rec.nmDouble[i];
  }

  return *this;
}


void  NmRec::SetSingle(short idx, long id, bool optional)
{
  if (idx >= nofSingles)
  {
    NmSingle * tmp = new NmSingle[idx + 1];
    memset(tmp, 0, (idx + 1) * sizeof(NmSingle));
    memcpy(tmp, nmSingle, nofSingles * sizeof(NmSingle));
    nofSingles = idx + 1;
    delete[] nmSingle;
    nmSingle = tmp;
  }

  nmSingle[idx].ltA = id;
  nmSingle[idx].nmOptional = optional;
}


void  NmRec::SetDoubles(short idx, long idA, long idB, bool optional)
{
  if (idx >= nofDoubles)
  {
    NmDouble * tmp = new NmDouble[idx + 1];
    memset(tmp, 0, (idx + 1) * sizeof(NmDouble));
    memcpy(tmp, nmDouble, nofDoubles * sizeof(NmDouble));
    nofDoubles = idx + 1;
    delete[] nmDouble;
    nmDouble = tmp;
  }

  nmDouble[idx].ltA = idA;
  nmDouble[idx].ltB = idB;
  nmDouble[idx].nmOptional = optional;
}


void  NmRec::SetNofSingles(short nof)
{
  delete[] nmSingle;
  nmSingle = new NmSingle[nof];
  memset(nmSingle, 0, nof * sizeof(NmSingle));
  nofSingles = nof;
}


void  NmRec::SetNofDoubles(short nof)
{
  delete[] nmDouble;
  nmDouble = new NmDouble[nof];
  memset(nmDouble, 0, nof * sizeof(NmDouble));
  nofDoubles = nof;
}


// -----------------------------------------------------------------------
bool  NmSingleStore::CreateTable()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  wxASSERT(connPtr);

  Statement *tmp = connPtr->CreateStatement();

  wxString  INTEGER  = connPtr->GetDataType(SQL_INTEGER);
  wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);

  wxString  sql = 
    "CREATE TABLE NmSingle (         "
    "nmID        "+INTEGER+"      NOT NULL,  "
    "nmNr        "+SMALLINT+"     NOT NULL,  "
    "ltA         "+INTEGER+"          NULL,  "
    "nmOptional  "+SMALLINT+"     NOT NULL DEFAULT 0, "
    "CONSTRAINT nmSingleKey PRIMARY KEY (nmID, nmNr) "
    
	  ")";

  tmp->ExecuteUpdate(sql);

  tmp->Close();
  delete tmp;
  
  return true;
}


bool  NmSingleStore::UpdateTable(long version)
{
  if (version <= 6)
    return CreateTable();
    
  if (version < 76)
  {  
    Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
    wxASSERT(connPtr);

    Statement *tmp = connPtr->CreateStatement();

    wxString  INTEGER  = connPtr->GetDataType(SQL_INTEGER);
    
    wxString sql = "ALTER TABLE NmSingle ALTER COLUMN ltA " + INTEGER + " NULL";

    try 
    {
      tmp->ExecuteUpdate(sql);
    }
    catch (SQLException &e)
    {
      infoSystem.Exception(sql, e);
    }
    
    tmp->Close();
    delete tmp;
  }

  if (version < 173)
  {
    Connection * connPtr = TTDbse::instance()->GetDefaultConnection();
    wxASSERT(connPtr);

    Statement * tmp = connPtr->CreateStatement();

    wxString  TINYINT = connPtr->GetDataType(SQL_TINYINT);  

    wxString sql = "ALTER TABLE NmSingle ADD nmOptional " + TINYINT + " NOT NULL DEFAULT 0";

    try
    {
      tmp->ExecuteUpdate(sql);
    }
    catch (SQLException & e)
    {
      infoSystem.Exception(sql, e);
    }

    tmp->Close();

    delete tmp;
  }

  return true;
}


bool  NmSingleStore::CreateConstraints()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str;
  
  try
  {
    tmp->ExecuteUpdate("ALTER TABLE NmSingle DROP CONSTRAINT nmSingleIdRef");
    tmp->ExecuteUpdate("ALTER TABLE NmSingle DROP CONSTRAINT nmSingleLtARef");
  }
  catch (SQLException &)
  {
  }

  tmp->ExecuteUpdate(str = 
      "ALTER TABLE NmSingle ADD CONSTRAINT nmSingleIdRef "
      "FOREIGN KEY (nmID) REFERENCES NmRec (nmID) ON DELETE CASCADE");

  tmp->ExecuteUpdate(str = 
      "ALTER TABLE NmSingle ADD CONSTRAINT nmSingleLtARef "
      "FOREIGN KEY (ltA) REFERENCES LtRec (ltID) ON DELETE No ACTION");

  delete tmp;
  return true;
}


bool  NmSingleStore::UpdateConstraints(long version)
{
  if (version <= 50)
    return CreateConstraints();

  return true;
}


NmSingleStore::NmSingleStore(NmStore &nm)
{
  nmID = nm.nmID;
}


NmSingleStore::~NmSingleStore()
{
}


void  NmSingleStore::Init()
{
  NmSingle::Init();
}


bool  NmSingleStore::Insert()
{
  PreparedStatement *stmtPtr = 0;

  wxString str = "INSERT INTO NmSingle (nmID, nmNr, ltA, nmOptional) "
                    "VALUES (?, ?, ?, ?)";

  try
  {    
    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    stmtPtr->SetData(1, &nmID);
    stmtPtr->SetData(2, &nmNr);
    stmtPtr->SetData(3, ltA ? &ltA : NULL);
    stmtPtr->SetData(4, &nmOptional);

    stmtPtr->Execute();
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


bool  NmSingleStore::Remove()
{
  wxString  str = 
      "DELETE FROM NmSingle WHERE nmID = " + ltostr(nmID);

  try
  {
    ExecuteUpdate(str);
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


bool  NmSingleStore::SelectAll()
{
  wxString str = 
      "SELECT nmID, nmNr, ltA, nmOptional FROM NmSingle WHERE nmID = " + ltostr(nmID);

  try
  {
    if (!ExecuteQuery(str))
      return false;

    BindCol(1, &nmID);
    BindCol(2, &nmNr);
    BindCol(3, &ltA);
    BindCol(4, &nmOptional);

    return true;
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}



// -----------------------------------------------------------------------
bool  NmDoubleStore::CreateTable()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  wxASSERT(connPtr);

  Statement *tmp = connPtr->CreateStatement();

  wxString  INTEGER  = connPtr->GetDataType(SQL_INTEGER);
  wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);

  wxString  sql = 
    "CREATE TABLE NmDouble (         "
    "nmID        "+INTEGER+"      NOT NULL,  "
    "nmNr        "+SMALLINT+"     NOT NULL,  "
    "ltA         "+INTEGER+"          NULL,  "
    "ltB         "+INTEGER+"          NULL,  "
    "nmOptional  "+ SMALLINT+"    NOT NULL DEFAULT 0, "
    "CONSTRAINT nmDoubleKey PRIMARY KEY (nmID, nmNr) "
	  ")";

  tmp->ExecuteUpdate(sql);

  tmp->Close();
  delete tmp;
  
  return true;
}


bool  NmDoubleStore::UpdateTable(long version)
{
  if (version <= 6)
    return CreateTable();

  if (version < 76)
  {  
    Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
    wxASSERT(connPtr);

    Statement *tmp = connPtr->CreateStatement();

    wxString  INTEGER  = connPtr->GetDataType(SQL_INTEGER);
    
    wxString sql;

    try 
    {
      sql = "ALTER TABLE NmDouble ALTER COLUMN ltA " + INTEGER + " NULL";
      tmp->ExecuteUpdate(sql);
      
      sql = "ALTER TABLE NmDouble ALTER COLUMN ltB " + INTEGER + " NULL";
      tmp->ExecuteUpdate(sql);
    }
    catch (SQLException &e)
    {
      infoSystem.Exception(sql, e);
    }
    
    tmp->Close();
    delete tmp;
  }

  if (version < 173)
  {
    Connection * connPtr = TTDbse::instance()->GetDefaultConnection();
    wxASSERT(connPtr);

    Statement * tmp = connPtr->CreateStatement();

    wxString  TINYINT = connPtr->GetDataType(SQL_TINYINT);

    wxString sql = "ALTER TABLE NmDouble ADD nmOptional " + TINYINT + " NOT NULL DEFAULT 0";

    try
    {
      tmp->ExecuteUpdate(sql);
    }
    catch (SQLException & e)
    {
      infoSystem.Exception(sql, e);
    }
  }

  return true;
}


bool  NmDoubleStore::CreateConstraints()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str;
  
  try
  {
    tmp->ExecuteUpdate("ALTER TABLE NmDouble DROP CONSTRAINT nmDoubleIdRef");
    tmp->ExecuteUpdate("ALTER TABLE NmDouble DROP CONSTRAINT nmDoubleLtARef");
    tmp->ExecuteUpdate("ALTER TABLE NmDouble DROP CONSTRAINT nmDoubleLtBRef");
  }
  catch (SQLException &)
  {
  }

  tmp->ExecuteUpdate(str = 
      "ALTER TABLE NmDouble ADD CONSTRAINT nmDoubleIdRef "
      "FOREIGN KEY (nmID) REFERENCES NmRec (nmID) ON DELETE CASCADE");

  tmp->ExecuteUpdate(str = 
      "ALTER TABLE NmDouble ADD CONSTRAINT nmDoubleLtARef "
      "FOREIGN KEY (ltA) REFERENCES LtRec (ltID) ON DELETE NO ACTION");

  tmp->ExecuteUpdate(str = 
      "ALTER TABLE NmDouble ADD CONSTRAINT nmDoubleLtBRef "
      "FOREIGN KEY (ltB) REFERENCES LtRec (ltID) ON DELETE NO ACTION");

  delete tmp;
  return true;
}


bool  NmDoubleStore::UpdateConstraints(long version)
{
  if (version <= 50)
    return CreateConstraints();

  return true;
}


NmDoubleStore::NmDoubleStore(NmStore &nm)
{
  nmID = nm.nmID;
}


NmDoubleStore::~NmDoubleStore()
{
}


void  NmDoubleStore::Init()
{
  NmDouble::Init();
}


bool  NmDoubleStore::Insert()
{
  PreparedStatement *stmtPtr = 0;

  wxString str = "INSERT INTO NmDouble (nmID, nmNr, ltA, ltB, nmOptional) "
                    "VALUES (?, ?, ?, ?, ?)";

  try
  {    
    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    stmtPtr->SetData(1, &nmID);
    stmtPtr->SetData(2, &nmNr);
    stmtPtr->SetData(3, ltA ? &ltA : NULL);
    stmtPtr->SetData(4, ltB ? &ltB : NULL);
    stmtPtr->SetData(5, &nmOptional);

    stmtPtr->Execute();
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


bool  NmDoubleStore::Remove()
{
  wxString  str = 
      "DELETE FROM NmDouble WHERE nmID = " + ltostr(nmID);

  try
  {
    ExecuteUpdate(str);
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


bool  NmDoubleStore::SelectAll()
{
  wxString str = 
      "SELECT nmID, nmNr, ltA, ltB, nmOptional FROM NmDouble WHERE nmID = " + ltostr(nmID);

  try
  {
    if (!ExecuteQuery(str))
      return false;

    BindCol(1, &nmID);
    BindCol(2, &nmNr);
    BindCol(3, &ltA);
    BindCol(4, &ltB);
    BindCol(5, &nmOptional);

    return true;
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}



// -----------------------------------------------------------------------
// Neue Tabelle in DB erzeugen
bool  NmStore::CreateTable()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  wxASSERT(connPtr);

  Statement *tmp = connPtr->CreateStatement();

  wxString  INTEGER  = connPtr->GetDataType(SQL_INTEGER);
  wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);

  wxString  sql = 
    "CREATE TABLE NmRec (         "
    "nmID        "+INTEGER+"      NOT NULL,  "
    "tmID        "+INTEGER+"      NOT NULL,  "
    "mtID        "+INTEGER+"      NOT NULL,  "
    "CONSTRAINT nmIdKey PRIMARY KEY (nmID)   "
	  ")";

  try
  {
    tmp->ExecuteUpdate(sql);

    NmSingleStore::CreateTable();
    NmDoubleStore::CreateTable();
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(sql, e);
    delete tmp;

    return false;
  }
  
  try
  {
    tmp->ExecuteUpdate(sql = "CREATE UNIQUE INDEX nmMtTmId ON NmRec (mtID, tmID)");
  }
  catch (SQLException &)
  {
  }

  tmp->Close();
  delete tmp;
  
  return true;
}


bool  NmStore::UpdateTable(long version)
{
  if (version <= 6)
    return CreateTable();
  
  if (version <= 60)
  {
    Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
    wxASSERT(connPtr);

    Statement *tmp = connPtr->CreateStatement();
    try
    {
      if (version <= 45)
        tmp->ExecuteUpdate("CREATE UNIQUE INDEX nmMtTmId ON NmRec (mtID, tmID)");
    }
    catch (SQLException &)
    {
    }

    tmp->Close();
    delete tmp;
  }  
  
  NmSingleStore::UpdateTable(version);
  NmDoubleStore::UpdateTable(version);
  
  return true;
}


bool  NmStore::CreateConstraints()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str;
  
  try
  {
    tmp->ExecuteUpdate("DROP TRIGGER nmInsertTrigger");
    tmp->ExecuteUpdate("DROP TRIGGER nmDeleteTrigger");
  }
  catch (SQLException &)
  {
  }
  
  try
  {
    tmp->ExecuteUpdate("ALTER TABLE NmRec DROP CONSTRAINT nmTmRef");
    tmp->ExecuteUpdate("ALTER TABLE NmRec DROP CONSTRAINT nmMtRef");
  }
  catch (SQLException &)
  {
  }

  try
  {
    tmp->ExecuteUpdate(str = 
        "CREATE OR ALTER TRIGGER nmInsertTrigger ON NMREC "
        "FOR INSERT AS "
        "UPDATE MtRec SET mtTimestamp = GETUTCDATE() "
        " WHERE mtID IN (SELECT mtID FROM inserted) ");
        
    tmp->ExecuteUpdate(str = 
        "CREATE OR ALTER TRIGGER nmDeleteTrigger ON NMREC "
        "FOR DELETE AS "
        "UPDATE MtRec SET mtTimestamp = GETUTCDATE() "
        " WHERE mtID IN (SELECT mtID FROM deleted) ");

    tmp->ExecuteUpdate(str = 
      "ALTER TABLE NmRec ADD CONSTRAINT nmTmRef "
      "FOREIGN KEY (tmID) REFERENCES TmRec (tmID) ON DELETE NO ACTION");

    tmp->ExecuteUpdate(str = 
      "ALTER TABLE NmRec ADD CONSTRAINT nmMtRef "
      "FOREIGN KEY (mtID) REFERENCES MtRec (mtID) ON DELETE CASCADE");

    NmSingleStore::CreateConstraints();
    NmDoubleStore::CreateConstraints();
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
  };

  delete tmp;
  return true;
}


bool  NmStore::UpdateConstraints(long version)
{
  if (version <= 62)
    return CreateConstraints();

  return true;
}


// -----------------------------------------------------------------------
// Konstruktor
NmStore::NmStore(Connection *connPtr)
       : StoreObj(connPtr)
{
}


NmStore::~NmStore()
{
}


void  NmStore::Init()
{
  NmRec::Init();
}


// -----------------------------------------------------------------------
bool  NmStore::Insert(const MtRec &mt, const TmEntry &tm)
{
  PreparedStatement *stmtPtr = 0;

  wxString str = "INSERT INTO NmRec (mtID, tmID, nmID) "
                    "VALUES (?, ?, ?)";

  try
  {    
    mtID = mt.mtID;
    tmID = tm.tmID;
    nmID = IdStore::ID(GetConnectionPtr());

    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    stmtPtr->SetData(1, &mtID);
    stmtPtr->SetData(2, &tmID);
    stmtPtr->SetData(3, &nmID);

    stmtPtr->Execute();

    for (short idxSingle = 0; idxSingle < nofSingles; idxSingle++)
    {
      // if (!nmSingle[idxSingle].ltA)
      //   continue;

      NmSingleStore  tmp(*this);
      tmp.nmNr = idxSingle+1;
      tmp.ltA = nmSingle[idxSingle].ltA;
      tmp.nmOptional = nmSingle[idxSingle].nmOptional;

      if (!tmp.Insert())
        return false;
    }

    for (short idxDouble = 0; idxDouble < nofDoubles; idxDouble++)
    {
      // if (!nmDouble[idxDouble].ltA || !nmDouble[idxDouble].ltB)
      //   continue;

      NmDoubleStore  tmp(*this);
      tmp.nmNr = idxDouble+1;
      tmp.ltA  = nmDouble[idxDouble].ltA;
      tmp.ltB  = nmDouble[idxDouble].ltB;
      tmp.nmOptional = nmDouble[idxDouble].nmOptional;

      if (!tmp.Insert())
        return false;
    }
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
  update.type = CRequest::UPDATE_NOMINATION;
  update.rec  = CRequest::MTREC;
  update.id   = mt.mtID;

  CTT32App::NotifyChange(update);

 return true;
}


bool  NmStore::Remove()
{
  wxString  str;

  try
  {
    NmSingleStore  tmpSingle(*this);
    if (!tmpSingle.Remove())
      return false;

    NmDoubleStore  tmpDouble(*this);
    if (!tmpDouble.Remove())
      return false;

    str = "DELETE FROM NmRec WHERE nmID = " + ltostr(nmID);
    ExecuteUpdate(str);
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

   // Notify Views
  CRequest update;
  update.type = CRequest::UPDATE_NOMINATION;
  update.rec  = CRequest::MTREC;
  update.id   = mtID;

  CTT32App::NotifyChange(update);

  return true;
}


// -----------------------------------------------------------------------
bool  NmStore::Next()
{
  Init();

  if (!StoreObj::Next())
    return false;

  NmSingleStore  tmpSingle(*this);
  tmpSingle.SelectAll();
  while (tmpSingle.Next())
    SetSingle(tmpSingle.nmNr-1, tmpSingle.ltA, tmpSingle.nmOptional);

  NmDoubleStore  tmpDouble(*this);
  tmpDouble.SelectAll();
  while (tmpDouble.Next())
    SetDoubles(tmpDouble.nmNr-1, tmpDouble.ltA, tmpDouble.ltB, tmpDouble.nmOptional);

  return true;
}


// -----------------------------------------------------------------------
bool  NmStore::SelectByMtTm(const MtRec &mt, const TmEntry &tm)
{
  wxString str = SelectString();
  str += " WHERE mtID = " + ltostr(mt.mtID);
  str += "   AND tmID = " + ltostr(tm.tmID);

  try
  {
    if (!ExecuteQuery(str))
      return false;

    BindRec();
    return true;
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


// -----------------------------------------------------------------------
wxString  NmStore::SelectString() const
{
  return "SELECT nmID, mtID, tmID FROM NmRec ";
}


void  NmStore::BindRec()
{
  BindCol(1, &nmID);
  BindCol(2, &mtID);
  BindCol(3, &tmID);
}