/* Copyright (C) 2020 Christoph Theis */

// Ranking points per Altersklasse

#include "stdafx.h"

#include "RpStore.h"

#include "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "ResultSet.h"
#include  "SQLException.h"
#include  "InfoSystem.h"

#include  "IdStore.h"
#include  "PlStore.h"

#include  "Request.h"
#include  "StrUtils.h"
#include  "wxStringTokenizerEx.h"

#include  <map>
#include  <fstream>


bool RpStore::CreateTable()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  wxASSERT(connPtr);

  Statement *tmp = connPtr->CreateStatement();

  wxString  INTEGER  = connPtr->GetDataType(SQL_INTEGER);
  wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);
  wxString  FLOAT    = connPtr->GetDataType(SQL_FLOAT);

  wxString  sql = 
    "CREATE TABLE RpRec ( "
    "rpID           "+INTEGER+"      NOT NULL, "
    "plID           "+INTEGER+"      NOT NULL, "
    "rpYear         "+SMALLINT+"     NOT NULL, "
    "rpRankPts      "+FLOAT+"        NOT NULL DEFAULT 0,"
    "CONSTRAINT rpKey PRIMARY KEY (rpID)   "    
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
    tmp->ExecuteUpdate(sql = "CREATE INDEX rpPlID ON RpRec (plID)");
    tmp->ExecuteUpdate(sql = "CREATE UNIQUE INDEX rpYearKey ON RpRec (plID, rpYear)");
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(sql, e);
  };

  tmp->Close();
  delete tmp;
  
  return true;
}


bool RpStore::UpdateTable(long version)
{
  if (version <= 125)
    return CreateTable();

  return true;
}


bool RpStore::CreateConstraints()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  INTEGER  = connPtr->GetDataType(SQL_INTEGER);
  wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);
  
  wxString  str;
  
  try
  {
    tmp->ExecuteUpdate("ALTER TABLE RpRec DROP CONSTRAINT rpPlRef");
  }
  catch (SQLException &)
  {
  }

  try
  {    
    tmp->ExecuteUpdate(str = 
      "ALTER TABLE RpRec ADD CONSTRAINT rpPlRef "
      "FOREIGN KEY (plID) REFERENCES PlRec (plID) ON DELETE CASCADE");
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
  };

  delete tmp;
  return true;
}


bool RpStore::UpdateConstraints(long version)
{
  if (version <= 125)
    return CreateConstraints();

  return true;
}


// -----------------------------------------------------------------------
RpStore::RpStore(Connection *ptr)
       : StoreObj(ptr)
{
}


RpStore::RpStore(const RpRec &rec, Connection *ptr)
       : StoreObj(ptr), RpRec(rec)
{
}


RpStore::~RpStore()
{
}


void  RpStore::Init()
{
  RpRec::Init();
}


// -----------------------------------------------------------------------
bool RpStore::Insert()
{
  PreparedStatement *stmtPtr = 0;

  wxString str = "INSERT INTO RpRec (rpID, plID, rpYear, rpRankPts) "
                    "VALUES(?, ?, ?, ?)";

  try
  {
    rpID = IdStore::ID(GetConnectionPtr());

    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    stmtPtr->SetData(1, &rpID);
    stmtPtr->SetData(2, &plID);
    stmtPtr->SetData(3, &rpYear);
    stmtPtr->SetData(4, &rpRankPts);

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
  update.rec  = CRequest::PLREC;
  update.id   = plID;

  CTT32App::NotifyChange(update);

  return true;
}


bool RpStore::RemoveByPl(long plID)
{
  wxString str ="DELETE FROM RpRec WHERE plID = " + ltostr(plID);
  
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
  update.type = CRequest::UPDATE;
  update.rec  = CRequest::PLREC;
  update.id   = plID;

  CTT32App::NotifyChange(update);

  return true;
}


bool RpStore::SelectByPl(long plID)
{
  wxString str = SelectString();
  str += " WHERE plID = " + ltostr(plID);

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
wxString RpStore::SelectString() const
{
  return "SELECT rpID, plID, rpYear, rpRankPts FROM RpRec";
}


bool RpStore::BindRec()
{
  int idx = 0;
  BindCol(++idx, &rpID);
  BindCol(++idx, &plID);
  BindCol(++idx, &rpYear);
  BindCol(++idx, &rpRankPts);

  return true;
}


// -----------------------------------------------------------------------
bool RpStore::Import(wxTextBuffer &is)
{
  long version = 1;

  wxString line = is.GetFirstLine();
  // Check header
  if (!CheckImportHeader(line, "#RANKINGPOINTS", version))
  {
    if (!infoSystem.Question(_("First comment is not %s but \"%s\". Continue anyway?"), wxT("#RANKINGPOINTS"), line.wx_str()))
      return false;
  }

  if (version > 1)
  {
    infoSystem.Error(_("Version %d of import file is not supported"), version);
    return false;
  }

  Connection *connPtr = TTDbse::instance()->GetNewConnection();

  const char *oldLocale = setlocale(LC_NUMERIC, "C");
  
  for (; !is.Eof(); line = is.GetNextLine())
  {
    CTT32App::ProgressBarStep();

    if (line.IsEmpty())
      continue;

    if (line.GetChar(0) == '#')
      continue;

    wxStringTokenizerEx tokens(line, ",;\t");

    wxString strPl = tokens.GetNextToken().Strip(wxString::both);

    // TODO: Fehlermeldungen
    if (!_strtol(strPl))
      continue;

    PlStore pl(connPtr);
    pl.SelectByNr(_strtol(strPl));
    pl.Next();
    pl.Close();
    if (!pl.WasOK())
      continue;

    RpStore rp(connPtr);

    bool commit = true;

    commit &= rp.RemoveByPl(pl.plID);

    while (tokens.HasMoreTokens())
    {
      wxString strAge = tokens.GetNextToken().Strip(wxString::both);
      wxString strPts = tokens.GetNextToken().Strip(wxString::both);

      if (strAge.IsEmpty() || strPts.IsEmpty())
        continue;

      if (!_strtol(strAge))
        continue;

      rp.plID = pl.plID;
      rp.rpYear = _strtol(strAge);
      rp.rpRankPts = _strtof(strPts);
      commit &= rp.Insert();
    }

    if (commit)
      connPtr->Commit();
    else
      connPtr->Rollback();
  }
    
  setlocale(LC_NUMERIC, oldLocale);

  delete connPtr;

  return true;
}


bool RpStore::Export(wxTextBuffer &os, long version)
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  PlStore  pl(connPtr);
  if (!pl.SelectAll())
    return false;

  std::map<short, long> plNrToId;

  while (pl.Next())
    plNrToId[pl.plNr] = pl.plID;

  pl.Close();
    
  // Rank pts are float
  const char *oldLocale = setlocale(LC_NUMERIC, "C");

  os.AddLine(wxString::Format("#RANKINGPOINTS %d", version));
  
  os.AddLine("# Pl. No.; Born; Ranking Pts; ...");
  
  for (std::map<short, long>::const_iterator it = plNrToId.cbegin(); it != plNrToId.cend(); it++)
  {
    RpStore rp(connPtr);
    rp.SelectByPl(it->second);
    if (rp.WasOK())
    {
      wxString line;
      line << it->second << ";";
      do
      {
        line << rp.rpYear << ";" << rp.rpRankPts << ";";
      } while (rp.Next());

      os.AddLine(line);
    }
  }

  setlocale(LC_NUMERIC, oldLocale);

  return true;
}