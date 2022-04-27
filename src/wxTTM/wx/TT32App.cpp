/* Copyright (C) 2020 Christoph Theis */

#include "stdafx.h"

#include <wx/progdlg.h>
#include <wx/xrc/xh_aui.h>

#include "tt32App.h"

#include "MainFrm.h"
#include "ChildFrm.h"

#include "FormViewEx.h"
#include "ImportOnlineEntries.h"
#include "ItemCtrl.h"

#include "Profile.h"
#include "InfoSystem.h"

#include "CpListStore.h"
#include "PlListStore.h"
#include "IdStore.h"

#include "Request.h"
#include "Rec.h"
#include "Res.h"

#if __has_include("../Crypt/crypt.h")
#  include "../Crypt/crypt.h"
#else
# include "../Crypt/crypt_template.h"
#endif

static int defaultType  = TT_REGULAR;
static int defaultTable = TT_ITTF;

// Muss hier stehen, weil es sonst nicht compiliert
static const wxString versionNumber = "22.01";
static const wxString version = "Version " + versionNumber;

static wxString licensee = " Christoph Theis";
static wxString copyright = "(C) Christoph Theis 2022";
static wxString expire = "";


wxProgressDialog * CTT32App::m_progressDialog = NULL;


IMPLEMENT_APP(CTT32App)


Profile     ttProfile;
InfoSystem  infoSystem;


CTT32App::~CTT32App()
{
}


bool CTT32App::OnInit()
{
  _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );  

  // Create Mutex fuer Inno Setup
  CreateMutex(NULL, FALSE, wxT("TTM"));

  wxTheApp->SetAppName("TTM");

  static const wxChar ps = wxFileName::GetPathSeparator();
  
  if (!wxApp::OnInit())
    return false;
    
  // INI-File suchen
  wxString iniDir = wxGetCwd();
  
  // Default: aktuelles Verzeichnis
  wxString iniFile = iniDir + ps + TT_PROFILE;
  
  // Wenn nicht: Common App Data
  if (!wxFile::Exists(iniFile))
    iniFile = wxStandardPaths::Get().GetConfigDir() + ps + TT_PROFILE;
    
  // Wenn nicht: User App Data
  if (!wxFile::Exists(iniFile))
    iniFile = wxStandardPaths::Get().GetUserLocalDataDir() + ps + TT_PROFILE;

  // Wenn nicht: User Local App Data
  if (!wxFile::Exists(iniFile))
    iniFile = wxStandardPaths::Get().GetUserLocalDataDir() + ps + TT_PROFILE;

  if (!wxFile::Exists(iniFile))
  {
    infoSystem.Fatal(wxT("Cannot locate \"tt32.ini\" file"));
  }
    
  wxSetWorkingDirectory(iniFile.SubString(0, iniFile.Len() - wxStrlen(TT_PROFILE) - 2));

  // Resourcen bestimmen
  wxString xrcPath = GetResourcesPath();

  if (!wxFile::Exists(xrcPath + "/" TT_XRCFILE))
    infoSystem.Fatal(wxT("Cannot locate resource file \"%S\", exiting"), TT_XRCFILE);

  wxLocale::AddCatalogLookupPathPrefix(xrcPath);

  ttProfile.Open(iniFile.data());  

  int langId = wxLocale::GetSystemLanguage();

  wxString lang = ttProfile.GetString(PRF_GLOBAL_SETTINGS, PRF_SETTINGS_LANGUAGE, wxT("en_US"));
  if (wxLocale::FindLanguageInfo(lang) != NULL)
    langId = wxLocale::FindLanguageInfo(lang)->Language;

  m_locale.Init((wxLanguage) langId, wxLOCALE_DONT_LOAD_DEFAULT);
  m_locale.AddCatalog("ttm");
  m_locale.AddCatalog("wxstd");

#if 0
  // Lizenzueberpruefung
  wxString licenseFileName = wxGetCwd() + ps + "license.ini";

  licensee = "";
  expire = "";

  if (!CheckLicenseCode(licenseFileName))
  {
    // Warnung nur, wenn es kein "last open" gibt:
    // Ansonsten haben Clients, die keine Lizenz brauchen, immer eine Warnung zu Beginn.
    if (ttProfile.GetString(PRF_OPEN, PRF_LAST).IsEmpty())
      infoSystem.Information(_("The license is not valid! You are allowed to use existing tournaments but you cannot create new ones."));
  }
  else if (HasLicenseExpired(licenseFileName))
  {
    infoSystem.Information(
        _("Your license has expired! You are allowed to use existing tournaments but you cannot create new ones."));
  }
  else
  {
    ReadLicense(licenseFileName);
  }
#endif
  
  // Setup von [Raster]
  if (!ttProfile.GetFirstKey(PRF_RASTER))
  {
    ttProfile.AddString(PRF_RASTER, PRF_RASTER_TINY, 
         wxT("Arial, 6, 0, 0, 0, 400, 0, 0, 0"));
    ttProfile.AddString(PRF_RASTER, PRF_RASTER_SMALL, 
         wxT("Arial, 8, 0, 0, 0, 400, 0, 0, 0"));
    ttProfile.AddString(PRF_RASTER, PRF_RASTER_MEDIUM,
         wxT("Arial, 9, 0, 0, 0, 400, 0, 0, 0"));
    ttProfile.AddString(PRF_RASTER, PRF_RASTER_NORMAL,
         wxT("Arial, 10, 0, 0, 0, 400, 0, 0, 0"));
    ttProfile.AddString(PRF_RASTER, PRF_RASTER_PAGE,
         wxT("Arial, 10, 0, 0, 0, 400, 0, 0, 0"));
    ttProfile.AddString(PRF_RASTER, PRF_RASTER_GROUP,
         wxT("Arial, 12, 0, 0, 0, 400, 0, 0, 0"));
    ttProfile.AddString(PRF_RASTER, PRF_RASTER_COMP,
         wxT("Arial, 14, 0, 0, 0, 800, 0, 0, 0"));
    ttProfile.AddString(PRF_RASTER, PRF_RASTER_NORMALB,
         wxT("Arial, 10, 0, 0, 0, 700, 0, 0, 0"));
    ttProfile.AddString(PRF_RASTER, PRF_RASTER_MEDIUMB,
         wxT("Arial, 9, 0, 0, 0, 700, 0, 0, 0"));
    ttProfile.AddString(PRF_RASTER, PRF_RASTER_SMALLB,
         wxT("Arial, 8, 0, 0, 0, 700, 0, 0, 0"));
  }
  
  if (!ttProfile.GetFirstKey(PRF_REPORTS))
  {
    ttProfile.AddString(PRF_REPORTS, wxT("Registration"), _("Registration"));
    // ttProfile.AddString(PRF_REPORTS, wxT("TeamEntries"), _("List of Team Players"));
    ttProfile.AddString(PRF_REPORTS, wxT("PlayerlistPerName"), _("List of Players Sorted by Name"));
    ttProfile.AddString(PRF_REPORTS, wxT("PlayerlistPerNumber"), _("List of Players Sorted by Number"));
    ttProfile.AddString(PRF_REPORTS, wxT("Participants"), _("List of Players with their Events"));
    ttProfile.AddString(PRF_REPORTS, wxT("Entries"), _("Participants per Event"));
    ttProfile.AddString(PRF_REPORTS, wxT("ChampionshipEntries"), _("Participants in Championship"));
    ttProfile.AddString(PRF_REPORTS, wxT("ConsolationEntries"), _("Participants in Consolation"));
    ttProfile.AddString(PRF_REPORTS, wxT("PartnerMissing"), _("Players with Partner Missing"));
  }
  
  if ( wxStrlen(ttProfile.GetString(PRF_REPORTS, wxT("Ranking"))) == 0 )
  {
    ttProfile.DeleteString(PRF_REPORTS, wxT("RankingSingle"));
    ttProfile.DeleteString(PRF_REPORTS, wxT("RankingDouble"));
    ttProfile.DeleteString(PRF_REPORTS, wxT("RankingTeam"));
    
    ttProfile.AddString(PRF_REPORTS, wxT("Ranking"), _("Ranking by Association"));
  }

  if ( wxStrlen(ttProfile.GetString(PRF_REPORTS, wxT("WorldRanking"))) == 0 )  
    ttProfile.AddString(PRF_REPORTS, wxT("WorldRanking"), _("International Ranking"));

  if ( wxStrlen(ttProfile.GetString(PRF_REPORTS, wxT("MatchList"))) == 0 )  
    ttProfile.AddString(PRF_REPORTS, wxT("MatchList"), _("List of Matches"));

  if (wxStrlen(ttProfile.GetString(PRF_REPORTS, wxT("FinalStandings"))) == 0)
    ttProfile.AddString(PRF_REPORTS, wxT("FinalStandings"), _("Final Standings"));

  // Init image handler
  ::wxInitAllImageHandlers();

  // Init der Resourcen    
  wxXmlResource::Get()->InitAllHandlers();
  wxXmlResource::Get()->AddHandler(new CItemCtrlXmlResourceHandler());
  wxXmlResource::Get()->AddHandler(new wxAuiXmlHandler());
  
  wxXmlResource::Get()->Load(xrcPath + "/" TT_XRCFILE);
  
  m_pMainWnd = (CMainFrame *) wxXmlResource::Get()->LoadObject(NULL, "MainFrame", "wxMDIParentFrame");
  
  m_pMainWnd->SetIcon(wxIcon("main"));  

  SetMenuBar("NOMenuBar");

  m_pMainWnd->Maximize(true);
  
  wxInitDialogEvent wxInitDialogEvent_;
  m_pMainWnd->GetEventHandler()->ProcessEvent(wxInitDialogEvent_);
  m_pMainWnd->Show(true);

  OpenLastTournament();

  Connect(IDC_PROGRESSBAR_STEP, wxThreadEventHandler(CTT32App::OnProgressBarStep), NULL, this);
  Connect(IDC_PROGRESSBAR_EXIT, wxThreadEventHandler(CTT32App::OnProgressBarExit), NULL, this);

  return true;
}


