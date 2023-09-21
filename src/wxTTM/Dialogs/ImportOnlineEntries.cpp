/* Copyright (C) 2020 Christoph Theis */


#include "stdafx.h"

#include "ImportOnlineEntries.h"

#include "StListStore.h"
#include "PlStore.h"
#include "UpStore.h"
#include "NaStore.h"
#include "CpStore.h"
#include "RpStore.h"
#include "LtStore.h"
#include "TmStore.h"

#include "ComboBoxEx.h"
#include "ListItem.h"

#include "InfoSystem.h"
#include "TT32App.h"

#include "StrUtils.h"

#include <map>
#include <set>
#include <vector>
#include <utility>
#include <algorithm>

IMPLEMENT_DYNAMIC_CLASS(CImportOnlineEntries, wxWizard)

BEGIN_EVENT_TABLE(CImportOnlineEntries, wxWizard)
  EVT_WIZARD_PAGE_CHANGING(wxID_ANY, CImportOnlineEntries::OnPageChanging)
  EVT_WIZARD_PAGE_SHOWN(wxID_ANY, CImportOnlineEntries::OnPageShown)

  EVT_TEXT(XRCID("Url"), CImportOnlineEntries::OnTextUrl)
  EVT_BUTTON(XRCID("SelectDirectory"), CImportOnlineEntries::OnSelectDirectory)

  EVT_CHECKBOX(XRCID("ImportEntries"), CImportOnlineEntries::OnChangeImportEntries)
END_EVENT_TABLE()

static int GetInt(XmlRpcValue &val)
{
  switch (val.getType())
  {
    case XmlRpcValue::Type::TypeInt :
      return (int) val;
    case XmlRpcValue::Type::TypeString :
      return atoi(val);
    default :
      return 0;
  }
}


static int GetDouble(XmlRpcValue &val)
{
  switch (val.getType())
  {
    case XmlRpcValue::Type::TypeInt :
      return (double) val;
    case XmlRpcValue::Type::TypeString :
      return atof(val);
    case XmlRpcValue::Type::TypeDouble :
      return val;
    default :
      return 0;
  }
}


class TournamentItem : public ListItem
{
  public:
    TournamentItem(int id, const char *name_, const char *description_) : ListItem(id, description_), name(name_) {}

  public:
    wxString name;
};


struct Competition
{
  int id = 0;
  wxString name;
  wxString description;
  wxString category;
  wxString type;
  wxString sex;
  int born = 0;
};


struct Nation
{
  int id = 0;
  wxString name;
  wxString description;
};


struct Person {
  int id = 0;
  wxString firstName;
  wxString lastName;
  wxString displayName;
  wxString sex;
  int      naID = 0;
  int      born = 0;
  wxString externID;
  double   rankPts = 0.;
  int      startNr = 0;
  wxString naName;
  wxString phone;
  wxString comment;

  bool operator<(const Person &b);
};

struct Club
{
  int id = 0;
  int naID = 0;
  int cpID = 0;
  wxString name;
  wxString description;
  std::list<Person> players;
};

bool Person::operator<(const Person &b)
{
  const Person &a = *this;

  if (a.startNr != b.startNr)
    return a.startNr < b.startNr;

  if (a.naName != b.naName)
    return a.naName < b.naName;

  if (a.sex != b.sex)
    return a.sex == "M";

  if (a.lastName != b.lastName)
    return a.lastName < b.lastName;

  return a.firstName < b.firstName;
}


struct Registration {
  int id;
  int playerID;
  int singleID;
  int doubleID;
  int doublePartnerID;
  int mixedID;
  int mixedPartnerID;
  int teamID;
  int teamNo;
  bool cancelled;
  bool singleCancelled;
  bool doubleCancelled;
  bool mixedCancelled;
  bool teamCancelled;
};
                


