/* Copyright (C) 2020 Christoph Theis */

// DB-Tabelle der Spieler

#include  "stdafx.h"
#include  "UpStore.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "ResultSet.h"
#include  "SQLException.h"
#include  "InfoSystem.h"
#include  "Request.h"

#include  "IdStore.h"

#include  "Rec.h"
#include  "StrUtils.h"
#include  "wxStringTokenizerEx.h"

#include  <stdio.h>
#include  <fstream>
#include  <stdlib.h>


bool UpRec::Read(const wxString &line)
{
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
  wxString strSex   = tokens.GetNextToken();
  wxString strNa    = tokens.GetNextToken().Strip(wxString::both);
  wxString strEmail = tokens.GetNextToken().Strip(wxString::both);
  wxString strPhone = tokens.GetNextToken().Strip(wxString::both);
  
  if (!strNr.IsEmpty() && *strNr.t_str() == '#')
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
      else if (strLast.IsEmpty())
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

  if (!strNr.IsEmpty() && !strLast.IsEmpty() && !strSex.IsEmpty())
  {
    Init();

    upNr       = _strtol(strNr);
    wxStrncpy((wxChar *) psName.psLast, strLast, sizeof(psName.psLast) / sizeof(wxChar) -1);
    wxStrncpy((wxChar *) psName.psFirst, strFirst, sizeof(psName.psFirst) / sizeof(wxChar) -1);
      
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

    wxStrncpy(psEmail, strEmail, sizeof(psEmail) / sizeof(wxChar) - 1);
    wxStrncpy(psPhone, strPhone, sizeof(psPhone) / sizeof(wxChar) - 1);
  }

  return true;
}


bool UpRec::Write(wxString &line) const
{
  // Nacht UTF-8 konvertieren
  line << upNr           << ";"
       << psName.psLast  << ";" 
       << psName.psFirst << ";"
       << psSex          << ";"
       << naName         << ";"    // Name der Nation
       << psEmail        << ";"
       << psPhone        << ";"
  ;

  return true;
}



// -----------------------------------------------------------------------
// Neue Tabelle in DB erzeugen
bool  UpStore::CreateTable()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  Statement *tmp = connPtr->CreateStatement();

  wxString  WVARCHAR = connPtr->GetDataType(SQL_WVARCHAR);
  wxString  INTEGER  = connPtr->GetDataType(SQL_INTEGER);
  wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);
  wxString  FLOAT    = connPtr->GetDataType(SQL_FLOAT);

  wxString  sql = 
    "CREATE TABLE UpRec (       "
    "upID        "+INTEGER+"    NOT NULL,  "
    "psID        "+INTEGER+"    NOT NULL,  "
    "naID        "+INTEGER+",   "
    "upNr        "+INTEGER+"    NOT NULL,  "
    "CONSTRAINT upIdKey PRIMARY KEY (upID) "
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
    tmp->ExecuteUpdate(sql = "CREATE UNIQUE INDEX upNrKey ON UpRec (upNr)");
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(sql, e);
  };

  delete tmp;
  
  return true;
}


bool  UpStore::UpdateTable(long version)
{
  if (version <= 122)
    return CreateTable();
    
  return true;
}


bool  UpStore::CreateConstraints()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  Statement *tmp = connPtr->CreateStatement();

  wxString  str;

  try
  {
    tmp->ExecuteUpdate("DROP TRIGGER upUpdateTrigger");
  }
  catch (SQLException &)
  {
  }
  
  try
  {
    tmp->ExecuteUpdate("ALTER TABLE UpRec DROP CONSTRAINT upPsRef");
    tmp->ExecuteUpdate("ALTER TABLE UpREc DROP CONSTRAINT upNaRef");
  }
  catch (SQLException &)
  {
  }

  try
  {
    tmp->ExecuteUpdate(str = 
        "CREATE TRIGGER upUpdateTrigger ON UpRec FOR UPDATE AS \n"
        " --- Update timestamp for last changed \n"
        "UPDATE PsRec SET psTimestamp = GETUTCDATE() \n"
        " WHERE psID IN (SELECT psID FROM deleted) \n;");
        
    tmp->ExecuteUpdate(str = 
      "ALTER TABLE UpRec ADD CONSTRAINT UpPsRef "
      "FOREIGN KEY (psID) REFERENCES PsRec (psID) ON DELETE CASCADE");

    tmp->ExecuteUpdate(str = 
      "ALTER TABLE UpRec ADD CONSTRAINT upNaRef "
      "FOREIGN KEY (naID) REFERENCES NaRec (naID) ON DELETE NO ACTION");
  }
  catch(SQLException &e)
  {
    infoSystem.Exception(str, e);
  };

  delete tmp;
  return true;
}


bool  UpStore::UpdateConstraints(long version)
{
  if (version <= 122)
    return CreateConstraints();

  return true;
}


// -----------------------------------------------------------------------
// Konstruktor
UpStore::UpStore(Connection *connPtr)
       : StoreObj(connPtr)
{
}


UpStore::~UpStore()
{
}


void  UpStore::Init()
{
  UpRec::Init();
}

// -----------------------------------------------------------------------
// Select
bool  UpStore::SelectAll()
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


