/* Copyright (C) 2020 Christoph Theis */

// Persistenz von PrintRasterOptions

#include "stdafx.h"

#include "PoStore.h"

#include  "TT32App.h"
#include  "TTDbse.h"
#include  "Statement.h"
#include  "ResultSet.h"
#include  "SQLException.h"

#include  "IdStore.h"

#include  "InfoSystem.h"
#include  "Profile.h"
#include  "Res.h"
#include  "wx/jsonval.h"
#include  "wx/jsonreader.h"
#include  "wx/jsonwriter.h"

#include  <list>

static wxString DEFAULT_SECTION = "__default__";

bool  PoStore::CreateTable()
{
  Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
  wxASSERT(connPtr);

  Statement *tmp = connPtr->CreateStatement();

  wxString  INTEGER = connPtr->GetDataType(SQL_INTEGER);
  wxString  SMALLINT = connPtr->GetDataType(SQL_SMALLINT);
  wxString  WVARCHAR = connPtr->GetDataType(SQL_WVARCHAR);

  wxString  sql =
    "CREATE TABLE PoRec ( "
    "poID        " + INTEGER  + "       NOT NULL,     "
    "poName      " + WVARCHAR + "(64)   NOT NULL,     "
    "poJSON      " + WVARCHAR + "(MAX)  DEFAULT NULL, "

    "CONSTRAINT poIdKey PRIMARY KEY (poID)   "
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
    tmp->ExecuteUpdate(sql = "CREATE UNIQUE INDEX poNameKey ON PoRec (poName)");
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(sql, e);
  };

  tmp->Close();
  delete tmp;

  return true;
}


bool  PoStore::UpdateTable(long version)
{
  if (version <= 134)
    return CreateTable();
  
  if (version < 151)
  {
    Connection *connPtr = TTDbse::instance()->GetDefaultConnection();
    Statement *stmtPtr = connPtr->CreateStatement();
    ResultSet *resPtr = NULL;

    wxString  WVARCHAR  = connPtr->GetDataType(SQL_WVARCHAR);
    
    wxString sql;
    try
    {
      sql = "ALTER TABLE PoRec ADD "
              "poJSON " + WVARCHAR + "(MAX) DEFAULT NULL";

      stmtPtr->ExecuteUpdate(sql);

      // Convert existing rows to JSON format
      wxString sql = "SELECT * FROM PoRec";
      ResultSet *resPtr = stmtPtr->ExecuteQuery(sql);

      std::vector<std::tuple<long, wxString, wxString>> list;

      while (resPtr->Next())
      {
        long id = 0;
        wxString name;
        int idx = 1, count = resPtr->GetColumnCount();
        wxJSONValue json(wxJSONTYPE_OBJECT);

        while (idx < count)
        {
          wxString col = resPtr->GetColumnLabel(idx);
          if (col == "poID")
          {
            resPtr->GetData(idx, id);
          }
          else if (col == "poName")
          {
            wxChar tmp[256];
            resPtr->GetData(idx, tmp, 255);
            name = tmp;
          }
          else
          {
            short val;
            resPtr->GetData(idx, val);
            json[col] = val;
          }

          ++idx;
        }

        wxJSONWriter writer(wxJSONWRITER_NONE);
        wxString tmp; 
        writer.Write(json, tmp);

        list.push_back({id, name, tmp});
      }

      // We got all, re-create the table
      delete resPtr;
      sql = "DROP TABLE PoRec";
      stmtPtr->Execute(sql);

      CreateTable();

      for (auto it : list)
      {
        sql = "INSERT INTO PoRec (poID, poName, poJSON) VALUES(" + ltostr(std::get<0>(it)) + ", '" + std::get<1>(it) + "', '" + std::get<2>(it) + "')";
        stmtPtr->ExecuteUpdate(sql);
      }
    }
    catch (SQLException & ex)
    {
      infoSystem.Exception(sql, ex);

      delete stmtPtr;
      delete resPtr;

      return false;
    }
  }

  return true;
}


// -----------------------------------------------------------------------
PoStore::PoStore(Connection * connPtr) : StoreObj(connPtr)
{
}


PoStore::PoStore(const PrintRasterOptions &options, Connection * connPtr) : StoreObj(connPtr), PoRec(options)
{
}


PoStore::~PoStore()
{
}


void PoStore::Init()
{
  PoRec::Init();
}

