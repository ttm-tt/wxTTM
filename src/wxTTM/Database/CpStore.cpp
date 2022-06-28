/* Copyright (C) 2020 Christoph Theis */

// DB-Tabelle der Wettbewerbe

#include  "stdafx.h"
#include  "CpStore.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "ResultSet.h"
#include  "SQLException.h"
#include  "InfoSystem.h"
#include  "Request.h"
#include  "Rec.h"

#include  "IdStore.h"
#include  "PlStore.h"
#include  "CpStore.h"
#include  "GrStore.h"
#include  "LtStore.h"
#include  "NtStore.h"
#include  "TmStore.h"
#include  "RkStore.h"
#include  "NaStore.h"
#include  "LtEntryStore.h"

#include  "wxStringTokenizerEx.h"

#include  <stdio.h>
#include  <fstream>
#include  <stdlib.h>

short CpRec::defPtsToWin = 11;  // Pts to win a game
short CpRec::defPtsAhead = 2;   // Pts to be ahead to win a game


bool CpRec::Read(const wxString &line, int version)
{
  wxStringTokenizerEx tokens(line, wxT(",;\t"));

  wxString strName = tokens.GetNextToken();
  wxString strDesc = tokens.GetNextToken();
  wxString strCat  = (version < 2 ? wxEmptyString : tokens.GetNextToken().wx_str());
  wxString strType = tokens.GetNextToken();
  wxString strSex  = tokens.GetNextToken();
  wxString strYear = tokens.GetNextToken();
  wxString strRO   = tokens.GetNextToken();

  if (!strName.IsEmpty() && !strDesc.IsEmpty() && !strSex.IsEmpty())
  {
    Init();

    wxStrncpy((wxChar *) cpName, strName.t_str(), sizeof(cpName) / sizeof(wxChar) -1);
    wxStrncpy((wxChar *) cpDesc, strDesc.t_str(), sizeof(cpDesc) / sizeof(wxChar) -1);
    wxStrncpy((wxChar *) cpCategory, strCat.t_str(), sizeof(cpCategory) / sizeof(wxChar) -1);
    
    cpYear = _strtol(strYear);

    switch (*strType.t_str())
    {
      case 's' :
      case 'S' :
        cpType = CP_SINGLE;
        break;
        
      case 'd' :
      case 'D' :
        cpType = CP_DOUBLE;
        break;
        
      case 'x' :
      case 'X' :
        cpType = CP_MIXED;
        break;
        
      case 't' :
      case 'T' :
        cpType = CP_TEAM;
        break;
        
      default :
        cpType = _strtos(strType);
        break;
    }
    
    switch (*strSex.t_str())
    {
      case 'm' :
      case 'M' :
        cpSex = SEX_MALE;
        break;
        
      case 'w' :
      case 'W' :
      case 'f' :
      case 'F' :
        cpSex = SEX_FEMALE;
        break;
        
      case 'x' :
      case 'X' :
        cpSex = SEX_MIXED;
        break;
        
      default :
        cpSex = _strtos(strSex);
        break;
    }
    
    if (cpType == CP_MIXED)
      cpSex  = SEX_MIXED;
  }

  return true;
}


bool CpRec::Write(wxString &str, int version) const
{
  static wxChar types[] = {wxT(' '), wxT('s'), wxT('d'), wxT('x'), wxT('t')};
  static wxChar sexes[] = {wxT(' '), wxT('m'), wxT('w'), wxT('x')};
  
  str << cpName << ";" 
      << cpDesc << ";"
  ;

  if (version > 1)
    str << cpCategory << ";";

  str
      // << types[cp.cpType] << ";"
      // << sexes[cp.cpSex] << ";"
      << cpType << ";"
      << cpSex  << ";"
      << cpYear << ";";

  return true;
}



// -----------------------------------------------------------------------
bool  CpRec::IsAllowed(const PlRec &pl) const
{
  if (cpSex != SEX_MIXED)
  {
    if (cpSex != pl.psSex)
      return false;
  }

  if (cpYear && pl.psBirthday)
  {
    if (CTT32App::instance()->GetType() == TT_SCI)
      return (cpYear >= pl.psBirthday);
    else // if (CTT32App::instance()->GetType() == TT_YOUTH)
      return (cpYear <= pl.psBirthday);
  }

  return true;
}