int  CTT32App::OnExit()
{
  return 0;
}

// -----------------------------------------------------------------------
class CUpdateViewEvent : public wxCommandEvent
{
  public:
    CUpdateViewEvent(const CRequest &req) 
      : wxCommandEvent(IDC_UPDATEVIEW) 
    {
      SetClientData(new CRequest(req));
    }

    ~CUpdateViewEvent() {delete (CRequest *) GetClientData();}

};

void  CTT32App::NotifyChange(CRequest &req)
{
  if (wxTheApp == nullptr)
    return;

  wxTheApp->QueueEvent(new CUpdateViewEvent(req));
}



void  CTT32App::CommitChanges()
{
}


void  CTT32App::AbortChanges()
{
}


class CTT32Thread : public wxThread
{
  public:
    CTT32Thread(unsigned (*func)(void *arg), void *arg) 
      : wxThread(wxTHREAD_JOINABLE), m_func(func), m_arg(arg)       
    {
      Create();
    }
    
    virtual ExitCode Entry()
    {
      int rc = (*m_func)(m_arg);
      CTT32App::ProgressBarExit(rc);

      return (ExitCode) rc;
    }
    
    void *m_arg;
    unsigned (*m_func)(void *arg);
};


class CProgressDialog : public wxProgressDialog
{
  public:
    CProgressDialog(const wxString &title, long count, wxWindow *parent, wxThread *thread, bool indefinite)
      : wxProgressDialog(title, title, count > 0 ? count : 100, parent), m_thread(thread), m_indefinite(indefinite), m_step(0)
    {
      if (thread)
        thread->Run();

      if (m_indefinite)
      {
        timer.SetOwner(this);
        timer.Start(50);
        Connect(wxEVT_TIMER, wxTimerEventHandler(CProgressDialog::OnTimer), NULL, this);
        Connect(wxEVT_SHOW, wxShowEventHandler(CProgressDialog::OnShow), NULL, this);
      }
    }

   ~CProgressDialog()
    {

    }

  public:
    bool Update(int value, const wxString &msg = wxEmptyString, bool *skip = 0)
    {
      if (!m_indefinite)
        return wxProgressDialog::Update(value, msg, skip);
      else if (!msg.IsEmpty() && msg != GetMessage())
        return wxProgressDialog::Update(0, msg, skip);
      else
        return Pulse();
    }

    bool IsIndefinite() const {return m_indefinite;}

    int  IncrementCounter() { return ++m_step;}
    
  private:
    void OnTimer(wxTimerEvent &evt)
    {
      if (!m_thread || !m_thread->IsAlive())
        timer.Stop();
      else
        Pulse();
    }

    void OnShow(wxShowEvent &evt)
    {
      if (!evt.IsShown())
      {
        if (timer.IsRunning())
          timer.Stop();

        m_thread->Wait();

        delete m_thread;
        m_thread = NULL;
      }
    }

    wxTimer timer;
    wxThread *m_thread;
    bool    m_indefinite;
    int     m_step;
};


bool CTT32App::ProgressBarThread(unsigned (*func)(void * arg), void *arg, 
                                const wxString &buf, long count, bool indefinite)
{
  if (m_progressDialog)
  {
    if (m_progressDialog->IsShown())
      return false;
  }

  delete m_progressDialog;

  m_progressDialog = new CProgressDialog(
      buf, count, CTT32App::instance()->m_pMainWnd, new CTT32Thread(func, arg), indefinite);
  
  return true;
}

// wxProgressDialog::Update ruft DispatchEvent / YieldFor auf, das wiederum die naechsten Events verarbeiten wuerde
// Ausgenommen davon sind Events der Category USER_INPUT aber nicht THREAD. Daher eine eigene Klase bauen,
// die GetEventCategory entsprechend ueberlaedt
class MyThreadEvent : public wxThreadEvent
{
  public:
    MyThreadEvent(wxEventType evtType = wxEVT_THREAD, int id = wxID_ANY) : wxThreadEvent(evtType, id) {}
    wxEventCategory GetEventCategory() const{return wxEVT_CATEGORY_USER_INPUT;}
    wxEvent * Clone() const {return new MyThreadEvent(*this);}
};


// Step progress bar
void  CTT32App::ProgressBarStep()
{
  if (wxTheApp == nullptr)
    return;
  wxGetApp().AddPendingEvent(MyThreadEvent(IDC_PROGRESSBAR_STEP));
}


void CTT32App::ProgressBarExit(int retCode)
{
  if (wxTheApp == nullptr)
    return;
  wxGetApp().AddPendingEvent(MyThreadEvent(IDC_PROGRESSBAR_EXIT));
}

void CTT32App::OnProgressBarStep(wxThreadEvent &)
{
  if (m_progressDialog)
  {
    int val =  ((CProgressDialog *) m_progressDialog)->IncrementCounter();
    if (val <= m_progressDialog->GetRange())
      m_progressDialog->Update(val);
    else
      OutputDebugString(wxT("OnProgressBarStep called to many times"));
  }
}


void CTT32App::SetProgressBarText(const wxString &text)
{
  if (m_progressDialog)
    m_progressDialog->Update(0, text);
}


void CTT32App::OnProgressBarExit(wxThreadEvent &)
{
  if (!m_progressDialog)
    return;

  // Ich loesche m_progressDialog, Destruktor oder Update koennten aber wiederum Exit aufrufen
  wxProgressDialog *tmp = m_progressDialog;
  m_progressDialog = NULL;

  if (tmp)
    tmp->Update(tmp->GetRange());

  delete(tmp);
}


wxPanel * CTT32App::OpenView(const wxString &title, const wxChar *xrcName, ...)
{
  va_list vaList;
  va_start(vaList, xrcName);

  return OpenViewChildFrame("ChildFrame", title, xrcName, vaList);
}


wxPanel * CTT32App::OpenViewNoResize(const wxString &title, const wxChar *xrcName, ...)
{
  va_list vaList;
  va_start(vaList, xrcName);

  return OpenViewChildFrame("ChildFrameNoResize", title, xrcName, vaList);
}


wxPanel * CTT32App::OpenViewChildFrame(const wxString &childFrame, const wxString &title, const wxChar *xrcName, va_list vaList)
{
  CChildFrame *child = (CChildFrame *) wxXmlResource::Get()->LoadObject(m_pMainWnd, childFrame, "wxMDIChildFrame");
      
  CFormViewEx *panel = (CFormViewEx *) wxXmlResource::Get()->LoadPanel(child, xrcName);

  if (!panel)
  {
    delete child;
    return NULL;
  }
  
  child->SetTitle(title);
  child->SetIcon(wxIcon("child"));
  child->SetClientSize(panel->GetSize());
  child->SetSize(panel->GetSize());
  // child->CenterOnParent();

  // panel->RestoreSettings();  

  wxInitDialogEvent wxInitDialogEvent_;
  panel->GetEventHandler()->ProcessEvent(wxInitDialogEvent_);
  
  panel->Edit(vaList);
  
  va_end(vaList);
  
  child->Show();

  return panel;
}