// -----------------------------------------------------------------------
bool PoStore::Read(const wxString &name)
{
  Init();

  // Wenn profile und section existieren, dann dort lesen, sonst aus DB
  wxString fileName = CTT32App::instance()->GetPath() + wxFileName::GetPathSeparator() + "PrintOptions.ini";
  wxString section = name.IsEmpty() ? DEFAULT_SECTION : name;

  if (!name.IsEmpty() && ReadFromDatabase(name))
    return true;

  if (!wxFile::Exists(fileName))
    return false;

  Profile profile(fileName);

  return ReadFromProfile(profile, section);
}


bool PoStore::Write(const wxString &name, bool isPrivate)
{
  if (!name.IsEmpty() && NameToID(name) > 0)
    return WriteToDatabse(name);
    
  wxStrncpy(poName, name, sizeof(poName) / sizeof(wxChar));

  wxString fileName = CTT32App::instance()->GetPath() + wxFileName::GetPathSeparator() + "PrintOptions.ini";
  wxString section = name.IsEmpty() ? DEFAULT_SECTION : name;

  Profile profile(fileName);

  return WriteToProfile(profile, section);
}


bool PoStore::Delete(const wxString &name)
{
  // <default> kann nicht geloescht werden
  if (name.IsEmpty())
    return false;

  wxString fileName = CTT32App::instance()->GetPath() + wxFileName::GetPathSeparator() + "PrintOptions.ini";
  Profile profile(fileName);

  // Reihenfolge von OR ist Absicht, beide Statements sollen ausgefuehrt werden
  bool ret = DeleteFromDatabase(name);
  ret = DeleteFromProfile(profile, name) || ret;

  return ret;
}


std::list<wxString> PoStore::List() const
{
  std::list<wxString> list;

  wxString fileName = CTT32App::instance()->GetPath() + wxFileName::GetPathSeparator() + "PrintOptions.ini";
  
  Profile profile(fileName);

  // Erster Eintrag ist leer (default)
  list.push_back(wxEmptyString);

  // Dann die Datenbank
  PoStore po;
  po.SelectAll();
  while (po.Next())
    list.push_back(po.poName);
  po.Close();

  list.sort();

  for (wxString section = profile.GetFirstSection(); !section.IsEmpty(); section = profile.GetNextSection())
  {
    if (section == DEFAULT_SECTION)
      continue;

    list.push_back(section);
  }

  list.sort();
  list.unique();

  return list;
}


bool PoStore::Exists(const wxString &name)
{
  if (name.IsEmpty())
    return true;

  if (NameToID(name) > 0)
    return true;

  wxString fileName = CTT32App::instance()->GetPath() + wxFileName::GetPathSeparator() + "PrintOptions.ini";
  wxString section = name.IsEmpty() ? DEFAULT_SECTION : name;

  Profile profile(fileName);

  return !profile.GetFirstKey(section).IsEmpty();
}


bool PoStore::Publish(const wxString &name)
{
  wxString fileName = CTT32App::instance()->GetPath() + wxFileName::GetPathSeparator() + "PrintOptions.ini";

  if (name.IsEmpty())
    return false;

  Profile profile(fileName);

  if (!ReadFromProfile(profile, name))
    return false;

  if (!WriteToDatabse(name))
    return false;

  DeleteFromProfile(profile, name);

  return true;
}


