/* Copyright (C) 2020 Christoph Theis */

// Tabelle der Spiele
// TODO: Import von Team-Events
#include  "stdafx.h"
#include  "MtStore.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "ResultSet.h"
#include  "SQLException.h"
#include  "InfoSystem.h"
#include  "Request.h"

#include  "IdStore.h"
#include  "CpStore.h"
#include  "GrStore.h"
#include  "StStore.h"
#include  "TmStore.h"

#include  "MtListStore.h"
#include  "MtEntryStore.h"
#include  "GrListStore.h"

#include  "wxStringTokenizerEx.h"

#include  <fstream>



// -----------------------------------------------------------------------
MtSet & MtSet::operator=(const MtSet &rec)
{
  if (this != &rec)
    memcpy(this, &rec, sizeof(MtSet));

  return *this;
}


MtMatch & MtMatch::operator=(const MtMatch &rec)
{
  if (this != &rec)
    memcpy(this, &rec, sizeof(MtMatch));

  return *this;
}


// -----------------------------------------------------------------------
MtRec & MtRec::operator=(const MtRec &rec)
{
  if (this != &rec)
    memcpy(this, &rec, sizeof(MtRec));

  return *this;
}


// -----------------------------------------------------------------------
short  MtRec::QryWinnerAX() const
{
  // Fehlen beide Spieler? Dann gibt es keinen Sieger
  if (!stA && !stX)
    return 0;

  // Walkover
  if (mtWalkOverA)
    return (mtWalkOverX ? 0 : -1);
  if (mtWalkOverX)
    return (mtWalkOverA ? 0 : +1);

  // Sieger eines Spieles
  if (mtResA * 2 > (mtMatches > 1 && mtEvent.mtMS == 0 ? mtMatches : mtBestOf))
    return +1;
  else if (mtResX * 2 > (mtMatches > 1 && mtEvent.mtMS == 0 ? mtMatches : mtBestOf))
    return -1;

  // Freilose (bei zweien gewinnt das obere)
  if (IsXBye())
    return +1;
  else if (IsABye())
    return -1;

  // Sonstige Zweifelsfaelle
  return 0;
}


bool MtRec::IsFinished() const
{
  if (QryWinnerAX() != 0)
    return true;
    
  // Unentschieden in Mannschaftsspielen
  if (mtMatches > 1 && mtEvent.mtMS == 0 && (mtResA + mtResX) == mtMatches)
    return true;

  // Doppeltes w/o etc.
  if (mtWalkOverA || mtWalkOverX)
    return true;

  if (mtInjuredA || mtInjuredX)
    return true;

  if (mtDisqualifiedA || mtDisqualifiedX)
    return true;
    
  return false;
}


// -----------------------------------------------------------------------
bool  MtSetStore::CreateTable()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  wxASSERT(connPtr);

  Statement *tmp = connPtr->CreateStatement();

  wxString  INTEGER  = connPtr->GetDataType(SQL_INTEGER);
  wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);

  wxString  sql = 
    "CREATE TABLE MtSet (  "
    "mtID        "+INTEGER+"      NOT NULL,  "
    "mtMS        "+SMALLINT+"     NOT NULL DEFAULT 0, "
    "mtSet       "+SMALLINT+"     NOT NULL,  "
    "mtResA      "+SMALLINT+"     NOT NULL,  "
    "mtResX      "+SMALLINT+"     NOT NULL,  "
    "CONSTRAINT mtSetKey PRIMARY KEY (mtID, mtMS, mtSet) "

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
    tmp->ExecuteUpdate(sql = "CREATE INDEX mtSetIdKey ON MtSet (mtID)");
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(sql, e);
  };

  tmp->Close();
  delete tmp;
  
  return true;
}


bool  MtSetStore::UpdateTable(long version)
{
  return true;
}


bool  MtSetStore::CreateConstraints()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str;
  
  try
  {
    tmp->ExecuteUpdate("ALTER TABLE MtSet DROP CONSTRAINT mtSetIdRef");
  }
  catch (SQLException &)
  {
  }

  try
  {
    tmp->ExecuteUpdate(str = 
      "ALTER TABLE MtSet ADD CONSTRAINT mtSetIdRef "
      "FOREIGN KEY (mtID) REFERENCES MtRec (mtID) ON DELETE CASCADE");
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
  };

  delete tmp;
  return true;
}


bool  MtSetStore::UpdateConstraints(long version)
{
  if (version < 112)
    return CreateConstraints();
    
  return true;
}


// -----------------------------------------------------------------------
MtSetStore::MtSetStore(Connection *ptr)
  : StoreObj(ptr)
{
  Init();
}


MtSetStore::MtSetStore(const MtRec &mt, Connection *ptr)
          : StoreObj(ptr)
{
  Init();
  mtID = mt.mtID;
}


// -----------------------------------------------------------------------
void  MtSetStore::Init()
{
  long tmp = mtID;
  MtSet::Init();
  mtID = tmp;
}


bool  MtSetStore::SelectAll(short ms)
{
  // XXX: PreparedStatement
  wxString  str = 
    "SELECT mtID, mtMS, mtSet, mtResA, mtResX "
    "  FROM MtSet WHERE mtID = " + ltostr(mtID);

  if (ms != 0)
    str += " AND mtMS = " + ltostr(ms) + " ORDER BY mtSet";
  else
    str += " ORDER BY mtMS, mtSet";

  try
  {
    ExecuteQuery(str);

    BindCol(1, &mtID);
    BindCol(2, &mtMS);
    BindCol(3, &mtSet);
    BindCol(4, &mtResA);
    BindCol(5, &mtResX);
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


bool  MtSetStore::SelectAll(const std::set<long> &ids)
{
  // XXX: PreparedStatement
  wxString  str =
    "SELECT mtID, mtMS, mtSet, mtResA, mtResX "
    "  FROM MtSet WHERE mtID IN (" + ltostr(ids) + ") ";

    str += " ORDER BY mtMS, mtSet";

  try
  {
    ExecuteQuery(str);

    BindCol(1, &mtID);
    BindCol(2, &mtMS);
    BindCol(3, &mtSet);
    BindCol(4, &mtResA);
    BindCol(5, &mtResX);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


// -----------------------------------------------------------------------
bool  MtMatchStore::CreateTable()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  wxASSERT(connPtr);

  Statement *tmp = connPtr->CreateStatement();

  wxString  INTEGER  = connPtr->GetDataType(SQL_INTEGER);
  wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);

  wxString  sql = 
    "CREATE TABLE MtMatch (  "
    "mtID            "+INTEGER+"      NOT NULL,  "
    "mtMS            "+SMALLINT+"     NOT NULL DEFAULT 0, "
    "mtResA          "+SMALLINT+"     NOT NULL,  "
    "mtResX          "+SMALLINT+"     NOT NULL,  "
    "mtWalkOverA     "+SMALLINT+"     NOT NULL DEFAULT 0, "
    "mtWalkOverX     "+SMALLINT+"     NOT NULL DEFAULT 0, "
    "mtDisqualifiedA "+SMALLINT+"     NOT NULL DEFAULT 0, "
    "mtDisqualifiedX "+SMALLINT+"     NOT NULL DEFAULT 0, "
    "mtInjuredA      "+SMALLINT+"     NOT NULL DEFAULT 0, "
    "mtInjuredX      "+SMALLINT+"     NOT NULL DEFAULT 0, "
    // "mtNotPlayed "+SMALLINT+"     DEFAULT 0, "
    "CONSTRAINT mtMatchKey PRIMARY KEY (mtID, mtMS) "
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
    tmp->ExecuteUpdate(sql = "CREATE INDEX mtMatchIdKey ON MtMatch (mtID)");
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(sql, e);
  };

  tmp->Close();
  delete tmp;
  
  return true;
}


bool  MtMatchStore::UpdateTable(long version)
{
  if (version < 10)
    return CreateTable();

  if (version < 99)
  {
    Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
    wxASSERT(connPtr);

    Statement *tmp = connPtr->CreateStatement();

    wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);
    
    wxString sql = 
        "ALTER TABLE MtMatch ADD "
          "mtWalkOverA " + SMALLINT + " NOT NULL DEFAULT 0, "
          "mtWalkOverX " + SMALLINT + " NOT NULL DEFAULT 0  ";
    
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

    sql = "UPDATE MtMatch SET mtWalkOverA = 1 "
      "WHERE MtMatch.mtMS > 0 AND MtMatch.mtID IN (SELECT mtID FROM MtRec WHERE MtRec.mtWalkOverA > 0) AND "
      "      NOT EXISTS (SELECT * FROM MtSet WHERE MtSet.mtSet > 0 AND MtSet.mtMS = MtMatch.mtMS AND mtResA > 0)";

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

    sql = "UPDATE MtMatch SET mtWalkOverX = 1 "
      "WHERE MtMatch.mtMS > 0 AND MtMatch.mtID IN (SELECT mtID FROM MtRec WHERE MtRec.mtWalkOverX > 0) AND "
      "      NOT EXISTS (SELECT * FROM MtSet WHERE MtSet.mtSet > 0 AND MtSet.mtMS = MtMatch.mtMS AND mtResX > 0)";

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

    delete tmp;
  }

  if (version < 104)
  {
    Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
    wxASSERT(connPtr);

    Statement *tmp = connPtr->CreateStatement();

    wxString SMALLINT = connPtr->GetDataType(SQL_SMALLINT);

    wxString sql = 
        "ALTER TABLE MtMatch ADD "
          "mtInjuredA      " + SMALLINT + " NOT NULL DEFAULT 0, "
          "mtInjuredX      " + SMALLINT + " NOT NULL DEFAULT 0, "
          "mtDisqualifiedA " + SMALLINT + " NOT NULL DEFAULT 0, "
          "mtDisqualifiedX " + SMALLINT + " NOT NULL DEFAULT 0  ";

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
      // Diese Spalten duerfen nie NULL sein
      tmp->ExecuteUpdate(sql = "UPDATE MtMatch SET mtMS = 0 WHERE mtMS IS NULL");
      tmp->ExecuteUpdate(sql = "UPDATE MtMatch SET mtWalkOverA = 0 WHERE mtWalkOverA IS NULL");
      tmp->ExecuteUpdate(sql = "UPDATE MtMatch SET mtWalkOverX = 0 WHERE mtWalkOverX IS NULL");

      tmp->ExecuteUpdate(sql = "ALTER TABLE MtMatch ALTER COLUMN mtMS " + SMALLINT + " NOT NULL");
      tmp->ExecuteUpdate(sql = "ALTER TABLE MtMatch ALTER COLUMN mtWalkOverA " + SMALLINT + " NOT NULL");
      tmp->ExecuteUpdate(sql = "ALTER TABLE MtMatch ALTER COLUMN mtWalkOverX " + SMALLINT + " NOT NULL");
    }
    catch (SQLException &e)
    {
      infoSystem.Exception(sql, e);

      delete tmp;

      return false;
    }

    delete tmp;
  }
    
  return true;
}


bool  MtMatchStore::CreateConstraints()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str;
  
  try
  {
    tmp->ExecuteUpdate("ALTER TABLE MtMatch DROP CONSTRAINT mtMatchIdRef");
  }
  catch (SQLException &)
  {
  }

  try
  {
    tmp->ExecuteUpdate(str = 
      "ALTER TABLE MtMatch ADD CONSTRAINT mtMatchIdRef "
      "FOREIGN KEY (mtID) REFERENCES MtRec (mtID) ON DELETE CASCADE");
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
    delete tmp;
    return false;
  };

  delete tmp;
  return true;
}


bool  MtMatchStore::UpdateConstraints(long version)
{
  if (version < 104)
    return CreateConstraints();
   
  return true;
}


// -----------------------------------------------------------------------
MtMatchStore::MtMatchStore(const MtRec &mt, Connection *ptr)
            : StoreObj(ptr)
{
  Init();
  mtID = mt.mtID;
}



// -----------------------------------------------------------------------
void  MtMatchStore::Init()
{
  long tmp = mtID;
  MtMatch::Init();
  mtID = tmp;
}


bool  MtMatchStore::SelectAll(short ms)
{
  wxString  str = 
    "SELECT mtID, mtMS, mtResA, mtResX,       "
    "       mtWalkOverA, mtWalkOverX,         "
    "       mtInjuredA, mtInjuredX,           "
    "       mtDisqualifiedA, mtDisqualifiedX  "
    "  FROM MtMatch WHERE mtID = " + ltostr(mtID);

  if (ms != 0)
    str += " AND mtMS = " + ltostr(ms);
  else
    str += " ORDER BY mtMS";

  try
  {
    ExecuteQuery(str);

    int col = 0;

    BindCol(++col, &mtID);
    BindCol(++col, &mtMS);
    BindCol(++col, &mtResA);
    BindCol(++col, &mtResX);
    BindCol(++col, &mtWalkOverA);
    BindCol(++col, &mtWalkOverX);
    BindCol(++col, &mtInjuredA);
    BindCol(++col, &mtInjuredX);
    BindCol(++col, &mtDisqualifiedA);
    BindCol(++col, &mtDisqualifiedX);
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


// -----------------------------------------------------------------------
bool  MtStore::CreateTable()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  wxASSERT(connPtr);

  Statement *tmp = connPtr->CreateStatement();

  wxString  INTEGER   = connPtr->GetDataType(SQL_INTEGER);
  wxString  SMALLINT  = connPtr->GetDataType(SQL_SMALLINT);
  wxString  TIMESTAMP = connPtr->GetDataType(SQL_TIMESTAMP);

  wxString  sql = 
    "CREATE TABLE MtRec (         "
    "mtID            "+INTEGER+"      NOT NULL,  "
    "mtNr            "+INTEGER+"      NOT NULL,  "
    "stA             "+INTEGER+",     "
    "stX             "+INTEGER+",     "
    "grID            "+INTEGER+"      NOT NULL,  "
    "mtRound         "+SMALLINT+"     NOT NULL,  "
    "mtMatch         "+SMALLINT+"     NOT NULL,  "
    "mtChance        "+SMALLINT+"     NOT NULL,  "
    "mtDateTime      "+TIMESTAMP+",   "
    "mtTable         "+SMALLINT+",    "
    "mtUmpire        "+INTEGER+",     "
    "mtUmpire2       "+INTEGER+",     "
    "mtPrinted       "+SMALLINT+"     NOT NULL DEFAULT 0, "
    "mtChecked       "+SMALLINT+"     NOT NULL DEFAULT 0, "
    "mtReverse       "+SMALLINT+"     NOT NULL DEFAULT 0, "
    "mtMatches       "+SMALLINT+"     NOT NULL,  "
    "mtBestOf        "+SMALLINT+"     NOT NULL,  "
    "mtWalkOverA     "+SMALLINT+"     NOT NULL DEFAULT 0, "
    "mtWalkOverX     "+SMALLINT+"     NOT NULL DEFAULT 0, "
    "mtInjuredA      "+SMALLINT+"     NOT NULL DEFAULT 0, "
    "mtInjuredX      "+SMALLINT+"     NOT NULL DEFAULT 0, "
    "mtDisqualifiedA "+SMALLINT+"     NOT NULL DEFAULT 0, "
    "mtDisqualifiedX "+SMALLINT+"     NOT NULL DEFAULT 0, "
    "mtResA          "+SMALLINT+"     NOT NULL DEFAULT 0, "
    "mtResX          "+SMALLINT+"     NOT NULL DEFAULT 0, "
    "mtTimestamp     "+TIMESTAMP+"    DEFAULT GETUTCDATE()"
    "CONSTRAINT mtIdKey PRIMARY KEY (mtID)   "
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
    tmp->ExecuteUpdate(sql = "CREATE UNIQUE INDEX mtNrKey ON MtRec (mtNr)");
    tmp->ExecuteUpdate(sql = "CREATE INDEX mtStAKey ON MtRec (stA)");
    tmp->ExecuteUpdate(sql = "CREATE INDEX mtStXKey ON MtRec (stX)");
    tmp->ExecuteUpdate(sql = "CREATE INDEX mtDataTimeKey ON MtRec (mtDateTime)");
    tmp->ExecuteUpdate(sql = "CREATE INDEX mtEventKey ON MtRec (grID, mtRound, mtMatch, mtChance)");
    // Mit diesem Schluessel dauert ein SELCECT * FROM MtDoubleList WHERE grID = ...
    // gleichmal 20 Sekunden statt 1 Sekunde!
    // tmp->ExecuteUpdate(sql = "CREATE INDEX mtGrKey  ON MtRec (grID)");
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(sql, e);
  };

  MtSetStore::CreateTable();

  MtMatchStore::CreateTable();
  
  UpdateStoredProcedure(0);

  tmp->Close();
  delete tmp;
  
  return true;
}