wxPanel * CTT32App::OpenDialog(bool modal, const wxString &title, const wxChar *xrcName, ...)
{
  va_list vaList;
  va_start(vaList, xrcName);
  
  wxDialog *child = new wxDialog(m_pMainWnd, wxID_ANY, title);

  CFormViewEx *panel = (CFormViewEx *) wxXmlResource::Get()->LoadPanel(child, xrcName);
  
  if (!panel)
  {
    delete child;
    return NULL;
  }
  
  child->SetIcon(wxIcon("child"));
  child->SetClientSize(panel->GetSize());
  child->SetSize(panel->GetSize());

  wxInitDialogEvent wxInitDialogEvent_;
  panel->GetEventHandler()->ProcessEvent(wxInitDialogEvent_);
  
  panel->Edit(vaList);
  
  va_end(vaList);
  
  if (modal)
    child->ShowModal();
  else
    child->Show();

  return panel;
}


// -----------------------------------------------------------------------
void CTT32App::ShowHtmlDialog(const wxString &filename)
{
  wxFileName file;
  file.SetFullName(filename);

  // Falls nicht im cwd dann beim Exe suchen (default bei Installation)
  if (!file.Exists())
    file.SetPath(wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath());

  // Falls auch nicht dort, dann aus workspace (aktuellste Version)
  if (!file.Exists())
  {
    // file.SetPath("\\user\\cht\\wxTTM\\src\\Resources");
    wxFileName exe = wxFileName(wxStandardPaths::Get().GetExecutablePath());
    exe.RemoveLastDir();
    exe.RemoveLastDir();
    exe.RemoveLastDir();
    exe.AppendDir("Resources");
    wxString path = exe.GetPath();
    file.SetPath(path);
  }

  if (file.Exists())
    wxLaunchDefaultBrowser(file.GetFullPath());
}

// -----------------------------------------------------------------------
void CTT32App::ShowAboutDialog()
{
  wxAboutDialogInfo info;

  info.SetName(wxT("TTM"));
  info.SetDescription(_T("Table Tennis Manager"));
  info.SetCopyright(copyright);
  info.SetVersion(version);
  info.SetWebSite("http://downloads.ttm.co.at/ttm/changes.html", _("View changes"));

#if 0
  if (!expire.IsEmpty())
  {
    int day  = wxAtoi(expire.c_str()) % 100;
    int mon  = (wxAtoi(expire.c_str()) / 100) % 100;
    int year = wxAtoi(expire.c_str()) / 10000;

    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    tm.tm_year = year - 1900;
    tm.tm_mon  = mon-1;
    tm.tm_mday = day;

    wxString expireStr = wxDateTime(tm).FormatDate();
  
    info.SetLicense(wxString::Format(_("Licensed by %s. License expires %s"), licensee, expireStr.c_str()));
  }
  else
  {
    info.SetLicense(wxString::Format(_("Unlicensed copy")));
  }
#endif

  ::wxAboutBox(info, m_pMainWnd);
}



// -----------------------------------------------------------------------
// Turnierspezifische Einstellungen
bool  CTT32App::GetPrintCombinedScoresheet() const
{
  return ttProfile.GetInt(PRF_LOCAL_SETTINGS, PRF_PRINT_COMBINED_SCORESHEET, 0) ?
    true : false;
}


void  CTT32App::SetPrintCombinedScoresheet(bool set)
{
  ttProfile.AddInt(PRF_LOCAL_SETTINGS, PRF_PRINT_COMBINED_SCORESHEET, set ? 1 : 0);
}


bool  CTT32App::GetPrintKONamesBold() const
{
  return false;
}


int  CTT32App::GetPrintAssociation() const
{
  return ttProfile.GetInt(PRF_LOCAL_SETTINGS, PRF_PRINT_NATION, 1);
}


void  CTT32App::SetPrintAssociation(int set)
{
  ttProfile.AddInt(PRF_LOCAL_SETTINGS, PRF_PRINT_NATION, set);
}


int  CTT32App::GetPrintAssociationTeam() const
{
  return ttProfile.GetInt(PRF_LOCAL_SETTINGS, PRF_PRINT_NATION_TEAM, 1);
}


void  CTT32App::SetPrintAssociationTeam(int set)
{
  ttProfile.AddInt(PRF_LOCAL_SETTINGS, PRF_PRINT_NATION_TEAM, set);
}


int  CTT32App::GetPrintNationNameWidth() const
{
  // -1 means "auto"
  int ret = ttProfile.GetInt(PRF_LOCAL_SETTINGS, PRF_PRINT_NATION_NAME_WIDTH, -2);
  if (ret == -2)
    ret = ttProfile.GetInt(PRF_GLOBAL_SETTINGS, PRF_PRINT_NATION_NAME_WIDTH, -1);

  return ret;
}


void  CTT32App::SetPrintNationNameWidth(int width)
{
  ttProfile.AddInt(PRF_LOCAL_SETTINGS, PRF_PRINT_NATION_NAME_WIDTH, width);
}


int  CTT32App::GetPrintNationDescWidth() const
{
  // -1 means "auto"
  int ret = ttProfile.GetInt(PRF_LOCAL_SETTINGS, PRF_PRINT_NATION_DESC_WIDTH, -2);
  if (ret == -2)
    ret = ttProfile.GetInt(PRF_GLOBAL_SETTINGS, PRF_PRINT_NATION_DESC_WIDTH, -1);

  return ret;
}


void  CTT32App::SetPrintNationDescWidth(int width)
{
  ttProfile.AddInt(PRF_LOCAL_SETTINGS, PRF_PRINT_NATION_DESC_WIDTH, width);
}


int  CTT32App::GetPrintNationRegionWidth() const
{
  // -1 means "auto"
  int ret = ttProfile.GetInt(PRF_LOCAL_SETTINGS, PRF_PRINT_NATION_REGION_WIDTH, -2);
  if (ret == -2)
    ret = ttProfile.GetInt(PRF_GLOBAL_SETTINGS, PRF_PRINT_NATION_REGION_WIDTH, -1);

  return ret;
}


void  CTT32App::SetPrintNationRegionWidth(int width)
{
  ttProfile.AddInt(PRF_LOCAL_SETTINGS, PRF_PRINT_NATION_REGION_WIDTH, width);
}


int  CTT32App::GetPrintTeamNameWidth() const
{
  // -1 means "auto"
  int ret = ttProfile.GetInt(PRF_LOCAL_SETTINGS, PRF_PRINT_TEAM_NAME_WIDTH, -2);
  if (ret == -2)
    ret = ttProfile.GetInt(PRF_GLOBAL_SETTINGS, PRF_PRINT_TEAM_NAME_WIDTH, -1);

  return ret;
}


void  CTT32App::SetPrintTeamNameWidth(int width)
{
  ttProfile.AddInt(PRF_LOCAL_SETTINGS, PRF_PRINT_TEAM_NAME_WIDTH, width);
}


int  CTT32App::GetPrintStartNrWidth() const
{
  // -1 means "auto"
  int ret = ttProfile.GetInt(PRF_LOCAL_SETTINGS, PRF_PRINT_PLNR_WIDTH, -2);
  if (ret == -2) // Undefined, try global
    ret = ttProfile.GetInt(PRF_GLOBAL_SETTINGS, PRF_PRINT_PLNR_WIDTH, -1);

  return ret;
}


void CTT32App::SetPrintStartNrWidth(int width)
{
  ttProfile.AddInt(PRF_LOCAL_SETTINGS, PRF_PRINT_PLNR_WIDTH, width);
}

bool  CTT32App::GetPrintPreview() const
{
  if ( ttProfile.GetInt(PRF_GLOBAL_SETTINGS, PRF_SETTINGS_PREVIEW, 0) )
    return true;

  return false;
}