// -----------------------------------------------------------------------
bool PoStore::ReadFromProfile(Profile &profile, const wxString &section)
{
  if (profile.GetFirstKey(section).IsEmpty())
    return false;

  poID = 0;
  wxStrncpy(poName, section == DEFAULT_SECTION ? "" : section, sizeof(poName) / sizeof(wxChar));

  rrResults = profile.GetBool(section, PRF_PROPTIONS_RRRESULTS, 0);
  rrSignature = profile.GetBool(section, PRF_PROPTIONS_RRSIGNATURE, 0);
  rrTeamDetails = profile.GetBool(section, PRF_PROPTIONS_RRTMDETAILS, 0);
  rrIgnoreByes = profile.GetBool(section, PRF_PROPTIONS_RRIGNOREBYES, 0);
  rrNewPage = profile.GetBool(section, PRF_PROPTIONS_RRNEWPAGE, 0);
  rrSlctRound = profile.GetBool(section, PRF_PROPTIONS_RRSLCTRD, 0);
  rrFromRound = profile.GetInt(section, PRF_PROPTIONS_RRFROMRD, 0);
  rrToRound = profile.GetInt(section, PRF_PROPTIONS_RRTORD, 0);
  rrSchedule = profile.GetBool(section, PRF_PROPTIONS_RRSCHEDULE, 0);
  rrCombined = profile.GetBool(section, PRF_PROPTIONS_RRCOMBINED, 0);
  rrConsolation = profile.GetBool(section, PRF_PROPTIONS_RRCONSOLATION, 0);
  rrLastResults = profile.GetBool(section, PRF_PROPTIONS_RRLASTRESULTS, 0);
  rrPrintNotes = profile.GetBool(section, PRF_PROPTIONS_RRPRINTNOTES, 0);
  koSlctRound = profile.GetBool(section, PRF_PROPTIONS_KOSLCTRD, 0);
  koNoQuRounds = profile.GetBool(section, PRF_PROPTIONS_KONOQUROUNDS, 0);
  koFromRound = profile.GetInt(section, PRF_PROPTIONS_KOFROMRD, 0);
  koToRound = profile.GetInt(section, PRF_PROPTIONS_KOTORD, 0);
  koLastRounds = profile.GetBool(section, PRF_PROPTIONS_KOLASTRD, 0);
  koSlctMatch = profile.GetBool(section, PRF_PROPTIONS_KOSLCTMT, 0);
  koFromMatch = profile.GetInt(section, PRF_PROPTIONS_KOFROMMT, 0);
  koToMatch = profile.GetInt(section, PRF_PROPTIONS_KOTOMT, 0);
  koLastMatches = profile.GetBool(section, PRF_PROPTIONS_KOLASTMT, 0);
  koNr = profile.GetBool(section, PRF_PROPTIONS_KONR, 0);
  koTeamDetails = profile.GetBool(section, PRF_PROPTIONS_KOTMDETAILS, 0);
  koIgnoreByes = profile.GetBool(section, PRF_PROPTIONS_KOIGNOREBYES, 0);
  koNewPage = profile.GetBool(section, PRF_PROPTIONS_KONEWPAGE, 0);
  koLastResults = profile.GetBool(section, PRF_PROPTIONS_RRLASTRESULTS, 0);
  koInbox = profile.GetBool(section, PRF_PROPTIONS_KOINBOX, 0);
  koPrintPosNrs = profile.GetBool(section, PRF_PROPTIONS_KOPRINTPOSNRS, 0);
  koPrintNotes = profile.GetBool(section, PRF_PROPTIONS_KOPRINTNOTES, 0);

  return true;
}

bool PoStore::WriteToProfile(Profile &profile, const wxString &section) const
{
  profile.AddBool(section, PRF_PROPTIONS_RRRESULTS, rrResults);
  profile.AddBool(section, PRF_PROPTIONS_RRSIGNATURE, rrSignature);
  profile.AddBool(section, PRF_PROPTIONS_RRTMDETAILS, rrTeamDetails);
  profile.AddBool(section, PRF_PROPTIONS_RRIGNOREBYES, rrIgnoreByes);
  profile.AddBool(section, PRF_PROPTIONS_RRNEWPAGE, rrNewPage);
  profile.AddBool(section, PRF_PROPTIONS_RRSLCTRD, rrSlctRound);
  profile.AddInt(section, PRF_PROPTIONS_RRFROMRD, rrFromRound);
  profile.AddInt(section, PRF_PROPTIONS_RRTORD, rrToRound);
  profile.AddBool(section, PRF_PROPTIONS_RRSCHEDULE, rrSchedule);
  profile.AddBool(section, PRF_PROPTIONS_RRCOMBINED, rrCombined);
  profile.AddBool(section, PRF_PROPTIONS_RRCONSOLATION, rrConsolation);
  profile.AddBool(section, PRF_PROPTIONS_RRLASTRESULTS, rrLastResults);
  profile.AddBool(section, PRF_PROPTIONS_RRPRINTNOTES, rrPrintNotes);
  profile.AddBool(section, PRF_PROPTIONS_KOSLCTRD, koSlctRound);
  profile.AddBool(section, PRF_PROPTIONS_KONOQUROUNDS, koNoQuRounds);
  profile.AddInt(section, PRF_PROPTIONS_KOFROMRD, koFromRound);
  profile.AddInt(section, PRF_PROPTIONS_KOTORD, koToRound);
  profile.AddBool(section, PRF_PROPTIONS_KOLASTRD, koLastRounds);
  profile.AddBool(section, PRF_PROPTIONS_KOSLCTMT, koSlctMatch);
  profile.AddInt(section, PRF_PROPTIONS_KOFROMMT, koFromMatch);
  profile.AddBool(section, PRF_PROPTIONS_KOLASTMT, koLastMatches);
  profile.AddInt(section, PRF_PROPTIONS_KOTOMT, koToMatch);
  profile.AddBool(section, PRF_PROPTIONS_KONR, koNr);
  profile.AddBool(section, PRF_PROPTIONS_KOTMDETAILS, koTeamDetails);
  profile.AddBool(section, PRF_PROPTIONS_KOIGNOREBYES, koIgnoreByes);
  profile.AddBool(section, PRF_PROPTIONS_KONEWPAGE, koNewPage);
  profile.AddBool(section, PRF_PROPTIONS_KOLASTRESULTS, koLastResults);
  profile.AddBool(section, PRF_PROPTIONS_KOINBOX, koInbox);
  profile.AddBool(section, PRF_PROPTIONS_KOPRINTPOSNRS, koPrintPosNrs);
  profile.AddBool(section, PRF_PROPTIONS_KOPRINTNOTES, koPrintNotes);

  return true;
}