bool  MtStore::UpdateTable(long version)
{
  if (version == 0)
    return CreateTable();

  MtSetStore::UpdateTable(version);

  MtMatchStore::UpdateTable(version);
  
  if (version < 60)
  {
    Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
    Statement *stmtPtr = connPtr->CreateStatement();

    wxString  TIMESTAMP = connPtr->GetDataType(SQL_TIMESTAMP);
    wxString str;
    
    try 
    {      
      str = "ALTER TABLE MtRec ADD mtTimestamp "+TIMESTAMP+" DEFAULT GETUTCDATE()";
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
  
  if (version < 104)
  {
    Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
    wxASSERT(connPtr);

    Statement *tmp = connPtr->CreateStatement();

    wxString SMALLINT = connPtr->GetDataType(SQL_SMALLINT);

    wxString sql = 
        "ALTER TABLE MtRec ADD "
          "mtInjuredA      " + SMALLINT + " NOT NULL DEFAULT 0, "
          "mtInjuredX      " + SMALLINT + " NOT NULL DEFAULT 0, "
          "mtDisqualifiedA " + SMALLINT + " NOT NULL DEFAULT 0, "
          "mtDisqualifiedX " + SMALLINT + " NOT NULL DEFAULT 0  ";

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
      // Diese Spalten duerfen nie NULL sein
      tmp->ExecuteUpdate(sql = "UPDATE MtRec SET mtPrinted = 0 WHERE mtPrinted IS NULL");
      tmp->ExecuteUpdate(sql = "UPDATE MtRec SET mtReverse = 0 WHERE mtReverse IS NULL");
      tmp->ExecuteUpdate(sql = "UPDATE MtRec SET mtWalkOverA = 0 WHERE mtWalkOverA IS NULL");
      tmp->ExecuteUpdate(sql = "UPDATE MtRec SET mtWalkOverX = 0 WHERE mtWalkOverX IS NULL");

      // DROP CONTRAINT braucht den Namen, den vergebe ich aber nicht explizit
      // Also in den Sys-Tabellen suchen
      Statement *dstmt = connPtr->CreateStatement();
      sql = 
          "SELECT d.name, c.name "
          "  FROM sys.tables t "
          "       INNER JOIN sys.default_constraints d ON d.parent_object_id = t.object_id "
          "       INNER JOIN sys.columns c ON c.object_id = t.object_id AND c.column_id = d.parent_column_id "
          " WHERE t.name = 'MtRec' AND c.name IN ('mtPrinted', 'mtReverse', 'mtWalkOverA', 'mtWalkOverX') "
      ;
      ResultSet *rs = dstmt->ExecuteQuery(sql);
      std::map<wxString, wxString> dmap;
      while (rs->Next())
      {
        wxChar d[128];
        wxChar c[128];

        rs->GetData(1, d, 128);
        rs->GetData(2, c, 128);

        dmap[wxString(d)] = wxString(c);
      }

      dstmt->Close();
      delete rs;
      delete dstmt;

      for (auto it : dmap)
        tmp->ExecuteQuery("ALTER TABLE MtRec DROP CONSTRAINT " + it.first);


      tmp->ExecuteUpdate(sql = "ALTER TABLE MtRec ADD DEFAULT 0 FOR mtPrinted");
      tmp->ExecuteUpdate(sql = "ALTER TABLE MtRec ADD DEFAULT 0 FOR mtReverse");
      tmp->ExecuteUpdate(sql = "ALTER TABLE MtRec ADD DEFAULT 0 FOR mtWalkOverA");
      tmp->ExecuteUpdate(sql = "ALTER TABLE MtRec ADD DEFAULT 0 FOR mtWalkOverX");

      tmp->ExecuteUpdate(sql = "ALTER TABLE MtRec ALTER COLUMN mtPrinted " + SMALLINT + " NOT NULL");
      tmp->ExecuteUpdate(sql = "ALTER TABLE MtRec ALTER COLUMN mtReverse " + SMALLINT + " NOT NULL");
      tmp->ExecuteUpdate(sql = "ALTER TABLE MtRec ALTER COLUMN mtWalkOverA " + SMALLINT + " NOT NULL");
      tmp->ExecuteUpdate(sql = "ALTER TABLE MtRec ALTER COLUMN mtWalkOverX " + SMALLINT + " NOT NULL");
    }
    catch (SQLException &e)
    {
      infoSystem.Exception(sql, e);

      delete tmp;

      return false;
    }

    delete tmp;
  }

  if (version < 106)
  {
    Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
    wxASSERT(connPtr);

    Statement *tmp = connPtr->CreateStatement();

    wxString SMALLINT = connPtr->GetDataType(SQL_SMALLINT);

    wxString sql = 
        "ALTER TABLE MtRec ADD "
          "mtChecked  " + SMALLINT + " NOT NULL DEFAULT 0 ";

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

    sql = 
      "UPDATE MtRec SET mtChecked = 1 "
      " WHERE (mtMatches = 1 AND (2 * mtResA > mtBestOf OR 2 * mtResX > mtBestOf)) OR "
      "       (mtMatches > 1 ANd (2 * mtResA > mtMatches OR 2 * mtResX > mtMatches)) "; 

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
  }

  if (version < 120)
  {
    Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
    wxASSERT(connPtr);

    Statement *tmp = connPtr->CreateStatement();

    wxString  INTEGER   = connPtr->GetDataType(SQL_INTEGER);

    wxString sql = 
        "ALTER TABLE MtRec ADD "
          "mtUmpire2  " + INTEGER;

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
  }

  if (!UpdateStoredProcedure(version))
    return false;

  return true;
}


bool  MtStore::CreateConstraints()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  wxString  INTEGER   = connPtr->GetDataType(SQL_INTEGER);

  Statement *tmp = connPtr->CreateStatement();

  wxString  str;
  
  try
  {
    tmp->ExecuteUpdate("ALTER TABLE MtRec DROP CONSTRAINT mtGrRef");
    tmp->ExecuteUpdate("ALTER TABLE MtRec DROP CONSTRAINT mtStARef");
    tmp->ExecuteUpdate("ALTER TABLE MtRec DROP CONSTRAINT mtStXRef");
  }
  catch (SQLException &)
  {
  }

  try
  {
    tmp->ExecuteUpdate(str = 
      "ALTER TABLE MtRec Add CONSTRAINT mtGrRef "
      "FOREIGN KEY (grID) REFERENCES GrRec (grID) ON DELETE CASCADE");

    tmp->ExecuteUpdate(str = 
      "ALTER TABLE MtRec ADD CONSTRAINT mtStARef "
      "FOREIGN KEY (stA) REFERENCES StRec (stID) ON DELETE NO ACTION");  

    tmp->ExecuteUpdate(str = 
      "ALTER TABLE MtRec ADD CONSTRAINT mtStXRef "
      "FOREIGN KEY (stX) REFERENCES StRec (stID) ON DELETE NO ACTION");
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
    delete tmp;
    return false;
  };

  delete tmp;
  
  return MtMatchStore::CreateConstraints() && 
         MtSetStore::CreateConstraints();
}


bool  MtStore::UpdateConstraints(long version)
{
  if (version < 112)
    return CreateConstraints();
    
  return true;
}


bool  MtStore::UpdateStoredProcedure(long version) 
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  // wirklich VARCHAR, wird in stored proc gebraucht
  wxString  VARCHAR   = connPtr->GetDataType(SQL_VARCHAR);  
  wxString  INTEGER   = connPtr->GetDataType(SQL_INTEGER);
  wxString  SMALLINT  = connPtr->GetDataType(SQL_SMALLINT);
  wxString  TIMESTAMP = connPtr->GetDataType(SQL_TIMESTAMP);

  Statement *tmp = connPtr->CreateStatement();

  wxString  str;
  
  try
  {
    tmp->ExecuteUpdate("DROP PROCEDURE mtUpdateRasterProc");
  }
  catch (SQLException &)
  {
    // infoSystem.Exception("DROP PROCEDURE ...", e);
  }

  try 
  {
    tmp->ExecuteUpdate("DROP PROCEDURE mtSetResultProc");
  }
  catch (SQLException &)
  {
    // infoSystem.Exception("DROP PROCEDURE ...", e);
  }
    
  try 
  {
    tmp->ExecuteUpdate("DROP FUNCTION DttbSortFunc");
  }
  catch (SQLException &)
  {
    // infoSystem.Exception("DROP PROCEDURE ...", e);
  }

  try 
  {
    tmp->ExecuteUpdate("DROP FUNCTION DttbSortDirectCompFunc");
  }
  catch (SQLException &)
  {
    // infoSystem.Exception("DROP PROCEDURE ...", e);
  }

  try 
  {
    tmp->ExecuteUpdate("DROP FUNCTION DttbCompFunc");
  }
  catch (SQLException &)
  {
    // infoSystem.Exception("DROP PROCEDURE ...", e);
  }

  try 
  {
    tmp->ExecuteUpdate("DROP FUNCTION DttbDirectCompFunc");
  }
  catch (SQLException &)
  {
    // infoSystem.Exception("DROP PROCEDURE ...", e);
  }
    
  try 
  {
    tmp->ExecuteUpdate("DROP FUNCTION IttfSortFunc");
  }
  catch (SQLException &)
  {
    // infoSystem.Exception("DROP PROCEDURE ...", e);
  }

  try 
  {
    tmp->ExecuteUpdate("DROP FUNCTION IttfSortDirectCompFunc");
  }
  catch (SQLException &)
  {
    // infoSystem.Exception("DROP PROCEDURE ...", e);
  }
    
  try 
  {
    tmp->ExecuteUpdate("DROP FUNCTION SumUpFunc");
  }
  catch (SQLException &)
  {
    // infoSystem.Exception("DROP PROCEDURE ...", e);
  }

  try 
  {
    tmp->ExecuteUpdate("DROP FUNCTION mtResultsFunc");
  }
  catch (SQLException &)
  {
    // infoSystem.Exception("DROP PROCEDURE ...", e);
  }
    
  try 
  {
    tmp->ExecuteUpdate("DROP FUNCTION DiffFunc");
  }
  catch (SQLException &)
  {
    // infoSystem.Exception("DROP PROCEDURE ...", e);
  }

  try 
  {
    tmp->ExecuteUpdate("DROP FUNCTION QuotFunc");
  }
  catch (SQLException &)
  {
    // infoSystem.Exception("DROP PROCEDURE ...", e);
  }
    
  try 
  {
    tmp->ExecuteUpdate("DROP FUNCTION TbSortFunc");
  }
  catch (SQLException &)
  {
    // infoSystem.Exception("DROP PROCEDURE ...", e);
  }

  try
  {
    tmp->ExecuteUpdate("DROP TRIGGER mtUpdateTrigger");
  }
  catch (SQLException &)
  {
  }
  
  try
  {
#   include "MtStore.sql"  
#   include "TbSort.sql"

    tmp->ExecuteUpdate(str = 
        "CREATE TRIGGER mtUpdateTrigger ON MtRec FOR UPDATE AS \n"
        " --- Update timestamp for last changed \n"
        "DECLARE @mtNr " + INTEGER + ";\n"
        "UPDATE MtRec SET mtTimestamp = GETUTCDATE() \n"
        " WHERE mtID IN (SELECT mtID FROM deleted); \n"
        
        " --- Update printed flag if match data have changed but only if the match has not started yet \n"
        "UPDATE MtRec SET mtPrinted = 0 \n"
        " WHERE mtResA = 0 AND mtResX = 0 AND mtID IN \n"
        "       (SELECT deleted.mtID FROM deleted INNER JOIN inserted ON deleted.mtID = inserted.mtID \n"
        "         WHERE deleted.mtDateTime != inserted.mtDateTime OR \n"
        "               deleted.mtTable != inserted.mtTable OR \n"
        "               ISNULL(deleted.stA, 0) != ISNULL(inserted.stA, 0) OR \n"
        "               ISNULL(deleted.stX, 0) != ISNULL(inserted.stX, 0) OR \n"
        "               ISNULL(deleted.mtUmpire, 0) != ISNULL(inserted.mtUmpire, 0) OR \n"
        "               ISNULL(deleted.mtUmpire2, 0) != ISNULL(inserted.mtUmpire2, 0) \n"
        "       ); \n"

        " DECLARE mtCheckedCursor CURSOR LOCAL FOR \n"
        "   (SELECT inserted.mtNr \n"
        "      FROM inserted LEFT OUTER JOIN deleted ON inserted.mtID = deleted.mtID \n"
	      "     WHERE inserted.mtChecked <> deleted.mtChecked) \n"
        " OPEN mtCheckedCursor \n"
        " FETCH NEXT FROM mtCheckedCursor INTO @mtNr \n"
        " WHILE (@@FETCH_STATUS = 0) \n"
        " BEGIN \n"
        "   EXEC mtUpdateRasterProc @mtNr, 1; \n"
        "   EXEC mtUpdateRasterProc @mtNr, 0; \n"
        "   FETCH NEXT FROM mtCheckedCursor INTO  @mtNr \n"
        " END \n"
    );
  }
  catch (SQLException &e) 
  {
     infoSystem.Exception(str, e);
     return false;
  }
  
  // Update der Berechtigungen. Wenn die DB neu ist, geht es hier schief,
  // weil die Rollen noch nicht bekannt sind, sie werden aber spaeter vergeben.
  // D.h., einen Fehler hier kann man ignorieren.
  try
  {

    tmp->ExecuteUpdate(str = "GRANT EXECUTE ON mtSetResultProc TO ttm_results");
    tmp->ExecuteUpdate(str = "GRANT EXECUTE ON mtUpdateRasterProc TO ttm_results");
  }
  catch (SQLException &)
  {
  }  

  return true;
}