void CTT32App::SetPrintPreview(bool state) const
{
  ttProfile.AddInt(PRF_GLOBAL_SETTINGS, PRF_SETTINGS_PREVIEW, state ? 1 : 0);
  if (state)
    ttProfile.AddInt(PRF_GLOBAL_SETTINGS, PRF_SETTINGS_PDF, 0);
}


bool  CTT32App::GetPrintPdf() const
{
  if ( ttProfile.GetInt(PRF_GLOBAL_SETTINGS, PRF_SETTINGS_PDF, 0) )
    return true;

  return false;
}


void CTT32App::SetPrintPdf(bool state) const
{
  ttProfile.AddInt(PRF_GLOBAL_SETTINGS, PRF_SETTINGS_PDF, state ? 1 : 0);
  if (state)
    ttProfile.AddInt(PRF_GLOBAL_SETTINGS, PRF_SETTINGS_PREVIEW, 0);
}


double CTT32App::GetPrintCaptionMarginKO() const
{
  return ((double) ttProfile.GetInt(PRF_LOCAL_SETTINGS, PRF_PRINT_CAPTIONMARGINKO, 100)) / 100.;
}


void CTT32App::SetPrintCaptionMarginKO(double margin)
{
  ttProfile.AddInt(PRF_LOCAL_SETTINGS, PRF_PRINT_CAPTIONMARGINKO, std::floor(100. * margin));
}


wxString CTT32App::GetDateFormat() const
{
  wxString format = ttProfile.GetString(PRF_LOCAL_SETTINGS, PRF_SETTINGS_DATESTRING);
  if (format.IsEmpty())
    format = ttProfile.GetString(PRF_GLOBAL_SETTINGS, PRF_SETTINGS_DATESTRING, wxT("%a, %d %b"));  // %d %b %y

  return format;
}


wxString CTT32App::GetTimeFormat() const
{
  wxString format = ttProfile.GetString(PRF_LOCAL_SETTINGS, PRF_SETTINGS_TIMESTRING);
  if (format.IsEmpty())
    format = ttProfile.GetString(PRF_GLOBAL_SETTINGS, PRF_SETTINGS_TIMESTRING, wxT("%H:%M"));

  return format;
}


// -----------------------------------------------------------------------
wxString CTT32App::GetTournament() const
{
  return tournament;
}


wxString CTT32App::GetPath() const
{
  return wxString(ttProfile.GetString(PRF_LOCAL_SETTINGS, PRF_PATH));
}


wxString CTT32App::GetBackupPath() const
{
  return wxString(ttProfile.GetString(PRF_LOCAL_SETTINGS, PRF_SETTINGS_BACKUPPATH));
}


void CTT32App::SetBackupPath(const wxString &path)
{
  ttProfile.AddString(PRF_LOCAL_SETTINGS, PRF_SETTINGS_BACKUPPATH, path);
}


int CTT32App::GetBackupTime() const
{
  return ttProfile.GetInt(PRF_LOCAL_SETTINGS, PRF_SETTINGS_BACKUPTIME, 30);
}


void CTT32App::SetBackupTime(int t)
{
  ttProfile.AddInt(PRF_LOCAL_SETTINGS, PRF_SETTINGS_BACKUPTIME, t);
}


bool CTT32App::GetBackupAppendTimestamp() const
{
  return ttProfile.GetBool(PRF_LOCAL_SETTINGS, PRF_SETTINGS_BACKUPAPPENDTIMESTAMP, false);
}


void CTT32App::SetBackupAppendTimestamp(bool a)
{
  ttProfile.AddBool(PRF_LOCAL_SETTINGS, PRF_SETTINGS_BACKUPAPPENDTIMESTAMP, a);
}


bool CTT32App::GetBackupKeepLast() const
{
  // Make sure at least one if "append timestamp" and "keep last" is set
  return ttProfile.GetBool(PRF_LOCAL_SETTINGS, PRF_SETTINGS_BACKUPKEEPLAST, !GetBackupAppendTimestamp());
}


void CTT32App::SetBackupKeepLast(bool a)
{
  ttProfile.AddBool(PRF_LOCAL_SETTINGS, PRF_SETTINGS_BACKUPKEEPLAST, a);
}


int CTT32App::GetBackupKeepNofItems() const
{
  return ttProfile.GetInt(PRF_LOCAL_SETTINGS, PRF_SETTINGS_BACKUPKEEPNOFITEMS, 0);
}


void CTT32App::SetBackupKeepNofItems(int a)
{
  ttProfile.AddInt(PRF_LOCAL_SETTINGS, PRF_SETTINGS_BACKUPKEEPNOFITEMS, a);
}


void  CTT32App::SetDefaultCP(const wxString &cpName) const
{
  ttProfile.AddString(PRF_LOCAL_SETTINGS, PRF_DEFCP, cpName);
  ((CMainFrame *) m_pMainWnd)->SetDefaultCP(cpName);
}


void  CTT32App::SetDefaultGR(const wxString &grName) const
{
  ttProfile.AddString(PRF_LOCAL_SETTINGS, PRF_DEFGR, grName);
  ((CMainFrame *) m_pMainWnd)->SetDefaultGR(grName);
}


void  CTT32App::SetDefaultNA(const wxString &naName) const
{
  ttProfile.AddString(PRF_LOCAL_SETTINGS, PRF_DEFNA, naName);
  ((CMainFrame *) m_pMainWnd)->SetDefaultNA(naName);
}


wxString CTT32App::GetDefaultCP() const
{
  return ttProfile.GetString(PRF_LOCAL_SETTINGS, PRF_DEFCP);
}


wxString CTT32App::GetDefaultGR() const
{
  return ttProfile.GetString(PRF_LOCAL_SETTINGS, PRF_DEFGR);
}


wxString CTT32App::GetDefaultNA() const
{
  return ttProfile.GetString(PRF_LOCAL_SETTINGS, PRF_DEFNA);
}


void CTT32App::SetType(short type, bool writeDB)
{
  if (type == GetType())
    return;
    
  if (writeDB)
    IdStore::SetType(type);
  ttProfile.AddInt(PRF_LOCAL_SETTINGS, PRF_SETTINGS_TYPE, type);
}

short CTT32App::GetType() const
{
  return ttProfile.GetInt(PRF_LOCAL_SETTINGS, PRF_SETTINGS_TYPE);
}


short CTT32App::GetDefaultType() const
{
  return defaultType;
}


void CTT32App::SetTable(short table, bool writeDB)
{
  if (table == GetTable())
    return;
    
  if (writeDB)
    IdStore::SetTable(table);
  ttProfile.AddInt(PRF_LOCAL_SETTINGS, PRF_SETTINGS_TABLE, table);
}

short CTT32App::GetTable() const
{
  return ttProfile.GetInt(PRF_LOCAL_SETTINGS, PRF_SETTINGS_TABLE);
}


short CTT32App::GetDefaultTable() const
{
  return defaultTable;
}


void  CTT32App::SetReportTitle(const wxString &title, bool writeDB) const
{
  wxString oldTitle = GetReportTitle();
  if (!oldTitle.IsEmpty() && !title.IsEmpty() && !wxStrcoll(oldTitle, title))
    return;
    
  if (writeDB)
    IdStore::SetReportTitle(title);
  ttProfile.AddString(PRF_LOCAL_SETTINGS, PRF_STRTITLE, title);
}


wxString CTT32App::GetReportTitle() const
{
  return ttProfile.GetString(PRF_LOCAL_SETTINGS, PRF_STRTITLE);
}


void  CTT32App::SetReportSubtitle(const wxString &subtitle, bool writeDB) const
{
  wxString oldSubTitle = GetReportSubtitle();
  if (oldSubTitle.IsEmpty() && subtitle.IsEmpty() && !wxStrcoll(oldSubTitle, subtitle))
    return;
    
  if (writeDB)
    IdStore::SetReportSubtitle(subtitle);
  ttProfile.AddString(PRF_LOCAL_SETTINGS, PRF_STRSUBTITLE, subtitle);
}


wxString CTT32App::GetReportSubtitle() const
{
  return ttProfile.GetString(PRF_LOCAL_SETTINGS, PRF_STRSUBTITLE);
}


bool CTT32App::GetPrintBanner() const
{
  return ttProfile.GetInt(PRF_LOCAL_SETTINGS, PRF_SETTINGS_PRINTBANNER, 0) != 0;
}


void CTT32App::SetPrintBanner(bool printBanner) const
{
  ttProfile.AddInt(PRF_LOCAL_SETTINGS, PRF_SETTINGS_PRINTBANNER, printBanner ? 1 : 0);
}