// -----------------------------------------------------------------------
bool CImportOnlineEntries::Import()
{
  wxWizard *wizard = (wxWizard *) wxXmlResource::Get()->LoadObject(NULL, "ImportOnlineEntries", "wxWizard");
  if (!wizard)
  {
    return false;
  }

  if (StListStore().Count() > 0)
  {
    XRCCTRL(*wizard, "ClearTournament", wxCheckBox)->SetValue(false);
    XRCCTRL(*wizard, "ClearTournament", wxCheckBox)->Enable(false);

    XRCCTRL(*wizard, "ImportCompetitions", wxCheckBox)->SetValue(false);
    XRCCTRL(*wizard, "ImportCompetitions", wxCheckBox)->Enable(false);

    XRCCTRL(*wizard, "ImportAssociations", wxCheckBox)->SetValue(false);
    XRCCTRL(*wizard, "ImportAssociations", wxCheckBox)->Enable(false);

    XRCCTRL(*wizard, "ImportPlayers", wxCheckBox)->SetValue(false);
    XRCCTRL(*wizard, "ImportPlayers", wxCheckBox)->Enable(false);

    XRCCTRL(*wizard, "ImportRanking", wxCheckBox)->SetValue(false);
    XRCCTRL(*wizard, "ImportRanking", wxCheckBox)->Enable(false);
    // But keep it enabled

    XRCCTRL(*wizard, "ImportEntries", wxCheckBox)->SetValue(false);
    XRCCTRL(*wizard, "ImportEntries", wxCheckBox)->Enable(false);
    XRCCTRL(*wizard, "ImportEntriesSingles", wxCheckBox)->SetValue(false);
    XRCCTRL(*wizard, "ImportEntriesSingles", wxCheckBox)->Enable(false);
    XRCCTRL(*wizard, "ImportEntriesDoubles", wxCheckBox)->SetValue(false);
    XRCCTRL(*wizard, "ImportEntriesDoubles", wxCheckBox)->Enable(false);
    XRCCTRL(*wizard, "ImportEntriesMixed", wxCheckBox)->SetValue(false);
    XRCCTRL(*wizard, "ImportEntriesMixed", wxCheckBox)->Enable(false);
    XRCCTRL(*wizard, "ImportEntriesTeams", wxCheckBox)->SetValue(false);
    XRCCTRL(*wizard, "ImportEntriesTeams", wxCheckBox)->Enable(false);
    XRCCTRL(*wizard, "ImportEntriesClubs", wxCheckBox)->SetValue(false);
    XRCCTRL(*wizard, "ImportEntriesClubs", wxCheckBox)->Enable(false);
  }

  wxSize minSize = wizard->GetSize();
  // Irgendwie ist der Wizard zu schmal, was oben berechnet wird.
  // Ich haette ihn gerne etwas groesser, aber GetSize ignoriert, was ich im Editor aufziehe
  minSize += wxSize(minSize.GetWidth() / 3, 0);

  wxWizardPage *firstPage = XRCCTRL(*wizard, "ImportOnlineEntriesConnect", wxWizardPage);
  if (!firstPage)
  {
    return false;
  }

  wizard->GetPageAreaSizer()->Add(firstPage);
  wizard->GetPageAreaSizer()->SetMinSize(minSize);

  wizard->RunWizard(firstPage);

  wizard->Destroy();

  return true;
}


// -----------------------------------------------------------------------
CImportOnlineEntries::CImportOnlineEntries()
{
  inThread = false;
}


CImportOnlineEntries::~CImportOnlineEntries()
{
}


// -----------------------------------------------------------------------
void CImportOnlineEntries::OnPageChanging(wxWizardEvent &evt)
{
  wxWindow *currentPage = GetCurrentPage();
  if (currentPage->GetId() == XRCID("ImportOnlineEntriesConnect"))
    OnPageChangingConnect(evt);
  else if (currentPage->GetId() == XRCID("ImportOnlineEntriesSelect"))
    OnPageChangingSelect(evt);
  else if (currentPage->GetId() == XRCID("ImportOnlineEntriesImport"))
    OnPageChangingImport(evt);
}


struct ConnectParams
{
  CImportOnlineEntries *dlg;
  wxString url;
  wxString user;
  wxString pwd;
};


unsigned CImportOnlineEntries::ConnectThread(void *args)
{
  ConnectParams *params = (ConnectParams *) args;

  CImportOnlineEntries *dlg = params->dlg;

  dlg->ConnectThreadImpl(params->url, params->user, params->pwd);

  wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, wxID_FORWARD);
  evt.SetEventObject(dlg->FindWindow(wxID_FORWARD));
  dlg->GetEventHandler()->AddPendingEvent(evt);

  delete params;

  return 0;
}


void CImportOnlineEntries::ConnectThreadImpl(const wxString &url, const wxString &user, const wxString &pwd)
{
  if (!xmlRpcClient.connect(url.ToStdString().c_str(), user.IsEmpty() ? NULL : user.ToStdString().c_str(), pwd.IsEmpty() ? NULL : pwd.ToStdString().c_str()))
  {
    return;
  }

  XmlRpcValue result;

  bool res = xmlRpcClient.execute("onlineentries.listTournaments", XmlRpcValue(), result);

  if (!res)
  {
    wxString error = xmlRpcClient.getError();
    const wxChar *tmp = error.t_str();
    infoSystem.Error(_("Could not read list of tournaments: \n%s"), tmp);
    return;
  }

  if (result.getType() != XmlRpcValue::Type::TypeArray)
  {
    infoSystem.Error(_("Could not read list of tournaments"));
    return;
  }

  CComboBoxEx *tournaments = XRCCTRL(*this, "Tournaments", CComboBoxEx);
  tournaments->Clear();

  for (int idx = 0; idx < result.size(); idx++)
  {
    XmlRpcValue t;

    if (result[idx]["Tournament"].getType() == XmlRpcValue::Type::TypeStruct)
      t = result[idx]["Tournament"];
    else
      t = result[idx];

    tournaments->AddListItem(new TournamentItem(t["id"], t["name"], t["description"]));
  }

  if (tournaments->GetCount() == 0)
  {
    infoSystem.Information(_("No tournaments found"));
    return;
  }

  tournaments->SetSelection(0);
}


void CImportOnlineEntries::OnPageChangingConnect(wxWizardEvent &evt)
{
  // Nur wenn wir vorwaerts gehen
  if ( !evt.GetDirection() )
    return;

  if (inThread)
  {
    // Seitenwechsel wurde vom Code veranlasst
    inThread = false;
    return;
  }

  // Immer veto und per Code die Seite wechseln
  evt.Veto();

  inThread = true;

  if (XRCCTRL(*this, "Directory", wxTextCtrl)->GetValue().IsEmpty())
    XRCCTRL(*this, "Directory", wxTextCtrl)->SetValue(CTT32App::instance()->GetPath());

  ConnectParams *params = new ConnectParams;

  params->url = XRCCTRL(*this, "Url", wxTextCtrl)->GetValue();
  params->user = XRCCTRL(*this, "User", wxTextCtrl)->GetValue();
  params->pwd = XRCCTRL(*this, "Password", wxTextCtrl)->GetValue();

  params->dlg = this;

  CTT32App::ProgressBarThread(CImportOnlineEntries::ConnectThread, params, _("Reading List of Tournaments"), 0, true);
}