// -----------------------------------------------------------------------
// Neue Tabelle in DB erzeugen
bool  CpStore::CreateTable()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  wxASSERT(connPtr);

  Statement *tmp = connPtr->CreateStatement();

  wxString  WVARCHAR = connPtr->GetDataType(SQL_WVARCHAR);
  wxString  INTEGER  = connPtr->GetDataType(SQL_INTEGER);
  wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);

  wxString  sql = 
    "CREATE TABLE CpRec (         "
    "cpID           "+INTEGER+"      NOT NULL,  "
    "cpName         "+WVARCHAR+"(8)  NOT NULL,  "
	  "cpDesc         "+WVARCHAR+"(64),           "
	  "cpCategory     "+WVARCHAR+"(64) DEFAULT NULL, "
	  "cpType         "+SMALLINT+"     NOT NULL,  "
	  "cpSex          "+SMALLINT+"     NOT NULL,  "
	  "cpYear         "+INTEGER+",     "
    "syID           "+INTEGER+",     "
    "cpBestOf       "+SMALLINT+"     NOT NULL DEFAULT 0, "
    "cpPtsToWin     "+SMALLINT+"     NOT NULL DEFAULT 11, "
    "cpPtsAhead     "+SMALLINT+"     NOT NULL DEFAULT 2,  "
    "cpPtsToWinLast "+SMALLINT+"     NOT NULL DEFAULT 11, "
    "cpPtsAheadLast "+SMALLINT+"     NOT NULL DEFAULT 2,  "
    "CONSTRAINT cpIdKey PRIMARY KEY (cpID),  "
    "cpCreatedBy " + WVARCHAR + "(64) NOT NULL DEFAULT (SUSER_SNAME()), "
    "cpModifiedBy AS (SUSER_SNAME()), "
    "cpStartTime datetime2 GENERATED ALWAYS AS ROW START NOT NULL DEFAULT SYSUTCDATETIME(), "
    "cpEndTime datetime2 GENERATED ALWAYS AS ROW END NOT NULL DEFAULT CAST('9999-12-31 23:59:59.9999999' AS datetime2), "
    "PERIOD FOR SYSTEM_TIME (cpStartTime, cpEndTime) "
    ") WITH (SYSTEM_VERSIONING = ON (HISTORY_TABLE = dbo.CpHist))";

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
    tmp->ExecuteUpdate(sql = "CREATE UNIQUE INDEX cpNameKey ON CpRec (cpName)");
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(sql, e);
  };

  tmp->Close();
  delete tmp;
  
  return true;
}