bool CTT32App::GetPrintLogo() const
{
  return ttProfile.GetInt(PRF_LOCAL_SETTINGS, PRF_SETTINGS_PRINTLOGO, 0) != 0;
}


void CTT32App::SetPrintLogo(bool printLogo) const
{
  ttProfile.AddInt(PRF_LOCAL_SETTINGS, PRF_SETTINGS_PRINTLOGO, printLogo? 1 : 0);
}


bool CTT32App::GetPrintSponsor() const
{
  return ttProfile.GetInt(PRF_LOCAL_SETTINGS, PRF_SETTINGS_PRINTSPONSOR, 0) != 0;
}


void CTT32App::SetPrintSponsor(bool printSponsor) const
{
  ttProfile.AddInt(PRF_LOCAL_SETTINGS, PRF_SETTINGS_PRINTSPONSOR, printSponsor ? 1 : 0); 
}


wxString CTT32App::GetPrintLanguage() const
{
  wxString printLang = ttProfile.GetString(PRF_LOCAL_SETTINGS, PRF_SETTINGS_PRINTLANG);
      
  if (!printLang)
    printLang = ttProfile.GetString(PRF_GLOBAL_SETTINGS, PRF_SETTINGS_LANGUAGE);
        
  return printLang;
}


void CTT32App::SetPrintLanguage(const wxString &printLang)
{
  ttProfile.AddString(PRF_LOCAL_SETTINGS, PRF_SETTINGS_PRINTLANG, printLang);
}


bool CTT32App::GetPrintScaleToPaperSize() const
{
  return ttProfile.GetInt(PRF_LOCAL_SETTINGS, PRF_SETTINGS_PRINTSCALEPAPER, 0) != 0;
}


void CTT32App::SetPrintScaleToPaperSize(bool printScale)
{
  ttProfile.AddInt(PRF_LOCAL_SETTINGS, PRF_SETTINGS_PRINTSCALEPAPER, printScale ? 1 : 0);
}


wxString CTT32App::GetLanguage() const
{
  wxString lang = ttProfile.GetString(PRF_GLOBAL_SETTINGS, PRF_SETTINGS_LANGUAGE);
  
  if (!lang)
    lang = wxLocale(wxLocale::GetSystemLanguage()).GetCanonicalName();

  return lang;
}


void CTT32App::SetLanguage(const wxString &lang)
{
  ttProfile.AddString(PRF_GLOBAL_SETTINGS, PRF_SETTINGS_LANGUAGE, lang);
  
  const wxLanguageInfo *info = wxLocale::FindLanguageInfo(lang);

  wxTranslations *trans = new wxTranslations();
  if (info)
    trans->SetLanguage(info->CanonicalName);

  wxTranslations::Set(trans);
}


bool CTT32App::GetPrintPlayersSignature() const
{
  return ttProfile.GetInt(PRF_LOCAL_SETTINGS, PRF_SETTINGS_SCOREPLSIG, 1) != 0;
}


void CTT32App::SetPrintPlayersSignature(bool f, bool writeDB) 
{
  ttProfile.AddInt(PRF_LOCAL_SETTINGS, PRF_SETTINGS_SCOREPLSIG, f ? 1 : 0);

  if (writeDB)
    IdStore::SetPrintPlayersSignature(f);
}


bool CTT32App::GetPrintScoreCoaches() const
{
  return ttProfile.GetInt(PRF_LOCAL_SETTINGS, PRF_SETTINGS_SCORECOACHES, 1) != 0;
}


void CTT32App::SetPrintScoreCoaches(bool f, bool writeDB)
{
  ttProfile.AddInt(PRF_LOCAL_SETTINGS, PRF_SETTINGS_SCORECOACHES, f ? 1 : 0);

  if (writeDB)
    IdStore::SetPrintScoreCoaches(f);
}


bool CTT32App::GetPrintScoreUmpires() const
{
  return ttProfile.GetInt(PRF_LOCAL_SETTINGS, PRF_SETTINGS_SCOREUMPIRES, 1) != 0;
}


void CTT32App::SetPrintScoreUmpires(bool f, bool writeDB)
{
  ttProfile.AddInt(PRF_LOCAL_SETTINGS, PRF_SETTINGS_SCOREUMPIRES, f ? 1 : 0);

  if (writeDB)
    IdStore::SetPrintScoreUmpires(f);
}


bool CTT32App::GetPrintScoreExtras() const
{
  return ttProfile.GetInt(PRF_LOCAL_SETTINGS, PRF_SETTINGS_SCOREEXTRAS, 0) != 0;
}


bool CTT32App::GetPrintScoreUmpireName() const
{
  return ttProfile.GetInt(PRF_LOCAL_SETTINGS, PRF_SETTINGS_SCOREUMPIRENAME, 1) != 0;
}


void CTT32App::SetPrintScoreUmpireName(bool f, bool writeDB)
{
  ttProfile.AddInt(PRF_LOCAL_SETTINGS, PRF_SETTINGS_SCOREUMPIRENAME, f ? 1 : 0);

  if (writeDB)
    IdStore::SetPrintScoreUmpireName(f);
}


void CTT32App::SetPrintScoreExtras(bool f, bool writeDB)
{
  ttProfile.AddInt(PRF_LOCAL_SETTINGS, PRF_SETTINGS_SCOREEXTRAS, f ? 1 : 0);

  if (writeDB)
    IdStore::SetPrintScoreExtras(f);
}


bool CTT32App::GetPrintScoreStartEnd() const
{
  return ttProfile.GetInt(PRF_LOCAL_SETTINGS, PRF_SETTINGS_SCORESTARTENDTIME, 0) != 0;
}


void CTT32App::SetPrintScoreStartEnd(bool f, bool writeDB)
{
  ttProfile.AddInt(PRF_LOCAL_SETTINGS, PRF_SETTINGS_SCORESTARTENDTIME, f ? 1 : 0);

  if (writeDB)
    IdStore::SetPrintScoreRemarks(f);
}


bool CTT32App::GetPrintScoreRemarks() const
{
  return ttProfile.GetInt(PRF_LOCAL_SETTINGS, PRF_SETTINGS_SCOREREMARKS, 0) != 0;
}


void CTT32App::SetPrintScoreRemarks(bool f, bool writeDB)
{
  ttProfile.AddInt(PRF_LOCAL_SETTINGS, PRF_SETTINGS_SCOREREMARKS, f ? 1 : 0);

  if (writeDB)
    IdStore::SetPrintScoreRemarks(f);
}


bool CTT32App::GetPrintScoreServiceTimeout() const
{
  return ttProfile.GetInt(PRF_LOCAL_SETTINGS, PRF_SETTINGS_SCORESERVICETIMEOUT, 0) != 0;
}


void CTT32App::SetPrintScoreServiceTimeout(bool f, bool writeDB)
{
  ttProfile.AddInt(PRF_LOCAL_SETTINGS, PRF_SETTINGS_SCORESERVICETIMEOUT, f ? 1 : 0);

  if (writeDB)
    IdStore::SetPrintScoreServiceTimeout(f);
}


void CTT32App::SetOvFgColor(const wxString &which, const wxColor &color)
{
  wxArrayString colors = wxStringTokenize(ttProfile.GetString(PRF_SETTINGS, PRF_OVLIST_COLORS, ""), ";");

  wxString result;
  bool found = false;

  for (size_t i = 0; i < colors.GetCount(); i++)
  {
    wxString tmp = colors[i];
    if (tmp.StartsWith(which + ":"))
    {
      found = true;
      wxArrayString sa = wxStringTokenize(tmp.Mid(which.Length() + 1), " ");
      result += which;
      result += ":";
      result += ColorToString(color);
      result += " ";
      if (sa.GetCount() > 1)
        result += sa[1];
      else
        result += ColorToString(wxNullColour);
      result += ";";
    }
    else
      result += tmp + ";";
  }

  if (!found)
  {
    result += which;
    result += ":";
    result += color.GetAsString(wxC2S_HTML_SYNTAX);
    result += " ";
    result += ColorToString(wxNullColour);
    result += ";";
  }

  ttProfile.AddString(PRF_SETTINGS, PRF_OVLIST_COLORS, result);
}