void CImportOnlineEntries::OnPageChangingSelect(wxWizardEvent &evt)
{
  if ( !evt.GetDirection() )
    return;

  wxString path = XRCCTRL(*this, "Directory", wxTextCtrl)->GetValue();

  if (!path.IsEmpty())
  {
    if ( wxFileExists(wxFileName(path, "cp.csv").GetFullPath()) ||
         wxFileExists(wxFileName(path, "na.csv").GetFullPath()) ||
         wxFileExists(wxFileName(path, "pl.csv").GetFullPath()) ||
         wxFileExists(wxFileName(path, "ph.csv").GetFullPath()) ||
         wxFileExists(wxFileName(path, "lts.csv").GetFullPath()) ||
         wxFileExists(wxFileName(path, "ltd.csv").GetFullPath()) ||
         wxFileExists(wxFileName(path, "ltx.csv").GetFullPath()) ||
         wxFileExists(wxFileName(path, "ltt.csv").GetFullPath()) ||
         wxFileExists(wxFileName(path, "ltc.csv").GetFullPath()) )
    {
      if ( !infoSystem.Confirmation(_("Overwrite exsting import files?")) )
      {
        evt.Veto();
        return;
      }
    }
  }

  if (!path.IsEmpty())
  {
    cpFileName = wxFileName(path, "cp.csv").GetFullPath();
    naFileName = wxFileName(path, "na.csv").GetFullPath();
    plFileName = wxFileName(path, "pl.csv").GetFullPath();
    phFileName = wxFileName(path, "ph.csv").GetFullPath();
    rpFileName = wxFileName(path, "rp.csv").GetFullPath();
    ltsFileName = wxFileName(path, "lts.csv").GetFullPath();
    ltdFileName = wxFileName(path, "ltd.csv").GetFullPath();
    ltxFileName = wxFileName(path, "ltx.csv").GetFullPath();
    lttFileName = wxFileName(path, "ltt.csv").GetFullPath();
    ltcFileName = wxFileName(path, "ltc.csv").GetFullPath();
  }
  else
  {
    cpFileName = wxFileName::CreateTempFileName("cp");
    naFileName = wxFileName::CreateTempFileName("na");
    plFileName = wxFileName::CreateTempFileName("pl");
    phFileName = wxFileName::CreateTempFileName("ph");
    rpFileName = wxFileName::CreateTempFileName("rp");
    ltsFileName = wxFileName::CreateTempFileName("lts");
    ltdFileName = wxFileName::CreateTempFileName("ltd");
    ltxFileName = wxFileName::CreateTempFileName("ltx");
    lttFileName = wxFileName::CreateTempFileName("ltt");
    ltcFileName = wxFileName::CreateTempFileName("ltc");
  }
}


struct ImportParams
{
  CImportOnlineEntries *dlg;
};


unsigned CImportOnlineEntries::ImportThread(void *args)
{
  const ImportParams *params = (const ImportParams *) args;

  CImportOnlineEntries *dlg = params->dlg;

  delete params;

  if (!dlg->ImportThreadRead())
    return 1;

  if (!dlg->ImportThreadImport())
    return 2;

  wxCommandEvent evt(wxEVT_COMMAND_BUTTON_CLICKED, wxID_FORWARD);
  evt.SetEventObject(dlg->FindWindow(wxID_FORWARD));
  dlg->GetEventHandler()->AddPendingEvent(evt);

  return 0;
}