bool  PoStore::DeleteFromProfile(Profile &profile, const wxString &section)
{
  if (profile.GetFirstKey(section).IsEmpty())
    return false;

  profile.DeleteSection(section);

  return true;
}


// -----------------------------------------------------------------------
bool PoStore::ReadFromDatabase(const wxString &name)
{
  bool ret = SelectByName(name) && Next();
  Close();

  return ret;
}


bool PoStore::WriteToDatabse(const wxString &name)
{
  wxStrncpy(poName, name.c_str(), sizeof(poName) / sizeof(wxChar));

  return InsertOrUpdate();
}


bool PoStore::DeleteFromDatabase(const wxString &name)
{
  long id = NameToID(name);
  return Remove(id);
}


// -----------------------------------------------------------------------
// Select
bool  PoStore::SelectAll()
{
  wxString str = SelectString();

  try
  {
    if (!ExecuteQuery(str))
      return false;
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
  }

  return true;
}


bool  PoStore::SelectById(long id)
{
  wxString str = SelectString();
  str += " WHERE poID = ";
  str += ltostr(id);
  str += "";

  try
  {
    if (!ExecuteQuery(str))
      return false;

    return true;
  }
  catch (SQLException &e)
  {
    infoSystem.Exception(str, e);
    return false;
  }

  return true;
}