// -----------------------------------------------------------------------
bool  MtStore::Insert(const GrRec &gr)
{
  PreparedStatement *stmtPtr = 0;

  wxString str = "INSERT INTO MtRec "
                    "       (mtID, mtNr,    mtMatches, mtBestOf,  "
                    "        grID, mtRound, mtMatch,   mtChance,  "
                    "        mtReverse)                           "
                    "VALUES (?,    ?,       ?,         ?,         "
                    "        ?,    ?,       ?,         ?,         "
                    "        ?        )                           ";

  try
  {
    mtID = IdStore::ID(GetConnectionPtr());
    mtNr = GetNextNumber();
    mtEvent.grID = gr.grID;

    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    stmtPtr->SetData(1, &mtID);
    stmtPtr->SetData(2, &mtNr);
    stmtPtr->SetData(3, &mtMatches);
    stmtPtr->SetData(4, &mtBestOf);
    stmtPtr->SetData(5, &mtEvent.grID);
    stmtPtr->SetData(6, &mtEvent.mtRound);
    stmtPtr->SetData(7, &mtEvent.mtMatch);
    stmtPtr->SetData(8, &mtEvent.mtChance);
    stmtPtr->SetData(9, &mtReverse);

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


bool  MtStore::Update()
{
  return true;
}


bool  MtStore::Remove(const GrRec &gr)
{
  // if ( !MtSetStore(*this, GetConnectionPtr()).Remove(gr) ||
  //      !MtMatchStore(*this, GetConnectionPtr()).Remove(gr) )
  //   return false;

  wxString str = "DELETE FROM MtRec WHERE grID = " + ltostr(gr.grID);

  try
  {
    ExecuteUpdate(str);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


void  MtStore::Init()
{
  MtRec::Init();
}



// -----------------------------------------------------------------------
bool  MtStore::SelectByGr(const GrRec &gr)
{
  wxString  str = SelectString();
  str += " WHERE mt.grID  = " + ltostr(gr.grID) +
         " ORDER BY mtRound, mtMatch";

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


bool  MtStore::SelectByEvent(const MtEvent &event)
{
  wxString  str = SelectString();
  str += " WHERE mt.grID  = " + ltostr(event.grID) +
         "   AND mtRound  = " + ltostr(event.mtRound) + 
         "   AND mtMatch  = " + ltostr(event.mtMatch) +
         "   AND mtChance = " + ltostr(event.mtChance);

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


bool  MtStore::SelectById(long id)
{
  wxString  str = SelectString();
  str += " WHERE mt.mtID = " + ltostr(id);

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


bool  MtStore::SelectByNr(long nr)
{
  wxString  str = SelectString();
  str += " WHERE mtNr = " + ltostr(nr);

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
bool  MtStore::SetTeamAByNr(short stNr)
{
  wxString  str;

  str = "UPDATE MtRec SET stA = ";

  if (!stNr)
  {
    str += "NULL ";
  }
  else
  {
    str += "(SELECT stID FROM StRec WHERE stNr = " + ltostr(stNr);
    str += " AND grID = " + ltostr(mtEvent.grID);
    str += ") ";
  }

  str += "WHERE mtID = " + ltostr(mtID);

  try
  {
    ExecuteUpdate(str);
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  // Notify Views
  CRequest update;
  update.type = CRequest::UPDATE;
  update.rec  = CRequest::MTREC;
  update.id   = mtID;

  CTT32App::NotifyChange(update);

  return true;
}


bool  MtStore::SetTeamXByNr(short stNr)
{
  wxString  str;

  str = "UPDATE MtRec SET stX = ";

  if (!stNr)
  {
    str += "NULL ";
  }
  else
  {
    str += "(SELECT stID FROM StRec WHERE stNr = " + ltostr(stNr);
    str += " AND grID = " + ltostr(mtEvent.grID);
    str += ") ";
  }

  str += "WHERE mtID = " + ltostr(mtID);

  try
  {
    ExecuteUpdate(str);
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  // Notify Views
  CRequest update;
  update.type = CRequest::UPDATE;
  update.rec  = CRequest::MTREC;
  update.id   = mtID;

  CTT32App::NotifyChange(update);

  return true;
}


bool  MtStore::SetTeamsByNr(short stNrA, short stNrX)
{
  wxString  str;

  str = "UPDATE MtRec SET stA = ";

  if (!stNrA)
  {
    str += "NULL, ";
  }
  else
  {
    str += "(SELECT stID FROM StRec WHERE stNr = " + ltostr(stNrA);
    str += " AND grID = " + ltostr(mtEvent.grID);
    str += "), ";
  }

  str += "stX = ";

  if (!stNrX)
    str += "NULL ";
  else
  {
    str += "(SELECT stID FROM StRec WHERE stNr = " + ltostr(stNrX);
    str += " AND grID = " + ltostr(mtEvent.grID);
    str += ") ";
  }

  str += "WHERE mtID = " + ltostr(mtID);

  try
  {
    ExecuteUpdate(str);
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  // Notify Views
  CRequest update;
  update.type = CRequest::UPDATE;
  update.rec  = CRequest::MTREC;
  update.id   = mtID;

  CTT32App::NotifyChange(update);

  return true;
}

// -----------------------------------------------------------------------
bool  MtStore::SetTeamAById(long  stID)
{
  wxString  str;

  if (!stID)
  {
    str = "UPDATE MtRec SET stA = NULL WHERE mtID = " + ltostr(mtID);
  }
  else
  {
    str = "UPDATE MtRec SET stA = " + ltostr(stID);
    str += " WHERE mtID = " + ltostr(mtID);
  }

  try
  {
    ExecuteUpdate(str);
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  // Notify Views
  CRequest update;
  update.type = CRequest::UPDATE;
  update.rec  = CRequest::MTREC;
  update.id   = mtID;

  CTT32App::NotifyChange(update);

  return true;
}


bool  MtStore::SetTeamXById(long stID)
{
  wxString  str;

  if (!stID)
  {
    str = "UPDATE MtRec SET stX = NULL WHERE mtID = " + ltostr(mtID);
  }
  else
  {
    str = "UPDATE MtRec SET stX = " + ltostr(stID);
    str += " WHERE mtID = " + ltostr(mtID);
  }

  try
  {
    ExecuteUpdate(str);
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  // Notify Views
  CRequest update;
  update.type = CRequest::UPDATE;
  update.rec  = CRequest::MTREC;
  update.id   = mtID;

  CTT32App::NotifyChange(update);

  return true;
}


// -----------------------------------------------------------------------
bool MtStore::SetBestOf(short bestOf)
{
  wxString  str;

  str = "UPDATE MtRec SET mtBestOf = " + ltostr(bestOf) + " WHERE mtID = " + ltostr(mtID);

  try
  {
    ExecuteUpdate(str);
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  // Notify Views
  CRequest update;
  update.type = CRequest::UPDATE;
  update.rec  = CRequest::MTREC;
  update.id   = mtID;

  CTT32App::NotifyChange(update);

  return true;
}


// -----------------------------------------------------------------------
bool  MtStore::UpdateReverseFlag()
{
  if (!mtID)
    return false;

  wxString  str = 
    "UPDATE MtRec SET mtReverse = " + ltostr(mtReverse) +
    " WHERE mtID = " + ltostr(mtID);

  try
  {
    ExecuteUpdate(str);
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  // Notify Views
  CRequest update;
  update.type = CRequest::UPDATE_REVERSE;
  update.rec  = CRequest::MTREC;
  update.id   = mtID;

  CTT32App::NotifyChange(update);

  return true;
}

// -----------------------------------------------------------------------
bool  MtStore::UpdateWalkOver()
{
  if (!mtID)
    return false;

  wxString  str = 
    "UPDATE MtRec SET "
    "   mtWalkOverA = " + ltostr(mtWalkOverA) + ", "
    "   mtWalkOverX = " + ltostr(mtWalkOverX) +
    " WHERE mtID = " + ltostr(mtID);

  try
  {
    ExecuteUpdate(str);
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  // Notify Views
  CRequest update;
  update.type = CRequest::UPDATE_REVERSE;
  update.rec  = CRequest::MTREC;
  update.id   = mtID;

  CTT32App::NotifyChange(update);

  return true;
}

bool MtStore::UpdateResult(
  short bestOf, MtSet *mtSets, 
  short walkOverA, short walkOverX, 
  short injuredA, short injuredX,
  short disqualifiedA, short disqualifiedX
)
{
  // Change BestOf, it isn't done in the stored proc anymore
  if (!SetBestOf(bestOf))
    return false;

  wxString  str = "Update Result";

  wxASSERT(mtSets);
  wxASSERT(mtSets[0].mtID == mtID);

  wxString sets;
  for (int i = 0; i < bestOf; i++)
  {
    wxASSERT(mtSets[i].mtID == mtID);
    wxASSERT(mtSets[i].mtMS == mtSets[0].mtMS);

    sets += wxString::Format("%02d%02d", mtSets[i].mtResA, mtSets[i].mtResX);
  }
  
  wxString sql = "mtSetResultProc " + 
        ltostr(mtNr) + ", " + ltostr(mtSets[0].mtMS) + ", " + 
        ltostr(mtBestOf) + ", " + "'" + sets + "'" + ", " + 
        ltostr(walkOverA ? 1 : 0) + ", " + ltostr(walkOverX ? 1 : 0) + ", " +
        ltostr(injuredA ? 1 : 0) + ", " + ltostr(injuredX ? 1 : 0) + ", " +
        ltostr(disqualifiedA ? 1 : 0) + ", " + ltostr(disqualifiedX ? 1 : 0);
  try
  {
    ExecuteUpdate(sql);
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(sql, e);
    
    return false;
  }

  // Notify Views
  CRequest update;
  update.type = CRequest::UPDATE_RESULT;
  update.rec  = CRequest::MTREC;
  update.id   = mtID;

  CTT32App::NotifyChange(update);

  return true;
}


bool  MtStore::UpdateSchedule()
{
  wxString  str = 
    "UPDATE MtRec SET mtDateTime = ?, mtTable = ?, mtUmpire = ?, mtUmpire2 = ? WHERE mtID = "+ ltostr(mtID);

  PreparedStatement *stmtPtr = 0;
  try
  {
    // Leere Zeiten al <null> eintragen, sonst mault MS-SQL
    timestamp *tmpPtr = mtPlace.mtDateTime.year ? 
                        &mtPlace.mtDateTime     : 0;
    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    stmtPtr->SetData(1, tmpPtr);
    stmtPtr->SetData(2, &mtPlace.mtTable);
    stmtPtr->SetData(3, mtUmpire ? &mtUmpire : 0);
    stmtPtr->SetData(4, mtUmpire2 ? &mtUmpire2 : 0);
    stmtPtr->Execute();

  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
    delete stmtPtr;

    return false;
  }

  delete stmtPtr;

  // Notify Views
  CRequest update;
  update.type = CRequest::UPDATE_SCHEDULE;
  update.rec  = CRequest::MTREC;
  update.id   = mtID;

  CTT32App::NotifyChange(update);

  return true;
}


bool  MtStore::UpdateScheduleMatch(MtStore::MtEvent &mtEv, 
                                   MtStore::MtPlace &mtPl,
                                   short umpire, short umpire2)
{
  Statement *stmtPtr = 0;
  ResultSet *resPtr  = 0;

  wxString str;

  // Wenn mtMatch negativ ist, dann ist es das n-te Spiel ohne Freilos
  if (mtEv.mtMatch < 0)
  {
    str =
      "SELECT mtID FROM ( "
      "  SELECT ROW_NUMBER() OVER (ORDER BY mtMatch) AS row, mtID "
      "    FROM MtRec "
      "         LEFT OUTER JOIN StRec stA ON MtRec.stA = stA.stID "
      "         LEFT OUTER JOIN StRec stX ON MtRec.stX = stX.stID "
      "   WHERE MtRec.grID = " + ltostr(mtEv.grID) +
      "     AND mtRound = " + ltostr(mtEv.mtRound) +
      "     AND mtChance = " + ltostr(mtEv.mtChance) +
      "     AND (stA.stId IS NULL OR stA.tmID IS NOT NULL) "
      "     AND (stX.stID IS NULL OR stX.tmID IS NOT NULL) "
      "  ) AS tmp WHERE row = " + ltostr(-mtEv.mtMatch)
      ;
  }
  else
  {
    str = 
        "SELECT mtID FROM MtRec "
        " WHERE grID = "+ltostr(mtEv.grID) + 
        "   AND mtRound = "+ltostr(mtEv.mtRound) +
        "   AND mtMatch >= "+ltostr(mtEv.mtMatch) +
        "   AND mtChance = "+ltostr(mtEv.mtChance) +
        " ORDER BY mtMatch"
    ;
  }


  long id = 0;
  std::vector<long> idList;

  try
  {
    stmtPtr = GetConnectionPtr()->CreateStatement();
    resPtr = stmtPtr->ExecuteQuery(str);

    if (!resPtr->Next() || !resPtr->GetData(1, id) || resPtr->WasNull())
      id = 0;
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
    delete stmtPtr;
    delete resPtr;

    return false;
  }

  delete stmtPtr;
  delete resPtr;

  if (id)
    UpdateScheduleMatch(id, mtPl, umpire, umpire2);

  return id != 0;
}

bool  MtStore::UpdateScheduleRound(MtStore::MtEvent &mtEvFrom,
                                   MtStore::MtPlace &mtPlFrom,
                                   short &nofTables, bool decTable,
                                   short umpire, short umpire2)
{
  if (nofTables == 1)
  {
    nofTables = 0;
    return UpdateScheduleMatch(mtEvFrom, mtPlFrom, umpire, umpire2);
  }

  Statement *stmtPtr = 0;
  ResultSet *resPtr  = 0;

  // Auswahl aller Gruppenids
  wxString  str = 
      "SELECT mtID FROM MtRec "
      " WHERE grID = "+ltostr(mtEvFrom.grID) + 
      "   AND mtRound = "+ltostr(mtEvFrom.mtRound) +
      "   AND mtMatch >= "+ltostr(mtEvFrom.mtMatch) +
      "   AND mtChance = "+ltostr(mtEvFrom.mtChance) +
      " ORDER BY mtMatch";


  long  id;
  std::vector<long> idList;

  try
  {
    stmtPtr = GetConnectionPtr()->CreateStatement();
    resPtr = stmtPtr->ExecuteQuery(str);
    resPtr->BindCol(1, &id);

    while (resPtr->Next())
      idList.push_back(id);
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
    delete stmtPtr;
    delete resPtr;

    return false;
  }

  for (std::vector<long>::iterator it = idList.begin();
       it != idList.end(); it++)
  {
    UpdateScheduleMatch((*it), mtPlFrom, umpire, umpire2);

    if (mtPlFrom.mtTable)
    {
      if (decTable)
      {
        if (!--mtPlFrom.mtTable)
        {
          --nofTables;
          break;
        }
      }
      else
      {
        mtPlFrom.mtTable++;
      }
    }

    if (!--nofTables)
      break;
  }

  delete stmtPtr;
  delete resPtr;

  return true;
}


bool  MtStore::UpdateScheduleRoundExcludeByes(
                                   MtStore::MtEvent &mtEvFrom,
                                   MtStore::MtPlace &mtPlFrom,
                                   short &nofTables, bool decTable,
                                   short umpire, short umpire2)
{
  Statement *stmtPtr = 0;
  ResultSet *resPtr  = 0;

  // Auswahl aller Match-IDs
  wxString  str = 
      "SELECT mtID "
      "  FROM MtRec mt LEFT OUTER JOIN StRec stA ON mt.stA = stA.stID "
      "                LEFT OUTER JOIN StRec stX ON mt.stX = stX.stID "
      "                LEFT OUTER JOIN XxRec xxA ON mt.stA = xxA.stID "
      "                LEFT OUTER JOIN XxRec xxX ON mt.stX = xxX.stID "
      " WHERE mt.grID = "+ltostr(mtEvFrom.grID) + 
      "   AND mt.mtRound  = "+ltostr(mtEvFrom.mtRound) +
      "   AND mt.mtMatch >= "+ltostr(mtEvFrom.mtMatch) +
      "   AND mt.mtChance = "+ltostr(mtEvFrom.mtChance) +
      "   AND (mt.stA IS NULL OR stA.tmID IS NOT NULL OR xxA.stID IS NOT NULL) "
      "   AND (mt.stX IS NULL OR stX.tmID IS NOT NULL OR xxX.stID IS NOT NULL) "
      " ORDER BY mtMatch";


  long  id;
  std::vector<long> idList;

  try
  {
    stmtPtr = GetConnectionPtr()->CreateStatement();
    resPtr = stmtPtr->ExecuteQuery(str);
    resPtr->BindCol(1, &id);

    while (resPtr->Next())
      idList.push_back(id);
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
    delete stmtPtr;
    delete resPtr;

    return false;
  }

  for (std::vector<long>::iterator it = idList.begin();
       it != idList.end(); it++)
  {
    UpdateScheduleMatch((*it), mtPlFrom, umpire, umpire2);

    if (mtPlFrom.mtTable)
    {
      if (decTable)
      {
        if (!--mtPlFrom.mtTable)
          break;
      }
      else
      {
        mtPlFrom.mtTable++;
      }
    }

    if (!--nofTables)
      break;
  }

  delete stmtPtr;
  delete resPtr;

  // Notify Views
  CRequest update;
  update.type = CRequest::UPDATE_SCHEDULE;
  update.rec  = CRequest::MTREC;
  update.id   = 0;

  CTT32App::NotifyChange(update);

  return true;
}


bool  MtStore::UpdateScheduleGroup(MtStore::MtEvent &mtEv,
                                   MtStore::MtPlace &mtPl,
                                   short umpire, short umpire2)
{
  wxString  str = 
    "UPDATE MtRec SET mtDateTime = ?, mtTable = ?, mtUmpire = ?, mtUmpire2 = ? "
    " WHERE grID     = "+ltostr(mtEv.grID);

  PreparedStatement *stmtPtr = 0;
  try
  {
    // Leere Zeiten und Tische als <null> eintragen. Bei den Zeiten mault sonst MS-SQL
    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    stmtPtr->SetData(1, mtPl.mtDateTime.year ? &mtPl.mtDateTime : 0);
    stmtPtr->SetData(2, mtPl.mtTable ? &mtPl.mtTable : 0);
    stmtPtr->SetData(3, umpire ? &umpire : 0);
    stmtPtr->SetData(4, umpire2 ? &umpire2 : 0);
    
    stmtPtr->Execute();

  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
    delete stmtPtr;

    return false;
  }

  delete stmtPtr;

  // Notify Views
  CRequest update;
  update.type = CRequest::UPDATE_SCHEDULE;
  update.rec  = CRequest::MTREC;
  update.id   = 0;

  CTT32App::NotifyChange(update);

  return true;
}


bool  MtStore::UpdateScheduleGroup(MtStore::MtEvent &mtEvFrom,
                                   MtStore::MtPlace &mtPlFrom,
                                   short &nofTables, bool decTable,
                                   short umpire, short umpire2)
{
  if (nofTables == 1)
  {
    nofTables = 0;
    return UpdateScheduleGroup(mtEvFrom, mtPlFrom, umpire, umpire2);
  }

  Statement *stmtPtr = 0;
  ResultSet *resPtr  = 0;

  // Auswahl aller Gruppenids
  wxString  str = 
      "SELECT grID FROM GrRec "
      " WHERE cpID IN   (SELECT cpID    FROM GrRec WHERE grID = " + ltostr(mtEvFrom.grID) + ") "
      "   AND grName >= (SELECT grName  FROM GrRec WHERE grID = " + ltostr(mtEvFrom.grID) + ") "
      "   AND grStage = (SELECT grStage FROM GrRec WHERE grID = " + ltostr(mtEvFrom.grID) + ") "
      " ORDER BY grName";


  long  id;
  std::vector<long> idList;

  try
  {
    stmtPtr = GetConnectionPtr()->CreateStatement();
    resPtr = stmtPtr->ExecuteQuery(str);
    resPtr->BindCol(1, &id);

    while (resPtr->Next())
      idList.push_back(id);
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
    delete stmtPtr;
    delete resPtr;

    return false;
  }

  for (std::vector<long>::iterator it = idList.begin();
       it != idList.end(); it++)
  {
    mtEvFrom.grID = (*it);
    UpdateScheduleGroup(mtEvFrom, mtPlFrom, umpire, umpire2);

    if (mtPlFrom.mtTable)
    {
      if (decTable)
      {
        if (!--mtPlFrom.mtTable)
          break;
      }
      else
      {
        mtPlFrom.mtTable++;
      }
    }

    if (!--nofTables)
      break;
  }

  delete stmtPtr;
  delete resPtr;

  // Notify Views
  CRequest update;
  update.type = CRequest::UPDATE_SCHEDULE;
  update.rec  = CRequest::MTREC;
  update.id   = 0;

  CTT32App::NotifyChange(update);

  return true;
}


bool  MtStore::UpdateScheduleRoundsGroup(
                                   MtStore::MtEvent &mtEv,
                                   MtStore::MtPlace &mtPl, 
                                   short umpire, short umpire2)
{
  wxString  str = 
    "UPDATE MtRec SET mtDateTime = ?, mtTable = ?, mtUmpire = ?, mtUmpire2 = ? "
    " WHERE grID  = " + ltostr(mtEv.grID) +
    " AND mtRound = " + ltostr(mtEv.mtRound);

  PreparedStatement *stmtPtr = 0;
  try
  {
    // Leere Zeiten al <null> eintragen, sonst mault MS-SQL
    timestamp *tmpPtr = mtPl.mtDateTime.year ? &mtPl.mtDateTime : 0;
    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    stmtPtr->SetData(1, tmpPtr);
    stmtPtr->SetData(2, &mtPl.mtTable);
    stmtPtr->SetData(3, umpire ? &umpire : 0);
    stmtPtr->SetData(4, umpire2 ? &umpire2 : 0);
    stmtPtr->Execute();

  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
    delete stmtPtr;

    return false;
  }

  delete stmtPtr;

  // Notify Views
  CRequest update;
  update.type = CRequest::UPDATE_SCHEDULE;
  update.rec  = CRequest::MTREC;
  update.id   = 0;

  CTT32App::NotifyChange(update);

  return true;
}


bool  MtStore::UpdateScheduleRoundsGroup(
                                   MtStore::MtEvent &mtEvFrom,
                                   MtStore::MtPlace &mtPlFrom,
                                   short &nofTables, bool decTable,
                                   short umpire, short umpire2)
{
  if (nofTables == 1)
    return UpdateScheduleRound(mtEvFrom, mtPlFrom, nofTables, decTable, umpire, umpire2);

  Statement *stmtPtr = 0;
  ResultSet *resPtr  = 0;

  // Auswahl aller Gruppenids
  wxString  str = 
      "SELECT grID FROM GrRec "
      " WHERE cpID IN   (SELECT cpID    FROM GrRec WHERE grID = "+ltostr(mtEvFrom.grID)+") "
      "   AND grName >= (SELECT grName  FROM GrRec WHERE grID = "+ltostr(mtEvFrom.grID)+") "
      "   AND grStage = (SELECT grStage FROM GrRec WHERE grID = "+ltostr(mtEvFrom.grID)+") "
      " ORDER BY grName";


  long  id;
  std::vector<long> idList;

  try
  {
    stmtPtr = GetConnectionPtr()->CreateStatement();
    resPtr = stmtPtr->ExecuteQuery(str);
    resPtr->BindCol(1, &id);

    while (resPtr->Next())
      idList.push_back(id);
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
    delete stmtPtr;
    delete resPtr;

    return false;
  }

  delete stmtPtr;
  delete resPtr;

  for (std::vector<long>::iterator it = idList.begin();
       it != idList.end(); it++)
  {
    mtEvFrom.grID = (*it);
    UpdateScheduleRound(mtEvFrom, mtPlFrom, nofTables, decTable, umpire, umpire2);

    // Naechste Gruppe wieder mit erstem Spiel starten
    mtEvFrom.mtMatch = 1;

    if (!nofTables)
      break;
  }

  return true;
}


bool  MtStore::UpdateScheduleRoundsGroupExcludeByes(
  MtStore::MtEvent &mtEvFrom,
  MtStore::MtPlace &mtPlFrom,
  short &nofTables, bool decTable,
  short umpire, short umpire2)
{
  if (nofTables == 1)
    return UpdateScheduleRoundExcludeByes(mtEvFrom, mtPlFrom, nofTables, decTable, umpire, umpire2);

  Statement *stmtPtr = 0;
  ResultSet *resPtr = 0;

  // Auswahl aller Gruppenids
  wxString  str =
    "SELECT grID FROM GrRec "
    " WHERE cpID IN   (SELECT cpID    FROM GrRec WHERE grID = " + ltostr(mtEvFrom.grID) + ") "
    "   AND grName >= (SELECT grName  FROM GrRec WHERE grID = " + ltostr(mtEvFrom.grID) + ") "
    "   AND grStage = (SELECT grStage FROM GrRec WHERE grID = " + ltostr(mtEvFrom.grID) + ") "
    " ORDER BY grName";


  long  id;
  std::vector<long> idList;

  try
  {
    stmtPtr = GetConnectionPtr()->CreateStatement();
    resPtr = stmtPtr->ExecuteQuery(str);
    resPtr->BindCol(1, &id);

    while (resPtr->Next())
      idList.push_back(id);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    delete stmtPtr;
    delete resPtr;

    return false;
  }

  delete stmtPtr;
  delete resPtr;

  for (std::vector<long>::iterator it = idList.begin();
    it != idList.end(); it++)
  {
    mtEvFrom.grID = (*it);
    UpdateScheduleRoundExcludeByes(mtEvFrom, mtPlFrom, nofTables, decTable, umpire, umpire2);

    // Naechste Gruppe wieder mit erstem Spiel starten
    mtEvFrom.mtMatch = 1;

    if (!nofTables)
      break;
  }

  return true;
}


bool  MtStore::UpdateScheduleMatchesGroup(
                                   MtStore::MtEvent &mtEvFrom,
                                   MtStore::MtPlace &mtPlFrom,
                                   short &nofTables, bool decTable,
                                   short umpire, short umpire2)
{
  if (nofTables == 1)
    return UpdateScheduleRound(mtEvFrom, mtPlFrom, nofTables, decTable, umpire, umpire2);

  Statement *stmtPtr = 0;
  ResultSet *resPtr  = 0;

  // Auswahl aller Gruppenids
  wxString  str = 
      "SELECT grID FROM GrRec "
      " WHERE cpID IN   (SELECT cpID    FROM GrRec WHERE grID = "+ltostr(mtEvFrom.grID)+") "
      "   AND grName >= (SELECT grName  FROM GrRec WHERE grID = "+ltostr(mtEvFrom.grID)+") "
      "   AND grStage = (SELECT grStage FROM GrRec WHERE grID = "+ltostr(mtEvFrom.grID)+") "
      " ORDER BY grName";


  long  id;
  std::vector<long> idList;

  try
  {
    stmtPtr = GetConnectionPtr()->CreateStatement();
    resPtr = stmtPtr->ExecuteQuery(str);
    resPtr->BindCol(1, &id);

    while (resPtr->Next())
      idList.push_back(id);
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
    delete stmtPtr;
    delete resPtr;

    return false;
  }

  delete stmtPtr;
  delete resPtr;

  for (std::vector<long>::iterator it = idList.begin();
       it != idList.end(); it++)
  {
    mtEvFrom.grID = (*it);
    UpdateScheduleMatch(mtEvFrom, mtPlFrom, umpire, umpire2);

    --nofTables;

    if (mtPlFrom.mtTable)
    {
      if (decTable)
        --mtPlFrom.mtTable;
      else
        ++mtPlFrom.mtTable;

      if (!mtPlFrom.mtTable)
        break;
    }

    if (!nofTables)
      break;
  }

  return true;
}


bool  MtStore::UpdateScheduleMatch(
    long id, MtStore::MtPlace &mtPl, short umpire, short umpire2)
{
  wxString  str = 
    "UPDATE MtRec SET mtDateTime = ?, mtTable = ?, mtUmpire = ?, mtUmpire2 = ? WHERE mtID = "+ ltostr(id);

  PreparedStatement *stmtPtr = 0;
  try
  {
    // Leere Zeiten al <null> eintragen, sonst mault MS-SQL
    timestamp *tmpPtr = mtPl.mtDateTime.year ? &mtPl.mtDateTime : 0;
    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    stmtPtr->SetData(1, tmpPtr);
    stmtPtr->SetData(2, &mtPl.mtTable);
    stmtPtr->SetData(3, umpire ? &umpire : 0);
    stmtPtr->SetData(4, umpire2 ? &umpire2 : 0);
    stmtPtr->Execute();

  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
    delete stmtPtr;

    return false;
  }

  delete stmtPtr;

  // Notify Views
  CRequest update;
  update.type = CRequest::UPDATE_SCHEDULE;
  update.rec  = CRequest::MTREC;
  update.id   = id;

  CTT32App::NotifyChange(update);

  return true;
}


// -----------------------------------------------------------------------
bool  MtStore::UpdateScoreChecked(long id, bool checked)
{
  wxString  str = 
    "UPDATE MtRec SET mtChecked = " + ltostr(checked ? 1 : 0) + 
    " WHERE mtID = " + ltostr(id);
  try
  {
    ExecuteUpdate(str);
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);

    return false;
  }

  // Notify Views
  CRequest update;
  update.type = CRequest::UPDATE;
  update.rec  = CRequest::MTREC;
  update.id   = id;

  CTT32App::NotifyChange(update);

  return true;
}


// -----------------------------------------------------------------------
bool  MtStore::UpdateScorePrinted(long id, bool printed)
{
  wxString  str = 
    "UPDATE MtRec SET mtPrinted = " + ltostr(printed ? 1 : 0) + 
    " WHERE mtID = " + ltostr(id);
  try
  {
    ExecuteUpdate(str);
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);

    return false;
  }

  // Notify Views
  CRequest update;
  update.type = CRequest::UPDATE;
  update.rec  = CRequest::MTREC;
  update.id   = id;

  CTT32App::NotifyChange(update);

  return true;
}


bool  MtStore::UpdateScorePrintedForRound(long id, short round, bool printed)
{
  wxString  str = 
    "UPDATE MtRec SET mtPrinted = " + ltostr(printed ? 1 : 0) + 
    " WHERE grID = " + ltostr(id) + " AND mtRound = " + ltostr(round);
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


bool  MtStore::UpdateScorePrintedForGroup(long id, bool printed)
{
  wxString  str = 
    "UPDATE MtRec SET mtPrinted = " + ltostr(printed ? 1 : 0) + 
    " WHERE grID = " + ltostr(id);
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


bool  MtStore::UpdateScorePrintedScheduled(
    const MtStore::MtPlace &from,
    const MtStore::MtPlace &to, bool printed)
{
  wxString  str = 
    "UPDATE MtRec SET mtPrinted = " + ltostr(printed ? 1 : 0) + 
    " WHERE mtDateTime >= '" + tstostr(from.mtDateTime) + "'" +
    "   AND mtTable >= " + ltostr(from.mtTable) +
    "   AND mtDateTime <= '" + tstostr(to.mtDateTime) + "'" +
    "   AND mtTable <= " + ltostr(to.mtTable);

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


bool  MtStore::UpdateScorePrintedForTeam(const StRec &st, bool printed)
{
  wxString  str = 
    "UPDATE MtRec SET mtPrinted = " + ltostr(printed ? 1 : 0) + 
    " WHERE stA = " + ltostr(st.stID) + " OR stX = " + ltostr(st.stID);

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


// -----------------------------------------------------------------------
bool  MtStore::UpdateTimestamp(const StRec &st)
{
  wxString  str = 
    "UPDATE MtRec SET mtTimestamp = GETUTCDATE() " 
    " WHERE stA = " + ltostr(st.stID) + " OR stX = " + ltostr(st.stID);
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

// -----------------------------------------------------------------------
timestamp MtStore::GetEarliestMatchTime(const MtEvent &event)
{
  timestamp ts;
  
  memset(&ts, 0, sizeof(ts));
  
  wxString str = "SELECT MIN(mtDateTime) FROM MtRec "
    "WHERE grId = " + ltostr(event.grID) + 
    "  AND mtRound = " + ltostr(event.mtRound) + 
    "  AND mtChance = " + ltostr(event.mtChance) + 
    "  AND DAY(mtDateTime) <> 0";
    
  Statement *stmtPtr = GetConnectionPtr()->CreateStatement();
  wxASSERT(stmtPtr);
  
  ResultSet *resPtr = stmtPtr->ExecuteQuery(str);
  wxASSERT(resPtr);
  
  if (resPtr->Next())
  {
    if (!resPtr->GetData(1, ts) || resPtr->WasNull(1))
      memset(&ts, 0, sizeof(ts));
  }
    
  delete resPtr;
  delete stmtPtr;
  
  return ts;
}


timestamp MtStore::GetLatestMatchTime(const MtEvent &event)
{
  timestamp ts;
  
  memset(&ts, 0, sizeof(ts));
  
  wxString str = "SELECT MAX(mtDateTime) FROM MtRec "
    "WHERE grId = " + ltostr(event.grID) + 
    "  AND mtRound = " + ltostr(event.mtRound) + 
    "  AND mtChance = " + ltostr(event.mtChance);
    
  Statement *stmtPtr = GetConnectionPtr()->CreateStatement();
  wxASSERT(stmtPtr);
  
  ResultSet *resPtr = stmtPtr->ExecuteQuery(str);
  wxASSERT(resPtr);
  
  if (resPtr->Next())
  {
    if (!resPtr->GetData(1, ts) || resPtr->WasNull(1))
      memset(&ts, 0, sizeof(ts));
  }
    
  delete resPtr;
  delete stmtPtr;
  
  return ts;
}


short MtStore::GetLastPlayedRound(const MtEvent &event)
{
  short rd = 0;
  
  wxString str = "SELECT MAX(mtRound) FROM MtRec "
    "WHERE grID = " + ltostr(event.grID) +
    "  AND mtChance = " + ltostr(event.mtChance) + 
    "  AND (mtResA > 0 OR mtResX > 0)";
    
  Statement *stmtPtr = GetConnectionPtr()->CreateStatement();
  wxASSERT(stmtPtr);
  
  ResultSet *resPtr = stmtPtr->ExecuteQuery(str);
  wxASSERT(resPtr);
  
  if (!resPtr->Next() || !resPtr->GetData(1, rd) || resPtr->WasNull())
    rd = 0;
    
  delete resPtr;
  delete stmtPtr;
  
  return rd;
}



short MtStore::CountMatchesInGroupGroup(const MtEvent &evt)
{
  wxString  str = 
    "SELECT COUNT(*) FROM GrRec "
      " WHERE cpID IN   (SELECT cpID    FROM GrRec WHERE grID = " + ltostr(evt.grID) + ") "
      "   AND grName >= (SELECT grName  FROM GrRec WHERE grID = " + ltostr(evt.grID) + ") "
      "   AND grStage = (SELECT grStage FROM GrRec WHERE grID = " + ltostr(evt.grID) + ") "
  ;

  short count = 0;

  Statement *stmtPtr = GetConnectionPtr()->CreateStatement();
  wxASSERT(stmtPtr);
  
  ResultSet *resPtr = stmtPtr->ExecuteQuery(str);
  wxASSERT(resPtr);
  
  if (!resPtr->Next() || !resPtr->GetData(1, count) || resPtr->WasNull())
    count = 0;
    
  delete resPtr;
  delete stmtPtr;
  
  return count;

}

short MtStore::CountMatchesInRoundGroup(const MtEvent &evt)
{
  wxString str = 
    "SELECT COUNT(*) FROM MtRec mt INNER JOIN GrRec gr ON mt.grID = gr.grID "
    " WHERE (gr.grNofMatches = 0 OR mt.mtMatch <= (gr.grNofMatches / POWER(2, mt.mtRound - 1))) AND "
    "       mt.mtRound = " + ltostr(evt.mtRound) + " AND "
    "       gr.cpID = (SELECT cpID FROM GrRec WHERE grID = " + ltostr(evt.grID) + ") AND "
    "       gr.grStage = (SELECT grStage FROM GrRec WHERE grID = " + ltostr(evt.grID) + ") AND "
    "       gr.grName >= (SELECT grName FROM GrRec WHERE grID = " + ltostr(evt.grID) + ")";

  short count = 0;

  Statement *stmtPtr = GetConnectionPtr()->CreateStatement();
  wxASSERT(stmtPtr);
  
  ResultSet *resPtr = stmtPtr->ExecuteQuery(str);
  wxASSERT(resPtr);
  
  if (!resPtr->Next() || !resPtr->GetData(1, count) || resPtr->WasNull())
    count = 0;
    
  delete resPtr;
  delete stmtPtr;
  
  return count;

}


short MtStore::CountMatchesInMatchGroup(const MtEvent &evt)
{
  wxString str = 
    "SELECT COUNT(*) FROM MtRec mt INNER JOIN GrRec gr ON mt.grID = gr.grID "
    " WHERE mt.mtRound = " + ltostr(evt.mtRound) + " AND mt.mtMatch = " + ltostr(evt.mtMatch) + " AND "
    "       gr.cpID = (SELECT cpID FROM GrRec WHERE grID = " + ltostr(evt.grID) + ") AND "
    "       gr.grStage = (SELECT grStage FROM GrRec WHERE grID = " + ltostr(evt.grID) + ") AND "
    "       gr.grName >= (SELECT grName FROM GrRec WHERE grID = " + ltostr(evt.grID) + ")";

  short count = 0;

  Statement *stmtPtr = GetConnectionPtr()->CreateStatement();
  wxASSERT(stmtPtr);
  
  ResultSet *resPtr = stmtPtr->ExecuteQuery(str);
  wxASSERT(resPtr);
  
  if (!resPtr->Next() || !resPtr->GetData(1, count) || resPtr->WasNull())
    count = 0;
    
  delete resPtr;
  delete stmtPtr;
  
  return count;

}


short MtStore::GetHighestTableNumber(const MtPlace &place)
{
  short maxTable = 0;

  wxString str = "SELECT MAX(mtTable) FROM MtRec WHERE mtDateTime = '" + tstostr(place.mtDateTime) + "'";
  Statement *stmtPtr = GetConnectionPtr()->CreateStatement();
  ResultSet *resPtr = stmtPtr->ExecuteQuery(str);

  if (!resPtr->Next() || !resPtr->GetData(1, maxTable) || resPtr->WasNull())
    maxTable = 0;

  delete resPtr;
  delete stmtPtr;

  return maxTable;
}


// -----------------------------------------------------------------------
long  MtStore::GetNextNumber()
{
  Statement *stmtPtr;
  ResultSet *resPtr;

  wxString  str = "SELECT MAX(mtNr) FROM MtRec";

  stmtPtr = GetConnectionPtr()->CreateStatement();
  wxASSERT(stmtPtr);
  
  resPtr = stmtPtr->ExecuteQuery(str);
  wxASSERT(resPtr);

  long nr;
  if (!resPtr->Next() || !resPtr->GetData(1, nr) || resPtr->WasNull())
    nr = 0;

  delete resPtr;
  delete stmtPtr;
  
  return nr+1;
}


// -----------------------------------------------------------------------
wxString  MtStore::SelectString() const
{
  wxString  str = 
    "SELECT mt.mtID, mtNr, stA, stX, mtUmpire, mtUmpire2, mtReverse,     "
    "       mt.mtWalkOverA, mt.mtWalkOverX,         "
    "       mt.mtInjuredA, mt.mtInjuredX,           "
    "       mt.mtDisqualifiedA, mt.mtDisqualifiedX, "
    "       mtMatches, mtBestOf,                    "
    "       mt.grID, mtRound, mtMatch, mtChance,    "
    "       mtDateTime, mtTable,                    "
    "       mt.mtResA, mt.mtResX,                   "
    "       mtMatch.mtResA, mtMatch.mtResX,         "
    "       mtSet.mtResA, mtSet.mtResX,             "
    "       stA.tmID, stX.tmID,                     "
    "       xxA.stID, xxX.stID                      "
    "  FROM MtRec mt                                "
    "    LEFT OUTER JOIN StRec stA ON mt.stA = stA.stID "
    "    LEFT OUTER JOIN StRec stX ON mt.stX = stX.stID "
    "    LEFT OUTER JOIN mtMatch ON mt.mtID = mtMatch.mtID AND mtMatch.mtMS = 0 "
    "    LEFT OUTER JOIN mtSet ON mt.mtID = mtSet.mtID AND  "
    "                    mtSet.mtMS = 0 AND mtSet.mtSet = 0 "
    "    LEFT OUTER JOIN XxRec xxA ON stA = xxA.stID  "
    "    LEFT OUTER JOIN XxRec xxX ON stX = xxX.stID  ";

  return str;
}


bool  MtStore::BindRec()
{
  int col = 0;

  BindCol(++col, &mtID);
  BindCol(++col, &mtNr);
  BindCol(++col, &stA);
  BindCol(++col, &stX);
  BindCol(++col, &mtUmpire);
  BindCol(++col, &mtUmpire2);
  BindCol(++col, &mtReverse);
  BindCol(++col, &mtWalkOverA);
  BindCol(++col, &mtWalkOverX);
  BindCol(++col, &mtInjuredA);
  BindCol(++col, &mtInjuredX);
  BindCol(++col, &mtDisqualifiedA);
  BindCol(++col, &mtDisqualifiedX);
  BindCol(++col, &mtMatches);
  BindCol(++col, &mtBestOf);
  BindCol(++col, &mtEvent.grID);
  BindCol(++col, &mtEvent.mtRound);
  BindCol(++col, &mtEvent.mtMatch);
  BindCol(++col, &mtEvent.mtChance);
  BindCol(++col, &mtPlace.mtDateTime);
  BindCol(++col, &mtPlace.mtTable);
  BindCol(++col, &mtResA);
  BindCol(++col, &mtResX);
  BindCol(++col, &mtSetsA);
  BindCol(++col, &mtSetsX);
  BindCol(++col, &mtBallsA);
  BindCol(++col, &mtBallsX);
  BindCol(++col, &tmA);
  BindCol(++col, &tmX);
  BindCol(++col, &xxAstID);
  BindCol(++col, &xxXstID);

  return true;
}


// -----------------------------------------------------------------------
// Import / Export Results
// Aufbau: cpName, grName, mtRound, mtMatch, mtMs, \
//         mtPointsA, mtPointsX, mtSetsA, mtSetsX, \
//         mtBallsA[0], mtBallsX[0], ...

// Siehe PlStore zur Verwendung von std::ifstream
bool MtStore::ImportResults(const wxString &name)
{
  long version = 1;

  wxTextFile ifs(name);
  if (!ifs.Open())
    return false;

  wxString line = ifs.GetFirstLine();

  // Check header
  if (!CheckImportHeader(line, "#RESULTS", version))
  {
    ifs.Close();
    if (!infoSystem.Question(_("First comment is not %s but \"%s\". Continue anyway?"), wxT("#RESULTS"), line.c_str()))
      return false;
  }

  if (version > 1)
  {
    infoSystem.Error(_("Version %d of import file is not supported"), version);
    return false;
  }

  Connection *connPtr = TTDbse::instance()->GetNewConnection();

  connPtr->StartTransaction();

  // HACK: Die Variablen duerfen nicht laenger leben als connPtr [
  {
  CpStore  cp(connPtr);
  GrStore  gr(connPtr);
  MtStore  mt(connPtr);

  for (; !ifs.Eof(); line = ifs.GetNextLine())
  {   
    CTT32App::ProgressBarStep();

    if (line.GetChar(0) == '#')
      continue;

    // WB, Gruppe, Position auslesen
    wxStringTokenizerEx tokens(line, ",;\t");
    wxString strCp = tokens.GetNextToken();
    wxString strGr = tokens.GetNextToken();
    wxString strMtRound = tokens.GetNextToken();
    wxString strMtMatch = tokens.GetNextToken();
    wxString strMtMS    = tokens.GetNextToken();

    if (strCp.IsEmpty() || strGr.IsEmpty() || 
        strMtRound.IsEmpty() || strMtMatch.IsEmpty() || strMtMS.IsEmpty())
      continue;

    if ( gr.grID && gr.grModus == MOD_RR &&
         (wxStrcoll(strCp, cp.cpName) || wxStrcoll(strGr, gr.grName)) )
    {
      gr.SetTable();
    }

    if (wxStrcoll(strCp, cp.cpName))
    {
      // Naechster WB
      cp.SelectByName(strCp);
      if (!cp.Next())
        continue;

      // Ausserdem Gruppe lesen
      gr.SelectByName(strGr, cp);
      if (!gr.Next())
        continue;
    }
    
    if (wxStrcoll(strGr, gr.grName))
    {
      // Naechste Gruppe bei gleichem WB
      gr.SelectByName(strGr, cp);
      if (!gr.Next())
        continue;
    }

    // Event aufsetzen
    MtEvent  mtEvent;
    mtEvent.grID = gr.grID;
    mtEvent.mtRound  = _strtos(strMtRound);
    mtEvent.mtMatch  = _strtos(strMtMatch);
    mtEvent.mtChance = 0;

    if (!mt.SelectByEvent(mtEvent) || !mt.Next())
      continue;

    mt.Close();

    wxString strMtPointsA = tokens.GetNextToken();
    wxString strMtPointsX = tokens.GetNextToken();
    
    if (strMtPointsA.IsEmpty() || strMtPointsX.IsEmpty())
      continue;

    wxString strMtSetsA = tokens.GetNextToken();
    wxString strMtSetsX = tokens.GetNextToken();

    if (strMtSetsA.IsEmpty() || strMtSetsX.IsEmpty())
      continue;

    // mt.mtResA = _strtos(strMtSetsA);
    // mt.mtResX = _strtos(strMtSetsX);

    MtSet *mtSets = new MtSet[mt.mtBestOf];
    memset(mtSets, 0, mt.mtBestOf * sizeof(MtSet));
    
    wxString strMtReverse;
    wxString strMtWalkOver;

    for (int nr = 0; ; nr++)
    {
      wxString strMtSetA = tokens.GetNextToken();
      wxString strMtSetX = tokens.GetNextToken();
      
      if (strMtSetA.IsEmpty())
        break;
        
      if (strMtSetX.IsEmpty())
      {
        strMtReverse = strMtSetA;
        break;
      }

      if (!isdigit(*strMtSetA.t_str()))
      {
        strMtWalkOver = strMtSetA;
        break;
      }
      
      if (!isdigit(*strMtSetX.t_str()))
      {
        strMtReverse = strMtSetA;
        strMtWalkOver = strMtSetX;
        break;
      }
      
      if (nr >= mt.mtBestOf)
        continue;
        
      mtSets[nr].mtID = mt.mtID;
      mtSets[nr].mtMS = _strtos(strMtMS);
      mtSets[nr].mtSet = nr+1;
      mtSets[nr].mtResA = _strtos(strMtSetA);
      mtSets[nr].mtResX = _strtos(strMtSetX);
    }
    
    mt.mtWalkOverA = mt.mtWalkOverX = 0;
    
    if (strMtReverse.IsEmpty())
      strMtReverse = tokens.GetNextToken();
      
    if (strMtWalkOver.IsEmpty())
      strMtWalkOver = tokens.GetNextToken();
      
    if (!strMtReverse.IsEmpty())
    {
      mt.mtReverse = _strtos(strMtReverse) ? 1 : 0;
    }
    else
    {
      mt.mtReverse = 0;
    }
    
    mt.UpdateReverseFlag();
      
    if (!strMtWalkOver.IsEmpty())
    {
      if (*strMtWalkOver.t_str() == 'A' || *strMtWalkOver.t_str() == 'a')      
        mt.mtWalkOverA = 1;
      else if (*strMtWalkOver.t_str() == 'X' || *strMtWalkOver.t_str() == 'x')
        mt.mtWalkOverX = 1;
    }
    else
    {
      mt.mtWalkOverA = mt.mtWalkOverX = 0;
    }

    mt.UpdateResult(mt.mtBestOf, mtSets, mt.mtWalkOverA, mt.mtWalkOverX);

    // Update mtChecked. Das Flag wird gesetzt, wenn das Spiel fertig ist, dafuer muss es aber nochmal gelesen werden
    {
      MtStore mtMatch(connPtr);
      mtMatch.SelectById(mt.mtID);
      mtMatch.Next();

      mt.UpdateScoreChecked(mt.mtID, mtMatch.IsFinished());
    }

    if (gr.grModus != MOD_RR)
    {
      gr.SetWinner(mt);
      gr.SetLoser(mt);
    }

    delete[] mtSets;
  }

  // Abschliesendes Berechnen der Tabelle
  if (gr.grModus == MOD_RR)
    gr.SetTable();

  } // end HACK ]

  connPtr->Commit();
  delete connPtr;

  return true;
}


bool  MtStore::ExportResults(wxTextBuffer &os, short cpType, const std::vector<long> &idList, bool append)
{
  // Aufbau: cpName, grName, mtRound, mtMatch, mtMs, \
  //         mtPointsA, mtPointsX, mtSetsA, mtSetsX, \
  //         mtBallsA[0], mtBallsX[0], ...
  
  long version = 1;

  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  if (!append)
  {
    os.AddLine(wxString::Format("#RESULTS %d", version));

    wxString line;
    line << "# Event; Group; Round; Match; Team Match; Result A; Result X; Games A; Games X; ";
  
    for (int i = 0; i < 9; i++)
      line << " Game " << i << " Points A; Game " << i << " Points X;";
    
    line << " Reverse Flag; Walk over A/X";

    os.AddLine(line);
  }
        
  for (std::vector<long>::const_iterator it = idList.begin(); it != idList.end(); it++)
  {
    wxString line;
    long grID = (*it);

    Statement  *stmtPtr = connPtr->CreateStatement();
    ResultSet  *resPtr  = 0;
    
    wxString str = 
      "SELECT mtNr, cpName, grName, mtRound, mtMatch, mtMS, mtSet, "
      "       mtResA, mtResX, mtSetsA, mtSetsX, mtBallsA, mtBallsX, "
      "       mtReverse, "
      "       mtWalkOverA, mtWalkOverX, "
      "       mtDisqualifiedA, mtDisqualifiedX, "
      "       mtInjuredA, mtInjuredX "
      "  FROM MtList mt "
      " WHERE mt.grID = " + ltostr(grID) +
      "   AND mtSet <> 0 AND (mtResA <> 0 OR mtResX <> 0) "
      " ORDER BY mtRound, mtMatch, mtMS, mtSet" ;

    try
    {
      if ( stmtPtr->Execute(str) )
        resPtr = stmtPtr->GetResultSet(false);
    }
    catch (SQLException &e)
    {
      infoSystem.Exception(str, e);
      return false;
    }
    
    long    mtNr = 0, lastMtNr = 0;
    wxChar  cpName[9];
    wxChar  grName[9];
    short   mtRound;
    short   mtMatch;
    short   mtMS = 0, lastMtMS = 0;
    short   mtSet;
    short   mtResA;
    short   mtResX;
    short   mtSetsA;
    short   mtSetsX;
    short   mtBallsA;
    short   mtBallsX;
    short   mtReverse;
    short   mtWalkOverA, lastMtWalkOverA = 0;
    short   mtWalkOverX, lastMtWalkOverX = 0;
    
    resPtr->BindCol(1, &mtNr);
    resPtr->BindCol(2, cpName, sizeof(cpName));
    resPtr->BindCol(3, grName, sizeof(grName));
    resPtr->BindCol(4, &mtRound);
    resPtr->BindCol(5, &mtMatch);
    resPtr->BindCol(6, &mtMS);
    resPtr->BindCol(7, &mtSet);
    resPtr->BindCol(8, &mtResA);
    resPtr->BindCol(9, &mtResX);
    resPtr->BindCol(10, &mtSetsA);
    resPtr->BindCol(11, &mtSetsX);
    resPtr->BindCol(12, &mtBallsA);
    resPtr->BindCol(13, &mtBallsX);
    resPtr->BindCol(14, &mtReverse);
    resPtr->BindCol(15, &mtWalkOverA);
    resPtr->BindCol(16, &mtWalkOverX);

    
    short set = 0;
    while (resPtr->Next())
    {
      if (cpType == CP_TEAM && mtMS == 0)
        continue;

      if (mtNr != lastMtNr || mtMS != lastMtMS)
      {      
        if (lastMtNr)
        {
          while (set++ < 9)
            line << "0;0;";
            
          line << mtReverse << ";";
          if (lastMtWalkOverA)
            line << "A;";
          else if (lastMtWalkOverX)
            line << "X;";          
        }
        
        set = 0;
        
        os.AddLine(line);
        
        line << cpName << ";" << grName << ";" 
            << mtRound << ";" << mtMatch << ";" << mtMS << ";" 
            << mtResA << ";" << mtResX << ";" << mtSetsA << ";" << mtSetsX << ";" ;
      }

      lastMtNr = mtNr;
      lastMtMS = mtMS;
      
      lastMtWalkOverA = mtWalkOverA;
      lastMtWalkOverX = mtWalkOverX;

      ++set;
      line << mtBallsA << ";" << mtBallsX << ";";
    }
    
    while (set++ < 9)
      line << "0;0;";
            
    line << mtReverse << ";";
    
    if (lastMtWalkOverA)
      line << "A;";
    else if (lastMtWalkOverX)
      line << "X;";          

    os.AddLine(line);
    
    delete resPtr;
    delete stmtPtr;
  }

  return true;
}


bool  MtStore::ExportForRanking(wxTextBuffer &os, short cpType, const std::vector<long> &idList, bool append)
{
  // Aufbau: cpName, grName, mtRound, mtMatch, mtMs, \
  //         mtPointsA, mtPointsX, mtSetsA, mtSetsX, \
  //         mtBallsA[0], mtBallsX[0], ...
  
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  if (!append)
  {    
    os.AddLine(
          "# Scheduled;Match No;Event;Group;Stage;Round;Match;Indiv. Match;"
          "Pl. A ID;Pl. A Family Name;Pl. A Given Name;Pl. A Association;Pl. A Region;"
          "Pl. B ID;Pl. B Family Name;Pl. B Given Name;Pl. B Association;Pl. B Region;"
          "Pl. X ID;Pl. X Family Name;Pl. X Given Name;Pl. X Association;Pl. X Region;"
          "Pl. Y ID;Pl. Y Family Name;Pl. Y Given Name;Pl. Y Association;Pl. Y Region;"
          "Best Of;"
          "Game 1 Points A;Game 1 Points X;"
          "Game 2 Points A;Game 2 Points X;"
          "Game 3 Points A;Game 3 Points X;"
          "Game 4 Points A;Game 4 Points X;"
          "Game 5 Points A;Game 5 Points X;"
          "Game 6 Points A;Game 6 Points X;"
          "Game 7 Points A;Game 7 Points X;"
          "Result A;Result X;"
          "Lost by w/o A;Lost by w/o X;"
          "InjuredA;InjuredX;"
          "DisqualifiedA;DisqualifiedX"
    );
  }
  
  for (std::vector<long>::const_iterator it = idList.begin(); it != idList.end(); it++)
  {
    Statement  *stmtPtr = connPtr->CreateStatement();
    ResultSet  *resPtr  = 0;

    wxString str;
    long grID = (*it);

    if (cpType == CP_SINGLE)
      str = 
        "SELECT "
        "mtDateTime, mtNr, cpName, grName, grStage, mtRound, mtMatch, NULL AS mtMS, "
        "plAplExtID, plApsLast, plApsFirst, plAnaName, plAnaRegion, "
        "NULL AS plBplExtId, NULL AS plBpsLast, NULL AS plBpsFirst, NULL AS plBnaName, NULL AS plBnaRegion, "
        "plXplExtID,  plXpsLast, plXpsFirst, plXnaName, plXnaRegion, "
        "NULL AS plYplExtId, NULL AS plYpsLast, NULL AS plYpsFirst, NULL AS plYnaName, NULL AS plYnaRegion, "
        "mtBestOf, "
        "mtSet1.mtResA, mtSet1.mtResX, mtSet2.mtResA, mtSet2.mtResX, mtSet3.mtResA, mtSet3.mtResX, "
        "mtSet4.mtResA, mtSet4.mtResX, mtSet5.mtResA, mtSet5.mtResX, mtSet6.mtResA, mtSet6.mtResX, mtSet7.mtResA, mtSet7.mtResX, "
        "mt.mtResA, mt.mtResX, mtWalkOverA, mtWalkOverX, mtInjuredA, mtInjuredX, mtDisqualifiedA, mtDisqualifiedX "
        "FROM MtSingleList mt INNER JOIN GrList gr ON mt.grID = gr.grID INNER JOIN CpList cp ON gr.cpID = cp.cpID AND cp.cpType = 1 "
        "LEFT OUTER JOIN MtSet mtSet1 ON mtSet1.mtID = mt.mtID AND mtSet1.mtSet = 1 "
        "LEFT OUTER JOIN MtSet mtSet2 ON mtSet2.mtID = mt.mtID AND mtSet2.mtSet = 2 "
        "LEFT OUTER JOIN MtSet mtSet3 ON mtSet3.mtID = mt.mtID AND mtSet3.mtSet = 3 "
        "LEFT OUTER JOIN MtSet mtSet4 ON mtSet4.mtID = mt.mtID AND mtSet4.mtSet = 4 "
        "LEFT OUTER JOIN MtSet mtSet5 ON mtSet5.mtID = mt.mtID AND mtSet5.mtSet = 5 "
        "LEFT OUTER JOIN MtSet mtSet6 ON mtSet6.mtID = mt.mtID AND mtSet6.mtSet = 6 "
        "LEFT OUTER JOIN MtSet mtSet7 ON mtSet7.mtID = mt.mtID AND mtSet7.mtSet = 7 "
        "WHERE mtDateTime IS NOT NULL AND gr.grID = " + ltostr(grID) +
        "ORDER BY cpName, mtDateTime, mtNr ";
    else if (cpType == CP_DOUBLE || cpType == CP_MIXED)
      str = 
        "SELECT "
        "mtDateTime, mtNr, cpName, grName, grStage, mtRound, mtMatch, NULL AS mtMS, "
        "plAplExtID, plApsLast, plApsFirst, plAnaName, plAnaRegion, "
        "plBplExtId, plBpsLast, plBpsFirst, plBnaName, plBnaRegion, "
        "plXplExtID, plXpsLast, plXpsFirst, plXnaName, plXnaRegion, "
        "plYplExtId, plYpsLast, plYpsFirst, plYnaName, plYnaRegion, "
        "mtBestOf, "
        "mtSet1.mtResA, mtSet1.mtResX, mtSet2.mtResA, mtSet2.mtResX, mtSet3.mtResA, mtSet3.mtResX, "
        "mtSet4.mtResA, mtSet4.mtResX, mtSet5.mtResA, mtSet5.mtResX, mtSet6.mtResA, mtSet6.mtResX, mtSet7.mtResA, mtSet7.mtResX, "
        "mt.mtResA, mt.mtResX, mtWalkOverA, mtWalkOverX, mtInjuredX, mtDisqualifiedA, mtDisqualifiedX "
        "FROM MtDoubleList mt INNER JOIN GrList gr ON mt.grID = gr.grID INNER JOIN CpList cp ON gr.cpID = cp.cpID AND (cp.cpType = 2 OR cp.cpType = 3)"
        "LEFT OUTER JOIN MtSet mtSet1 ON mtSet1.mtID = mt.mtID AND mtSet1.mtSet = 1 "
        "LEFT OUTER JOIN MtSet mtSet2 ON mtSet2.mtID = mt.mtID AND mtSet2.mtSet = 2 "
        "LEFT OUTER JOIN MtSet mtSet3 ON mtSet3.mtID = mt.mtID AND mtSet3.mtSet = 3 "
        "LEFT OUTER JOIN MtSet mtSet4 ON mtSet4.mtID = mt.mtID AND mtSet4.mtSet = 4 "
        "LEFT OUTER JOIN MtSet mtSet5 ON mtSet5.mtID = mt.mtID AND mtSet5.mtSet = 5 "
        "LEFT OUTER JOIN MtSet mtSet6 ON mtSet6.mtID = mt.mtID AND mtSet6.mtSet = 6 "
        "LEFT OUTER JOIN MtSet mtSet7 ON mtSet7.mtID = mt.mtID AND mtSet7.mtSet = 7 "
        "WHERE mtDateTime IS NOT NULL AND gr.grID = " + ltostr(grID) +
        "ORDER BY cpName, mtDateTime, mtNr ";
    else if (cpType == CP_TEAM)
      str = 
        "SELECT "
        "mtDateTime, mtNr, cpName, grName, grStage, mtRound, mtMatch, mtSet1.mtMS, "
        "plAplExtID, plApsLast, plApsFirst, plAnaName, plAnaRegion, "
        "plBplExtId, plBpsLast, plBpsFirst, plBnaName, plBnaRegion, "
        "plXplExtID, plXpsLast, plXpsFirst, plXnaName, plXnaRegion, "
        "plYplExtId, plYpsLast, plYpsFirst, plYnaName, plYnaRegion, "
        "mtBestOf, "
        "mtSet1.mtResA, mtSet1.mtResX, mtSet2.mtResA, mtSet2.mtResX, mtSet3.mtResA, mtSet3.mtResX, "
        "mtSet4.mtResA, mtSet4.mtResX, mtSet5.mtResA, mtSet5.mtResX, mtSet6.mtResA, mtSet6.mtResX, mtSet7.mtResA, mtSet7.mtResX, "
        "mt.mtResA, mt.mtResX, mtWalkOverA, mtWalkOverX, mtInjuredX, mtDisqualifiedA, mtDisqualifiedX "
        "FROM MtIndividualList mt INNER JOIN GrList gr ON mt.grID = gr.grID INNER JOIN CpList cp ON gr.cpID = cp.cpID AND cp.cpType = 4 "
        "LEFT OUTER JOIN MtSet mtSet1 ON mtSet1.mtID = mt.mtID AND mtSet1.mtSet = 1 AND mtSet1.mtMS = mt.mtMS "
        "LEFT OUTER JOIN MtSet mtSet2 ON mtSet2.mtID = mt.mtID AND mtSet2.mtSet = 2 AND mtSet2.mtMS = mt.mtMS "
        "LEFT OUTER JOIN MtSet mtSet3 ON mtSet3.mtID = mt.mtID AND mtSet3.mtSet = 3 AND mtSet3.mtMS = mt.mtMS "
        "LEFT OUTER JOIN MtSet mtSet4 ON mtSet4.mtID = mt.mtID AND mtSet4.mtSet = 4 AND mtSet4.mtMS = mt.mtMS "
        "LEFT OUTER JOIN MtSet mtSet5 ON mtSet5.mtID = mt.mtID AND mtSet5.mtSet = 5 AND mtSet5.mtMS = mt.mtMS "
        "LEFT OUTER JOIN MtSet mtSet6 ON mtSet6.mtID = mt.mtID AND mtSet6.mtSet = 6 AND mtSet6.mtMS = mt.mtMS "
        "LEFT OUTER JOIN MtSet mtSet7 ON mtSet7.mtID = mt.mtID AND mtSet7.mtSet = 7 AND mtSet7.mtMS = mt.mtMS "
        "WHERE mtDateTime IS NOT NULL AND mtSet1.mtMS IS NOT NULL AND gr.grID = " + ltostr(grID) +
        "ORDER BY cpName, mtDateTime, mtNr, mt.mtMS";
    else
      return false;

    try
    {
      if ( stmtPtr->Execute(str) )
        resPtr = stmtPtr->GetResultSet(false);
    }
    catch (SQLException &e)
    {
      infoSystem.Exception(str, e);
      return false;
    }
    
    while (resPtr->Next())
    {
      if (resPtr->WasNull(1))
        continue;

      wxChar str[128];
      wxString line;

      for (int col = 1, numCol = resPtr->GetColumnCount(); col <= numCol; col++)
      {
        if (resPtr->GetData(col, str, 128) && !resPtr->WasNull())
          line << str;
        line << ";";
      }

      os.AddLine(line);
    }
    
    
    delete resPtr;
    delete stmtPtr;
  }

  return true;
}


bool  MtStore::ExportForRankingITTF(wxTextBuffer &os, short cpType, const std::vector<long> &idList, bool append)
{
  // Aufbau: cpName, grName, mtRound, mtMatch, mtMs, \
  //         mtPointsA, mtPointsX, mtSetsA, mtSetsX, \
  //         mtBallsA[0], mtBallsX[0], ...

  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  if (!append)
  {
    os.AddLine(
      "TourID; Level; IDA; IRGA; IDB; ORGB; IDX; ORGX; IDY; ORGY; "
      "Event; Stage; Group; Group No; Round; Seq; Format; Desc; "
      "Game 1 Points A;Game 1 Points X;"
      "Game 2 Points A;Game 2 Points X;"
      "Game 3 Points A;Game 3 Points X;"
      "Game 4 Points A;Game 4 Points X;"
      "Game 5 Points A;Game 5 Points X;"
      "Game 6 Points A;Game 6 Points X;"
      "Game 7 Points A;Game 7 Points X;"
      "Result A;Result X;"
      "Winner; Winner Doubles; IRM; Kind; "
      "Year; Date; Time; Table; "
      "Group Rank A; GroupRank X; Final Rank A; Final Rank X; Seeded A; Seeded X;"
    );
  }

  for (std::vector<long>::const_iterator it = idList.begin(); it != idList.end(); it++)
  {
    long grID = (*it);

    // Ich brauche die "Gruppennummer", das ist die wievielte Gruppe ist dieses
    long seq = 0;

    try 
    {
      GrListStore gr(connPtr);
      gr.SelectById(grID);
      gr.Next();
      gr.Close();

      Statement *stmtPtr = connPtr->CreateStatement();
      wxString str = "SELECT COUNT(*) FROM GrList gr "
          "WHERE gr.cpID = " + ltostr(gr.cpID) + " "
          "  AND ((grStage = '" + TransformString(gr.grStage) + "' AND grName <= '" + TransformString(gr.grName) + "') "
          "   OR (grStage < '" + TransformString(gr.grStage) + "')) "
      ;

      ResultSet *resPtr = stmtPtr->ExecuteQuery(str);
      if (!resPtr->Next() || !resPtr->GetData(1, seq) || resPtr->WasNull(1))
        seq = 1;

      delete resPtr;
      delete stmtPtr;
    }
    catch (SQLException &) 
    {
      seq = 1;
    }

    wxString sql;
    wxString teamSql;

    std::map<long, wxString> teamMatchMap;

    if (cpType == CP_SINGLE)
    {
      sql =
      "SELECT "
      "mt.mtID, "
      "NULL, NULL, plAplExtID, plAnaName, NULL, NULL, plXplExtID, plXnaName, NULL, NULL, "
      "cpName, grStage, grName, IIF(grModus = 1, " + ltostr(seq) + ", NULL), IIF(grModus = 2, grSize / POWER(2, (mtRound - 1)), mtRound), mtMatch, NULL, "
      "CONCAT(cpDesc, ' - ', IIF(grModus = 2, CONCAT('Round of ', grSize / POWER(2, (mtRound - 1))), CONCAT('Round ', mtRound)), ' - Match - ', mt.mtMatch), "
      "mtSet1.mtResA, mtSet1.mtResX, mtSet2.mtResA, mtSet2.mtResX, mtSet3.mtResA, mtSet3.mtResX, "
      "mtSet4.mtResA, mtSet4.mtResX, mtSet5.mtResA, mtSet5.mtResX, mtSet6.mtResA, mtSet6.mtResX, mtSet7.mtResA, mtSet7.mtResX, "
      "mt.mtResA, mt.mtResX, "
      "IIF(mt.mtResA > mt.mtResX, plAplExtID, plXplExtID), NULL, "
      "CASE WHEN mtWalkOverA > 0 OR mtWalkOverX > 0 THEN 'WO' WHEN mtInjuredA > 0 OR mtInjuredX > 0 THEN 'INJ' WHEN mtDisqualifiedA > 0 OR mtDisqualifiedX > 0 THEN 'DSQ' ELSE NULL END, "
      "'Singles', "
      "YEAR(mt.mtDateTime), FORMAT(mtDateTime,'yyyy-MM-dd'), FORMAT(mtDateTime, 'hh\\:mm'), mt.mtTable, "
      "IIF(grModus = 1, stA.stPos + grWinner - 1, NULL), IIF(grModus = 1, stX.stPos + grWinner - 1, NULL), "
      "IIF(grModus = 1, NULL, stA.stPos + grWinner - 1), IIF(grModus = 1, NULL, stX.stPos + grWinner - 1), "
      "stA.stSeeded, stX.stSeeded "
      "FROM MtSingleList mt INNER JOIN GrList gr ON mt.grID = gr.grID INNER JOIN CpList cp ON gr.cpID = cp.cpID AND cp.cpType = 1 "
      "LEFT OUTER JOIN StList stA ON mt.stA = stA.stID "
      "LEFT OUTER JOIN StList stX ON mt.stX = stX.stID "
      "LEFT OUTER JOIN MtSet mtSet1 ON mtSet1.mtID = mt.mtID AND mtSet1.mtSet = 1 "
      "LEFT OUTER JOIN MtSet mtSet2 ON mtSet2.mtID = mt.mtID AND mtSet2.mtSet = 2 "
      "LEFT OUTER JOIN MtSet mtSet3 ON mtSet3.mtID = mt.mtID AND mtSet3.mtSet = 3 "
      "LEFT OUTER JOIN MtSet mtSet4 ON mtSet4.mtID = mt.mtID AND mtSet4.mtSet = 4 "
      "LEFT OUTER JOIN MtSet mtSet5 ON mtSet5.mtID = mt.mtID AND mtSet5.mtSet = 5 "
      "LEFT OUTER JOIN MtSet mtSet6 ON mtSet6.mtID = mt.mtID AND mtSet6.mtSet = 6 "
      "LEFT OUTER JOIN MtSet mtSet7 ON mtSet7.mtID = mt.mtID AND mtSet7.mtSet = 7 "
      "WHERE mtDateTime IS NOT NULL AND gr.grID = " + ltostr(grID) +
      "ORDER BY cpName, mtDateTime, mtNr ";
    }
    else if (cpType == CP_DOUBLE || cpType == CP_MIXED)
    {
      sql =
      "SELECT "
      "mt.mtID, "
      "NULL, NULL, plAplExtID, plAnaName, plBplExtID, plBnaName, plXplExtID, plXnaName, plYplExtID, plYnaName, "
      "cpName, grStage, grName, IIF(grModus = 1, " + ltostr(seq) + ", NULL), IIF(grModus = 2, grSize / POWER(2, (mtRound - 1)), mtRound), mtMatch, NULL, "
      "CONCAT(cpDesc, ' - ', IIF(grModus = 2, CONCAT('Round of ', grSize / POWER(2, (mtRound - 1))), CONCAT('Round ', mtRound)), ' - Match - ', mt.mtMatch), "
      "mtSet1.mtResA, mtSet1.mtResX, mtSet2.mtResA, mtSet2.mtResX, mtSet3.mtResA, mtSet3.mtResX, "
      "mtSet4.mtResA, mtSet4.mtResX, mtSet5.mtResA, mtSet5.mtResX, mtSet6.mtResA, mtSet6.mtResX, mtSet7.mtResA, mtSet7.mtResX, "
      "mt.mtResA, mt.mtResX, "
      "IIF(mt.mtResA > mt.mtResX, plAplExtID, plXplExtID), IIF(mt.mtResA > mt.mtResX, plBplExtID, plYplExtID), "
      "CASE WHEN mtWalkOverA > 0 OR mtWalkOverX > 0 THEN 'WO' WHEN mtInjuredA > 0 OR mtInjuredX > 0 THEN 'INJ' WHEN mtDisqualifiedA > 0 OR mtDisqualifiedX > 0 THEN 'DSQ' ELSE NULL END, "
      "CASE cpType WHEN 2 THEN 'Doubles' ELSE 'Mixed' END, "
      "YEAR(mt.mtDateTime), FORMAT(mtDateTime,'yyyy-MM-dd'), FORMAT(mtDateTime, 'hh\\:mm'), mt.mtTable, "
      "IIF(grModus = 1, stA.stPos + grWinner - 1, NULL), IIF(grModus = 1, stX.stPos + grWinner - 1, NULL), "
      "IIF(grModus = 1, NULL, stA.stPos + grWinner - 1), IIF(grModus = 1, NULL, stX.stPos + grWinner - 1), "
      "stA.stSeeded, stX.stSeeded "
      "FROM MtDoubleList mt INNER JOIN GrList gr ON mt.grID = gr.grID INNER JOIN CpList cp ON gr.cpID = cp.cpID AND (cp.cpType = 2 OR cpType = 3) "
      "LEFT OUTER JOIN StList stA ON mt.stA = stA.stID "
      "LEFT OUTER JOIN StList stX ON mt.stX = stX.stID "
      "LEFT OUTER JOIN MtSet mtSet1 ON mtSet1.mtID = mt.mtID AND mtSet1.mtSet = 1 "
      "LEFT OUTER JOIN MtSet mtSet2 ON mtSet2.mtID = mt.mtID AND mtSet2.mtSet = 2 "
      "LEFT OUTER JOIN MtSet mtSet3 ON mtSet3.mtID = mt.mtID AND mtSet3.mtSet = 3 "
      "LEFT OUTER JOIN MtSet mtSet4 ON mtSet4.mtID = mt.mtID AND mtSet4.mtSet = 4 "
      "LEFT OUTER JOIN MtSet mtSet5 ON mtSet5.mtID = mt.mtID AND mtSet5.mtSet = 5 "
      "LEFT OUTER JOIN MtSet mtSet6 ON mtSet6.mtID = mt.mtID AND mtSet6.mtSet = 6 "
      "LEFT OUTER JOIN MtSet mtSet7 ON mtSet7.mtID = mt.mtID AND mtSet7.mtSet = 7 "
      "WHERE mtDateTime IS NOT NULL AND gr.grID = " + ltostr(grID) +
      "ORDER BY cpName, mtDateTime, mtNr ";
    }
    else if (cpType == CP_TEAM)
    {
      sql =
        "SELECT "
        "mt.mtID, "
        "NULL, NULL, plAplExtID, plAnaName, plBplExtID, plBnaName, plXplExtID, plXnaName, plYplExtID, plYnaName, "
        "cpName, grStage, grName, IIF(grModus = 1, " + ltostr(seq) + ", NULL), IIF(grModus = 2, grSize / POWER(2, (mtRound - 1)), mtRound), 10 * mtMatch + mt.mtMS, NULL, "
        "CONCAT('Match ', mt.mtMatch, ' M', mt.mtMS), "
        "mtSet1.mtResA, mtSet1.mtResX, mtSet2.mtResA, mtSet2.mtResX, mtSet3.mtResA, mtSet3.mtResX, "
        "mtSet4.mtResA, mtSet4.mtResX, mtSet5.mtResA, mtSet5.mtResX, mtSet6.mtResA, mtSet6.mtResX, mtSet7.mtResA, mtSet7.mtResX, "
        "mt.mtResA, mt.mtResX, "
        "IIF(mt.mtResA > mt.mtResX, plAplExtID, plXplExtID), IIF(mt.mtResA > mt.mtResX, plBplExtID, plYplExtID), "
        "CASE WHEN mtWalkOverA > 0 OR mtWalkOverX > 0 THEN 'WO' WHEN mtInjuredA > 0 OR mtInjuredX > 0 THEN 'INJ' WHEN mtDisqualifiedA > 0 OR mtDisqualifiedX > 0 THEN 'DSQ' ELSE NULL END, "
        "CASE WHEN nmType = 1 THEN 'Singles' ELSE 'Doubles' END, "
        "YEAR(mt.mtDateTime), FORMAT(mtDateTime,'yyyy-MM-dd'), FORMAT(mtDateTime, 'hh\\:mm'), mt.mtTable, "
        "NULL, NULL, "
        "NULL, NULL, "
        "NULL, NULL "
        "FROM MtIndividualList mt INNER JOIN GrList gr ON mt.grID = gr.grID INNER JOIN CpList cp ON gr.cpID = cp.cpID AND cp.cpType = 4 "
        "LEFT OUTER JOIN StList stA ON mt.stA = stA.stID "
        "LEFT OUTER JOIN StList stX ON mt.stX = stX.stID "
        "LEFT OUTER JOIN MtSet mtSet1 ON mtSet1.mtID = mt.mtID AND mtSet1.mtMS = mt.mtMS AND mtSet1.mtSet = 1 "
        "LEFT OUTER JOIN MtSet mtSet2 ON mtSet2.mtID = mt.mtID AND mtSet2.mtMS = mt.mtMS AND mtSet2.mtSet = 2 "
        "LEFT OUTER JOIN MtSet mtSet3 ON mtSet3.mtID = mt.mtID AND mtSet3.mtMS = mt.mtMS AND mtSet3.mtSet = 3 "
        "LEFT OUTER JOIN MtSet mtSet4 ON mtSet4.mtID = mt.mtID AND mtSet4.mtMS = mt.mtMS AND mtSet4.mtSet = 4 "
        "LEFT OUTER JOIN MtSet mtSet5 ON mtSet5.mtID = mt.mtID AND mtSet5.mtMS = mt.mtMS AND mtSet5.mtSet = 5 "
        "LEFT OUTER JOIN MtSet mtSet6 ON mtSet6.mtID = mt.mtID AND mtSet6.mtMS = mt.mtMS AND mtSet6.mtSet = 6 "
        "LEFT OUTER JOIN MtSet mtSet7 ON mtSet7.mtID = mt.mtID AND mtSet7.mtMS = mt.mtMS AND mtSet7.mtSet = 7 "
        "WHERE mtDateTime IS NOT NULL AND gr.grID = " + ltostr(grID) + " AND (mt.mtResA + mt.mtResX) > 0 "
        "ORDER BY cpName, mtDateTime, mt.mtNr, mt.mtMS ";

      teamSql =
        "SELECT "
        "mt.mtID, "
        "NULL, NULL, "
        "CONCAT('T', IIF(cp.cpSex = 1, 'M', 'W'), mt.tmAnaName, FORMAT(mt.tmAtmID, '0000')), tmAnaName, NULL, NULL, "
        "CONCAT('T', IIF(cp.cpSex = 1, 'M', 'W'), mt.tmXnaName, FORMAT(mt.tmXtmID, '0000')), tmXnaName, NULL, NULL, "
        "cpName, grStage, grName, IIF(grModus = 1, " + ltostr(seq) + ", NULL), IIF(grModus = 2, grSize / POWER(2, (mtRound - 1)), mtRound), mtMatch, NULL, "
        "CONCAT(cpDesc, ' - ', IIF(grModus = 2, CONCAT('Round of ', grSize / POWER(2, (mtRound - 1))), CONCAT('Round ', mtRound)), ' - Match - ', mt.mtMatch), "
        "IIF(mt.mtResA + mt.mtResX < 1, '', CAST((SELECT COUNT(*) FROM MtMatch WHERE MtMatch.mtID = mt.mtID AND MtMatch.mtMS > 0 AND MtMatch.mtMS <= 1 AND MtMatch.mtResA > MtMatch.mtResX) AS VARCHAR(2))), "
        "IIF(mt.mtResA + mt.mtResX < 1, '', CAST((SELECT COUNT(*) FROM MtMatch WHERE MtMatch.mtID = mt.mtID AND MtMatch.mtMS > 0 AND MtMatch.mtMS <= 1 AND MtMatch.mtResA < MtMatch.mtResX) AS VARCHAR(2))), "
        "IIF(mt.mtResA + mt.mtResX < 2, '', CAST((SELECT COUNT(*) FROM MtMatch WHERE MtMatch.mtID = mt.mtID AND MtMatch.mtMS > 0 AND MtMatch.mtMS <= 2 AND MtMatch.mtResA > MtMatch.mtResX) AS VARCHAR(2))), "
        "IIF(mt.mtResA + mt.mtResX < 2, '', CAST((SELECT COUNT(*) FROM MtMatch WHERE MtMatch.mtID = mt.mtID AND MtMatch.mtMS > 0 AND MtMatch.mtMS <= 2 AND MtMatch.mtResA < MtMatch.mtResX) AS VARCHAR(2))), "
        "IIF(mt.mtResA + mt.mtResX < 3, '', CAST((SELECT COUNT(*) FROM MtMatch WHERE MtMatch.mtID = mt.mtID AND MtMatch.mtMS > 0 AND MtMatch.mtMS <= 3 AND MtMatch.mtResA > MtMatch.mtResX) AS VARCHAR(2))), "
        "IIF(mt.mtResA + mt.mtResX < 3, '', CAST((SELECT COUNT(*) FROM MtMatch WHERE MtMatch.mtID = mt.mtID AND MtMatch.mtMS > 0 AND MtMatch.mtMS <= 3 AND MtMatch.mtResA < MtMatch.mtResX) AS VARCHAR(2))), "
        "IIF(mt.mtResA + mt.mtResX < 4, '', CAST((SELECT COUNT(*) FROM MtMatch WHERE MtMatch.mtID = mt.mtID AND MtMatch.mtMS > 0 AND MtMatch.mtMS <= 4 AND MtMatch.mtResA > MtMatch.mtResX) AS VARCHAR(2))), "
        "IIF(mt.mtResA + mt.mtResX < 4, '', CAST((SELECT COUNT(*) FROM MtMatch WHERE MtMatch.mtID = mt.mtID AND MtMatch.mtMS > 0 AND MtMatch.mtMS <= 4 AND MtMatch.mtResA < MtMatch.mtResX) AS VARCHAR(2))), "
        "IIF(mt.mtResA + mt.mtResX < 5, '', CAST((SELECT COUNT(*) FROM MtMatch WHERE MtMatch.mtID = mt.mtID AND MtMatch.mtMS > 0 AND MtMatch.mtMS <= 5 AND MtMatch.mtResA > MtMatch.mtResX) AS VARCHAR(2))), "
        "IIF(mt.mtResA + mt.mtResX < 5, '', CAST((SELECT COUNT(*) FROM MtMatch WHERE MtMatch.mtID = mt.mtID AND MtMatch.mtMS > 0 AND MtMatch.mtMS <= 5 AND MtMatch.mtResA < MtMatch.mtResX) AS VARCHAR(2))), "
        "IIF(mt.mtResA + mt.mtResX < 6, '', CAST((SELECT COUNT(*) FROM MtMatch WHERE MtMatch.mtID = mt.mtID AND MtMatch.mtMS > 0 AND MtMatch.mtMS <= 6 AND MtMatch.mtResA > MtMatch.mtResX) AS VARCHAR(2))), "
        "IIF(mt.mtResA + mt.mtResX < 6, '', CAST((SELECT COUNT(*) FROM MtMatch WHERE MtMatch.mtID = mt.mtID AND MtMatch.mtMS > 0 AND MtMatch.mtMS <= 6 AND MtMatch.mtResA < MtMatch.mtResX) AS VARCHAR(2))), "
        "IIF(mt.mtResA + mt.mtResX < 7, '', CAST((SELECT COUNT(*) FROM MtMatch WHERE MtMatch.mtID = mt.mtID AND MtMatch.mtMS > 0 AND MtMatch.mtMS <= 7 AND MtMatch.mtResA > MtMatch.mtResX) AS VARCHAR(2))), "
        "IIF(mt.mtResA + mt.mtResX < 7, '', CAST((SELECT COUNT(*) FROM MtMatch WHERE MtMatch.mtID = mt.mtID AND MtMatch.mtMS > 0 AND MtMatch.mtMS <= 7 AND MtMatch.mtResA < MtMatch.mtResX) AS VARCHAR(2))), "
        "mt.mtResA, mt.mtResX, "
        "IIF(mt.mtResA > mt.mtResX, CONCAT('T', IIF(cp.cpSex = 1, 'M', 'W'), mt.tmAnaName, FORMAT(mt.tmAtmID, '0000')), CONCAT('T', IIF(cp.cpSex = 1, 'M', 'W'), mt.tmXnaName, FORMAT(mt.tmXtmID, '0000'))), NULL, "
        "CASE WHEN mt.mtWalkOverA > 0 OR mt.mtWalkOverX > 0 THEN 'WO' WHEN mt.mtInjuredA > 0 OR mt.mtInjuredX > 0 THEN 'INJ' WHEN mt.mtDisqualifiedA > 0 OR mt.mtDisqualifiedX > 0 THEN 'DSQ' ELSE NULL END, "
        "'Team', "
        "YEAR(mt.mtDateTime), FORMAT(mtDateTime,'yyyy-MM-dd'), FORMAT(mtDateTime, 'hh\\:mm'), mt.mtTable, "
        "IIF(grModus = 1, stA.stPos + grWinner - 1, NULL), IIF(grModus = 1, stX.stPos + grWinner - 1, NULL), "
        "IIF(grModus = 1, NULL, stA.stPos + grWinner - 1), IIF(grModus = 1, NULL, stX.stPos + grWinner - 1), "
        "NULL, NULL " //  "stA.stSeeded, stX.stSeeded "
        "FROM MtTeamList mt INNER JOIN GrList gr ON mt.grID = gr.grID INNER JOIN CpList cp ON gr.cpID = cp.cpID AND cp.cpType = 4 "
        "LEFT OUTER JOIN StList stA ON mt.stA = stA.stID "
        "LEFT OUTER JOIN StList stX ON mt.stX = stX.stID "
      ;
    }
    else
      return false;

    // Zuerst die Zeilen fuer die Teammatches lesen
    if (cpType == CP_TEAM)
    {
      Statement  *stmtPtr = connPtr->CreateStatement();
      ResultSet  *resPtr = 0;

      try
      {
        if (stmtPtr->Execute(teamSql))
          resPtr = stmtPtr->GetResultSet(false);
      }
      catch (SQLException &e)
      {
        infoSystem.Exception(teamSql, e);
        return false;
      }

      while (resPtr->Next())
      {
        wxString line;
        long   mtID;
        wxChar str[128];

        line.Clear();

        int col = 1;
        int colCount = resPtr->GetColumnCount();

        resPtr->GetData(col++, mtID);

        for (; col <= colCount; col++)
        {
          if (resPtr->GetData(col, str, 128) && !resPtr->WasNull())
            line << str;
          line << ";";
        }

        teamMatchMap[mtID] = line;
      }

      delete resPtr;
      delete stmtPtr;
    }

    long lastMtID = 0;

    Statement  *stmtPtr = connPtr->CreateStatement();
    ResultSet  *resPtr = 0;

    try
    {
      if (stmtPtr->Execute(sql))
        resPtr = stmtPtr->GetResultSet(false);
    }
    catch (SQLException &e)
    {
      infoSystem.Exception(sql, e);
      return false;
    }

    while (resPtr->Next())
    {
      wxString line;

      long   mtID;
      wxChar str[128];

      int col = 1;
      int colCount = resPtr->GetColumnCount();

      resPtr->GetData(col++, mtID);

      if (cpType == CP_TEAM && mtID != lastMtID)
      {
        os.AddLine(teamMatchMap[mtID]);
        lastMtID = mtID;
      }

      for (; col <= colCount; col++)
      {
        if (resPtr->GetData(col, str, 128) && !resPtr->WasNull())
          line << str;
        line << ";";
      }

      os.AddLine(line);
    }

    delete resPtr;
    delete stmtPtr;
  }

  return true;
}


bool  MtStore::ExportForRankingETTU(wxTextBuffer &os, short cpType, const std::vector<long> &idList, bool append)
{
  /*
    Format Individual
    ID	date_time	event	rnd	match	player_id_a	assoc_a	player_id_b	assoc_b	player_id_x	assoc_x	player_id_y	assoc_y	
    g1_a	g1_x	g2_a	g2_x	g3_a	g3_x	g4_a	g4_x	g5_a	g5_x	g6_a	g6_x	g7_a	g7_x	
    res_a	res_x	wo_a	wo_x	yr	type
 
    Format Team
    ID	date_time	event	group_	stage	rnd	match_no	match_	player_id_a	assoc_a	player_id_b	assoc_b	player_id_x	assoc_x	player_id_y	assoc_y	
    g1_a	g1_x	g2_a	g2_x	g3_a	g3_x	g4_a	g4_x	g5_a	g5_x	g6_a	g6_x	g7_a	g7_x	
    res_a	res_x	wo_a	wo_x	res_team_a	res_team_x	yr	type
  */

  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  if (!append)
  {
    if (cpType == CP_TEAM)
      os.AddLine(
          "ID;date_time;event;group_;stage;rnd;match_no;match_;"
          "player_id_a;assoc_a;player_id_b;assoc_b;player_id_x;assoc_x;player_id_y;assoc_y;"
          "g1_a;g1_x;g2_a;g2_x;g3_a;g3_x;g4_a;g4_x;g5_a;g5_x;g6_a;g6_x;g7_a;g7_x;"
          "res_a;res_x;wo_a;wo_x;res_team_a;res_team_x;yr;type"         
      );
    else
      os.AddLine(
          "ID;date_time;event;rnd;match;"
          "player_id_a;assoc_a;player_id_b;assoc_b;player_id_x;assoc_x;player_id_y;assoc_y;"
          "g1_a;g1_x;g2_a;g2_x;g3_a;g3_x;g4_a;g4_x;g5_a;g5_x;g6_a;g6_x;g7_a;g7_x;"
          "res_a;res_x;wo_a;wo_x;yr;type"   
      );
  }

  for (std::vector<long>::const_iterator it = idList.begin(); it != idList.end(); it++)
  {
    long grID = (*it);

    wxString type;
    if (TTDbse::instance()->GetDatabase().StartsWith("EC"))
      type = "1";
    else if (TTDbse::instance()->GetDatabase().StartsWith("EYC"))
      type = "2";
    else if (TTDbse::instance()->GetDatabase().StartsWith("EU21C"))
      type = "3";
    else if (TTDbse::instance()->GetDatabase().StartsWith("EYTT"))
      type = "4";

    wxString sql;
    if (cpType == CP_TEAM)
    {
      sql = 
          "SELECT FORMAT(mtDateTime, 'yyyy-MM-dd hh:mm'), cpName, grName, grStage, "
          "       IIF((grModus <> 2) OR (grWinner <> 1), mtRound, CASE (grSize / POWER(2, mtRound)) WHEN 1 THEN 'F' WHEN 2 THEN 'SF' WHEN 4 THEN 'QF' ELSE CONCAT('R', grSize / POWER(2, mtRound - 1)) END), "
          "       CONCAT(mtNr, YEAR(mtDateTime), '" + type + "'), mtMatch, "
          "       plAplExtID, plAnaName, plBplExtID, plBnaName, plXplExtID, plXnaName, plYplExtID, plYnaName, "
          "       mtSet1.mtResA, mtSet1.mtResX, mtSet2.mtResA, mtSet2.mtResX, mtSet3.mtResA, mtSet3.mtResX, mtSet4.mtResA, mtSet4.mtResX, "
          "       mtSet5.mtResA, mtSet5.mtResX, mtSet6.mtResA, mtSet6.mtResX, mtSet7.mtResA, mtSet7.mtResX, mt.mtResA, mt.mtResX, "
          "       mt.mtWalkOverA, mt.mtWalkOverX, mt.mttmResA, mt.mttmResX, YEAR(mtDateTime), '" + type + "' "
          "  FROM MtIndividualList mt "
          "       INNER JOIN GrList gr ON mt.grID = gr.grID INNER JOIN CpList cp ON gr.cpID = cp.cpID AND cp.cpType = 4 "
          "       LEFT OUTER JOIN StList stA ON mt.stA = stA.stID "
          "       LEFT OUTER JOIN StList stX ON mt.stX = stX.stID "
          "       LEFT OUTER JOIN MtSet mtSet1 ON mtSet1.mtID = mt.mtID AND mtSet1.mtMS = mt.mtMS AND mtSet1.mtSet = 1 "
          "       LEFT OUTER JOIN MtSet mtSet2 ON mtSet2.mtID = mt.mtID AND mtSet2.mtMS = mt.mtMS AND mtSet2.mtSet = 2 "
          "       LEFT OUTER JOIN MtSet mtSet3 ON mtSet3.mtID = mt.mtID AND mtSet3.mtMS = mt.mtMS AND mtSet3.mtSet = 3 "
          "       LEFT OUTER JOIN MtSet mtSet4 ON mtSet4.mtID = mt.mtID AND mtSet4.mtMS = mt.mtMS AND mtSet4.mtSet = 4 "
          "       LEFT OUTER JOIN MtSet mtSet5 ON mtSet5.mtID = mt.mtID AND mtSet5.mtMS = mt.mtMS AND mtSet5.mtSet = 5 "
          "       LEFT OUTER JOIN MtSet mtSet6 ON mtSet6.mtID = mt.mtID AND mtSet6.mtMS = mt.mtMS AND mtSet6.mtSet = 6 "
          "       LEFT OUTER JOIN MtSet mtSet7 ON mtSet7.mtID = mt.mtID AND mtSet7.mtMS = mt.mtMS AND mtSet7.mtSet = 7 "
          " WHERE mtDateTime IS NOT NULL AND gr.grID = " + ltostr(grID) + " AND (mt.mtResA + mt.mtResX) > 0 "
          " ORDER BY cpName, mtDateTime, mt.mtNr "
      ;
    }
    else if (cpType == CP_SINGLE)
    {
      sql = 
          "SELECT FORMAT(mtDateTime, 'yyyy-MM-dd hh:mm'), cpName, "
          "       IIF((grModus <> 2) OR (grWinner <> 1), grStage, CASE (grSize / POWER(2, mtRound)) WHEN 1 THEN 'F' WHEN 2 THEN 'SF' WHEN 4 THEN 'QF' ELSE CONCAT('R', grSize / POWER(2, mtRound - 1)) END), "
          "       mtMatch, "
          "       plAplExtID, plAnaName, NULL AS plBplExtID, NULL AS plBnaName, plXplExtID, plXnaName, NULL AS plYplExtID, NULL AS plYnaName, "
          "       mtSet1.mtResA, mtSet1.mtResX, mtSet2.mtResA, mtSet2.mtResX, mtSet3.mtResA, mtSet3.mtResX, mtSet4.mtResA, mtSet4.mtResX, "
          "       mtSet5.mtResA, mtSet5.mtResX, mtSet6.mtResA, mtSet6.mtResX, mtSet7.mtResA, mtSet7.mtResX, mt.mtResA, mt.mtResX, "
          "       mt.mtWalkOverA, mt.mtWalkOverX, YEAR(mtDateTime), '" + type + "' "
          "  FROM MtSingleList mt "
          "       INNER JOIN GrList gr ON mt.grID = gr.grID INNER JOIN CpList cp ON gr.cpID = cp.cpID AND cp.cpType = 1 "
          "       LEFT OUTER JOIN StList stA ON mt.stA = stA.stID "
          "       LEFT OUTER JOIN StList stX ON mt.stX = stX.stID "
          "       LEFT OUTER JOIN MtSet mtSet1 ON mtSet1.mtID = mt.mtID AND mtSet1.mtSet = 1 "
          "       LEFT OUTER JOIN MtSet mtSet2 ON mtSet2.mtID = mt.mtID AND mtSet2.mtSet = 2 "
          "       LEFT OUTER JOIN MtSet mtSet3 ON mtSet3.mtID = mt.mtID AND mtSet3.mtSet = 3 "
          "       LEFT OUTER JOIN MtSet mtSet4 ON mtSet4.mtID = mt.mtID AND mtSet4.mtSet = 4 "
          "       LEFT OUTER JOIN MtSet mtSet5 ON mtSet5.mtID = mt.mtID AND mtSet5.mtSet = 5 "
          "       LEFT OUTER JOIN MtSet mtSet6 ON mtSet6.mtID = mt.mtID AND mtSet6.mtSet = 6 "
          "       LEFT OUTER JOIN MtSet mtSet7 ON mtSet7.mtID = mt.mtID AND mtSet7.mtSet = 7 "
          " WHERE mtDateTime IS NOT NULL AND gr.grID = " + ltostr(grID) + " AND (mt.mtResA + mt.mtResX) > 0 "
          " ORDER BY cpName, mtDateTime, mt.mtNr "
      ;
    }
    else if (cpType == CP_DOUBLE || cpType == CP_MIXED)
    {
      sql = 
          "SELECT FORMAT(mtDateTime, 'yyyy-MM-dd hh:mm'), cpName, "
          "       IIF((grModus <> 2) OR (grWinner <> 1), grStage, CASE (grSize / POWER(2, mtRound)) WHEN 1 THEN 'F' WHEN 2 THEN 'SF' WHEN 4 THEN 'QF' ELSE CONCAT('R', grSize / POWER(2, mtRound - 1)) END), "
          "       mtMatch, "
          "       plAplExtID, plAnaName, plBplExtID, plBnaName, plXplExtID, plXnaName, plYplExtID, 2plYnaName, "
          "       mtSet1.mtResA, mtSet1.mtResX, mtSet2.mtResA, mtSet2.mtResX, mtSet3.mtResA, mtSet3.mtResX, mtSet4.mtResA, mtSet4.mtResX, "
          "       mtSet5.mtResA, mtSet5.mtResX, mtSet6.mtResA, mtSet6.mtResX, mtSet7.mtResA, mtSet7.mtResX, mt.mtResA, mt.mtResX, "
          "       mt.mtWalkOverA, mt.mtWalkOverX, YEAR(mtDateTime), '" + type + "' "
          "  FROM MtDoubleList mt "
          "       INNER JOIN GrList gr ON mt.grID = gr.grID INNER JOIN CpList cp ON gr.cpID = cp.cpID AND (cp.cpType = 2 OR cp.cpType = 3) "
          "       LEFT OUTER JOIN StList stA ON mt.stA = stA.stID "
          "       LEFT OUTER JOIN StList stX ON mt.stX = stX.stID "
          "       LEFT OUTER JOIN MtSet mtSet1 ON mtSet1.mtID = mt.mtID AND mtSet1.mtSet = 1 "
          "       LEFT OUTER JOIN MtSet mtSet2 ON mtSet2.mtID = mt.mtID AND mtSet2.mtSet = 2 "
          "       LEFT OUTER JOIN MtSet mtSet3 ON mtSet3.mtID = mt.mtID AND mtSet3.mtSet = 3 "
          "       LEFT OUTER JOIN MtSet mtSet4 ON mtSet4.mtID = mt.mtID AND mtSet4.mtSet = 4 "
          "       LEFT OUTER JOIN MtSet mtSet5 ON mtSet5.mtID = mt.mtID AND mtSet5.mtSet = 5 "
          "       LEFT OUTER JOIN MtSet mtSet6 ON mtSet6.mtID = mt.mtID AND mtSet6.mtSet = 6 "
          "       LEFT OUTER JOIN MtSet mtSet7 ON mtSet7.mtID = mt.mtID AND mtSet7.mtSet = 7 "
          " WHERE mtDateTime IS NOT NULL AND gr.grID = " + ltostr(grID) + " AND (mt.mtResA + mt.mtResX) > 0 "
          " ORDER BY cpName, mtDateTime, mt.mtNr "
      ;
    }
    else
      continue;

    Statement  *stmtPtr = connPtr->CreateStatement();
    ResultSet  *resPtr = 0;

    try
    {
      if (stmtPtr->Execute(sql))
        resPtr = stmtPtr->GetResultSet(false);
    }
    catch (SQLException &e)
    {
      infoSystem.Exception(sql, e);
      return false;
    }

    while (resPtr->Next())
    {
      wxString line;

      wxChar str[128];

      int col = 1;
      int colCount = resPtr->GetColumnCount();

      // First an ID column
      line << ";";

      for (; col <= colCount; col++)
      {
        if (resPtr->GetData(col, str, 128) && !resPtr->WasNull())
          line << str;
        line << ";";
      }

      os.AddLine(line);
    }

    delete resPtr;
    delete stmtPtr;
  }

  return true;
}

// -----------------------------------------------------------------------
// Import / Export Schedule
bool  MtStore::ImportSchedule(const wxString &name)
{
  long version = 1;

  // Format: CP;GR;RD;MT;MS;Date;Time;Table
  wxTextFile ifs(name);
  if (!ifs.Open())
    return false;

  wxString line = ifs.GetFirstLine();

  // Check header
  if (!CheckImportHeader(line, "#SCHEDULES", version))
  {
    ifs.Close();
    if (!infoSystem.Question(_("First comment is not %s but \"%s\". Continue anyway?"), wxT("#SCHEDULES"), line.c_str()))
      return false;
  }

  if (version > 1)
  {
    infoSystem.Error(_("Version %d of import file is not supported"), version);
    return false;
  }

  Connection *connPtr = TTDbse::instance()->GetNewConnection();

  connPtr->StartTransaction();

  // HACK: Die Variablen duerfen nicht laenger leben als connPtr [
  {
  CpStore  cp(connPtr);
  GrStore  gr(connPtr);
  MtStore  mt(connPtr);

  for(; !ifs.Eof(); line = ifs.GetNextLine())
  {   
    CTT32App::ProgressBarStep();

    if (line.GetChar(0) == '#')
      continue;

    // WB, Gruppe, Position auslesen
    wxStringTokenizerEx tokens(line, ",;\t");
    wxString strCp = tokens.GetNextToken();
    wxString strGr = tokens.GetNextToken();
    wxString strMtRound = tokens.GetNextToken();
    wxString strMtMatch = tokens.GetNextToken();
    wxString strMtChance = tokens.GetNextToken();

    if (strCp.IsEmpty() || strGr.IsEmpty() || 
        strMtRound.IsEmpty() || strMtMatch.IsEmpty())
      continue;

    if (wxStrcoll(strCp, cp.cpName))
    {
      // Naechster WB
      cp.SelectByName(strCp);
      if (!cp.Next())
        continue;

      cp.Close();

      // Ausserdem Gruppe lesen
      gr.SelectByName(strGr, cp);
      if (!gr.Next())
        continue;

      gr.Close();
    }
    
    if (wxStrcoll(strGr, gr.grName))
    {
      // Naechste Gruppe bei gleichem WB
      gr.SelectByName(strGr, cp);
      if (!gr.Next())
        continue;

      gr.Close();
    }

    // Event aufsetzen
    MtRec::MtEvent  mtEvent;
    mtEvent.grID = gr.grID;
    mtEvent.mtRound  = _strtos(strMtRound);
    mtEvent.mtMatch  = _strtos(strMtMatch);
    mtEvent.mtChance = strMtChance.IsEmpty() ? 0 : _strtos(strMtChance);

    wxString strMtDate  = tokens.GetNextToken();
    wxString strMtTime  = tokens.GetNextToken();
    wxString strMtTable = tokens.GetNextToken();
    wxString strMtUmpire = tokens.GetNextToken();
    wxString strMtUmpire2 = tokens.GetNextToken();

    if (strMtDate.IsEmpty() || strMtTime.IsEmpty() || strMtTable.IsEmpty())
      continue;

    if (strMtDate.Find(wxT('.')) != wxNOT_FOUND)
    {
      wxStringTokenizer tokens(strMtDate, wxT("."));
      wxString tmp = "";
      wxString day = tokens.GetNextToken();
      wxString month = tokens.GetNextToken();
      wxString year = tokens.GetNextToken();

      tmp += year;

      if (month.Length() == 1)
        tmp += "0";
      tmp += month;

      if (day.Length() == 1)
        tmp += "0";
      tmp += day;

      strMtDate = tmp;
    }
    else if (strMtDate.Find(wxT('-')) != wxNOT_FOUND)
    {
      wxStringTokenizer tokens(strMtDate, wxT("-"));
      wxString tmp = "";
      wxString year = tokens.GetNextToken();
      wxString month = tokens.GetNextToken();
      wxString day = tokens.GetNextToken();

      tmp += year;

      if (month.Length() == 1)
        tmp += "0";
      tmp += month;

      if (day.Length() == 1)
        tmp += "0";
      tmp += day;

      strMtDate = tmp;
    }

    if (strMtTime.Find(wxT(':')) != wxNOT_FOUND)
    {
      wxStringTokenizer tokens(strMtTime, wxT(":"));
      wxString tmp = "";
      wxString token = tokens.GetNextToken();
      tmp += token;

      token = tokens.GetNextToken();
      if (token.Length() == 1)
        tmp += "0";
      tmp += token;

      strMtTime = tmp;
    } 

        
    long  lDate = _strtol(strMtDate);
    long  lTime = _strtol(strMtTime);

    MtRec::MtPlace  mtPlace;

    mtPlace.mtDateTime.year  = lDate / 10000;
    mtPlace.mtDateTime.month = (lDate % 10000) / 100;
    mtPlace.mtDateTime.day   = (lDate % 100);

    // Y2K mit zweistelligen Jahren
    if (lDate != 0)
    {
      if (mtPlace.mtDateTime.year < 70)
        mtPlace.mtDateTime.year += 2000;
      else if (mtPlace.mtDateTime.year < 100)
        mtPlace.mtDateTime.year += 1900;
    }
    
    mtPlace.mtDateTime.hour   = (lTime / 100);
    mtPlace.mtDateTime.minute = (lTime % 100);
    mtPlace.mtDateTime.second = 0;
    
    mtPlace.mtDateTime.fraction = 0;

    mtPlace.mtTable = _strtos(strMtTable);

    mt.mtUmpire = strMtUmpire.IsEmpty() ? 0 : _strtos(strMtUmpire);
    mt.mtUmpire2 = strMtUmpire2.IsEmpty() ? 0 : _strtos(strMtUmpire2);

    mt.UpdateScheduleMatch(mtEvent, mtPlace, 0, 0);
  }
  } // end HACK ]

  connPtr->Commit();
  delete connPtr;

  return true;
}


bool  MtStore::ExportSchedule(wxTextBuffer &os, short cpType, const std::vector<long> &idList, bool append)
{
  long version = 1;

  // Format: CP;GR;RD;MT;MS;Date;Time;Table

  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  
  if (!append)
  {
    os.AddLine(wxString::Format("#SCHEDULE %d", version));

    os.AddLine("# Event; Group; Round; Match; Chance; Date; Time; Table; Umpire; Assistant Umpire");
  }

  for (std::vector<long>::const_iterator it = idList.begin(); it != idList.end(); it++)
  {  
    long grID = (*it);

    Statement  *stmtPtr = connPtr->CreateStatement();
    ResultSet  *resPtr  = 0;
    
    wxString str = 
      "SELECT cpName, grName, mtRound, mtMatch, mtDateTime, mtTable, mtUmpire, mtUmpire2 "
      "  FROM MtList mt "
      " WHERE mt.grID = " + ltostr(grID) +
      "       AND (mtMS IS NULL OR mtMS = 0) AND (mtSet IS NULL OR mtSet = 0) "
    ;

    try
    {
      if ( stmtPtr->Execute(str) )
        resPtr = stmtPtr->GetResultSet(false);
    }
    catch (SQLException &e)
    {
      infoSystem.Exception(str, e);
      return false;
    }
    
    wxChar  cpName[9];
    wxChar  grName[9];
    short   mtRound;
    short   mtMatch;
    timestamp mtDateTime;
    short   mtTable;
    long    mtUmpire;
    long    mtUmpire2;
    
    resPtr->BindCol(1, cpName, sizeof(cpName));
    resPtr->BindCol(2, grName, sizeof(grName));
    resPtr->BindCol(3, &mtRound);
    resPtr->BindCol(4, &mtMatch);
    resPtr->BindCol(5, &mtDateTime);
    resPtr->BindCol(6, &mtTable);
    resPtr->BindCol(6, &mtUmpire);
    resPtr->BindCol(7, &mtUmpire2);

    mtTable = 0;
    mtUmpire = 0;
    mtUmpire2 = 0;
    memset(&mtDateTime, 0, sizeof(mtDateTime));
    
    while (resPtr->Next())
    {
      wxString line;

      line << cpName << ";" << grName << ";" << mtRound << ";" << mtMatch << ";0;";      
      line << mtDateTime.day + 100 * mtDateTime.month + 10000 * mtDateTime.year << ";";
      line << mtDateTime.minute + 100 * mtDateTime.hour << ";" << mtTable << ";" << mtUmpire << ";" << mtUmpire2 << ";";

      os.AddLine(line);

      mtTable = 0;
      memset(&mtDateTime, 0, sizeof(mtDateTime));    
    }
    
    delete resPtr;
    delete stmtPtr;
  }
  
  return true;
}