bool CImportOnlineEntries::ImportThreadRead()
{
  CTT32App::instance()->SetProgressBarText(_("Reading Tournament"));

  TournamentItem *itemPtr = (TournamentItem *) XRCCTRL(*this, "Tournaments", CComboBoxEx)->GetCurrentItem();
  int tid = itemPtr->GetID();

  XmlRpcValue args;
  args[0] = tid;

  XmlRpcValue competitions;
  XmlRpcValue nations;
  XmlRpcValue players;
  XmlRpcValue rankings;
  XmlRpcValue clubs;
  XmlRpcValue clubplayers;
  
  // rankings.initAsArray();

  bool res = true;

  res &= xmlRpcClient.execute("onlineentries.listCompetitions", args, competitions);
  res &= xmlRpcClient.execute("onlineentries.listNations", args, nations);
  res &= xmlRpcClient.execute("onlineentries.listPlayers", args, players);
  res &= xmlRpcClient.execute("onlineentries.listRankingpoints", args, rankings);
  res &= xmlRpcClient.execute("onlineentries.listClubs", args, clubs);

  if (!res)
  {
    wxString fault = xmlRpcClient.getError();
    if (fault.IsEmpty())
      fault = "Unkown error";

    infoSystem.Error(_("Could not read tournament:\n%s"), fault.wc_str());
    return false;
  }

  std::map<int, Competition> cpMap;

  for (int idx = 0; idx < competitions.size(); idx++)
  {
    XmlRpcValue val;
    if (competitions[idx]["Competition"].getType() == XmlRpcValue::Type::TypeStruct)
      val = competitions[idx]["Competition"];
    else
      val = competitions[idx];

    Competition cp;

    cp.id = GetInt(val["id"]);
    cp.name = wxString::FromUTF8((const char *) val["name"]);
    cp.description = wxString::FromUTF8((const char *) val["description"]);
    if (val.hasMember("category"))
      cp.category = wxString::FromUTF8((const char *) val["category"]);
    cp.type = wxString::FromUTF8((const char *) val["type_of"]);
    // Correct for CP_CLUB
    if (cp.type.MakeUpper() == "C")
      cp.type = "T";
    cp.sex = wxString::FromUTF8((const char *) val["sex"]);
    cp.born = GetInt(val["born"]);

    cpMap[cp.id] = cp;
  }

  std::map<int, Nation> naMap;

  for (int idx = 0; idx < nations.size(); idx++)
  {
    XmlRpcValue val;
    
    if (nations[idx]["Nation"].getType() == XmlRpcValue::Type::TypeStruct)
      val = nations[idx]["Nation"];
    else
      val = nations[idx];

    Nation na;

    na.id = GetInt(val["id"]);
    na.name = wxString::FromUTF8((const char *) val["name"]);
    na.description = wxString::FromUTF8((const char *) val["description"]);

    naMap[na.id] = na;
  }

  std::map<wxString, PlRec> existingPlMap;
  std::map<int, Person> plMap;
  int minStartNr = 0x7FFFFFFF;
  int maxStartNr = 0;
  bool clearTournament = XRCCTRL(*this, "ClearTournament", wxCheckBox)->GetValue();

  if ( !clearTournament )
  {
    Connection *connPtr = TTDbse::instance()->GetNewConnection();

    // PlStore kapseln
    {
      PlStore pl(connPtr);
      maxStartNr = pl.GetHighestNumber();

      pl.SelectAll();
      while (pl.Next())
      {
        if (*pl.plExtID)
          existingPlMap[pl.plExtID] = pl;
        else
          existingPlMap[wxString::Format("%s, %s (%s)", pl.psName.psLast, pl.psName.psFirst, pl.naName)] = pl;
      }
    }

    delete connPtr;
  }

  std::vector<Person> plList;

  for (int idx = 0; idx < players.size(); idx++)
  {
    XmlRpcValue person, player, participant, registration;

    if (players[idx]["Person"].getType() == XmlRpcValue::Type::TypeStruct)
    {
      // CakePHP 2.x
      person = players[idx]["Person"];
      // ETTU vs. Veterans
      if (person["Player"].getType() == XmlRpcValue::Type::TypeStruct)
        player = person["Player"];
      else
        player = person;

      participant = players[idx]["Participant"];
      registration = players[idx]["Registration"];
    }
    else
    {
      // CakePHP 3.x
      person = players[idx]["person"];
      // ETTU vs. Veterans
      if (person["player"].getType() == XmlRpcValue::Type::TypeStruct)
        player = person["player"];
      else
        player = person;

      participant = players[idx]["participant"];
      registration = players[idx];
    }
                    
    if ( (bool) participant["cancelled"])
      continue;

    Person pl;
    pl.id = GetInt(person["id"]);
    pl.firstName = wxString::FromUTF8((const char *) person["first_name"]);
    pl.lastName = wxString::FromUTF8((const char *) person["last_name"]);
    pl.displayName = wxString::FromUTF8((const char *) person["display_name"]);
    pl.sex = wxString::FromUTF8((const char *)person["sex"]);
    pl.naID = GetInt(person["nation_id"]);
    pl.phone = wxString::FromUTF8((const char *)person["phone"]);
    if (registration.hasMember("comment"))
      pl.comment = URLEncode(wxString::FromUTF8((const char *)registration["comment"]));

    // Unterschiede in Typ und Format vom Geburtsdatum
    XmlRpcValue dob = person["dob"].getType() == XmlRpcValue::Type::TypeStruct && person["dob"].hasMember("year") ? person["dob"]["year"] : person["dob"];

    switch (dob.getType())
    {
      case XmlRpcValue::Type::TypeDateTime :
        pl.born = ((struct tm &) dob).tm_year;
        break;

      case XmlRpcValue::Type::TypeString :
        pl.born = _strtoi(wxString::FromUTF8((const char *)dob).Left(4));
        break;

      case XmlRpcValue::Type::TypeInt :
        pl.born = GetInt(dob);
        break;

      default :
        pl.born = 0;
    }
    pl.externID = wxString::FromUTF8((const char *)player["extern_id"]);
    pl.startNr = GetInt(participant["start_no"]);
    pl.rankPts = GetDouble(player["rank_pts"]);

    pl.naName = naMap[pl.naID].name;

    if (!pl.startNr && existingPlMap.size())
    {
      wxString key = pl.externID;
      if (existingPlMap.find(key) == existingPlMap.end())
        key = wxString::Format("%s, %s (%s)", pl.lastName.wx_str(), pl.firstName.wx_str(), pl.naName.wx_str());
      if (existingPlMap.find(key) != existingPlMap.end())
        pl.startNr = existingPlMap[key].plNr;
    }

    if (pl.startNr < minStartNr)
      minStartNr = pl.startNr;
    if (pl.startNr > maxStartNr)
      maxStartNr = pl.startNr;

    plList.push_back(pl);
  }

  // Rankingpunkte lesen
  std::map<int, std::map<short, double>> rpMap;

  for (int idx = 0; idx < rankings.size(); idx++)
  {
    int id = GetInt(rankings[idx]["id"]);
    if (plMap.find(id) == plMap.end())
      continue;

    int year = GetInt(rankings[idx]["year"]);
    double rankPts = GetDouble(rankings[idx]["rank_pts"]);

    rpMap[id][year] = rankPts;
  }

  std::map<int, Registration> ltMap;
  std::map<int, std::set<std::pair<int, int>>> tmMap;
                
  for (int idx = 0; idx < players.size(); idx++)
  {
    XmlRpcValue registration, participant;

    if (players[idx]["Registration"].getType() == XmlRpcValue::Type::TypeStruct)
    {
      registration = players[idx]["Registration"];
      participant = players[idx]["Participant"];
    }
    else
    {
      registration = players[idx];
      participant = players[idx]["participant"];
    }
                    
    Registration lt;
    lt.id = GetInt(registration["id"]);
    lt.playerID = GetInt(registration["person_id"]);
    lt.singleID = GetInt(participant["single_id"]);
    lt.doubleID = GetInt(participant["double_id"]);
    lt.doublePartnerID = GetInt(participant["double_partner_id"]);
    lt.mixedID = GetInt(participant["mixed_id"]);
    lt.mixedPartnerID = GetInt(participant["mixed_partner_id"]);
    lt.teamID = GetInt(participant["team_id"]);
    lt.teamNo = GetInt(participant["team_no"]);
    lt.cancelled = (bool) participant["cancelled"];
    lt.singleCancelled = (bool) participant["single_cancelled"];
    lt.doubleCancelled = (bool) participant["double_cancelled"];
    lt.mixedCancelled = (bool) participant["mixed_cancelled"];
    lt.teamCancelled = (bool) participant["team_cancelled"];
                    
    if (lt.cancelled)
        continue;
                    
    ltMap[lt.id] = lt;

    if (lt.teamID && !lt.teamCancelled)
      tmMap[lt.teamID].insert(std::make_pair(plMap[lt.playerID].naID, lt.teamNo));
  }

  std::map<int, Club> clMap;

  for (int idx = 0; idx < clubs.size(); idx++)
  {
    Club club;
    int id = clubs[idx]["id"];
    club.cpID = clubs[idx]["competition_id"];
    club.naID = clubs[idx]["nation_id"];
    club.name = wxString::FromUTF8((const char *) clubs[idx]["name"]);
    club.description = wxString::FromUTF8((const char *) clubs[idx]["description"]);

    XmlRpcValue people = clubs[idx]["people"];

    for (int idx2 = 0; idx2 < people.size(); idx2++)
    {
      Person pl;
      pl.startNr = 0;
      pl.id = GetInt(people[idx2]["id"]);
      pl.firstName = wxString::FromUTF8((const char *) people[idx2]["first_name"]);
      pl.lastName = wxString::FromUTF8((const char *) people[idx2]["last_name"]);
      pl.displayName = wxString::FromUTF8((const char *) people[idx2]["display_name"]);
      pl.sex = wxString::FromUTF8((const char *) people[idx2]["sex"]);
      pl.naID = GetInt(people[idx2]["nation_id"]);

      plList.push_back(pl);
      club.players.push_back(pl);
    }

    clMap[id] = club;
  }

  // Spielern, die noch keine Startnummer haben, eine vergeben
  // Dazu werden die Spieler erst nach Startnummer, dann nach Nation, Geschlecht und Namen sortiert
  std::sort(plList.begin(), plList.end());

  for (std::vector<Person>::iterator it = plList.begin(); it != plList.end(); it++)
  {
    if ((*it).startNr == 0)
      (*it).startNr = ++maxStartNr;

    plMap[(*it).id] = (*it);
  }

  // Write Competitions
  wxFFile cpFile(cpFileName.GetFullPath(), "w");    
  cpFile.Write(wxChar(0xFEFF));
  cpFile.Write("#EVENTS 2\n");
  cpFile.Write("# Name; Desc; Category; Type; Sex; Year\n");
                
  for (std::map<int, Competition>::iterator it = cpMap.begin(); it != cpMap.end(); it++)
  {
    Competition cp = (*it).second;
    wxString line;
    line << cp.name << ";" 
         << cp.description << ";" 
         << cp.category << ";"
         << cp.type << ";" 
         << cp.sex << ";" 
         << cp.born << "\n";
    cpFile.Write(line);
  }

  cpFile.Close();

  // Write Associations
  wxFFile naFile(naFileName.GetFullPath(), "w");
  naFile.Write(wxChar(0xFEFF));
  naFile.Write("#NATIONS\n");
  naFile.Write("# Name; Desc\n");
                
  for (std::map<int, Nation>::iterator it = naMap.begin(); it != naMap.end(); it++)
  {
    Nation na = (*it).second;
    wxString line;
    line << na.name << ";" 
         << na.description << "\n";
    naFile.Write(line);
  }
             
  naFile.Close();

  // Write Players
  wxFFile plFile(plFileName.GetFullPath(), "w");
  plFile.Write(wxChar(0xFEFF));
  plFile.Write("#PLAYERS\n");
  plFile.Write("# Players Number;Last Name;First Name;Sex;Year of Birth;Association;External ID;Ranking Points;Comment\n");

  wxFFile phFile(phFileName.GetFullPath(), "w");
  phFile.Write(wxChar(0xFEFF));
  phFile.Write("# Players Number; Phone Number; Association (opt.)\n");
  bool phFileDirty = false;
        
  for (std::map<int, Person>::iterator it = plMap.begin(); it != plMap.end(); it++)
  {
    Person pl = (*it).second;

    {
      wxString line;
      line << pl.startNr << ";" 
           << pl.lastName << ";" 
           << pl.firstName << ";" 
           << pl.sex << ";" 
           << pl.born << ";"
           << pl.naName << ";" 
           << pl.externID << ";" 
           << wxString::FromCDouble(pl.rankPts) << ";"
           << pl.comment << ""
           << "\n"
      ;
      plFile.Write(line);
    }

    if (!pl.phone.IsEmpty())
    {
      wxString line;
      line << pl.startNr << ";" 
           << pl.phone << ";"
           << pl.naName << "\n"
      ;
      phFile.Write(line);
      phFileDirty = true;
    }
  }

  plFile.Close();
  phFile.Close();

  // Remove phone numbers if file is no phones have been found
  if (!phFileDirty)
    wxRemoveFile(phFileName.GetFullPath());

  // Write ranking
  wxFFile rpFile(rpFileName.GetFullPath(), "w");
  rpFile.Write(wxChar(0xFEFF));
  rpFile.Write("#RANKINGPOINTS\n");
  rpFile.Write("# Players Number;Year;Ranking Points;...\n");

  for (std::map<int, std::map<short, double>>::const_iterator plIt = rpMap.begin(); plIt != rpMap.cend(); plIt++)
  {
    wxString line;
    line << plMap[plIt->first].startNr << ";";
    for (std::map<short, double>::const_iterator rpIt = plIt->second.cbegin(); rpIt != plIt->second.cend(); rpIt++)
    {
      line << rpIt->first << ";" << wxString::FromCDouble(rpIt->second) << ";";
    }

    line << "\n";

    rpFile.Write(line);
  }

  rpFile.Close();

  // Write Entries
  wxFFile ltsFile(ltsFileName.GetFullPath(), "w");
  wxFFile ltdFile(ltdFileName.GetFullPath(), "w");
  wxFFile ltxFile(ltxFileName.GetFullPath(), "w");
  wxFFile lttFile(lttFileName.GetFullPath(), "w");
  wxFFile ltcFile(ltcFileName.GetFullPath(),"w");

  ltsFile.Write("#ENTRIES\n");                
  ltdFile.Write("#ENTRIES\n");                
  ltxFile.Write("#ENTRIES\n");                
  lttFile.Write("#ENTRIES\n"); 
  ltcFile.Write("#ENTRIES\n");

  // Teams
  lttFile.Write("# Team Name;Event;Team Desc;Association;National Ranking;International Ranking\n");
            
  for (std::map<int, std::set<std::pair<int, int>>>::iterator it = tmMap.begin(); it != tmMap.end(); it++)
  {
    int id = (*it).first;
    for (std::set<std::pair<int, int>>::iterator it2 = (*it).second.begin(); it2 != (*it).second.end(); it2++)
    {
      int id2 = (*it2).first;
      int no  = (*it2).second;
      wxString tmName = naMap[id2].name;

      if (no > 0)
        tmName += wxString::Format("%d", no);
      wxString tmDesc = naMap[id2].description;
      if (no > 0)
        tmDesc += " " + wxString::Format("%d", no);

      wxString line;
      line << tmName << ";"
           << cpMap[id].name << ";"
           << tmDesc << ";"
           << naMap[id2].name << ";"
           << "0" << ";"
           << "0" << "\n";

      lttFile.Write(line);
    }
  }
                
  // Players
  ltsFile.Write("# Players Number;Event;Partner;Association;National Ranking;International Ranking\n");
  ltdFile.Write("# Players Number;Event;Partner;Association;National Ranking;International Ranking\n");
  ltxFile.Write("# Players Number;Event;Partner;Association;National Ranking;International Ranking\n");
  lttFile.Write("# Players Number;Event;Partner;Association;National Ranking;International Ranking\n");
                
  for (std::map<int, Registration>::iterator it = ltMap.begin(); it != ltMap.end(); it++)
  {
    Registration lt = (*it).second;

    if (lt.cancelled)
      continue;
                    
    if (lt.singleID != 0 && !lt.singleCancelled) 
    {
      wxString line;
      line << plMap[lt.playerID].startNr << ";"
        << cpMap[lt.singleID].name << ";"
        << "0" << ";"
        << naMap[plMap[lt.playerID].naID].name << ";"
        << "0" << ";"
        << "0" << "\n";

      ltsFile.Write(line);
    }

    if (lt.doubleID && !lt.doubleCancelled)
    {
      std::map<int, Registration>::iterator partner = ltMap.find(lt.doublePartnerID);
      if (partner == ltMap.end() || (*partner).second.doubleCancelled || (*partner).second.doublePartnerID != lt.id)
        partner = ltMap.end();

      wxString line;
      line << plMap[lt.playerID].startNr << ";"
           << cpMap[lt.doubleID].name << ";";

      // Partner
      if (partner != ltMap.end())
        line << plMap[(*partner).second.playerID].startNr << ";";
      else
        line << "0" << ";";

      // Association
      if (partner == ltMap.end())
        line << ";"; 
      else if (plMap[lt.playerID].rankPts > plMap[(*partner).second.playerID].rankPts)
        line << naMap[plMap[lt.playerID].naID].name << ";";
      else if (plMap[lt.playerID].rankPts < plMap[(*partner).second.playerID].rankPts)
        line << naMap[plMap[(*partner).second.playerID].naID].name << ";"; 
      else if (plMap[lt.playerID].startNr < plMap[(*partner).second.playerID].startNr)
        line << naMap[plMap[lt.playerID].naID].name << ";";
      else if (plMap[lt.playerID].startNr > plMap[(*partner).second.playerID].startNr)
        line << naMap[plMap[(*partner).second.playerID].naID].name << ";";
      else
        line << naMap[plMap[lt.playerID].naID].name << ";";

      line << "0" << ";"
           << "0" << "\n";

      ltdFile.Write(line);
    }
         
    if (lt.mixedID && !lt.mixedCancelled)
    {
      std::map<int, Registration>::iterator partner = ltMap.find(lt.mixedPartnerID);
      if (partner == ltMap.end() || (*partner).second.mixedCancelled || (*partner).second.mixedPartnerID != lt.id)
        partner = ltMap.end();

      wxString line;
      line << plMap[lt.playerID].startNr << ";"
           << cpMap[lt.mixedID].name << ";";

      // Partner
      if (partner != ltMap.end())
        line << plMap[(*partner).second.playerID].startNr << ";";
      else
        line << "0" << ";";

      // Association
      if (partner == ltMap.end())
        line << ";"; // empty statement
      else if (plMap[lt.playerID].sex == "M")
        line << naMap[plMap[lt.playerID].naID].name << ";";
      else 
        line << naMap[plMap[(*partner).second.playerID].naID].name << ";"; 

      line << "0" << ";"
           << "0" << "\n";

      ltxFile.Write(line);
    }

    if (lt.teamID && !lt.teamCancelled)
    {
      wxString line;

      line << plMap[lt.playerID].startNr << ";"
           << cpMap[lt.teamID].name << ";"
           << naMap[plMap[lt.playerID].naID].name + (lt.teamNo > 0 ? wxString::Format("%d", lt.teamNo) : wxString::Format("")) << ";"
           << "0" << ";"
           << "0" << "\n";
                    
      lttFile.Write(line);
    }
  }

  // Clubs
  ltcFile.Write("# Club Name;Event;Club Desc;Association\n");

  for (auto it = clMap.begin(); it != clMap.end(); it++)
  {
    wxString line;

    line << (*it).second.name << ";"
         << cpMap[(*it).second.cpID].name << ";"
         << (*it).second.description << ";"
         << naMap[(*it).second.naID].name << ";"
          << "\n"
    ;

    ltcFile.Write(line);
  }

  // Club Players
  ltcFile.Write("# Players Number;Event;Club\n");

  for (auto it = clMap.begin(); it != clMap.end(); it++)
  {
    for (auto it2 = (*it).second.players.begin(); it2 != (*it).second.players.end(); it2++)
    {
      wxString line;

      Club club = (*it).second;
      Person pl = plMap[(*it2).id];
      
      line << pl.startNr << ";"
           << cpMap[club.cpID].name << ";"
           << club.name << ";"
           << "\n"
       ;

      ltcFile.Write(line);
    }
  }

  ltsFile.Close();
  ltdFile.Close();
  ltxFile.Close();
  lttFile.Close();
  ltcFile.Close();

  return true;
}


