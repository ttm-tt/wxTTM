/* Copyright (C) 2020 Christoph Theis */

// DB-Tabelle der Spieler

#include  "stdafx.h"
#include  "PlStore.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "ResultSet.h"
#include  "SQLException.h"
#include  "InfoSystem.h"
#include  "Request.h"

#include  "IdStore.h"
#include  "LtStore.h"
#include  "CpStore.h"

#include  "PlListStore.h"

#include  "Rec.h"
#include  "StrUtils.h"
#include  "wxStringTokenizerEx.h"

#include  <stdio.h>
#include  <fstream>
#include  <stdlib.h>


// PlRec mit psNote
struct PlRecImpExp : public PlRec
{
  wxString psNote;

  bool Read(const wxString &line);
  bool Write(wxString &line) const;
};


bool PlRecImpExp::Read(const wxString &line)
{
  // Ignore comments
  if (line.StartsWith("#"))
    return false;

  // Das erste Zeichen aus ",;\t" ist das Trennzeichen
  // Es kann naemlich vorkommen, dass ";" das Trennzeichen
  // ist, aber dennoch "," in den Strings vorkommen. 
  wxChar sep[2];
  const wxChar *sepChar = wxStrpbrk(line, wxT(",;\t"));
  if (sepChar == NULL)
    return false;
    
  sep[0] = *sepChar;
  sep[1] = 0;

  wxStringTokenizerEx tokens(line, sep);

  wxString strNr    = tokens.GetNextToken();
  wxString strLast  = tokens.GetNextToken().Strip(wxString::both);
  wxString strFirst = tokens.GetNextToken().Strip(wxString::both);
  wxString strSex   = tokens.GetNextToken().Strip(wxString::both);
  wxString strYear  = tokens.GetNextToken();
  wxString strNa    = tokens.GetNextToken().Strip(wxString::both);
  wxString strExtID = tokens.GetNextToken().Strip(wxString::both);
  wxString strRankPts = tokens.GetNextToken().Strip(wxString::both);
  wxString strComment = (tokens.HasMoreTokens() ? tokens.GetNextToken().Strip(wxString::both).c_str() : wxEmptyString);
  
  // If both, family and given name, are empty, skip
  if (strNr.IsEmpty() && strLast.IsEmpty() && strFirst.IsEmpty())
    return false;

  // Wenn der Vorname leer ist und der Nachname Leerzeichenenthaelt, 
  // den Nachnamen aufsplitten und den Vornamen abtrennen.
  // Per Konvention ist der Nachname "all upper case" oder ein Zeichen lang.
  if (!strLast.IsEmpty() && strFirst.IsEmpty() && strLast.Find(' ') != wxNOT_FOUND)
  {
    Init();
 
    wxStringTokenizerEx tokens(strLast, " ");
    strLast = "";

    while (tokens.HasMoreTokens())
    {
      wxString tmp = tokens.NextToken();
      if (tmp.IsEmpty())
        continue;
      else if (strLast.IsEmpty() && wxIsupper(tmp.GetChar(1)))
        strLast = tmp;
      else if (tmp.Length() == 1 || wxIsupper(tmp.GetChar(1)))
      {
        strLast += " ";
        strLast += tmp;
      }
      else if (strFirst.IsEmpty())
        strFirst = tmp;
      else
      {
        strFirst += " ";
        strFirst += tmp;
      }
    }    
  }

  // Gebrutstage auf Jahr beschraenken. Vorraussetzung ist ein 4-stelliges Jahr
  if (strYear.Length() > 4)
  {
    wxStringTokenizerEx tokens(strYear, "./-");
    while (tokens.HasMoreTokens())
    {
      wxString tmp = tokens.NextToken();
      if (tmp.Length() >= 4)
      {
        strYear = tmp;
        break;
      }
    }
  }
  
  if (!strLast.IsEmpty() && !strSex.IsEmpty())
  {
    Init();

    plNr       = _strtol(strNr);
    wxStrncpy((wxChar *) psName.psLast, strLast, sizeof(psName.psLast) / sizeof(wxChar) -1);
    wxStrncpy((wxChar *) psName.psFirst, strFirst, sizeof(psName.psFirst) / sizeof(wxChar) -1);
    psBirthday = _strtol(strYear);
      
    switch (*strSex.t_str())
    {
      case 'w' :
      case 'W' :
      case 'f' :
      case 'F' :
      case '2' :
        psSex = 2;
        break;
        
      case 'm' :
      case 'M' :
      case '1' :
        psSex = 1;
        break;
    }
    
    wxStrncpy(naName, strNa, sizeof(naName) / sizeof(wxChar) - 1);
    wxStrncpy(plExtID, strExtID, sizeof(plExtID) / sizeof(wxChar) - 1);

    strRankPts.ToCDouble(&plRankPts);
  }

  psNote = URLDecode(strComment);

  return true;
}