bool  PoStore::SelectByName(const wxString &name)
{
  wxString str = SelectString();
  str += " WHERE  poName = '" + TransformString(name) + "'";

  try
  {
    if (!ExecuteQuery(str))
      return false;

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
bool  PoStore::Insert()
{
  PreparedStatement *stmtPtr = 0;

  wxString str = "INSERT INTO PoRec (poID, poName, poJSON) VALUES(?, ?, ?)";

  try
  {
    poID = IdStore::ID(GetConnectionPtr());

    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    wxString str;
    WriteToJson(str);
    int idx = 0;

    stmtPtr->SetData(++idx, &poID);
    stmtPtr->SetData(++idx, poName);
    stmtPtr->SetData(++idx, (wxChar *) str.wx_str());

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


bool  PoStore::Update()
{
  PreparedStatement *stmtPtr = 0;


  wxString str = "UPDATE PoRec SET poName = ?, poJSON = ? WHERE poID = ?";

  try
  {
    stmtPtr = GetConnectionPtr()->PrepareStatement(str);
    if (!stmtPtr)
      return false;

    wxString str;

    WriteToJson(str);

    int idx = 0;

    stmtPtr->SetData(++idx, poName);
    stmtPtr->SetData(++idx, (wxChar *) str.wx_str());

    stmtPtr->SetData(++idx, &poID);

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


bool  PoStore::Remove(long id)
{
  // ID-Check
  if (!id)
    id = poID;

  if (!id)
    return true;

  wxString str = "DELETE FROM PoRec WHERE poID = " + ltostr(id);

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


// Check auf plExtID / plNr, ob WB existiert
bool  PoStore::InsertOrUpdate()
{
  poID = NameToID(poName);

  if (poID)
    return Update();
  else
    return Insert();
}


// -----------------------------------------------------------------------
long  PoStore::NameToID(const wxString &name)
{
  long id = 0;
  Statement *stmtPtr;

  stmtPtr = GetConnectionPtr()->CreateStatement();

  wxString  sql = "SELECT poID FROM PoRec WHERE poName = '" + TransformString(name) + "'";

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
wxString PoStore::SelectString() const
{
  return "SELECT poID, poName, poJSON FROM PoRec";
}


bool  PoStore::Next()
{
  if (!StoreObj::Next())
    return false;

  int idx = 0;
  wxChar str[4096];

  GetData(++idx, poID);
  GetData(++idx, poName, sizeof(poName));
  GetData(++idx, str, sizeof(str));

  ReadFromJson(wxString(str));

  return true;
}


bool PoStore::ReadFromJson(const wxString &str)
{
  wxJSONValue json;
  wxJSONReader reader;
  reader.Parse(str, &json);

  // RR Gruppen
  rrResults = GetShort(json, "rrResults");
  rrSignature = GetShort(json, "rrSignature");
  rrTeamDetails = GetShort(json, "rrTeamDetails");
  rrIgnoreByes = GetShort(json, "rrIgnoreByes");
  rrSchedule = GetShort(json, "rrSchedule");
  rrNewPage = GetShort(json, "rrNewPage");
  rrSlctRound = GetShort(json, "rrSlctRound");
  rrFromRound = GetShort(json, "rrFromRound");
  rrToRound = GetShort(json, "rrToRound");
  rrCombined = GetShort(json, "rrCombined");
  rrConsolation = GetShort(json, "rrConsolation");
  rrLastResults = GetShort(json, "rrLastResults");
  rrPrintNotes = GetShort(json, "rrPrintNotes");

  // KO Gruppen
  koSlctRound = GetShort(json, "koSlctRound");
  koFromRound = GetShort(json, "koFromRound");
  koToRound = GetShort(json, "koToRound");
  koLastRounds = GetShort(json, "koLastRounds");
  koNoQuRounds = GetShort(json, "koNoQuRounds");
  koSlctMatch = GetShort(json, "koSlctMatch");
  koFromMatch = GetShort(json, "koFromMatch");
  koToMatch = GetShort(json, "koToMatch");
  koLastMatches = GetShort(json, "koLastMatches");
  koNr = GetShort(json, "koNr");
  koTeamDetails = GetShort(json, "koTeamDetails");
  koIgnoreByes = GetShort(json, "koIgnoreByes");
  koNewPage = GetShort(json, "koNewPage");
  koLastResults = GetShort(json, "koLastResults");
  koInbox = GetShort(json, "koInbox");
  koPrintPosNrs = GetShort(json, "koPrintPosNrs");
  koPrintNotes = GetShort(json, "koPrintNotes");

  return true;
}

bool PoStore::WriteToJson(wxString &str) const
{
  wxJSONValue json;
  wxJSONWriter writer(wxJSONWRITER_NONE);

  // RR Gruppen
  json["rrResults"] = rrResults;
  json["rrSignature"] = rrSignature;
  json["rrTeamDetails"] = rrTeamDetails;
  json["rrIgnoreByes"] = rrIgnoreByes;
  json["rrSchedule"] = rrSchedule;
  json["rrNewPage"] = rrNewPage;
  json["rrSlctRound"] = rrSlctRound;
  json["rrFromRound"] = rrFromRound;
  json["rrToRound"] = rrToRound;
  json["rrCombined"] = rrCombined;
  json["rrConsolation"] = rrConsolation;
  json["rrLastResults"] = rrLastResults;
  json["rrPrintNotes"] = rrPrintNotes;

  // KO Gruppen
  json["koSlctRound"] = koSlctRound;
  json["koFromRound"] = koFromRound;
  json["koToRound"] = koToRound;
  json["koLastRounds"] = koLastRounds;
  json["koNoQuRounds"] = koNoQuRounds;
  json["koSlctMatch"] = koSlctMatch;
  json["koFromMatch"] = koFromMatch;
  json["koToMatch"] = koToMatch;
  json["koLastMatches"] = koLastMatches;
  json["koNr"] = koNr;
  json["koTeamDetails"] = koTeamDetails;
  json["koIgnoreByes"] = koIgnoreByes;
  json["koNewPage"] = koNewPage;
  json["koLastResults"] = koLastResults;
  json["koInbox"] = koInbox;
  json["koPrintPosNrs"] = koPrintPosNrs;
  json["koPrintNotes"] = koPrintNotes;

  writer.Write(json, str);

  return true;
}


short PoStore::GetShort(wxJSONValue &json, const wxString &key)
{
  if (!json.HasMember(key))
    return 0;
  if (json[key].IsBool())
    return json[key].AsBool() ? 1 : 0;
  else
    return json[key].AsShort();
}


