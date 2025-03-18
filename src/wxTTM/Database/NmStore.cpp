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
#include  "CpStore.h"
#include  "GrStore.h"
#include  "LtStore.h"
#include  "MdStore.h"
#include  "MtStore.h"
#include  "TmStore.h"
#include  "SyStore.h"

#include  "MtListStore.h"
#include  "MtEntryStore.h"
#include  "GrListStore.h"
#include  "TmEntryStore.h"

#include  "wxStringTokenizerEx.h"


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


// idx is 0-based
void  NmRec::SetSingle(short idx, long id, bool optional)
{
  if (idx >= nofSingles)
  {
    NmSingle * tmp = new NmSingle[idx + 1];
    memset(tmp, 0, (idx + 1) * sizeof(NmSingle));
    memcpy(tmp, nmSingle, nofSingles * sizeof(NmSingle));
    delete[] nmSingle;
    nmSingle = tmp;

    for (int i = nofSingles; i <= idx; ++i)
      nmSingle[i].isNew = true;

    nofSingles = idx + 1;
  }

  nmSingle[idx].ltA = id;
  nmSingle[idx].nmOptional = optional;
}


// idx is 0-based
void  NmRec::SetDoubles(short idx, long idA, long idB, bool optional)
{
  if (idx >= nofDoubles)
  {
    NmDouble * tmp = new NmDouble[idx + 1];
    memset(tmp, 0, (idx + 1) * sizeof(NmDouble));
    memcpy(tmp, nmDouble, nofDoubles * sizeof(NmDouble));
    delete[] nmDouble;
    nmDouble = tmp;

    for (int i = nofDoubles; i <= idx; ++i)
      nmDouble[i].isNew = true;

    nofDoubles = idx + 1;
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


NmSingleStore::NmSingleStore(NmStore &nm) : StoreObj(nm.GetConnectionPtr())
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


bool  NmSingleStore::Update()
{
  PreparedStatement* stmtPtr = 0;

  wxString str = "UPDATE NmSingle SET ltA = ?, nmOptional = ? WHERE nmID = ? AND nmNr= ?";

  try
  {
    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    stmtPtr->SetData(1, ltA ? &ltA : NULL);
    stmtPtr->SetData(2, &nmOptional);
    stmtPtr->SetData(3, &nmID);
    stmtPtr->SetData(4, &nmNr);

    stmtPtr->Execute();
  }
  catch (SQLException& e)
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
  wxString  str = "DELETE FROM NmSingle WHERE nmID = " + ltostr(nmID) + " AND nmNr = " + ltostr(nmNr);

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


NmDoubleStore::NmDoubleStore(NmStore &nm) : StoreObj(nm.GetConnectionPtr())
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


bool  NmDoubleStore::Update()
{
  PreparedStatement* stmtPtr = 0;

  wxString str = "UPDATE NmDouble SET ltA = ?, ltB = ?, nmOptional = ? WHERE nmID = ? AND nmNR = ?";

  try
  {
    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    stmtPtr->SetData(1, ltA ? &ltA : NULL);
    stmtPtr->SetData(2, ltB ? &ltB : NULL);
    stmtPtr->SetData(3, &nmOptional);
    stmtPtr->SetData(4, &nmID);
    stmtPtr->SetData(5, &nmNr);

    stmtPtr->Execute();
  }
  catch (SQLException& e)
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
      "DELETE FROM NmDouble WHERE nmID = " + ltostr(nmID) + " AND nmNr = " + ltostr(nmNr);

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
  return Insert(mt.mtID, tm.tmID);
}


bool NmStore::Insert(long mtID, long tmID)
{
  PreparedStatement *stmtPtr = 0;

  wxString str = "INSERT INTO NmRec (mtID, tmID, nmID) "
                    "VALUES (?, ?, ?)";

  try
  {    
    mtID = mtID;
    tmID = tmID;
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
      NmSingleStore  tmp(*this);
      tmp.nmNr = idxSingle+1;
      tmp.ltA = nmSingle[idxSingle].ltA;
      tmp.nmOptional = nmSingle[idxSingle].nmOptional;

      if (!tmp.Insert())
        return false;

      nmSingle[idxSingle].isNew = false;
    }

    for (short idxDouble = 0; idxDouble < nofDoubles; idxDouble++)
    {
      NmDoubleStore  tmp(*this);
      tmp.nmNr = idxDouble+1;
      tmp.ltA  = nmDouble[idxDouble].ltA;
      tmp.ltB  = nmDouble[idxDouble].ltB;
      tmp.nmOptional = nmDouble[idxDouble].nmOptional;

      if (!tmp.Insert())
        return false;

      nmDouble[idxDouble].isNew = false;
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
  update.id   = mtID;

  CTT32App::NotifyChange(update);

 return true;
}


bool  NmStore::Update()
{
  // Does not change match or team nor type of entries, only the players
  for (short idxSingle = 0; idxSingle < nofSingles; idxSingle++)
  {
    NmSingleStore  tmp(*this);
    tmp.nmNr = idxSingle + 1;
    tmp.ltA = nmSingle[idxSingle].ltA;
    tmp.nmOptional = nmSingle[idxSingle].nmOptional;

    bool ret = true;

    if (nmSingle[idxSingle].isNew)
      ret = tmp.Insert();
    else 
      ret = tmp.Update();

    if (!ret)
      return false;
     
    nmSingle[idxSingle].isNew = false;
  }

  for (short idxDouble = 0; idxDouble < nofDoubles; idxDouble++)
  {
    NmDoubleStore  tmp(*this);
    tmp.nmNr = idxDouble + 1;
    tmp.ltA = nmDouble[idxDouble].ltA;
    tmp.ltB = nmDouble[idxDouble].ltB;
    tmp.nmOptional = nmDouble[idxDouble].nmOptional;

    bool ret = true;

    if (nmDouble[idxDouble].isNew)
      ret = tmp.Insert();
    else
      ret = tmp.Update();

    if (!ret)
      return false;

    nmSingle[idxDouble].isNew = false;
  }

  // Notify Views
  CRequest update;
  update.type = CRequest::UPDATE_NOMINATION;
  update.rec = CRequest::MTREC;
  update.id = mtID;

  CTT32App::NotifyChange(update);

  return true;
}

bool  NmStore::Remove()
{
  wxString  str;

  try
  {
    // NmSingle and NmDouble will be removed via constraints
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
bool  NmStore::SelectByMtTm(const MtRec& mt, const TmEntry& tm)
{
  return SelectByMtTm(mt.mtID, tm.tmID);
}


bool NmStore::SelectByMtTm(long mtID, long tmID)
{
  wxString str = SelectString();
  str += " WHERE mtID = " + ltostr(mtID);
  str += "   AND tmID = " + ltostr(tmID);

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

// -----------------------------------------------------------------------
// Import / Export
bool NmStore::Import(wxTextBuffer& is)
{
  long version = 1;

  wxString line = is.GetFirstLine();

  // Check header
  if (!CheckImportHeader(line, "#LINEUPS", version))
  {
    is.Close();
    if (!infoSystem.Question(_("First comment is not %s but \"%s\". Continue anyway?"), wxT("#LINEUPS"), line.wx_str()))
      return false;
  }

  if (version > GrStore::GetMaxSupportedExportVersion())
  {
    infoSystem.Error(_("Version %d of import file is not supported"), version);
    return false;
  }

  // Read all CP, SY, and MD: there are not so many
  std::map<wxString, long> cpMap;
  std::map<wxString, long> syMap;
  std::map<wxString, long> mdMap;

  Connection* connPtr = TTDbse::instance()->GetNewConnection();

  connPtr->StartTransaction();

  // HACK: Die Variablen duerfen nicht laenger leben als connPtr [
  {
    CpStore cp(connPtr);
    GrStore gr(connPtr);
    SyStore sy(connPtr);
    MdStore md(connPtr);
    MtStore mt(connPtr);
    NmStore nm(connPtr);
    LtStore ltA(connPtr), ltB(connPtr);

    cp.SelectAll();
    while (cp.Next())
      cpMap[cp.cpName] = cp.cpID;
    cp.Close();

    cpMap[wxEmptyString] = 0;

    sy.SelectAll();
    while (sy.Next())
      syMap[sy.syName] = sy.syID;
    sy.Close();

    syMap[wxEmptyString] = 0;

    std::set<long> nmaSet, nmxSet;

    bool ax = false;

    // cpName, grName, sygrName, symtName, mtRound, mtChance, mtMatch, mtLeg, mtReverse,
    // ax, nmNr, nmType, nmOptional, plAplNr, plBplNr
    for (; !is.Eof(); line = is.GetNextLine())
    {
      CTT32App::ProgressBarStep();

      if (line.GetChar(0) == '#')
        continue;

      // # Event; Group; Round; Chance; Match; Reverse; A / X; Nr; Optional; Type; Player A; Player B; "
      
      wxStringTokenizerEx tokens(line, wxT(",;  "));

      wxString strCpName = tokens.GetNextToken();
      wxString strGrName = tokens.GetNextToken();
      wxString strMtRound = tokens.GetNextToken();
      wxString strMtChance = tokens.GetNextToken();
      wxString strMtMatch = tokens.GetNextToken();
      wxString strMtReverse = tokens.GetNextToken();
      wxString strAX = tokens.GetNextToken().Upper();;
      wxString strNmNr = tokens.GetNextToken();
      wxString strNmOptional = tokens.GetNextToken();
      wxString strNmType = tokens.GetNextToken().Upper();
      wxString strPlA = tokens.GetNextToken();
      wxString strPlB = tokens.GetNextToken();

      if (strCpName.IsEmpty() || strGrName.IsEmpty() ||
          strMtRound.IsEmpty() || strMtMatch.IsEmpty())
        continue;

      if (wxStrcoll(strCpName, cp.cpName))
      {
        // Naechster WB
        cp.SelectByName(strCpName);
        if (!cp.Next() || cp.cpType != CP_TEAM)
          continue;

        // Ausserdem Gruppe lesen
        gr.Init();
      }

      if (wxStrcoll(strGrName, gr.grName))
      {
        // Naechste Gruppe bei gleichem WB
        gr.SelectByName(strGrName, cp);
        if (!gr.Next())
          continue;
      }

      // Event aufsetzen
      MtStore::MtEvent  mtEvent;
      mtEvent.grID = gr.grID;
      mtEvent.mtRound = _strtos(strMtRound);
      mtEvent.mtMatch = _strtos(strMtMatch);
      mtEvent.mtChance = _strtos(strMtChance);

      if (!mt.mtID ||
        mt.mtEvent.grID != mtEvent.grID ||
        mt.mtEvent.mtRound != mtEvent.mtRound ||
        mt.mtEvent.mtChance != mtEvent.mtChance ||
        mt.mtEvent.mtMatch != mtEvent.mtMatch ||
        (mt.mtReverse != 0) != (strMtReverse == "1") ||
        ax != (strAX == "X"))
      {
        // Next match, store old nm
        if (nm.nmID)
          nm.Update();

        if (!mt.SelectByEvent(mtEvent) || !mt.Next())
          continue;

        mt.Close();

        ax = (strAX == "X"); // true if for team X

        nm.SelectByMtTm(mt.mtID, ax ? mt.tmX : mt.tmA);
        if (nm.Next())        
        {
          nm.Close();
          if (ax && nmxSet.find(nm.nmID) == nmxSet.end())
          {
            nm.Remove();
            nm.Init();
          }
          else if (!ax && nmaSet.find(nm.nmID) == nmaSet.end())
          {
            nm.Remove();
            nm.Init();
          }
        }
        else
        {
          nm.Close();
          nm.Init();
        }

        if (!nm.nmID)
          nm.Insert(mt.mtID, ax ? mt.tmX : mt.tmA);
      }

      if (ax)
        nmxSet.insert(nm.nmID);
      else
        nmaSet.insert(nm.nmID);

      mt.mtReverse = (strMtReverse == "1") ? 1 : 0;
      mt.UpdateReverseFlag();

      int nmNr;
      strNmNr.ToInt(&nmNr);

      bool nmOptional = (strNmOptional == "1");

      if (strNmType == "S")
      {
        int plAplNr = 0;
        if (!strPlA.IsEmpty() && strPlA.ToInt(&plAplNr))
        {
          ltA.SelectByCpPlNr(cp, plAplNr);
          ltA.Next();
          ltA.Close();
        }

        nm.SetSingle(nmNr-1, ltA.ltID, nmOptional);
      }
      else
      {
        int plAplNr = 0, plBplNr = 0;
        if (!strPlA.IsEmpty() && strPlA.ToInt(&plAplNr))
        {
          ltA.SelectByCpPlNr(cp, plAplNr);
          ltA.Next();
          ltA.Close();
        }
        if (!strPlB.IsEmpty() && strPlB.ToInt(&plBplNr))
        {
          ltB.SelectByCpPlNr(cp, plBplNr);
          ltB.Next();
          ltB.Close();
        }

        nm.SetDoubles(nmNr-1, ltA.ltID, ltB.ltID, nmOptional);
      }
    }

    if (nm.nmID)
      nm.Update();
  } // end HACK ]

  connPtr->Commit();
  delete connPtr;

  return true;
}


bool NmStore::Export(wxTextBuffer& os, short cpType, const std::vector<long>& idList, bool append, long version)
{
  if (!append)
  {
    os.AddLine(wxString::Format("#LINEUPS %d", version));
    os.AddLine("# Event; Group; Round; Chance; Match; Reverse; A/X; Nr; Optional; Type; Player A; Player B;");
  }

  // cpName, grName, sygrName, symtName, mtRound, mtMatch, mtLeg, mtReverse, 
  // ax, nmNr, nmType, nmOptional, plAplNr, plBplNr
  wxString sql =
      "SELECT cp.cpName AS cpName, gr.grName AS grName, NULL AS symtName, "
      "       mtRound, mtChance, mtMatch, mtReverse, 0 AS ax,    "
      "       nm.nmNr, nm.nmOptional, nm.nmType, nm.plAplNr, nm.plBplNr "
      "  FROM NmEntryList nm "
      "       INNER JOIN MtTeamList mt ON nm.mtID = mt.mtID AND nm.tmID = mt.tmAtmID "
      "       INNER JOIN GrList gr ON mt.grID = gr.grID "
      "       INNER JOIN CpList cp ON gr.cpID = cp.cpID "
      "       LEFT OUTER JOIN SyList sy ON gr.syID = sy.syID "
      " WHERE gr.grID IN (" + StoreObj::ltostr(idList) + ") AND cp.cpType = 4 "
      "UNION "
      "SELECT cp.cpName AS cpName, gr.grName AS grName, NULL AS symtName, "
      "       mtRound, mtChance, mtMatch, mtReverse, 1 AS ax,    "
      "       nm.nmNr, nm.nmOptional, nm.nmType, nm.plAplNr, nm.plBplNr "
      "  FROM NmEntryList nm "
      "       INNER JOIN MtTeamList mt ON nm.mtID = mt.mtID AND nm.tmID = mt.tmXtmID "
      "       INNER JOIN GrList gr ON mt.grID = gr.grID "
      "       INNER JOIN CpList cp ON gr.cpID = cp.cpID "
      "       LEFT OUTER JOIN SyList sy ON gr.syID = sy.syID "
      " WHERE gr.grID IN (" + StoreObj::ltostr(idList) + ") AND cp.cpType = 4 "
   // 
      "ORDER BY cpName, grName, mtRound, mtChance, mtMatch, ax, nm.nmNr "
    ;

  Connection* connPtr = TTDbse::instance()->GetDefaultConnection();
  Statement* stmtPtr = connPtr->CreateStatement();
  ResultSet* resPtr = 0;


  try
  {
    if (stmtPtr->Execute(sql))
      resPtr = stmtPtr->GetResultSet(false);
  }
  catch (SQLException& e)
  {
    infoSystem.Exception(sql, e);
    return false;
  }

  wxChar cpName[9], grName[9], symtName[9];
  short  mtRound, mtChance, mtMatch, mtReverse, nmNr, nmType, nmOptional, ax;
  short  plAplNr, plBplNr; 

  int idx = 0;

  resPtr->BindCol(++idx, cpName, sizeof(cpName));
  resPtr->BindCol(++idx, grName, sizeof(grName));
  resPtr->BindCol(++idx, symtName, sizeof(symtName));
  resPtr->BindCol(++idx, &mtRound);
  resPtr->BindCol(++idx, &mtChance);
  resPtr->BindCol(++idx, &mtMatch);
  resPtr->BindCol(++idx, &mtReverse);
  resPtr->BindCol(++idx, &ax);
  resPtr->BindCol(++idx, &nmNr);
  resPtr->BindCol(++idx, &nmOptional);
  resPtr->BindCol(++idx, &nmType);
  resPtr->BindCol(++idx, &plAplNr);
  resPtr->BindCol(++idx, &plBplNr);

  while (resPtr->Next())
  {
    wxString line;

    // If optional and player A was NULL, then skip
    if (nmOptional && resPtr->WasNull(13))
      continue;

    line << cpName << ";" << grName << ";" << mtRound << ";" << mtChance << ";" << mtMatch << ";"
         << mtReverse << ";"
         << (ax == 0 ? 'A' : 'X') << ";" << nmNr << ";" << nmOptional << ";" << (nmType == 1 ? 'S' : 'D') << ";"
         << (resPtr->WasNull(12) ? "" : ltostr(plAplNr)) << ";"
         << (resPtr->WasNull(13) || nmType == 1 ? "" : ltostr(plBplNr)) << ";"
      ;

    os.AddLine(line);
  }

  delete resPtr;
  delete stmtPtr;

  return true;
}