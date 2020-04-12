/* Copyright (C) 2020 Christoph Theis */


#include "stdafx.h"

#include "ImportOnlineEntries.h"

#include "StListStore.h"
#include "PlStore.h"
#include "NaStore.h"
#include "CpStore.h"
#include "RpStore.h"
#include "LtStore.h"

#include "ComboBoxEx.h"
#include "ListItem.h"

#include "InfoSystem.h"
#include "TT32App.h"

#include "StrUtils.h"

#include <map>
#include <set>
#include <vector>
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
    case XmlRpcValue::TypeInt :
      return (int) val;
    case XmlRpcValue::TypeString :
      return atoi(val);
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
  int id;
  wxString name;
  wxString description;
  wxString type;
  wxString sex;
  int born;
};


struct Nation
{
  int id;
  wxString name;
  wxString description;
};


struct Person {
  int id;
  wxString firstName;
  wxString lastName;
  wxString displayName;
  wxString sex;
  int      naID;
  int      born;
  wxString externID;
  int      rankPts;
  int      startNr;
  wxString naName;
  wxString phone;
  wxString comment;

  bool operator<(const Person &b);
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
  if (!xmlRpcClient.connect(url, user.IsEmpty() ? NULL : (const char *) user, pwd.IsEmpty() ? NULL : (const char *) pwd))
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

  if (result.getType() != XmlRpcValue::TypeArray)
  {
    infoSystem.Error(_("Could not read list of tournaments"));
    return;
  }

  CComboBoxEx *tournaments = XRCCTRL(*this, "Tournaments", CComboBoxEx);
  tournaments->Clear();

  for (int idx = 0; idx < result.size(); idx++)
  {
    XmlRpcValue t;

    if (result[idx]["Tournament"].getType() == XmlRpcValue::TypeStruct)
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
         wxFileExists(wxFileName(path, "lt.csv").GetFullPath()) )
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

  bool res = true;

  res &= xmlRpcClient.execute("onlineentries.listCompetitions", args, competitions);
  res &= xmlRpcClient.execute("onlineentries.listNations", args, nations);
  res &= xmlRpcClient.execute("onlineentries.listPlayers", args, players);
  res &= xmlRpcClient.execute("onlineentries.listRankingpoints", args, rankings);

  if (!res)
  {
    wxString fault =xmlRpcClient.getError();
    if (fault.IsEmpty())
      fault = "Unkown error";

    infoSystem.Error(_("Could not read tournament:\n%s"), fault.wc_str());
    return false;
  }

  std::map<int, Competition> cpMap;

  for (int idx = 0; idx < competitions.size(); idx++)
  {
    XmlRpcValue val;
    if (competitions[idx]["Competition"].getType() == XmlRpcValue::TypeStruct)
      val = competitions[idx]["Competition"];
    else
      val = competitions[idx];

    Competition cp;

    cp.id = GetInt(val["id"]);
    cp.name = wxString::FromUTF8((const char *) val["name"]);
    cp.description = wxString::FromUTF8((const char *) val["description"]);
    cp.type = wxString::FromUTF8((const char *) val["type_of"]);
    cp.sex = wxString::FromUTF8((const char *) val["sex"]);
    cp.born = GetInt(val["born"]);

    cpMap[cp.id] = cp;
  }

  std::map<int, Nation> naMap;

