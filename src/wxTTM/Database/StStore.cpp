/* Copyright (C) 2020 Christoph Theis */

// Tabelle der "Setzung"
#include  "stdafx.h"
#include  "StStore.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "ResultSet.h"
#include  "SQLException.h"
#include  "InfoSystem.h"
#include  "Request.h"
#include  "Rec.h"

#include  "wxStringTokenizerEx.h"

#include  "IdStore.h"
#include  "CpStore.h"
#include  "GrStore.h"
#include  "PlStore.h"
#include  "TmStore.h"
#include  "XxStore.h"

#include  "TmEntryStore.h"

#include  <fstream>


bool  StStore::CreateTable()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  wxASSERT(connPtr);

  Statement *tmp = connPtr->CreateStatement();

  wxString  INTEGER  = connPtr->GetDataType(SQL_INTEGER);
  wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);
  wxString  TIMESTAMP = connPtr->GetDataType(SQL_TIMESTAMP);
  wxString  WVARCHAR = connPtr->GetDataType(SQL_WVARCHAR);

  wxString  sql = 
    "CREATE TABLE StRec (         "
    "stID        "+INTEGER+"      NOT NULL,  "
    "grID        "+INTEGER+"      NOT NULL,  "
    "tmID        "+INTEGER+",     "
    "stNr        "+SMALLINT+"     NOT NULL,  "
    "stPos       "+SMALLINT+"     DEFAULT 0, "
    "stSeeded    "+SMALLINT+"     DEFAULT 0, "
    "stGaveup    "+SMALLINT+"     DEFAULT 0, "
    "stDisqu     "+SMALLINT+"     DEFAULT 0, "
    "stNoCons    "+SMALLINT+"     DEFAULT 0, "
    "stTimestamp "+TIMESTAMP+"    DEFAULT GETUTCDATE(), "
    "CONSTRAINT stIdKey PRIMARY KEY (stID),   "
    "stCreatedBy " + WVARCHAR + "(64) NOT NULL DEFAULT (SUSER_SNAME()), "
    "stModifiedBy AS (SUSER_SNAME()), "
    "stStartTime datetime2 GENERATED ALWAYS AS ROW START NOT NULL DEFAULT SYSUTCDATETIME(), "
    "stEndTime datetime2 GENERATED ALWAYS AS ROW END NOT NULL DEFAULT CAST('9999-12-31 23:59:59.9999999' AS datetime2), "
    "PERIOD FOR SYSTEM_TIME (stStartTime, stEndTime) "
    ") WITH (SYSTEM_VERSIONING = ON (HISTORY_TABLE = dbo.StHist))";

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
    tmp->ExecuteUpdate(sql = "CREATE INDEX stNrKey ON StRec (stNr)");
    // Mit diesem Index dauert ein SELECT * FROM StDoubleList WHERE grID = ...
    // mehr als 10mal solange!
    // tmp->ExecuteUpdate(sql = "CREATE INDEX stGrKey ON StRec (grID)");
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(sql, e);
  };

  tmp->Close();
  delete tmp;
  
  return true;
}