wxColor CTT32App::GetOvFgColor(const wxString &which) const
{
  wxArrayString colors = wxStringTokenize(ttProfile.GetString(PRF_SETTINGS, PRF_OVLIST_COLORS, ""), ";");

  wxColor result = wxNullColour;

  for (size_t i = 0; i < colors.GetCount(); i++)
  {
    wxString tmp = colors[i];
    if (tmp.StartsWith(which + ":"))
    {
      wxArrayString sa = wxStringTokenize(tmp.Mid(which.Length() + 1), " ");
      if (sa.GetCount() > 0)
        result = StringToColor(sa[0]);

      break;
    }
  }

  return result;
}


void CTT32App::SetOvBkColor(const wxString &which, const wxColor &color)
{
  wxArrayString colors = wxStringTokenize(ttProfile.GetString(PRF_SETTINGS, PRF_OVLIST_COLORS, ""), ";");

  wxString result;
  bool found = false;

  for (size_t i = 0; i < colors.GetCount(); i++)
  {
    wxString tmp = colors[i];
    if (tmp.StartsWith(which + ":"))
    {
      found = true;
      wxArrayString sa = wxStringTokenize(tmp.Mid(which.Length() + 1), " ");
      result += which;
      result += ":";
      if (sa.GetCount() > 0)
        result += sa[0];
      else
        result += ColorToString(wxNullColour);
      result += " ";
      result += ColorToString(color);
      result += ";";
    }
    else
      result += tmp + ";";
  }

  if (!found)
  {
    result += which;
    result += ":";
    result += ColorToString(wxNullColour);
    result += " ";
    result += ColorToString(color);
    result += ";";
  }

  ttProfile.AddString(PRF_SETTINGS, PRF_OVLIST_COLORS, result);
}


wxColor CTT32App::GetOvBkColor(const wxString &which) const
{
  wxArrayString colors = wxStringTokenize(ttProfile.GetString(PRF_SETTINGS, PRF_OVLIST_COLORS, ""), ";");

  wxColor result = wxNullColour;

  for (size_t i = 0; i < colors.GetCount(); i++)
  {
    wxString tmp = colors[i];
    if (tmp.StartsWith(which + ":"))
    {
      wxArrayString sa = wxStringTokenize(tmp.Mid(which.Length() + 1), " ");
      if (sa.GetCount() > 1)
        result = StringToColor(sa[1]);

      break;
    }
  }

  return result;
}




// -----------------------------------------------------------------------
bool  CTT32App::CreateTournament(
    const wxString &name, 
    const wxString &database, const wxString &server,
    bool windowsAuthentication, const wxString &user, const wxString &passwd,
    short type, short table)
{
#if 0
  if (expire.IsEmpty())
  {
    if (server.IsEmpty() || TTDbse::IsLocalAddress(server))
    {
      infoSystem.Error(_("No valid license found"));
      return false;
    }
  }
#endif

  if (name == "")
  {
    infoSystem.Error(_("Illegal tournament name"));
    return false;
  }
  
  if (database == "")
  {
    infoSystem.Error(_("Illegal database name"));
    return false;
  }

  static const char chars[] = "abcdefghijklmnopqrstvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_";
  
  for (const char *ptr = database.data(); *ptr; ptr++)
  {
    if (strchr(chars, *ptr) == NULL)
    {
      infoSystem.Error(_("Illegal database name"));
      return false;
    }
  }
  
  CloseTournament();
  
  if (!TTDbse::instance()->CreateDatabase(database, server, windowsAuthentication, user, passwd, type, table))
    return false;

  char *tmp = _getcwd(NULL, _MAX_PATH); 
  wxString dir = wxString(tmp) + "\\" + database;
    
  free(tmp);
  
  // Falls es eine remote DB war
  _mkdir(dir.data());

  m_pMainWnd->SetTitle(wxT("TTM - ") + name);

  // Update TT32.ini
  wxString connStr = TTDbse::instance()->GetConnectionString();
  
  ttProfile.AddString(PRF_TURNIER, name.data(), connStr.data());
  ttProfile.AddString(PRF_OPEN, PRF_LAST, name.data());
  ttProfile.AddString(name.data(), PRF_PATH, database.data());
  
  // SetXXX will das Turnier haben.
  tournament = name;
  
  SetMenuBar("ITTFMenuBar");
  
  // Nur in lokaler DB auch in DB schreiben, sonst nur in ini file
  SetType(type, TTDbse::IsLocalAddress(server));
  SetTable(table, TTDbse::IsLocalAddress(server));
  
  ((CMainFrame *) m_pMainWnd)->SetDefaultCP(GetDefaultCP());
  ((CMainFrame *) m_pMainWnd)->SetDefaultGR(GetDefaultGR());
  ((CMainFrame *) m_pMainWnd)->SetDefaultNA(GetDefaultNA());
  
  if (TTDbse::IsLocalAddress(server))
  {
    Connect(wxEVT_TIMER, wxTimerEventHandler(CTT32App::OnTimer), NULL, this);
    m_backupTimer.Start(GetBackupTime() * 60 * 1000, false);
  }
  
  return true;
}


bool  CTT32App::OpenTournament(const wxString &name)
{
  CloseTournament();
  
  wxString tmp = ttProfile.GetString(PRF_TURNIER, name);
  if (tmp.IsEmpty())
    return false;
    
  wxString connStr = tmp;
  wxString server = TTDbse::GetServer(connStr);
  wxString database = TTDbse::GetDatabase(connStr);

  bool throwError = !TTDbse::IsLocalAddress(server);
  throwError |= database.IsEmpty();
  throwError |= _taccess(ttProfile.GetString(name, PRF_PATH).t_str(), 00) != 0;

  short type = 0;
  short table = 0;
  
  if (!TTDbse::instance()->OpenDatabase(connStr, throwError))
  {
    if (throwError)
      return false;
      
    // Nochmals lesen, tmp ist eine statische Variable in ttProfile
    tmp = ttProfile.GetString(PRF_TURNIER, name);

    type = ttProfile.GetInt(name, PRF_SETTINGS_TYPE);
    table = ttProfile.GetInt(name, PRF_SETTINGS_TABLE);

    bool trustedConn = TTDbse::GetWindowsAuthentication(connStr);
    wxString user = trustedConn ? TTDbse::GetUser(connStr).c_str() : wxEmptyString;
    wxString pwd = trustedConn ? TTDbse::GetPassword(connStr).c_str() : wxEmptyString;

    TTDbse::instance()->DetachDatabase(connStr, false);
    if (!TTDbse::instance()->CreateDatabase(database, server, trustedConn, user, pwd, type, table))
      return false;
  }

  m_pMainWnd->SetTitle(wxT("TTM - ") + name);

  ttProfile.AddString(PRF_OPEN, PRF_LAST, name);

  tournament = name;
  
  type = IdStore::GetType();
  table = IdStore::GetTable();
  
  if (table == 0)
  {
    // Tabelle steht noch nicht in der DB
    if (ttProfile.GetInt(PRF_LOCAL_SETTINGS, PRF_SETTINGS_TABLE) == -1)
    {
      // Auch nicht im Profile, also ermitteln
      CpListStore cp;
      cp.SelectAll();
      cp.Next();
      cp.Close();
      
      short newType = defaultType;
      if (cp.cpYear == 0)
        newType = defaultType;
      else if (cp.cpYear < 1970)
        newType = TT_SCI;
      else if (cp.cpYear > 1990)
        newType = TT_YOUTH;
        
      wxString oldType = ttProfile.GetString(PRF_LOCAL_SETTINGS, PRF_SETTINGS_TYPE);
      if (!oldType)
      {
        IdStore::SetType(newType);
        IdStore::SetTable(defaultTable);
      }
      else if (!wxStrcmp(wxT("SCI"), oldType))
      {
        IdStore::SetType(newType);
        IdStore::SetTable(TT_ITTF);
      }
      else if (!wxStrcmp(wxT("DTTB"), oldType))
      {
        IdStore::SetType(newType);
        IdStore::SetTable(TT_DTTB);
      }
      else
      {      
        IdStore::SetType(newType);
        IdStore::SetTable(TT_ITTF);
      }
      
      IdStore::SetReportTitle(GetReportTitle());
      IdStore::SetReportSubtitle(GetReportSubtitle());
    }
  }
  
  // Intern cachen, ohne in die DB zu schreiben
  SetType(IdStore::GetType(), false);
  SetTable(IdStore::GetTable(), false);
  
  SetReportTitle(IdStore::GetReportTitle().data(), false);
  SetReportSubtitle(IdStore::GetReportSubtitle().data(), false);

  SetPrintScoreExtras(IdStore::GetPrintScoreExtras(), false);
  SetPrintPlayersSignature(IdStore::GetPrintPlayersSignature(), false);
  SetPrintScoreRemarks(IdStore::GetPrintScoreRemarks(), false);
  SetPrintScoreCoaches(IdStore::GetPrintScoreCoaches(), false);
  SetPrintScoreUmpires(IdStore::GetPrintScoreUmpires(), false);
  SetPrintScoreServiceTimeout(IdStore::GetPrintScoreServiceTimeout(), false);

  SetMenuBar("ITTFMenuBar");
  
  ((CMainFrame *) m_pMainWnd)->SetDefaultCP(GetDefaultCP());
  ((CMainFrame *) m_pMainWnd)->SetDefaultGR(GetDefaultGR());
  ((CMainFrame *) m_pMainWnd)->SetDefaultNA(GetDefaultNA());
  
  if (connStr.find(wxString("localhost")) != wxString::npos)
  {
    Connect(wxEVT_TIMER, wxTimerEventHandler(CTT32App::OnTimer), NULL, this);
    m_backupTimer.Start(GetBackupTime() * 60 * 1000, false);
  }
  
  return true;
}