bool PlRecImpExp::Write(wxString &line) const
{
  // Nacht UTF-8 konvertieren
  line << plNr << ";"
       << psName.psLast << ";"
       << psName.psFirst << ";"
       << psSex << ";"
       << psBirthday << ";"
       << naName << ";"        // Name der Nation
       << plExtID << ";"      // ITTF / DTTB ID
       << wxString::FromCDouble(plRankPts) << ";"    // Ranking points
       << URLEncode(psNote) << ""
  ;

  return true;
}



// -----------------------------------------------------------------------
// Neue Tabelle in DB erzeugen
bool  PlStore::CreateTable()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  Statement *tmp = connPtr->CreateStatement();

  wxString  WVARCHAR = connPtr->GetDataType(SQL_WVARCHAR);
  wxString  INTEGER  = connPtr->GetDataType(SQL_INTEGER);
  wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);
  wxString  FLOAT    = connPtr->GetDataType(SQL_FLOAT);
  wxString  TIMESTAMP = connPtr->GetDataType(SQL_TIMESTAMP);

  wxString  sql = 
    "CREATE TABLE PlRec (       "
    "plID        "+INTEGER+"    NOT NULL,  "
    "psID        "+INTEGER+"    NOT NULL,  "
    "naID        "+INTEGER+",   "
    "plNr        "+INTEGER+"    NOT NULL,  "
    "plExtID     "+WVARCHAR+"(64),         "
    "plRankPts   "+FLOAT+"      DEFAULT 0, "
    "CONSTRAINT plIdKey PRIMARY KEY (plID), "
    "plCreatedBy " + WVARCHAR + "(64) NOT NULL DEFAULT (SUSER_SNAME()), "
    "plModifiedBy AS (SUSER_SNAME()), "
    "plStartTime datetime2 GENERATED ALWAYS AS ROW START NOT NULL DEFAULT SYSUTCDATETIME(), "
    "plEndTime datetime2 GENERATED ALWAYS AS ROW END NOT NULL DEFAULT CAST('9999-12-31 23:59:59.9999999' AS datetime2), "
    "PERIOD FOR SYSTEM_TIME (plStartTime, plEndTime) "
    ") WITH (SYSTEM_VERSIONING = ON (HISTORY_TABLE = dbo.PlHist))";

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
    tmp->ExecuteUpdate(sql = "CREATE UNIQUE INDEX plNrKey ON PlRec (plNr)");
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(sql, e);
  };

  delete tmp;
  
  return true;
}