bool CImportOnlineEntries::ImportThreadImport()
{
  if (XRCCTRL(*this, "ClearTournament", wxCheckBox)->GetValue())
    ClearTournament();

  if (XRCCTRL(*this, "ImportCompetitions", wxCheckBox)->GetValue())
    ImportCP();

  if (XRCCTRL(*this, "ImportAssociations", wxCheckBox)->GetValue())
    ImportNA();

  if (XRCCTRL(*this, "ImportPlayers", wxCheckBox)->GetValue())
    ImportPL();

  if (XRCCTRL(*this, "ImportRanking", wxCheckBox)->GetValue())
    ImportRP();

  if (XRCCTRL(*this, "ImportEntries", wxCheckBox)->GetValue())
    ImportLT();

  return true;
}


void CImportOnlineEntries::ClearTournament()
{
  CTT32App::SetProgressBarText(_("Clear Tournament"));

  Connection *conn = TTDbse::instance()->GetNewConnection();

  std::vector<int> idList;

  {
    TmStore tm(conn);
    tm.SelectAll();
    while (tm.Next())
      idList.push_back(tm.tmID);
    tm.Close();

    for (auto it : idList)
      tm.Remove(it);

    PlStore pl(conn);
    pl.SelectAll();
    while (pl.Next())
      idList.push_back(pl.plID);
    pl.Close();
  
    for (auto it : idList)
      pl.Remove(it, true);

    idList.clear();

    UpStore up(conn);
    up.SelectAll();
    while (up.Next())
      idList.push_back(up.upID);
    up.Close();

    for (auto it : idList)
      up.Remove(it);

    idList.clear();
  
    NaStore na(conn);
    na.SelectAll();
    while (na.Next())
      idList.push_back(na.naID);

    for (auto it : idList)
      na.Remove(it);

    idList.clear();

    CpStore cp(conn);
    cp.SelectAll();
    while (cp.Next())
      idList.push_back(cp.cpID);

    for (auto it : idList)
      cp.Remove(it);

    idList.clear();


  }

  delete conn;
}