bool  CTT32App::OpenLastTournament()
{
  wxString tour = ttProfile.GetString(PRF_OPEN, PRF_LAST);
  if (!tour.IsEmpty())
    return OpenTournament(tour);

  return false;
}


bool  CTT32App::CloseTournament()
{  
  wxWindowList list = m_pMainWnd->GetChildren();
  for (wxWindowList::iterator it = list.begin(); it != list.end(); it++)
    (*it)->Close();

  m_pMainWnd->SetTitle(wxT("TTM"));

  // ttProfile.DeleteString(PRF_OPEN, PRF_LAST);

  tournament = "";

  SetMenuBar("NOMenuBar");

  if (m_backupTimer.IsRunning())
  {
    Disconnect(wxEVT_TIMER, wxTimerEventHandler(CTT32App::OnTimer), NULL, this);
    m_backupTimer.Stop();
  }

  TTDbse::instance()->CloseDatabase();
  
  return true;
}


bool  CTT32App::DetachTournament(const wxString &name, bool deleteDir)
{
  wxString tmp = ttProfile.GetString(PRF_TURNIER, name.data());
  if (!tmp)
    return false;
    
  if (name == tournament)
    CloseTournament();
  
  wxString connStr = tmp;

  // Wenn es eine lokale DB ist, im Server austragen.
  if (TTDbse::IsLocalAddress(TTDbse::GetServer(connStr)))
  {
    // Im Fehlerfall das ini-File belassen
    if (!TTDbse::instance()->DetachDatabase(connStr))
      return false;
  }

  if (deleteDir)
  {
    wxString dir = ttProfile.GetString(name, PRF_PATH);

    if (!dir.IsEmpty())
      wxFileName::Rmdir(dir, wxPATH_RMDIR_RECURSIVE);

    ttProfile.DeleteSection(name);
  }

  ttProfile.DeleteString(PRF_TURNIER, name);

  return true;
}


void CTT32App::ImportOnlineEntries()
{
  CImportOnlineEntries::Import();
}


// -----------------------------------------------------------------------
void CTT32App::BackupDatabase()
{
  time_t t = time(NULL);
  struct tm *tm = localtime(&t);
  
  wxChar tmp[16];
  wxSprintf(tmp, "-%04d%02d%02dT%02d%02d", 
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
            tm->tm_hour, tm->tm_min);
          
  wxString name = GetDatabase() + tmp + ".bak";
  
  wxFileDialog fileDlg(
      m_pMainWnd, _("Backup Database"), GetPath(), name, 
      "Backup Files (*.bak)|*.bak|All Files (*.*)|*.*||", wxFD_SAVE);
    
  if (fileDlg.ShowModal() != wxID_OK)
    return;
    
  name = fileDlg.GetPath();

  wxFileName saveName(name);      // Filename fuer Backup
  wxFileName tmpName(name);       // Filename, wie er an die DB uebergeben wird
  wxFileName dbPath(GetPath());   // Pfad, wo das DB-File liegt

  saveName.Normalize();
  tmpName.Normalize();
  dbPath.Normalize();

  // dbPath ist ein Verzeichnis, GetFullPath also der komplette Name
  // tmpName ist ein Dateiname, GetPath also das Verzeichnis ohne Dateiname und -erweiterung
  if (tmpName.GetPath() != dbPath.GetFullPath())
  {
    tmpName = wxFileName(GetPath(), wxString("tmp-") + fileDlg.GetFilename()).GetFullPath();
    tmpName.Normalize();
  }
  
  m_pMainWnd->SetCursor(wxCURSOR_WAIT);
  bool res = TTDbse::instance()->BackupDatabase(tmpName.GetFullPath());

  // Falls eine temp. Kopie im DB-Verzeichnis gemacht wurde
  if (res && tmpName != saveName)
  {
    wxCopyFile(tmpName.GetFullPath(), saveName.GetFullPath());
    wxRemoveFile(tmpName.GetFullPath());
  }

  // Ebenfalls Kopie im alternativen Verzeichnis
  wxString backupPath = GetBackupPath();
  if (!backupPath.IsEmpty() && backupPath != saveName.GetPath())
  {
    wxFileName fn(backupPath, saveName.GetFullName());
    if (fn.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL))
      wxCopyFile(saveName.GetFullPath(), fn.GetFullPath(), true);
  }


  m_pMainWnd->SetCursor(wxCURSOR_DEFAULT);
  
  if (res)
    infoSystem.Information(_("Backup successful"));
}


void CTT32App::RestoreDatabase()
{
  wxString name = GetDatabase() + ".bak";
  wxFileName tmpName(GetPath(), name);
  tmpName.Normalize();
  
  wxFileDialog fileDlg(
      m_pMainWnd, _("Backup Database"), GetPath(), name, 
      "Backup Files (*.bak)|*.bak|All Files (*.*)|*.*||", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    

  if (fileDlg.ShowModal() != wxID_OK)
    return;
    
  name = fileDlg.GetPath();

  if (name != tmpName.GetFullPath())
  {
    tmpName = wxFileName(GetPath(), wxString("tmp-") + fileDlg.GetFilename());
    tmpName.Normalize();

    wxCopyFile(name, tmpName.GetFullPath());
  }
  
  m_pMainWnd->SetCursor(wxCURSOR_WAIT);
  bool res = TTDbse::instance()->RestoreDatabase(tmpName.GetFullPath(), GetPath());

  wxRemoveFile(tmpName.GetFullPath());

  m_pMainWnd->SetCursor(wxCURSOR_DEFAULT);
  
  if (res)
    infoSystem.Information(_("Restore successful"));
}


// -----------------------------------------------------------------------
void CTT32App::OnTimer(wxTimerEvent &evt)
{
  wxChar tmp[16] = {0};

  if (GetBackupAppendTimestamp())
  {
    // Zeitstempel anhaengen, wenn so gewuenscht
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    wxSprintf(tmp, "-%04d%02d%02dT%02d%02d",
      tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
      tm->tm_hour, tm->tm_min);
  }

  wxChar ps = wxFileName::GetPathSeparator();

  wxString path = GetPath();

  if (!wxFileName(path).IsAbsolute())
    path = wxGetCwd() + ps + GetPath();

  // move <database>.<n>.bak to <database>.<n+1>.bak
  if (CTT32App::GetBackupKeepLast() && CTT32App::instance()->GetBackupKeepNofItems() > 0)
  {
    std::vector<wxString> pathes;
    pathes.push_back(path);
    if (!GetBackupPath().IsEmpty())
      pathes.push_back(GetBackupPath());

    for (auto &path : pathes)
    {
      // Numerierung beginnt bei 0
      int nof = CTT32App::GetBackupKeepNofItems() - 1;

      {
        // Remove last file
        wxString ext = wxString::Format(".%d.bak", nof);
        wxFileName fn(path, GetDatabase() + tmp + ext);
        if (fn.Exists())
          wxRemoveFile(fn.GetFullPath());
      }

      while (nof--)
      {
        // Move files to next number
        wxString extf = wxString::Format(".%d.bak", nof);
        wxString extt = wxString::Format(".%d.bak", nof + 1);
        wxFileName fnf(path, GetDatabase() + tmp + extf);
        wxFileName fnt(path, GetDatabase() + tmp + extt);

        if (fnf.Exists())
          wxRenameFile(fnf.GetFullPath(), fnt.GetFullPath(), true);
      }

      {
        // Move current file to first number
        wxFileName fnf(path, GetDatabase() + tmp + ".bak");
        wxFileName fnt(path, GetDatabase() + tmp + ".0.bak");

        if (fnf.Exists())
          wxRenameFile(fnf.GetFullPath(), fnt.GetFullPath(), true);
      }
    }
  }

  // Backup Database
  m_pMainWnd->SetCursor(wxCURSOR_WAIT);
  TTDbse::instance()->BackupDatabase(wxFileName(path, GetDatabase() + tmp + ".bak").GetFullPath());
  m_pMainWnd->SetCursor(wxCURSOR_DEFAULT);

  wxString backupPath = GetBackupPath();
  if (!backupPath.IsEmpty())
  {
    wxFileName fn(backupPath, GetDatabase() + tmp + ".bak");
    if (fn.Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL))
      wxCopyFile(wxFileName(path, GetDatabase() + tmp + ".bak").GetFullPath(), fn.GetFullPath(), true);
  }
}