bool  PlStore::UpdateTable(long version)
{
  if (version == 0)
    return CreateTable();
    
  if (version <= 62)
  {
    Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
    Statement *tmp = connPtr->CreateStatement();

    wxString  WVARCHAR  = connPtr->GetDataType(SQL_WVARCHAR);
    
    try
    {
      tmp->ExecuteUpdate("ALTER TABLE PlRec ADD "
          "plExtID "+WVARCHAR+"(64)");
    }
    catch (SQLException &)
    {
      delete tmp;
      return false;
    }
  }
  
  if (version <= 72)
  {
    Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
    Statement *tmp = connPtr->CreateStatement();

    wxString  FLOAT = connPtr->GetDataType(SQL_FLOAT);
    
    try
    {
      tmp->ExecuteUpdate("ALTER TABLE PlRec ADD "
          "plRankPts "+FLOAT+" DEFAULT 0");
    }
    catch (SQLException &)
    {
      delete tmp;
      return false;
    }
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
      str = "ALTER TABLE PlRec ADD "
        "plCreatedBy " + WVARCHAR + "(64) NOT NULL DEFAULT (SUSER_SNAME()), "
        "plModifiedBy AS (SUSER_SNAME()), "
        "plStartTime datetime2 GENERATED ALWAYS AS ROW START NOT NULL DEFAULT SYSUTCDATETIME(), "
        "plEndTime datetime2 GENERATED ALWAYS AS ROW END NOT NULL DEFAULT CAST('9999-12-31 23:59:59.9999999' AS datetime2), "
        "PERIOD FOR SYSTEM_TIME (plStartTime, plEndTime)"
        ;
      stmtPtr->ExecuteUpdate(str);

      str = "ALTER TABLE PlRec SET (SYSTEM_VERSIONING = ON(HISTORY_TABLE = dbo.PlHist))";
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

  if (version < 150)
  {
    Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
    Statement *tmp = connPtr->CreateStatement();

    wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);

    try
    {
      tmp->ExecuteUpdate("ALTER TABLE PlRec ADD "
                         "plDeleted " + SMALLINT + " NOT NULL DEFAULT 0");
    }
    catch (SQLException &)
    {
      delete tmp;
      return false;
    }
  }

  if (version < 181)
  {
    Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
    Statement *tmp = connPtr->CreateStatement();

    wxString  TIMESTAMP = connPtr->GetDataType(SQL_TIMESTAMP);

    wxString sql = "ALTER TABLE PsRec ADD psDeleteTime " + TIMESTAMP + " DEFAULT NULL";

    try
    {
      tmp->ExecuteUpdate(sql);

      // Update PsRec comes before PlRec
      sql = "UPDATE PsRec SET psDeleteTime = GETUTCDATE() "
            " WHERE psID IN (SELECT ps.psID FROM PlRec pl INNER JOIN PsRec ps ON pl.psID = ps.psID WHERE plDeleted = 1)";
      tmp->ExecuteUpdate(sql);

      // We must drop any constraints on the column we want to delete
      // DROP CONTRAINT braucht den Namen, den vergebe ich aber nicht explizit
      // Also in den Sys-Tabellen suchen
      Statement *dstmt = connPtr->CreateStatement();
      sql = 
          "SELECT d.name, c.name "
          "  FROM sys.tables t "
          "       INNER JOIN sys.default_constraints d ON d.parent_object_id = t.object_id "
          "       INNER JOIN sys.columns c ON c.object_id = t.object_id AND c.column_id = d.parent_column_id "
          " WHERE t.name = 'PlRec' AND c.name IN ('plDeleted') "
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
        tmp->ExecuteQuery("ALTER TABLE PlRec DROP CONSTRAINT " + it.first);

      sql = "ALTER TABLE PlRec DROP COLUMN plDeleted";
      tmp->ExecuteUpdate(sql);
    }
    catch (SQLException &e)
    {
      infoSystem.Exception(sql, e);

      delete tmp;
      return false;
    }
  }

  return true;
}


bool  PlStore::CreateConstraints()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str;

  try
  {
    tmp->ExecuteUpdate("DROP TRIGGER plUpdateTrigger");
  }
  catch (SQLException &)
  {
  }
  
  try
  {
    tmp->ExecuteUpdate("ALTER TABLE PlRec DROP CONSTRAINT plPsRef");
    tmp->ExecuteUpdate("ALTER TABLE PlREc DROP CONSTRAINT plNaRef");
  }
  catch (SQLException &)
  {
  }

  try
  {
    tmp->ExecuteUpdate(str = 
        "CREATE OR ALTER TRIGGER plUpdateTrigger ON PlRec FOR UPDATE AS \n"
        " --- Update timestamp for last changed \n"
        "UPDATE PsRec SET psTimestamp = GETUTCDATE() \n"
        " WHERE psID IN (SELECT psID FROM deleted) \n;");
        
    tmp->ExecuteUpdate(str = 
      "ALTER TABLE PlRec ADD CONSTRAINT plPsRef "
      "FOREIGN KEY (psID) REFERENCES PsRec (psID) ON DELETE CASCADE");

    tmp->ExecuteUpdate(str = 
      "ALTER TABLE PlRec ADD CONSTRAINT plNaRef "
      "FOREIGN KEY (naID) REFERENCES NaRec (naID) ON DELETE NO ACTION");
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
  };

  delete tmp;
  return true;
}