// Auto Open / Close wxTextFile
class wxTextFileAuto : public wxTextFile
{
  public:
    wxTextFileAuto(const wxString &fileName) : wxTextFile(fileName) { Open(); }
   ~wxTextFileAuto() { Close(); }
};

void CImportOnlineEntries::ImportCP()
{
  CTT32App::SetProgressBarText(_("Import Competitions"));

  // Import CP
  wxTextFileAuto is(cpFileName.GetFullPath());
  CpStore::Import(is);
}

void CImportOnlineEntries::ImportNA()
{
  CTT32App::SetProgressBarText(_("Import Associations"));

  // Import NA
  wxTextFileAuto is(naFileName.GetFullPath());
  NaStore::Import(is);
}

void CImportOnlineEntries::ImportPL()
{
  CTT32App::SetProgressBarText(_("Import Players"));

  // Import PL
  wxTextFileAuto is(plFileName.GetFullPath());
  PlStore::Import(is);
}

void CImportOnlineEntries::ImportRP()
{
  CTT32App::SetProgressBarText(_("Import Rankikng"));

  // Import RP
  wxTextFileAuto is(rpFileName.GetFullPath());
  RpStore::Import(is);
}

void CImportOnlineEntries::ImportLT()
{
  if (XRCCTRL(*this, "ImportEntriesSingles", wxCheckBox)->GetValue())
  {
    // Import LTS
    CTT32App::SetProgressBarText(_("Import Singles"));

    wxTextFileAuto is(ltsFileName.GetFullPath());
    LtStore::Import(is);
  }

  if (XRCCTRL(*this, "ImportEntriesDoubles", wxCheckBox)->GetValue())
  {
    // Import LTD
    CTT32App::SetProgressBarText(_("Import Doubles"));

    wxTextFileAuto is(ltdFileName.GetFullPath());
    LtStore::Import(is);
  }

  if (XRCCTRL(*this, "ImportEntriesMixed", wxCheckBox)->GetValue())
  {
    // Import LTX
    CTT32App::SetProgressBarText(_("Import Mixed"));

    wxTextFileAuto is(ltxFileName.GetFullPath());
    LtStore::Import(is);
  }

  if (XRCCTRL(*this, "ImportEntriesTeams", wxCheckBox)->GetValue())
  {
    // Import LTT
    CTT32App::SetProgressBarText(_("Import Teams"));

    wxTextFileAuto is(lttFileName.GetFullPath());
    LtStore::Import(is);
  }

  if (XRCCTRL(*this, "ImportEntriesClubs", wxCheckBox)->GetValue())
  {
    // Import LTT
    CTT32App::SetProgressBarText(_("Import Clubs"));

    wxTextFileAuto is(ltcFileName.GetFullPath());
    LtStore::Import(is);
  }
}