bool  CpStore::UpdateTable(long version)
{
  if (version == 0)
    return CreateTable();
      
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  wxString  INTEGER  = connPtr->GetDataType(SQL_INTEGER);
  wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);
  wxString  WVARCHAR = connPtr->GetDataType(SQL_WVARCHAR);

  if (version <= 124)
  {
    wxString str;
    
    str = "ALTER TABLE CpRec ADD "
      "syID        " + INTEGER +  ", "
      "cpBestOf    " + SMALLINT + " DEFAULT 0 "
    ;

    Statement *stmtPtr = NULL;

    try 
    {      
      stmtPtr = connPtr->CreateStatement();
      if (!stmtPtr)
        return false;

      stmtPtr->ExecuteUpdate(str);        
    }
    catch (SQLException &)
    {
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

    wxString str;

    try
    {
      str = "ALTER TABLE CpRec ADD "
        "cpCreatedBy " + WVARCHAR + "(64) NOT NULL DEFAULT (SUSER_SNAME()), "
        "cpModifiedBy AS (SUSER_SNAME()), "
        "cpStartTime datetime2 GENERATED ALWAYS AS ROW START NOT NULL DEFAULT SYSUTCDATETIME(), "
        "cpEndTime datetime2 GENERATED ALWAYS AS ROW END NOT NULL DEFAULT CAST('9999-12-31 23:59:59.9999999' AS datetime2), "
        "PERIOD FOR SYSTEM_TIME (cpStartTime, cpEndTime)"
        ;
      stmtPtr->ExecuteUpdate(str);

      str = "ALTER TABLE CpRec SET (SYSTEM_VERSIONING = ON(HISTORY_TABLE = dbo.CpHist))";
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

  if (version < 153)
  {
    // Setting of how to play (pts to win, etc)
    Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
    Statement *stmtPtr = connPtr->CreateStatement();

    wxString str;

    try
    {
      str = "ALTER TABLE CpRec ADD "
          "cpPtsToWin     "+SMALLINT+" NOT NULL DEFAULT 11, "
          "cpPtsAhead     "+SMALLINT+" NOT NULL DEFAULT 2,  "
          "cpPtsToWinLast "+SMALLINT+" NOT NULL DEFAULT 11, "
          "cpPtsAheadLast "+SMALLINT+" NOT NULL DEFAULT 2  "
        ;
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

  if (version < 154)
  {
    // Setting of how to play (pts to win, etc)
    Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
    Statement *stmtPtr = connPtr->CreateStatement();

    wxString str;

    try
    {
      str = "ALTER TABLE CpRec ADD "
    	  "cpCategory     "+WVARCHAR+"(64) DEFAULT NULL "
        ;
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


bool  CpStore::CreateConstraints()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str;
  
  try
  {
    tmp->ExecuteQuery("ALTER TABLE CpRec DROP CONSTRAINT cpSyRef");
  }
  catch (SQLException &)
  {
  }

  try
  {
    tmp->ExecuteUpdate(str = 
      "ALTER TABLE CpRec ADD CONSTRAINT cpSyRef "
      "FOREIGN KEY (syID) REFERENCES SyRec (syID) ON DELETE NO ACTION");
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
  };

  delete tmp;
  return true;
}


bool  CpStore::UpdateConstraints(long version)
{
  if (version <= 124)
    return CreateConstraints();

  return true;
}


// -----------------------------------------------------------------------
// Konstruktor
CpStore::CpStore(Connection *connPtr)
       : StoreObj(connPtr)
{
}


CpStore::~CpStore()
{
}


// -----------------------------------------------------------------------
// Select
bool  CpStore::SelectAll()
{
  wxString str = SelectString();
  str += "";

  try
  {
    if (!ExecuteQuery(str))
      return false;

    BindRec();
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  return true;
}


bool  CpStore::SelectById(long id)
{
  wxString str = SelectString();
  str += " WHERE cpID = " + ltostr(id);

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


bool  CpStore::SelectByName(const wxString &name)
{
  wxString str = SelectString();
  str += " WHERE cpName = '";
  str += name;
  str += "'";

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


bool  CpStore::Insert()
{
  PreparedStatement *stmtPtr = 0;

  wxString str = "INSERT INTO CpRec (cpID, cpName, cpDesc, cpCategory, cpType, cpSex, cpYear, syID, cpBestOf, "
                    "cpPtsToWin, cpPtsToWinLast, cpPtsAhead, cpPtsAheadLast) "
                    "VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

  try
  {
    cpID = IdStore::ID(GetConnectionPtr());

    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    int idx = 0;

    stmtPtr->SetData(++idx, &cpID);
    stmtPtr->SetData(++idx, cpName);
    stmtPtr->SetData(++idx, cpDesc);
    stmtPtr->SetData(++idx, *cpCategory ? cpCategory : NULL);
    stmtPtr->SetData(++idx, &cpType);
    stmtPtr->SetData(++idx, &cpSex);
    stmtPtr->SetData(++idx, &cpYear);
    stmtPtr->SetData(++idx, syID ? &syID : NULL);
    stmtPtr->SetData(++idx, &cpBestOf);
    stmtPtr->SetData(++idx, cpPtsToWin ?  &cpPtsToWin : &defPtsToWin);
    stmtPtr->SetData(++idx, cpPtsToWinLast ? &cpPtsToWinLast : &defPtsToWin);
    stmtPtr->SetData(++idx, cpPtsAhead ? &cpPtsAhead : &defPtsAhead);
    stmtPtr->SetData(++idx, cpPtsAheadLast ? &cpPtsAheadLast : &defPtsAhead);

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
  update.rec  = CRequest::CPREC;
  update.id   = cpID;

  CTT32App::NotifyChange(update);

  return true;
}


bool  CpStore::Update()
{
  PreparedStatement *stmtPtr = 0;

  wxString str = "UPDATE CpRec "
                    "SET cpName = ?, cpDesc = ?, cpCategory = ?, "
                     "   cpType = ?, cpSex = ?, cpYear = ?, "
                     "   syID = ?, cpBestOf = ?, "
                     "   cpPtsToWin = ?, cpPtsToWinLast = ?, "
                     "   cpPtsAhead = ?, cpPtsAheadLast = ? "
                    "WHERE cpID = ?";

  try
  {
    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    int idx = 0;

    stmtPtr->SetData(++idx, cpName);
    stmtPtr->SetData(++idx, cpDesc);
    stmtPtr->SetData(++idx, *cpCategory ? cpCategory : NULL);
    stmtPtr->SetData(++idx, &cpType);
    stmtPtr->SetData(++idx, &cpSex);
    stmtPtr->SetData(++idx, &cpYear);
    stmtPtr->SetData(++idx, syID ? &syID : NULL);
    stmtPtr->SetData(++idx, &cpBestOf);
    stmtPtr->SetData(++idx, cpPtsToWin ?  &cpPtsToWin : &defPtsToWin);
    stmtPtr->SetData(++idx, cpPtsToWinLast ? &cpPtsToWinLast : &defPtsToWin);
    stmtPtr->SetData(++idx, cpPtsAhead ? &cpPtsAhead : &defPtsAhead);
    stmtPtr->SetData(++idx, cpPtsAheadLast ? &cpPtsAheadLast : &defPtsAhead);
    stmtPtr->SetData(++idx, &cpID);

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
  update.rec  = CRequest::CPREC;
  update.id   = cpID;

  CTT32App::NotifyChange(update);

  return true;
}


bool  CpStore::Remove(long id)
{
  // ID-Check
  if (!id)
    id = cpID;

  if (!id)
    return true;
  
  wxString str;
  
  try
  {
    // Erst die Teams aus den Gruppen austragen. Hier faengt sonst 
    // ein ON DELETE NO ACTION
    str = "UPDATE StRec SET tmID = NULL "
          " WHERE tmID IN (SELECT tmID FROM TmRec WHERE cpID = " + ltostr(id) + ")";
    ExecuteUpdate(str);
    
    // Dann die Spielermeldungen rauswerfen. Das geht nicht automatisch
    // in der DB, da hier ein ON DELETE NO ACTION wirkt
    str = "DELETE FROM TmRec WHERE cpID = " + ltostr(id);
    ExecuteUpdate(str);
    
    // Meldungen extra loeschen
    str = "DELETE FROM LtRec WHERE cpID = " + ltostr(id);
    ExecuteUpdate(str);          

    // Und dann den Wettbewerb (loescht Gruppen etc.)
    str = "DELETE FROM CpRec WHERE cpID = " + ltostr(id);
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
  update.rec  = CRequest::CPREC;
  update.id   = id;

  CTT32App::NotifyChange(update);

  return true;
}


// Check auf cpName, ob WB existiert
bool  CpStore::InsertOrUpdate()
{
  if (cpType == 0 || *cpName == 0)
    return false;

  Statement *stmtPtr;
  ResultSet *resPtr;

  long  id;
  
  stmtPtr = GetConnectionPtr()->CreateStatement();

  wxString  sql = "SELECT cpID FROM CpRec WHERE cpName = '";
  sql += cpName;
  sql += "'";

  resPtr = stmtPtr->ExecuteQuery(sql);
  resPtr->BindCol(1, &id);
  bool  exist = (resPtr->Next() && !resPtr->WasNull(1));

  delete resPtr;
  delete stmtPtr;

  if (exist)
  {
    cpID = id;
    return Update();
  }
  else
    return Insert();
}


// -----------------------------------------------------------------------
// Import / Export
// Siehe PlStore zur Verwendung von std::ifstream
bool  CpStore::Import(wxTextBuffer &is)
{
  long version = 1;

  wxString line = is.GetFirstLine();

  // Check header
  if (!CheckImportHeader(line, "#EVENTS", version))
  {
    if (!infoSystem.Question(_("First comment is not %s but \"%s\". Continue anyway?"), wxT("#EVENTS"), line.wx_str()))
      return false;
  }

  if (version > 2)
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

    CpStore  cp(connPtr);

    if (cp.Read(line, version))
    {
      connPtr->StartTransaction();

      if (cp.InsertOrUpdate())
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


bool  CpStore::Export(wxTextBuffer &os, long version)
{
  CpStore  cp;
  if (!cp.SelectAll())
    return false;

  os.AddLine(wxString::Format("#EVENTS %d", version));

  // Encapsulate local vars
  {
    wxString line;
    line << "# "
         << "Name;"
         << "Description;"
    ;
    if (version > 1)
      line << "Category;";
    line 
        << "Type;"
        << "Sex;"
        << "Year;"
    ;

    os.AddLine(line);
  }

  while (cp.Next())
  {
    wxString line;
    if (cp.Write(line, version))
      os.AddLine(line);
  }

  return true;
}


long CpStore::GetMaxSupportedExportVersion()
{
  return 2;
}


// -----------------------------------------------------------------------
bool  CpStore::EnlistPlayer(const PlRec &pl)
{
  LtStore  lt(GetConnectionPtr());

  return EnlistPlayer(pl, lt);
}


bool  CpStore::EnlistPlayer(const PlRec &pl, LtStore &lt)
{
  if (!IsAllowed(pl))
    return false;

  if (lt.SelectByCpPl(cpID, pl.plID) && lt.Next())
    return true;

  if (!lt.Insert(*this, pl))
    return false;

  return true;
}


bool  CpStore::RemovePlayer(const PlRec &pl)
{
  LtStore  lt(GetConnectionPtr());
  if (!lt.SelectByCpPl(cpID, pl.plID) || !lt.Next())
    return true;

  lt.Close();
  
  return RemovePlayer(lt);
}


bool  CpStore::RemovePlayer(LtStore &lt)
{
  // Break up Team, if not Team
  if (cpType != CP_TEAM)
  {
    NtStore  nt(GetConnectionPtr());
    while (nt.SelectByLt(lt) && nt.Next())
    {
      nt.Close();

      // Delete team from all groups
      std::list<std::pair<long, short>> grposList;

      wxString str = "SELECT grID, stNr FROM StRec WHERE tmID = " + ltostr(nt.tmID);

      try
      {
        long id;
        short nr;

        Statement *stmtPtr = GetConnectionPtr()->CreateStatement();
        ResultSet *resPtr  = stmtPtr->ExecuteQuery(str);

        while (resPtr->Next())
        {
          resPtr->GetData(1, id);
          resPtr->GetData(2, nr);
          std::pair<long, short> p(id, nr);
          grposList.push_back(p);
        }

        delete resPtr;
        delete stmtPtr;
      }
      catch (SQLException &e)
      {
        infoSystem.Exception(str, e);
        return false;
      }

      for (std::list<std::pair<long, short>>::iterator it = grposList.begin(); it != grposList.end(); it++)
      {
        GrStore gr(GetConnectionPtr());
        gr.SelectById((*it).first);
        gr.Next();
        gr.Close();
       
        gr.SetTeam((*it).second, TmRec());
      }

      if (!TmStore(GetConnectionPtr()).Remove(nt.tmID))
        return false;
    }
  }

  // Und jetzt den Spieler abmelden
  return lt.Remove();
}


bool  CpStore::CreateSingle(const PlRec &pl, short natlRank, short intlRank)
{
  LtStore  lt(GetConnectionPtr());
  if (!lt.SelectByCpPl(cpID, pl.plID) || !lt.Next())
    return false;
 
  NaRec  na;
  na.naID = pl.naID;

  return CreateSingle(lt, na, natlRank, intlRank);
}


bool  CpStore::CreateSingle(LtStore &lt, const NaRec &na, short natlRank, short intlRank)
{
  TmStore  tm(GetConnectionPtr());
  RkStore  rk(GetConnectionPtr());
  
  // Tm- und Rk-Record einfuegen, wenn sie noch nicht existieren
  tm.SelectByLt(lt);
  if ( !tm.Next() )
  {
    // TM existiert noch nicht, also einfuegen.
    if (!tm.Insert(*this) || !tm.AddEntry(lt, 1))
      return false;
  }
  
  tm.Close();
    
  // Update von Ranking (Remove / Insert)
  rk.SelectByTm(tm);
  if (rk.Next())
  {
    rk.Close();
    
    // RK existert. Ranking testen
    if ( (!natlRank || natlRank == rk.rkNatlRank) && 
         (!intlRank || intlRank == rk.rkIntlRank) )
      return true;
  }
    
  // RK existiert nicht oder Ranking ist falsch.
  rk.Remove(tm);
  
  return rk.Insert(tm, na, natlRank, intlRank);
}


bool  CpStore::CreateDouble(const PlRec &pl, const PlRec &bd, 
                            const NaRec &na, short natlRank, short intlRank)
{
  // Fehler, wenn kein Spieler angegeben wurde
  if (!pl.plID)
    return false;

  LtEntryStore  ltpl(GetConnectionPtr()), ltbd(GetConnectionPtr());

  // Meldung des Spielers ermitteln (muss existieren)
  ltpl.SelectByCpPl(cpID, pl.plID);
  if (!ltpl.Next())
    return false;

  // Meldung des Partners ermitteln, wenn einer angegeben ist
  if (bd.plID)
  {
    // Muss in dem Fall auch existieren
    ltbd.SelectByCpPl(cpID, bd.plID);
    if (!ltbd.Next())
      return false;
  }

  // CreateDouble loest auch ein Doppel auf!
  if (CheckDoubleOrder(ltpl, ltbd, na))
    return CreateDouble((const LtRec &) ltpl, (const LtRec &) ltbd, na, natlRank, intlRank);
  else
    return CreateDouble((const LtRec &) ltbd, (const LtRec &) ltpl, na, natlRank, intlRank);
}


bool  CpStore::CreateDouble(const LtRec &ltpl, const LtRec &ltbd, 
                            const NaRec &na, short natlRank, short intlRank)
{
  // Die Spielermeldung muss existieren
  if (!ltpl.ltID)
    return false;

  NtStore  ntpl(GetConnectionPtr()), ntbd(GetConnectionPtr());
  TmStore  tmpl(GetConnectionPtr()), tmbd(GetConnectionPtr());
  RkStore  rkpl(GetConnectionPtr()), rkbd(GetConnectionPtr());

  // Doppel Spieler ermitteln
  ntpl.SelectByLt(ltpl);
  ntpl.Next();
  ntpl.Close();

  tmpl.SelectById(ntpl.tmID);
  tmpl.Next();
  tmpl.Close();

  rkpl.SelectByTm(tmpl);
  rkpl.Next();
  rkpl.Close();

  // Wenn der Partner nicht existiert, ist es eine Abmeldung
  if (tmpl.tmID && !ltbd.ltID)
  {
    return TmStore(GetConnectionPtr()).Remove(tmpl.tmID);
  }

  // Doppel Partner ermitteln
  ntbd.SelectByLt(ltbd);
  ntbd.Next();
  ntbd.Close();

  tmbd.SelectById(ntbd.tmID);
  tmbd.Next();
  tmpl.Close();

  rkbd.SelectByTm(tmbd);
  rkbd.Next();
  rkbd.Close();

  // Wir sind fertig, wenn die beiden uebereinstimmen
  if ( ntpl.tmID && (ntpl.tmID == ntbd.tmID) )
  {
    // Keine Aenderung der Nation und Spieler ist fuehrend
    if (rkpl.naID == na.naID && ntpl.ntNr == 1)
    {
      if ( (!natlRank || rkpl.rkNatlRank == natlRank) && 
           (!intlRank || rkpl.rkIntlRank == intlRank) )
        return true;
        
      rkpl.Remove(tmpl);
      return rkpl.Insert(tmpl, na, natlRank, intlRank);
    }

    // Spieler umdrehen, wenn sich Nation aendert
    if (!tmpl.RemoveEntry(ltpl))
      return false;

    if (!tmpl.RemoveEntry(ltbd))
      return false;

    if (!tmpl.AddEntry(ltpl, 1))
      return false;

    if (!tmpl.AddEntry(ltbd, 2))
      return false;

    if (!rkpl.Remove(tmpl))
      return false;

    if (!rkpl.Insert(tmpl, na, natlRank, intlRank))
      return false;

    return true;
  }

  // Beide Doppel loeschen (sorgen auch fuer Meldungen)
  if (tmpl.tmID && !tmpl.Remove() || tmbd.tmID && !tmbd.Remove())
    return false;

  // Neues Doppel anlegen
  TmStore  tm(GetConnectionPtr());
  RkStore  rk(GetConnectionPtr());
  if (!tm.Insert(*this))
    return false;

  // Meldungen einfuegen. 
  // Muss for Ranking passieren, da das Ranking berechnet wird.
  if (!tm.AddEntry(ltpl, 1))
    return false;

  if (!tm.AddEntry(ltbd, 2))
    return false;

  if (na.naID && !rk.Insert(tm, na, natlRank, intlRank))
    return false;

  return true;
}


bool  CpStore::CheckDoubleOrder(const LtEntry &pl, const LtEntry &bd, const NaRec &na)
{
  // Auf jedem Fall OK, wenn der Partner nicht spielt
  if (bd.ltID == 0)
    return true;
    
  if (cpType == CP_MIXED)
  {
    if (CTT32App::instance()->GetTable() == TT_DTTB)
    {
      if (pl.psSex == SEX_FEMALE)
        return true;
      else
        return false;
    }
    else
    {
      if (pl.psSex == SEX_MALE)
        return true;
      else
        return false;
    }
  }
  else if (na.naID == 0)
  {
    if (false) 
    {
      int rc = wxStrcoll(pl.psName.psLast, bd.psName.psLast);
      if (rc == 0)
        rc = wxStrcoll(pl.psName.psFirst, bd.psName.psFirst);

      return rc <= 0;
    }

    // Keine Nation: Rangfolge bestimmen
    if (pl.ltRankPts < bd.ltRankPts)
      return false;
    else if (pl.ltRankPts > bd.ltRankPts)
      return true;
    else if (pl.plNr < bd.plNr)
      return true;
    else
      return false;
  }
  else
  {
    // Nation muss die vom Spieler sein.
    // Alles andere spielt keine Rolle, das ist Aufgabe vom Input
    if (pl.naID == na.naID)
      return true;
    else
      return false;
  }
}


bool  CpStore::CreateTeam(TmStore &tm, const NaRec &na, short natlRank, short intlRank)
{
  if (cpType != CP_TEAM)
    return false;

  RkStore  rk(GetConnectionPtr());

  if (tm.tmID)
  {
    rk.SelectByTm(tm);
    if (rk.Next())
    {
      rk.Close();
      
      if ( rk.naID == na.naID &&
           (!natlRank || rk.rkNatlRank == natlRank) &&
           (!intlRank || rk.rkIntlRank == intlRank) )
           return true;
           
       rk.Remove(tm);
    }
  }
  else
  {
    if (!tm.Insert(*this))
      return false;
  }

  if (na.naID)
  {
    if (!rk.Insert(tm, na, natlRank, intlRank))
      return false;
  }

  return true;
}


// -----------------------------------------------------------------------
wxString  CpStore::SelectString() const
{
  return   
    "SELECT cpID, cpName, cpDesc, cpCategory, cpType, cpSex, cpYear, syID, "
    "       cpBestOf, cpPtsToWin, cpPtsToWinLast, cpPtsAhead, cpPtsAheadLast FROM CpRec ";
}


void  CpStore::BindRec()
{
  int idx = 0;

  BindCol(++idx, &cpID);
  BindCol(++idx, cpName, sizeof(cpName));
  BindCol(++idx, cpDesc, sizeof(cpDesc));
  BindCol(++idx, cpCategory, sizeof(cpCategory));
  BindCol(++idx, &cpType);
  BindCol(++idx, &cpSex);
  BindCol(++idx, &cpYear);
  BindCol(++idx, &syID);
  BindCol(++idx, &cpBestOf);
  BindCol(++idx, &cpPtsToWin);
  BindCol(++idx, &cpPtsToWinLast);
  BindCol(++idx, &cpPtsAhead);
  BindCol(++idx, &cpPtsAheadLast);
}


void  CpStore::Init()
{
  CpRec::Init();
}