// -----------------------------------------------------------------------
wxString CTT32App::GetResourcesPath() const
{
  wxFileName file;
  file.SetFullName(TT_XRCFILE);

  // Falls nicht im cwd dann beim Exe suchen (default bei Installation)
  if (!file.Exists())
    file.SetPath(wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath());

  // Falls auch nicht dort, dann aus workspace (aktuellste Version)
  if (!file.Exists())
  {
    // file.SetPath("\\user\\cht\\wxTTM\\src\\Resources");
    wxFileName exe = wxFileName(wxStandardPaths::Get().GetExecutablePath());
    exe.RemoveLastDir();
    exe.RemoveLastDir();
    exe.RemoveLastDir();
    exe.AppendDir("Resources");
    wxString path = exe.GetPath();
    file.SetPath(path);
  }

  if (file.Exists())
    return file.GetPath();

  return "";
}


// -----------------------------------------------------------------------
bool CTT32App::IsLicenseValid() const
{
#if 0
  wxString name = wxGetCwd() + wxFileName::GetPathSeparator() + "License.ini";

  return CheckLicenseCode(name) && !HasLicenseExpired(name);
#else
  return true;
#endif
}


bool CTT32App::CheckLicenseCode(const wxString &name) const
{
#if 0
  char tmpLicensee[256];
  char tmpExpires[256];
  char buffer[512];
  char code[64];

  GetPrivateProfileStringA("License", "licensee", "", tmpLicensee, sizeof(tmpLicensee), name);
  GetPrivateProfileStringA("License", "expires", "", tmpExpires, sizeof(tmpExpires), name);
  GetPrivateProfileStringA("License", "code", "", code, sizeof(code), name);

  strcpy(buffer, tmpLicensee);
  strcat(buffer, tmpExpires);

  long tmp = crypt(buffer);

  return (tmp == atol(code));
#else
  return true;
#endif
}


bool CTT32App::HasLicenseExpired(const wxString &name) const
{
#if 0
  char tmpExpires[256];

  GetPrivateProfileStringA("License", "expires", "", tmpExpires, sizeof(tmpExpires), name);

  wxString exp = tmpExpires;

  if (exp.IsEmpty())
    return true;

   time_t t = time(0);
  struct tm *tm = localtime(&t);

  int day  = wxAtoi(exp.c_str()) % 100;
  int mon  = (wxAtoi(exp.c_str()) / 100) % 100;
  int year = wxAtoi(exp.c_str()) / 10000;
  
  if (tm->tm_year + 1900 > year)
    return true;
  else if (tm->tm_year + 1900 < year)
    return false;

  if (tm->tm_mon + 1 > mon)
    return true;
  else if (tm->tm_mon + 1 < mon)
    return false;

  if (tm->tm_mday > day)
    return true;

  return false;
#else
  return false;
#endif
}


// -----------------------------------------------------------------------
void CTT32App::CheckForUpdate()
{
  wxURL url("http://downloads.ttm.co.at/ttm/current.txt");

  if (url.GetError() == wxURL_NOERR)
  {
      wxString newVersionNumnber;
      wxInputStream *in = url.GetInputStream();

      if (in && in->IsOk())
      {
          wxStringOutputStream sos(&newVersionNumnber);
          in->Read(sos);

          newVersionNumnber.Trim();

          if (newVersionNumnber > versionNumber)
          {
            wxDialog * dlg = wxXmlResource::Get()->LoadDialog(m_pMainWnd, "UpdateAvailable");

            // Aus seltsamen Gruenden hat der Close-Button nicht die "affirmative ID"
            dlg->FindWindow("Close")->SetId(dlg->GetAffirmativeId());

            dlg->ShowModal();
          }
          else
          {
            infoSystem.Information(_("You are using the latest version of TTM"));
          }

          return;
      }
  }

  // Irgend ein Fehler ist aufgetreten
  infoSystem.Error(_("Could not determine the current version of TTM"));
}


// -----------------------------------------------------------------------
void CTT32App::InstallLicense()
{
  wxString name = "License.ini";

  wxFileDialog fileDlg(
    m_pMainWnd, _("Install License"), wxStandardPaths::Get().GetDocumentsDir(), name, 
      "License Files (*.ini)|*.ini|All Files (*.*)|*.*||", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    
  if (fileDlg.ShowModal() != wxID_OK)
    return;
    
  name = fileDlg.GetPath();

  if (!CheckLicenseCode(name))
  {
    infoSystem.Error(_("The license is not valid"));
  }
  else if (HasLicenseExpired(name))
  {
    infoSystem.Error(_("The license has expired"));
  }
  else if (!::wxCopyFile(name, "License.ini"))
  {
    infoSystem.Error(_("Could not copy license file"));
  }
  else
  {
    infoSystem.Information(_("License file was copied"));
    ReadLicense(wxGetCwd() + wxFileName::GetPathSeparator() + "License.ini");
  }
}


void CTT32App::ReadLicense(const wxString &name)
{

  // Check License file
  // Wenn die Lizenz nicht gueltig ist, gelten die hart codierten defaults fuer
  // Licensee und expire
  if (wxFile::Exists(name))
  {
    char tmpLicensee[256];
    char tmpExpires[256];
    char buffer[512];
    char code[64];

    GetPrivateProfileStringA("License", "licensee", "", tmpLicensee, sizeof(tmpLicensee), name);
    GetPrivateProfileStringA("License", "expires", "", tmpExpires, sizeof(tmpExpires), name);
    GetPrivateProfileStringA("License", "code", "", code, sizeof(code), name);

    strcpy(buffer, tmpLicensee);
    strcat(buffer, tmpExpires);

    if (crypt(buffer) == atol(code))
    {
      licensee = tmpLicensee;
      expire  = tmpExpires;

      defaultType = GetPrivateProfileIntA("Defaults", "Type", TT_REGULAR, name);
      defaultTable = GetPrivateProfileIntA("Defaults", "Table", -1, name);
      if (defaultTable == -1)
        defaultTable = GetPrivateProfileIntA("Defaults", "TableMode", -1, name);
      if (defaultTable == -1)
        defaultTable = GetPrivateProfileIntA("Defaults", "GroupMode", -1, name);
      if (defaultTable == -1)
        defaultTable = TT_ITTF;
    }
    else
    {
      licensee = "";
      expire = "";
    }
  }
  else
  {
    licensee = "";
    expire = "";
  }
}


wxString CTT32App::ColorToString(const wxColor &color) const 
{
  if (color == wxNullColour || !color.IsOk())
    return "default";

  return color.GetAsString(wxC2S_HTML_SYNTAX);
}


wxColor CTT32App::StringToColor(const wxString &name) const
{
  if (name == "default")
    return wxNullColour;
  else
    return wxColor(name);
}


void CTT32App::SetMenuBar(const wxString &menuBar)
{
  // m_pMainWnd->SetWindowMenu(NULL);
  m_pMainWnd->SetMenuBar(wxXmlResource::Get()->LoadMenuBar(menuBar));
}