bool  UpStore::SelectById(long id)
{
  wxString str = SelectString();
  str += " WHERE  UpRec.upID = ";
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


bool  UpStore::SelectByNr(long nr)
{
  wxString str = SelectString();
  str += " WHERE  UpRec.upNr = ";
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


bool UpStore::SelectByName(const wxString &name)
{
  wxString str = SelectString();
  str += " WHERE psLast + ' ' + psFirst = '" + TransformString(name) + "'";

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
bool  UpStore::Insert()
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

  wxString str = "INSERT INTO UpRec (upID, naID, psID, upNr) "
                    "VALUES(?, ?, ?, ?)";

  try
  {
    upID = IdStore::ID(GetConnectionPtr());

    if (!upNr)
      upNr = GetNextNumber();

    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    stmtPtr->SetData(1, &upID);
    stmtPtr->SetData(2, naID ? &naID : (long *) NULL);
    stmtPtr->SetData(3, &psID);
    stmtPtr->SetData(4, &upNr);

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
  req.rec  = CRequest::UPREC;
  req.id   = upID;

  CTT32App::NotifyChange(req);

  return true;
}


bool  UpStore::Update()
{
  PreparedStatement *stmtPtr = 0;

  // Update PsStore
  PsStore  ps(*this, GetConnectionPtr());
  ps.Update();
  
  naID = NaStore(GetConnectionPtr()).NameToID(naName);

  wxString str = "UPDATE UpRec "
                    "SET upNr = ?, naID = ? "
                    "WHERE upID = ?";

  try
  {
    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    stmtPtr->SetData(1, &upNr);
    stmtPtr->SetData(2, naID ? &naID : (long *) NULL);
    stmtPtr->SetData(3, &upID);

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
  update.rec  = CRequest::UPREC;
  update.id   = upID;

  CTT32App::NotifyChange(update);

  return true;
}


bool  UpStore::Remove(long id)
{
  // ID-Check
  if (!id)
    id = upID;

  if (!id)
    return true;
  
  wxString str = "DELETE FROM UpRec WHERE upID = " + ltostr(id);

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
  update.rec  = CRequest::UPREC;
  update.id   = id;

  CTT32App::NotifyChange(update);

  return true;
}


// Check auf plExtID / plNr, ob WB existiert
bool  UpStore::InsertOrUpdate()
{
  UpStore up(GetConnectionPtr());
  
  up.SelectByNr(upNr);
  up.Next();
  up.Close();
    
  if (up.upID)
  {
    upID = up.upID;
    
    // psID gibt es unnoetigerweise doppelt, einmal in PsRec, einmal in PlRec.
    // Einfach rauswerfen will ich aber noch nicht.
    UpRec::psID = up.UpRec::psID;
    PsRec::psID = up.PsRec::psID;

    // Startnummer uebernehmen, wenn sie nicht explizit gesetzt war
    if (!upNr && up.upNr)
      upNr = up.upNr;
    
    return Update();
  }
  else
  {
    return Insert();
  }
}


long  UpStore::NrToID(long nr)
{
  long id = 0;
  Statement *stmtPtr;
  
  stmtPtr = GetConnectionPtr()->CreateStatement();

  wxString  sql = "SELECT upID FROM UpRec WHERE upNr = " + ltostr(nr);

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
long  UpStore::GetHighestNumber()
{
  Statement *stmtPtr;
  ResultSet *resPtr;

  wxString  str = "SELECT MAX(upNr) FROM UpRec";

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


long UpStore::GetNextNumber()
{
  return GetHighestNumber() + 1;
}


// -----------------------------------------------------------------------
// SelectString
wxString  UpStore::SelectString() const
{
  return 
         " SELECT upID, upNr, UpRec.psID, "
         "        PsRec.psID, psLast, psFirst, psSex, "
         "        UpRec.naID, naName, naDesc, naRegion, "
         "        psEmail, psPhone"
         " FROM UpRec INNER JOIN PsRec ON UpRec.psID = PsRec.psID "
         "            LEFT OUTER JOIN NaRec ON UpRec.naID = NaRec.naID ";
}


void  UpStore::BindRec()
{
  BindCol(1, &upID);
  BindCol(2, &upNr);
  BindCol(3, &(UpRec::psID));
  BindCol(4, &(PsRec::psID));
  BindCol(5, psName.psLast, sizeof(psName.psLast));
  BindCol(6, psName.psFirst, sizeof(psName.psFirst));
  BindCol(7, &psSex);
  BindCol(8, &naID);
  BindCol(9, naName, sizeof(naName));
  BindCol(10, naDesc, sizeof(naDesc));
  BindCol(11, naRegion, sizeof(naRegion));
  BindCol(12, psEmail, sizeof(psEmail));
  BindCol(13, psPhone, sizeof(psPhone));
}


// -----------------------------------------------------------------------
// Import / Export
// Siehe oben wg. std::ifstream
bool  UpStore::Import(wxTextBuffer &is)
{
  long version = 1;

  wxString line = is.GetFirstLine();

  // Check header
  if (!CheckImportHeader(line, "#UMPIRES", version))
  {
    if (!infoSystem.Question(_("First comment is not %s but \"%s\". Continue anyway?"), wxT("#UMPIRES"), line.wx_str()))
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
    
    UpStore  up(connPtr);

    if (!up.Read(line))
      continue;

    connPtr->StartTransaction();

    if (up.InsertOrUpdate())
      connPtr->Commit();
    else
      connPtr->Rollback();
  }
  
  setlocale(LC_NUMERIC, oldLocale);

  delete connPtr;

  return true;
}


bool UpStore::Export(wxTextBuffer &os, long version)
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();

  UpStore  up(connPtr);
  if (!up.SelectAll())
    return false;
    
  os.AddLine(wxString::Format("#UMPIRES %d", version));
  
  os.AddLine("# Up. No.; Last Name; Given Name; Sex; Association; Email; Phone");
  
  while (up.Next())
  {
    wxString line;
    if (up.Write(line))
      os.AddLine(line);
  }
  
  return true;
}