  for (int idx = 0; idx < nations.size(); idx++)
  {
    XmlRpcValue val;
    
    if (nations[idx]["Nation"].getType() == XmlRpcValue::TypeStruct)
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

    if (players[idx]["Person"].getType() == XmlRpcValue::TypeStruct)
    {
      // CakePHP 2.x
      person = players[idx]["Person"];
      // ETTU vs. Veterans
      if (person["Player"].getType() == XmlRpcValue::TypeStruct)
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
      if (person["player"].getType() == XmlRpcValue::TypeStruct)
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
    XmlRpcValue dob = person["dob"].getType() == XmlRpcValue::TypeStruct ? person["dob"]["date"] : person["dob"];

    switch (dob.getType())
    {
      case XmlRpcValue::TypeDateTime :
        pl.born = ((struct tm &) dob).tm_year;
        break;

      case XmlRpcValue::TypeString :
        pl.born = atoi(wxString::FromUTF8((const char *)dob).Left(4));
        break;

      default :
        pl.born = 0;
    }
    pl.externID = wxString::FromUTF8((const char *)player["extern_id"]);
    pl.startNr = GetInt(participant["start_no"]);
    switch (player["rank_pts"].getType())
    {
      case XmlRpcValue::TypeDouble :
        pl.rankPts = (double) player["rank_pts"];
        break;

      case XmlRpcValue::TypeString :
        pl.rankPts = atof(player["rank_pts"]);
        break;

      default :
        pl.rankPts = 0;
        break;
    }

    pl.naName = naMap[pl.naID].name;

    if (!pl.startNr && existingPlMap.size())
    {
      wxString key = pl.externID;
      if (existingPlMap.find(key) == existingPlMap.end())
        key = wxString::Format("%s, %s (%s)", (const char *) pl.lastName, (const char *) pl.firstName, (const char *) pl.naName);
      if (existingPlMap.find(key) != existingPlMap.end())
        pl.startNr = existingPlMap[key].plNr;
    }

    if (pl.startNr < minStartNr)
      minStartNr = pl.startNr;
    if (pl.startNr > maxStartNr)
      maxStartNr = pl.startNr;

    plList.push_back(pl);
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


  // Rankingpunkte lesen
  std::map<int, std::map<short, double>> rpMap;

  for (int idx = 0; idx < rankings.size(); idx++)
  {
    int id = GetInt(rankings[idx]["id"]);
    if (plMap.find(id) == plMap.end())
      continue;

    int year = GetInt(rankings[idx]["year"]);
    double rankPts = 0;

    switch (rankings[idx]["rank_pts"].getType())
    {
      case XmlRpcValue::TypeDouble :
        rankPts = (double) rankings[idx]["rank_pts"];
        break;

      case XmlRpcValue::TypeString :
        rankPts = atof(rankings[idx]["rank_pts"]);
        break;

      default :
        rankPts = 0;
        break;
    }

    rpMap[id][year] = rankPts;
  }

  std::map<int, Registration> ltMap;
  std::map<int, std::set<int>> tmMap;
                
  for (int idx = 0; idx < players.size(); idx++)
  {
    XmlRpcValue registration, participant;

    if (players[idx]["Registration"].getType() == XmlRpcValue::TypeStruct)
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
    lt.cancelled = (bool) participant["cancelled"];
    lt.singleCancelled = (bool) participant["single_cancelled"];
    lt.doubleCancelled = (bool) participant["double_cancelled"];
    lt.mixedCancelled = (bool) participant["mixed_cancelled"];
    lt.teamCancelled = (bool) participant["team_cancelled"];
                    
    if (lt.cancelled)
        continue;
                    
    ltMap[lt.id] = lt;

    if (lt.teamID && !lt.teamCancelled)
      tmMap[lt.teamID].insert(plMap[lt.playerID].naID);
  }

  // Write Competitions
  wxFFile cpFile(cpFileName.GetFullPath(), "w");    
  cpFile.Write(wxChar(0xFEFF));
  cpFile.Write("#EVENTS\n");
  cpFile.Write("# Name; Desc; Type; Sex; Year\n");
                
  for (std::map<int, Competition>::iterator it = cpMap.begin(); it != cpMap.end(); it++)
  {
    Competition cp = (*it).second;
    wxString line;
    line << cp.name << ";" 
         << cp.description << ";" 
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
           << pl.rankPts << ";"
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
      line << rpIt->first << ";" << rpIt->second << ";";
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

  ltsFile.Write("#ENTRIES\n");                
  ltdFile.Write("#ENTRIES\n");                
  ltxFile.Write("#ENTRIES\n");                
  lttFile.Write("#ENTRIES\n");                

  // Teams
  lttFile.Write("# Team Name;Event;Team Desc;Association;National Ranking;International Ranking\n");
            
  for (std::map<int, std::set<int>>::iterator it = tmMap.begin(); it != tmMap.end(); it++)
  {
    int id = (*it).first;
    for (std::set<int>::iterator it2 = (*it).second.begin(); it2 != (*it).second.end(); it2++)
    {
      int id2 = (*it2);
      wxString line;
      line << naMap[id2].name << ";"
           << cpMap[id].name << ";"
           << naMap[id2].description << ";"
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
           << naMap[plMap[lt.playerID].naID].name << ";"
           << "0" << ";"
           << "0" << "\n";
                    
      lttFile.Write(line);
    }
  }

  ltsFile.Close();
  ltdFile.Close();
  ltxFile.Close();
  lttFile.Close();

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
    PlStore pl(conn);
    pl.SelectAll();
    while (pl.Next())
      idList.push_back(pl.plID);
    pl.Close();
  
    for (std::vector<int>::iterator it = idList.begin(); it != idList.end(); it++)
      pl.Remove(*it);

    idList.clear();
  
    NaStore na(conn);
    na.SelectAll();
    while (na.Next())
      idList.push_back(na.naID);

    for (std::vector<int>::iterator it = idList.begin(); it != idList.end(); it++)
      na.Remove(*it);

    idList.clear();

    CpStore cp(conn);
    cp.SelectAll();
    while (cp.Next())
      idList.push_back(cp.cpID);

    for (std::vector<int>::iterator it = idList.begin(); it != idList.end(); it++)
      cp.Remove(*it);

    idList.clear();
  }

  delete conn;
}

void CImportOnlineEntries::ImportCP()
{
  CTT32App::SetProgressBarText(_("Import Competitions"));

  // Import CP
  CpStore::Import(cpFileName.GetFullPath());
}

void CImportOnlineEntries::ImportNA()
{
  CTT32App::SetProgressBarText(_("Import Associations"));

  // Import NA
  NaStore::Import(naFileName.GetFullPath());
}

void CImportOnlineEntries::ImportPL()
{
  CTT32App::SetProgressBarText(_("Import Players"));

  // Import PL
  PlStore::Import(plFileName.GetFullPath());
}

void CImportOnlineEntries::ImportRP()
{
  CTT32App::SetProgressBarText(_("Import Rankikng"));

  // Import RP
  RpStore::Import(rpFileName.GetFullPath());
}

void CImportOnlineEntries::ImportLT()
{
  if (XRCCTRL(*this, "ImportEntriesSingles", wxCheckBox)->GetValue())
  {
    // Import LTS
    CTT32App::SetProgressBarText(_("Import Singles"));

    LtStore::Import(ltsFileName.GetFullPath());
  }

  if (XRCCTRL(*this, "ImportEntriesDoubles", wxCheckBox)->GetValue())
  {
    // Import LTS
    CTT32App::SetProgressBarText(_("Import Doubles"));

    LtStore::Import(ltdFileName.GetFullPath());
  }

  if (XRCCTRL(*this, "ImportEntriesMixed", wxCheckBox)->GetValue())
  {
    // Import LTS
    CTT32App::SetProgressBarText(_("Import Mixed"));

    LtStore::Import(ltxFileName.GetFullPath());
  }

  if (XRCCTRL(*this, "ImportEntriesTeams", wxCheckBox)->GetValue())
  {
    // Import LTS
    CTT32App::SetProgressBarText(_("Import Teams"));

    LtStore::Import(lttFileName.GetFullPath());
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