bool  PlStore::UpdateConstraints(long version)
{
  if (version < 86)
    return CreateConstraints();

  return true;
}


// -----------------------------------------------------------------------
// Konstruktor
PlStore::PlStore(Connection *connPtr)
       : StoreObj(connPtr)
{
}


PlStore::~PlStore()
{
}


void  PlStore::Init()
{
  PlRec::Init();
}

// -----------------------------------------------------------------------
// Select
bool  PlStore::SelectAll()
{
  wxString str = SelectString();

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


bool  PlStore::SelectById(long id)
{
  wxString str = SelectString();
  str += " WHERE  PlRec.plID = ";
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


bool  PlStore::SelectByNr(long nr)
{
  wxString str = SelectString();
  str += " WHERE  PlRec.plNr = ";
  str += ltostr(nr);
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


bool  PlStore::SelectByExtId(const wxString &extId)
{
  wxString str = SelectString();
  str += " WHERE  PlRec.plExtID = '";
  str += extId;
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


bool PlStore::SelectByName(const wxString &name, const wxString &na)
{
  wxString str = SelectString();
  str += " WHERE psLast + ' ' + psFirst = '" + TransformString(name) + "'";
  if (!na.IsEmpty())
    str += " AND naName = '" + TransformString(na) + "'";

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


bool PlStore::SelectByNames(const wxString &last, const wxString &first, const wxString &na)
{
  wxString str = SelectString();
  str += " WHERE psLast = '" + TransformString(last) + "' AND psFirst = '" + TransformString(first) + "' ";
  if (!na.IsEmpty())
    str += " AND naName = '" + TransformString(na) + "'";

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
bool  PlStore::Insert()
{
  PreparedStatement *stmtPtr = 0;

  // Insert PsStore
  PsStore  ps(*this, GetConnectionPtr());
  ps.Insert();

  // Referenz aktualisieren
  psID = ps.psID;

  // Person muss (jetzt) existierten
  if (!psID)
    return false;

  // Nation muss existieren
  naID = NaStore(GetConnectionPtr()).NameToID(naName);

  wxString str = "INSERT INTO PlRec (plID, naID, psID, plNr, plExtID, plRankPts) "
                    "VALUES(?, ?, ?, ?, ?, ?)";

  try
  {
    plID = IdStore::ID(GetConnectionPtr());

    if (!plNr)
      plNr = GetNextNumber();

    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    stmtPtr->SetData(1, &plID);
    stmtPtr->SetData(2, naID ? &naID : (long *) NULL);
    stmtPtr->SetData(3, &psID);
    stmtPtr->SetData(4, &plNr);
    stmtPtr->SetData(5, plExtID);
    stmtPtr->SetData(6, &plRankPts);

    stmtPtr->Execute();
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    delete stmtPtr;

    return false;
  }

  delete stmtPtr;

  CRequest  req;

  req.type = CRequest::INSERT;
  req.rec  = CRequest::PLREC;
  req.id   = plID;

  CTT32App::NotifyChange(req);

  return true;
}


bool  PlStore::Update()
{
  PreparedStatement *stmtPtr = 0;

  // Update PsStore
  PsStore  ps(*this, GetConnectionPtr());
  ps.Update();
  
  naID = NaStore(GetConnectionPtr()).NameToID(naName);

  wxString str = "UPDATE PlRec "
                    "SET plNr = ?, naID = ?, plExtID = ?, plRankPts = ? "
                    "WHERE plID = ?";

  try
  {
    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    stmtPtr->SetData(1, &plNr);
    stmtPtr->SetData(2, naID ? &naID : (long *) NULL);
    stmtPtr->SetData(3, plExtID);
    stmtPtr->SetData(4, &plRankPts);
    stmtPtr->SetData(5, &plID);

    stmtPtr->Execute();
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
  update.type = CRequest::UPDATE;
  update.rec  = CRequest::PLREC;
  update.id   = plID;

  CTT32App::NotifyChange(update);

  return true;
}


bool  PlStore::Remove(long id, bool force)
{
  // ID-Check
  if (!id)
    id = plID;

  if (!id)
    return true;
  
  PlStore pl(GetConnectionPtr());
  if (plID == id)
    pl = *this;
  else
  {
    pl.SelectById(id);
    pl.Next();
    pl.Close();
  }

  if (pl.plDeleted || force)
    return PsStore(GetConnectionPtr()).Remove(pl.psID);

  // Als erstes aus allen CP austragen
  LtStore  lt(GetConnectionPtr());
  CpStore  cp(GetConnectionPtr());

  while (lt.SelectByPl(id) && lt.Next())
  {
    lt.Close();
    cp.SelectById(lt.cpID);
    cp.Next();
    cp.Close();

    if (!cp.RemovePlayer(lt))
      return false;
  }

  wxString str = "UPDATE PsRec SET psDeleteTime = GETUTCDATE() WHERE psID = " + ltostr(pl.psID);

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
  update.id   = id;

  CTT32App::NotifyChange(update);

  return true;
}


bool PlStore::Restore(long id)
{
  if (!id)
    id = plID;

  if (!id)
    return true;
  
  PlStore pl(GetConnectionPtr());
  if (plID == id)
    pl = *this;
  else
  {
    pl.SelectById(id);
    pl.Next();
    pl.Close();
  }

  wxString str = "UPDATE PsRec SET psDeleteTime = NULL WHERE psID = ?";

  PreparedStatement *stmtPtr = nullptr;

  try
  {
    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    stmtPtr->SetData(1, &pl.psID);
    stmtPtr->Execute();
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
  update.type = CRequest::UPDATE;
  update.rec  = CRequest::PLREC;
  update.id   = id;

  CTT32App::NotifyChange(update);

  return true;
}


// Check auf plExtID / plNr, ob WB existiert
bool  PlStore::InsertOrUpdate()
{
  PlStore pl(GetConnectionPtr());
  
  // First search for plNr
  if (plNr)
  {
      pl.SelectByNr(plNr);
      pl.Next();
      pl.Close();
  }

  // I none found but we have plExtID 
  // (which may not be unique, so don't look for it first)
  if (!pl.plID && *plExtID)
  {
    pl.SelectByExtId(plExtID);
    
    pl.Next();
    pl.Close();
  }

  // Looking for name, but only if we don't have an plExtId
  // (otherwise we would have found it)
  if (!pl.plID && *psName.psLast && !*plExtID)
  {
    pl.SelectByNames(psName.psLast, psName.psFirst);

    if (pl.Next())
    {
      long tmpPlID = pl.plID;
      if (pl.Next())
      {
        // Duplicate entries
        pl.Close();
        return false;
      }

      // Read again with newly found id
      pl.SelectById(tmpPlID);
      pl.Next();
      pl.Close();
    }
  }
  
  if (pl.plID)
  {
    plID = pl.plID;
    
    // psID gibt es unnoetigerweise doppelt, einmal in PsRec, einmal in PlRec.
    // Einfach rauswerfen will ich aber noch nicht.
    PlRec::psID = pl.PlRec::psID;
    PsRec::psID = pl.PsRec::psID;

    // Startnummer uebernehmen, wenn sie nicht explizit gesetzt war
    if (!plNr && pl.plNr)
      plNr = pl.plNr;

    // Dto. plExtID, we don't have it 
    if (!*plExtID && *pl.plExtID)
        wxStrcpy(plExtID, pl.plExtID);
    
    return Update();
  }
  else
  {
    return Insert();
  }
}


bool  PlStore::InsertNote(const wxString &note)
{
  PsStore ps(*this, GetConnectionPtr());

  if (!ps.InsertNote(note))
    return false;

  // Notify Views
  CRequest update;
  update.type = CRequest::UPDATE;
  update.rec  = CRequest::PLREC;
  update.id   = plID;

  CTT32App::NotifyChange(update);

  return true;
}


wxString PlStore::GetNote()
{
  PsStore ps(*this, GetConnectionPtr());
  return ps.GetNote();
}


long  PlStore::NrToID(long nr)
{
  long id = 0;
  Statement *stmtPtr;
  
  stmtPtr = GetConnectionPtr()->CreateStatement();

  wxString  sql = "SELECT plID FROM PlRec WHERE plNr = " + ltostr(nr);

  ResultSet *resPtr = stmtPtr->ExecuteQuery(sql);
  if (!resPtr || !resPtr->Next())
    id = 0;
  else if (!resPtr->GetData(1, id) || resPtr->WasNull())
    id = 0;
    
  delete resPtr;
  delete stmtPtr;

  return id;
}


// -----------------------------------------------------------------------
long  PlStore::GetHighestNumber()
{
  Statement *stmtPtr;
  ResultSet *resPtr;

  wxString  str = "SELECT MAX(plNr % 10000) FROM PlRec";

  stmtPtr = GetConnectionPtr()->CreateStatement();
  wxASSERT(stmtPtr);
  
  resPtr = stmtPtr->ExecuteQuery(str);
  wxASSERT(resPtr);

  long nr;
  if (!resPtr->Next() || !resPtr->GetData(1, nr) || resPtr->WasNull())
    nr = 0;

  delete resPtr;
  delete stmtPtr;
  
  return nr;
}


long PlStore::GetNextNumber()
{
  return GetHighestNumber() + 1;
}


// -----------------------------------------------------------------------
// SelectString
wxString  PlStore::SelectString() const
{
  return 
         " SELECT plID, plNr, PlRec.psID, "
         "        PsRec.psID, psLast, psFirst, psBirthday, psSex, psArrived, "
         "        PlRec.naID, naName, naDesc, naRegion, plExtID, plRankPts, "
         "        psDeleteTime, IIF(psDeleteTime IS NULL, 0, 1) AS plDeleted "
         " FROM PlRec INNER JOIN PsRec ON PlRec.psID = PsRec.psID "
         "            LEFT OUTER JOIN NaRec ON PlRec.naID = NaRec.naID ";
}


void  PlStore::BindRec()
{
  int idx = 0;

  BindCol(++idx, &plID);
  BindCol(++idx, &plNr);
  BindCol(++idx, &(PlRec::psID));
  BindCol(++idx, &(PsRec::psID));
  BindCol(++idx, psName.psLast, sizeof(psName.psLast));
  BindCol(++idx, psName.psFirst, sizeof(psName.psFirst));
  BindCol(++idx, &psBirthday);
  BindCol(++idx, &psSex);
  BindCol(++idx, &psArrived);
  BindCol(++idx, &naID);
  BindCol(++idx, naName, sizeof(naName));
  BindCol(++idx, naDesc, sizeof(naDesc));
  BindCol(++idx, naRegion, sizeof(naRegion));
  BindCol(++idx, plExtID, sizeof(plExtID));
  BindCol(++idx, &plRankPts);
  BindCol(++idx, &psDeleteTime);
  BindCol(++idx, &plDeleted);
}


// -----------------------------------------------------------------------
// Import / Export
// Siehe oben wg. std::ifstream
bool  PlStore::Import(wxTextBuffer &is)
{
  long version = 1;

  wxString line = is.GetFirstLine();
  if (!CheckImportHeader(line, "#PLAYERS", version))
  {
    if (!infoSystem.Question(_("First comment is not %s but \"%s\". Continue anyway?"), wxT("#PLAYERS"), line.wx_str()))
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
    
    PlStore  pl(connPtr);
    PlRecImpExp plRec;

    if (!plRec.Read(line))
      continue;

    connPtr->StartTransaction();

    ((PlRec &) pl) = plRec;

    if (pl.InsertOrUpdate())
    {
      if (!plRec.psNote.IsEmpty())
        pl.InsertNote(plRec.psNote);

      connPtr->Commit();
    }
    else
      connPtr->Rollback();
  }
  
  setlocale(LC_NUMERIC, oldLocale);

  delete connPtr;

  return true;
}


bool PlStore::Export(wxTextBuffer &os, long version)
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  PlListStore  pl(connPtr);
  if (!pl.SelectAll())
    return false;
    
  // Because rank pts are float
  const char *oldLocale = setlocale(LC_NUMERIC, "C");

  os.AddLine(wxString::Format("#PLAYERS %d", version));
  
  os.AddLine("# Pl. No.; Last Name; Given Name; Sex; Year Born; Association; Ext. ID; Ranking Pts.");
  
  while (pl.Next())
  {
    // Skip deleted players
    if (pl.plDeleted)
      continue;

    PlRecImpExp plRec;
    (PlRec &) plRec = pl;

    wxString line;
    if (plRec.Write(line))
      os.AddLine(line);
  }

  setlocale(LC_NUMERIC, oldLocale);

  return true;
}