void CImportOnlineEntries::OnPageChangingImport(wxWizardEvent &evt)
{
  if ( !evt.GetDirection() )
    return;

  if (inThread)
  {
    // Seitenwechsel wurde vom Code veranlasst
    inThread = false;
    return;
  }

  inThread = true;

  evt.Veto();

  ImportParams *params = new ImportParams;
  params->dlg = this;

  CTT32App::ProgressBarThread(CImportOnlineEntries::ImportThread, params, _("Import Tournament"), 0, true);
}


// -----------------------------------------------------------------------
void CImportOnlineEntries::OnPageShown(wxWizardEvent &evt)
{
  if (GetCurrentPage()->GetId() == XRCID("ImportOnlineEntriesConnect"))
  {
    // Wenn die Seite angezeigt wird, ist der Text nach links gescrollt und man sieht nur das Ende
    wxTextCtrl *url = XRCCTRL(*this, "Url", wxTextCtrl);

    if ( !url->GetValue().IsEmpty() )
    {
      long from, to;
      url->GetSelection(&from, &to);
      url->SetInsertionPoint(0);
      url->SetSelection(from, to);
    }
  }
}


// -----------------------------------------------------------------------
void CImportOnlineEntries::OnTextUrl(wxCommandEvent &)
{
  wxWindow *currentPage = GetCurrentPage();
  wxTextCtrl *url = XRCCTRL(*currentPage, "Url", wxTextCtrl);
  wxWindow *forward = this->FindWindow(wxID_FORWARD);
  forward->Enable(url->IsEmpty() == false);
}


void CImportOnlineEntries::OnSelectDirectory(wxCommandEvent &)
{
  wxDirDialog dlg(this, _("Select Directory for Import Files"), CTT32App::instance()->GetPath());
  if (dlg.ShowModal() != wxID_OK)
    return;

  XRCCTRL(*this, "Directory", wxTextCtrl)->SetValue(dlg.GetPath());
}


void CImportOnlineEntries::OnChangeImportEntries(wxCommandEvent &)
{
  bool enabled = XRCCTRL(*this, "ImportEntries", wxCheckBox)->GetValue();

  // Disable wenn ImportEntries nicht selektiert ist
  XRCCTRL(*this, "ImportEntriesSingles", wxCheckBox)->Enable(enabled);
  XRCCTRL(*this, "ImportEntriesDoubles", wxCheckBox)->Enable(enabled);
  XRCCTRL(*this, "ImportEntriesMixed", wxCheckBox)->Enable(enabled);
  XRCCTRL(*this, "ImportEntriesTeams", wxCheckBox)->Enable(enabled);
}