bool  StStore::UpdateTable(long version)
{
  if (version == 0)
    return CreateTable();

  if (version <= 67)
  {
    Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
    Statement *stmtPtr = connPtr->CreateStatement();

    wxString  TIMESTAMP = connPtr->GetDataType(SQL_TIMESTAMP);
    wxString str;
    
    try 
    {      
      str = "ALTER TABLE StRec ADD stTimestamp "+TIMESTAMP+" DEFAULT GETUTCDATE()";
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
      str = "ALTER TABLE StRec ADD "
        "stCreatedBy " + WVARCHAR + "(64) NOT NULL DEFAULT (SUSER_SNAME()), "
        "stModifiedBy AS (SUSER_SNAME()), "
        "stStartTime datetime2 GENERATED ALWAYS AS ROW START NOT NULL DEFAULT SYSUTCDATETIME(), "
        "stEndTime datetime2 GENERATED ALWAYS AS ROW END NOT NULL DEFAULT CAST('9999-12-31 23:59:59.9999999' AS datetime2), "
        "PERIOD FOR SYSTEM_TIME (stStartTime, stEndTime)"
        ;
      stmtPtr->ExecuteUpdate(str);

      str = "ALTER TABLE StRec SET (SYSTEM_VERSIONING = ON(HISTORY_TABLE = dbo.StHist))";
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


bool  StStore::CreateConstraints()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();
  
  try
  {
    tmp->ExecuteUpdate("DROP TRIGGER stUpdateTrigger");
  }
  catch (SQLException &)
  {
  }

  try
  {
    tmp->ExecuteUpdate("ALTER TABLE StRec DROP CONSTRAINT stGrRef");
    tmp->ExecuteUpdate("ALTER TABLE StRec DROP CONSTRAINT stTmRef");
  }
  catch (SQLException &)
  {
  }

  wxString  str;

  try
  {
    // ISNULL, weil man nicht mal mit <> Werte mit NULL vergleichen kann
    // 0 ist keine gueltige ID
    tmp->ExecuteUpdate(str = 
      "CREATE TRIGGER stUpdateTrigger ON StRec FOR UPDATE AS \n"
      "  DECLARE @mtID int; \n"
      "  DECLARE @mtNr int; \n"
      "  DECLARE @mtRound smallint; \n"
      "  DECLARE @mtChance smallint; \n"
      " \n"
      "--- Update timestamp for last changed \n"
      "  UPDATE StRec SET stTimestamp = GETUTCDATE() \n"
      "   WHERE stID IN(SELECT stID FROM deleted); \n"
      " \n"
      "--- Update raster and printed when team changed \n"
      "  DECLARE mtUpdateCursor CURSOR LOCAL FOR \n"
      "    (SELECT mtID, mtNr, mtRound, mtChance \n"
      "       FROM MtRec \n"
      "            INNER JOIN inserted ON(MtRec.stA = inserted.stID OR MtRec.stX = inserted.stID) \n"
      "            INNER JOIN deleted  ON(MtRec.stA = deleted.stID  OR MtRec.stX = deleted.stID) \n"
      "      WHERE inserted.stID = deleted.stID AND ISNULL(inserted.tmID, 0) <> ISNULL(deleted.tmID, 0) \n"
      "  ); \n"
      "  OPEN mtUpdateCursor \n"
      "  FETCH NEXT FROM mtUpdateCursor INTO @mtID, @mtNr, @mtRound, @mtChance \n"
      "  WHILE(@@FETCH_STATUS = 0) \n"
      "  BEGIN \n"
      "--- In first round move winner and loser recursively to next round \n"
      "    IF @mtRound = 1 AND @mtChance = 0 \n"
      "    BEGIN \n"
      "      EXEC mtUpdateRasterProc @mtNr, 1; \n"
      "      EXEC mtUpdateRasterProc @mtNr, 0; \n"
      "    END \n"
      " \n"
      "--- In all rounds update mtPrinted (score sheet outdated) and timestamp (record updated) \n"
      "    UPDATE MtRec SET mtPrinted = 0, mtTimestamp = GETUTCDATE() WHERE mtID = @mtID \n"
      " \n"
      "    FETCH NEXT FROM mtUpdateCursor INTO @mtID, @mtNr, @mtRound, @mtChance \n"
      "  END \n"
    );

    tmp->ExecuteUpdate(str = 
      "ALTER TABLE StRec ADD CONSTRAINT stGrRef "
      "FOREIGN KEY (grID) REFERENCES GrRec (grID) ON DELETE CASCADE");

    // Eigentlich ON DELETE SET NULL, das gibt es aber in MS-SQL noch nicht.
    // INSTEAD OF DELETE Trigger koennen in TmRec nicht gesetzt werden, weil
    // diese Tabelle selbst ein ON DELETE SET NULL hat.
    tmp->ExecuteUpdate(str = 
      "ALTER TABLE StRec ADD CONSTRAINT stTmRef "
      "FOREIGN KEY (tmID) REFERENCES TmRec (tmID) ON DELETE NO ACTION");
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
  };

  delete tmp;
  return true;
}


bool  StStore::UpdateConstraints(long version)
{
  if (version <= 141)
    return CreateConstraints();

  return true;
}


// -----------------------------------------------------------------------
StStore::StStore(Connection *ptr)
       : StoreObj(ptr)
{
}


void  StStore::Init()
{
  StRec::Init();
}


// -----------------------------------------------------------------------
bool  StStore::Insert(const GrRec &gr)
{
  PreparedStatement *stmtPtr = 0;

  wxString str = "INSERT INTO StRec "
                    "      (stID, grID, stNr, "
                    "       stSeeded, stGaveup, stDisqu, stNocons) "
                    "VALUES(?, ?, ?, 0, 0, 0, 0)";

  try
  {
    stID = IdStore::ID(GetConnectionPtr());
    grID = gr.grID;
    stPos = gr.grWinner;

    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    stmtPtr->SetData(1, &stID);
    stmtPtr->SetData(2, &grID);
    stmtPtr->SetData(3, &stNr);

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
  update.rec  = CRequest::STREC;
  update.id   = stID;

  CTT32App::NotifyChange(update);

  return true;
}


bool  StStore::Update()
{
  PreparedStatement *stmtPtr = 0;

  wxString str = 
      "UPDATE StRec SET "
      "stPos = ?, stSeeded = ?, stGaveup = ?, stDisqu = ?, stNocons = ? "
      "WHERE stID = ?";

  try
  {
    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    stmtPtr->SetData(1, (stPos ? &stPos : (short *) 0));
    stmtPtr->SetData(2, &stSeeded);
    stmtPtr->SetData(3, &stGaveup);
    stmtPtr->SetData(4, &stDisqu);
    stmtPtr->SetData(5, &stNocons);
    stmtPtr->SetData(6, &stID);

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


bool  StStore::UpdatePosition()
{
  wxString str = 
      "UPDATE StRec SET stPos = " + ltostr(stPos) + 
      " WHERE stID = " + ltostr(stID);

  try
  {
    ExecuteUpdate(str);
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  // Notify Views
  CRequest update;
  update.type = CRequest::UPDATE;
  update.rec  = CRequest::STREC;
  update.id   = stID;

  CTT32App::NotifyChange(update);

  return true;
}


bool  StStore::Remove(const GrRec &gr)
{
  wxString str = "DELETE FROM StRec WHERE grID = " + ltostr(gr.grID);

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
  update.rec  = CRequest::STREC;
  update.id   = stID;

  CTT32App::NotifyChange(update);

  return true;
}


// ----------------------------------------------------------------------
bool  StStore::SelectAll(const GrRec &gr)
{
  wxString str = SelectString();
  str += " WHERE grID = " + ltostr(gr.grID);

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


// ----------------------------------------------------------------------
bool  StStore::SelectById(long id)
{
  wxString str = SelectString();
  str += " WHERE stID = ";
  str += ltostr(id);
  str += "";

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


// ----------------------------------------------------------------------
bool  StStore::SelectAll(const CpRec &cp, const wxString &stage)
{
  wxString str = SelectString();
  str += " WHERE grID IN ";
  str += "(SELECT grID FROM GrRec WHERE cpID = ";
  str += ltostr(cp.cpID);
  str += " AND grStage = '";
  str += stage;
  str += "') ORDER BY grID, stPos";

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


// ----------------------------------------------------------------------
bool  StStore::SelectByNr(long grID, short nr)
{
  wxString str = SelectString();
  str += " WHERE grID = ";
  str += ltostr(grID);
  str += " AND stNr = ";
  str += ltostr(nr);

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


// ----------------------------------------------------------------------
bool  StStore::SelectSeeded(const CpRec &cp, const wxString &stage)
{
  wxString str = SelectString();
  str += " WHERE stSeeded = 1 AND grID IN ";
  str += "(SELECT grID FROM GrRec WHERE cpID = ";
  str += ltostr(cp.cpID);
  str += " AND grStage = '";
  str += stage;
  str += "')";

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


// ----------------------------------------------------------------------
bool  StStore::RemoveTeam(long id)
{
  wxString str = 
      "UPDATE StRec SET "
      "   tmID = NULL, stSeeded = 0, stGaveup = 0, stDisqu = 0, stNocons = 0 "
      " WHERE tmID = " + ltostr(id);
  
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

bool  StStore::ClearAllTeams(const GrRec &gr)
{
  wxString  str = "UPDATE StRec SET tmID = NULL WHERE grID = " + ltostr(gr.grID);

  try
  {
    XxStore(GetConnectionPtr()).Remove(gr, 0);

    ExecuteUpdate(str);
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  // Die Funktion kommt aus dem Einlesen einer Auslosung.
  // Also kein Update, das kommt erst beim Setzen der Spieler
  return true;
}


bool  StStore::SetTeam(long id, unsigned flags)
{
  GrRec  gr;
  gr.grID = grID;

  return SetTeam(gr, stNr, id, flags);
}


bool StStore::SetTeam(const GrRec &gr, short nr, long id, unsigned flags)
{
  TmEntry  tm;
  tm.team.cpType = 0;  // Nicht CP_GROUP
  tm.tmID = id;
  
  return SetTeam(gr, nr, tm, flags);
}


bool  StStore::SetTeam(const TmEntry &tm, unsigned flags)
{
  GrRec gr;
  gr.grID = grID;

  return SetTeam(gr, stNr, tm, flags);
}


bool StStore::SetTeam(const GrRec &gr, short nr, const TmEntry &tm, unsigned flags)
{
  wxString  str = "UPDATE StRec SET tmID = ";

  try
  {
    // XxStore muss extra geloescht werden
    // XxStore(GetConnectionPtr()).Remove(*this);

    if (tm.team.cpType == CP_GROUP) //  && tm.team.gr.grID != 0)
    {
      short  pos = tm.team.gr.grPos;
      GrRec  grQual;
      grQual.grID = tm.team.gr.grID;
      XxStore(GetConnectionPtr()).Insert(*this, grQual, pos);

      str += "(SELECT tmID FROM StRec WHERE grID = " + ltostr(grQual.grID) +
             "    AND stPos = " + ltostr(pos) + ")";
    }
    else if (!tm.tmID)
    {
      str += "NULL ";
    }
    else
    {
      str += ltostr(tm.tmID);
    }
    
    if ( !(flags & ST_KEEPSEEDED) )
      str += ", stSeeded = " + ltostr((flags & ST_SEEDED) ? 1 : 0);
           // ", stGaveup = " + ltostr((flags & ST_GAVEUP) ? 1 : 0) +
           // ", stDisqu  = " + ltostr((flags & ST_DISQU)  ? 1 : 0) +
           // ", stNocons = " + ltostr((flags & ST_NOCONS) ? 1 : 0);

    str += " WHERE grID = " + ltostr(gr.grID) + " AND stNr = " + ltostr(nr);

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
  update.rec  = CRequest::STREC;
  update.id   = GetId(gr, nr);

  CTT32App::NotifyChange(update);

  return true;
}


// ----------------------------------------------------------------------
long  StStore::GetId(const GrRec &gr, short nr)
{
  Statement *stmtPtr;
  ResultSet *resPtr;

  wxString  str = 
    "SELECT stID FROM StRec WHERE grID = " + ltostr(gr.grID) + 
    " AND stNr = " + ltostr(nr);

  long  id;

  try
  {
    stmtPtr = GetConnectionPtr()->CreateStatement();
    wxASSERT(stmtPtr);
  
    resPtr = stmtPtr->ExecuteQuery(str);
    wxASSERT(resPtr);

    if (!resPtr->Next() || !resPtr->GetData(1, id) || resPtr->WasNull())
      id = 0;

    delete resPtr;
    delete stmtPtr;
  
    return id;
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
    return 0;
  }
}


// ----------------------------------------------------------------------
bool  StStore::SetSeeded(bool seeded)
{
  PreparedStatement *stmtPtr = 0;

  wxString str = 
      "UPDATE StRec SET stSeeded = ? WHERE stID = ?";

  try
  {
    short tmp = seeded ? 1 : 0;

    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    stmtPtr->SetData(1, &tmp);
    stmtPtr->SetData(2, &stID);

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


bool  StStore::SetGaveup(bool gaveup)
{
  PreparedStatement *stmtPtr = 0;

  wxString str = 
      "UPDATE StRec SET stGaveup = ? WHERE stID = ?";

  try
  {
    short tmp = gaveup ? 1 : 0;

    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    stmtPtr->SetData(1, &tmp);
    stmtPtr->SetData(2, &stID);

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

bool  StStore::SetDisqualified(bool disqu)
{
  PreparedStatement *stmtPtr = 0;

  wxString str = 
      "UPDATE StRec SET stDisqu = ? WHERE stID = ?";

  try
  {
    short tmp = disqu ? 1 : 0;

    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    stmtPtr->SetData(1, &tmp);
    stmtPtr->SetData(2, &stID);

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

bool  StStore::SetNoConsolation(bool noCons)
{
  PreparedStatement *stmtPtr = 0;

  wxString str = 
      "UPDATE StRec SET stNocons = ? WHERE stID = ?";

  try
  {
    short tmp = noCons ? 1 : 0;

    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    stmtPtr->SetData(1, &tmp);
    stmtPtr->SetData(2, &stID);

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



// ----------------------------------------------------------------------
wxString  StStore::SelectString() const
{
  wxString  str = 
    "SELECT stID, stNr, stPos, "
    "stSeeded, stDisqu, stGaveup, stNocons, tmID, grID "
    "FROM stRec ";

  return str;
}


bool  StStore::BindRec()
{
  BindCol(1, &stID);
  BindCol(2, &stNr);
  BindCol(3, &stPos);
  BindCol(4, &stSeeded);
  BindCol(5, &stDisqu);
  BindCol(6, &stGaveup);
  BindCol(7, &stNocons);
  BindCol(8, &tmID);
  BindCol(9, &grID);

  return true;
}


// -----------------------------------------------------------------------
// Import / Export
// Aufbau:
// cpName;grName;stNr;{plNr | plNR;bdNr | tmName};

// Siehe PlStore zur Verwendung von std::istream
bool  StStore::Import(const wxString &name)
{
  long version = 1;

  wxTextFile ifs(name);
  if (!ifs.Open())
    return false;

  wxString line = ifs.GetFirstLine();

  // Check header
  if (!CheckImportHeader(line, "#POSITIONS", version))
  {
    ifs.Close();
    if (!infoSystem.Question(_("First comment is not %s but \"%s\". Continue anyway?"), wxT("#GROUPS"), line.c_str()))
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
  TmStore  tm(connPtr);
  
  bool skipGroup = false;

  for (; !ifs.Eof(); line = ifs.GetNextLine())
  {   
    CTT32App::ProgressBarStep();

    if (line.GetChar(0) == '#')
      continue;

    // WB, Gruppe, Position auslesen
    wxStringTokenizerEx tokens(line, ",;\t");
    wxString strCp = tokens.GetNextToken();
    wxString strGr = tokens.GetNextToken();
    wxString strSt = tokens.GetNextToken();

    if (strCp.IsEmpty() || strGr.IsEmpty() || strSt.IsEmpty())
      continue;

    if (strCp != cp.cpName)
    {
      // Gruppe lesen erzwingen
      *gr.grName = 0;
      
      // Die Gruppe ist erstmal wieder OK
      skipGroup = false;
      
      // Naechster WB
      cp.SelectByName(strCp);
      if (!cp.Next())
      {
        if (!infoSystem.Question(_("Event %s does not exist. Continue anyway?"), strCp))
        {
          connPtr->Rollback();
          return false;
        }
        else
          continue;
      }
    }
    else if (!cp.cpID)
      continue;
    
    if (strGr != gr.grName)
    {
      skipGroup = false;
      
      // Naechste Gruppe bei gleichem WB
      gr.SelectByName(strGr, cp);
      if (!gr.Next())
      {
        if (!infoSystem.Question(_("Group %s of event %s does not exist. Continue anyway?"), gr.grDesc, cp.cpDesc))
        {
          connPtr->Rollback();
          return false;
        }
        else
        {
          skipGroup = true;
        }
      }

      gr.Close();  // XXX Warum ist das noetig?

      if (!skipGroup && gr.QryStarted())
      {
        infoSystem.Error(_("Group %s of event %s has already started!"), gr.grDesc, cp.cpDesc);
        skipGroup = true;
      }

      if (gr.QryDraw())
      {
        if (!skipGroup && gr.QryScheduled())
        {
          infoSystem.Error(_("A draw has been performed and scheduled for group %s of event %s!"), gr.grDesc, cp.cpDesc);
          skipGroup = true;
        }
    
        if (!skipGroup && !infoSystem.Confirmation(_("A draw has been performed for group %s of event %s. Continue anyway?"), gr.grDesc, cp.cpDesc))
          skipGroup = true;
      }
    }
    else if (!gr.grID)
      continue;
    
    if (skipGroup)
      continue;

    if (_strtos(strSt) > gr.grSize)
    {
      if (!infoSystem.Question(_("Pos %d exceeds size of group %s of event %s. Continue anyway?"), _strtos(strSt), gr.grDesc, cp.cpDesc))
      {
        connPtr->Rollback();
        return false;
      }
      else
        continue;
    }

    // Spieler / Team auslesen
    wxString strPlTm = tokens.GetNextToken();
    wxString strSeeded = tokens.GetNextToken();
    wxString strPos    = tokens.GetNextToken();

    if (strPlTm.IsEmpty())
      continue;

    if (cp.cpType != CP_TEAM)
      tm.SelectByCpPlNr(cp, _strtol(strPlTm));
    else
      tm.SelectByCpTmName(cp, strPlTm);

    if (!tm.Next())
    {
      gr.SetTeam(_strtos(strSt), TmRec());
    }
    else
    {
      // Und speichern (Schreibt auch Gewinner fort)
      gr.SetTeam(_strtos(strSt), tm, _strtos(strSeeded));
    }

  }
  }  // HACK ]

  connPtr->Commit();
  delete connPtr;

  return true;
}


bool  StStore::Export(const wxString &name, short cpType, const std::vector<long> & idList, bool append)
{
  long version = 1;

  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  
  wxChar cpName[9], grName[9];
  short  stNr, stPos;
  short  plNr, bdNr;
  wxChar tmName[9];
  short  stSeeded, stGaveup, stDisqu, stNocons;

  std::ofstream  os(name.t_str(), std::ios::out | (append ? std::ios::app : 0));

  if (!append)
  {
    const wxString bom(wxChar(0xFEFF));
    os << bom.ToUTF8();

    os << "#POSITIONS " << version << std::endl;
  }
  
  short lastCpType = 0;

  for (std::vector<long>::const_iterator it = idList.begin(); it != idList.end(); it++)
  {
    wxString ofs;
    long grID = (*it);

    Statement  *stmtPtr = connPtr->CreateStatement();
    ResultSet  *resPtr  = 0;
    
    wxString str;
    switch (cpType)
    {
      case CP_SINGLE :      
        str = "SELECT cpName, grName, stNr, "
              "       stPos, stSeeded, stGaveup, stDisqu, stNocons, "
              "       plNr "
              "  FROM StRec st INNER JOIN TmSingleList tm ON st.tmID = tm.tmID "
              "                INNER JOIN GrRec gr ON st.grID = gr.grID        "
              "                INNER JOIN CpRec cp ON gr.cpID = cp.cpID        "
              " WHERE st.grID = " + StoreObj::ltostr(grID) +
              " ORDER BY st.stNr";
        break;
        
      case CP_DOUBLE :
      case CP_MIXED  :
        str = "SELECT cpName, grName, stNr, "
              "       stPos, stSeeded, stGaveup, stDisqu, stNocons, "
              "       plplNr, bdplNr "
              "  FROM StRec st INNER JOIN TmDoubleList tm ON st.tmID = tm.tmID "
              "                INNER JOIN GrRec gr ON st.grID = gr.grID        "
              "                INNER JOIN CpRec cp ON gr.cpID = cp.cpID        "
              " WHERE st.grID = " + StoreObj::ltostr(grID) +
              " ORDER BY st.stNr";
        break;
        
      case CP_TEAM :
        str = "SELECT cpName, grName, stNr, "
              "       stPos, stSeeded, stGaveup, stDisqu, stNocons, "
              "       tmName "
              "  FROM StRec st "
              "                INNER JOIN GrRec gr ON st.grID = gr.grID        "
              "                INNER JOIN CpRec cp ON gr.cpID = cp.cpID        "
              "                LEFT OUTER JOIN TmTeamList tm ON st.tmID = tm.tmID "              
              " WHERE st.grID = " + StoreObj::ltostr(grID) +
              " ORDER BY st.stNr";
        break;
        
      default :
        return false;
    }
    
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
    
    switch (cpType)
    {
      case CP_SINGLE :
        resPtr->BindCol(1, cpName, sizeof(cpName));
        resPtr->BindCol(2, grName, sizeof(grName));
        resPtr->BindCol(3, &stNr);
        resPtr->BindCol(4, &stPos);
        resPtr->BindCol(5, &stSeeded);
        resPtr->BindCol(6, &stGaveup);
        resPtr->BindCol(7, &stDisqu);
        resPtr->BindCol(8, &stNocons);
        resPtr->BindCol(9, &plNr);
        break;
        
      case CP_DOUBLE :
      case CP_MIXED :
        resPtr->BindCol(1, cpName, sizeof(cpName));
        resPtr->BindCol(2, grName, sizeof(grName));
        resPtr->BindCol(3, &stNr);
        resPtr->BindCol(4, &stPos);
        resPtr->BindCol(5, &stSeeded);
        resPtr->BindCol(6, &stGaveup);
        resPtr->BindCol(7, &stDisqu);
        resPtr->BindCol(8, &stNocons);
        resPtr->BindCol(9, &plNr);
        resPtr->BindCol(10, &bdNr);
        break;
        
      case CP_TEAM :
        resPtr->BindCol(1, cpName, sizeof(cpName));
        resPtr->BindCol(2, grName, sizeof(grName));
        resPtr->BindCol(3, &stNr);
        resPtr->BindCol(4, &stPos);
        resPtr->BindCol(5, &stSeeded);
        resPtr->BindCol(6, &stGaveup);
        resPtr->BindCol(7, &stDisqu);
        resPtr->BindCol(8, &stNocons);
        resPtr->BindCol(9, tmName, sizeof(tmName));
        break;
        
      default :
        break;
    }
        
    while (resPtr->Next())
    {
      switch (cpType)
      {
        case CP_SINGLE :
          if (lastCpType != cpType)
            ofs << "# Event; Group; Group Pos.; Pl. No; Seeded; Final Position" << '\n';
            
          ofs << cpName << ";" << grName << ";" << stNr << ";" << plNr << ";";
          break;
          
        case CP_DOUBLE :
        case CP_MIXED :
          if (lastCpType != cpType)
            ofs << "# Event; Group; Group Pos.; Pl. No; Seeded; Final Position" << '\n';
            
          ofs << cpName << ";" << grName << ";" << stNr << ";" << plNr << ";";
          break;
          
        case CP_TEAM :
          if (lastCpType != cpType)
            ofs << "# Event; Group; Group Pos.; Team Name; Seeded; Final Position" << '\n';
            
          ofs << cpName << ";" << grName << ";" << stNr << ";" << tmName << ";";
          break;
          
        default :
          break;
      }
      
      unsigned seeded = 0;
      if (stSeeded)
        seeded |= StStore::ST_SEEDED;
      if (stGaveup)
        seeded |= StStore::ST_GAVEUP;
      if (stDisqu)
        seeded |= StStore::ST_DISQU;
      if (stNocons)
        seeded |= StStore::ST_NOCONS;
         
      ofs << seeded << ";" << stPos << '\n';
      
      lastCpType = cpType;
    }   

    os << ofs.ToUTF8();
    
    delete resPtr;
    delete stmtPtr;
  }
  
  os.close();

  return true;
}
