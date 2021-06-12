/* Copyright (C) 2020 Christoph Theis */

#ifndef TT32APP_H_
#define TT32APP_H_

#include <wx/wx.h>

class CTT32App;
class Profile;
class InfoSystem;
class CRequest;

class wxProgressDialog;

extern Profile ttProfile;
extern InfoSystem infoSystem;

DECLARE_APP(CTT32App);

class CTT32App : public wxApp
{
  public:
    static CTT32App * instance() {return &wxGetApp();}
    
    // Sammelt Aenderungrequests und verschickt Message an sich
    static void  NotifyChange(CRequest &req);

    // Benachrichtigt die Views
    static void  CommitChanges();
    // Entfernt die Aenderungen
    static void  AbortChanges();

    static bool ProgressBarThread(unsigned (*func)(void * arg), void *arg, 
                                  const wxString &buf, long count, bool indefinite = false);
    // Verschickt eine Message fuer UWM_PROGRESS_STEP, ...
    static void  ProgressBarStep();
    
    // Stoppt den ProgressBar
    static void  ProgressBarExit(int retCode);

    static void SetProgressBarText(const wxString &);
    
  private:
    static wxProgressDialog * m_progressDialog;
    
  public:
   ~CTT32App();

  public:
    // xrcName muss "wxChar *" bleiben, weil sonst va_list nicht funktioniert
    wxPanel *  OpenView(const wxString &title, const wxChar *xrcName, ...);
    wxPanel *  OpenViewNoResize(const wxString &title, const wxChar *xrcName, ...);

    wxPanel *  OpenDialog(bool modal, const wxString &title, const wxChar *xrcName, ...);

    void  ShowHtmlDialog(const wxString &);

    void  ShowAboutDialog();
  
	public:
	  virtual bool OnInit();
	  
	  virtual int  OnExit();
	  
// Einstellungen in der Turnierspezifischen INI-Datei
  public:
    bool  GetPrintCombinedScoresheet() const;
    void  SetPrintCombinedScoresheet(bool set);
    bool  GetPrintKONamesBold() const;
    void  SetPrintKONamesBold(bool set);
    int   GetPrintAssociation() const;
    int   GetPrintAssociationTeam() const;
    void  SetPrintAssociation(int set);
    void  SetPrintAssociationTeam(int set);
    int   GetPrintNationNameWidth() const;
    void  SetPrintNationNameWidth(int width);
    int   GetPrintNationDescWidth() const;
    void  SetPrintNationDescWidth(int width);
    int   GetPrintNationRegionWidth() const;
    void  SetPrintNationRegionWidth(int width);
    int   GetPrintTeamNameWidth() const;
    void  SetPrintTeamNameWidth(int width);
    int   GetPrintStartNrWidth() const;
    void  SetPrintStartNrWidth(int width);
    bool  GetPrintPreview() const;
    void  SetPrintPreview(bool state) const;
    bool  GetPrintPdf() const;
    void  SetPrintPdf(bool state) const;

    double GetPrintCaptionMarginKO() const;
    void   SetPrintCaptionMarginKO(double margin);
    
    void  SetType(short type, bool writeDB = true);
    short GetType() const;
    short GetDefaultType() const;
    
    void  SetTable(short table, bool writeDB = true);
    short GetTable() const;
    short GetDefaultTable() const;
    
    void  SetReportTitle(const wxString &, bool writeDB = true) const;
    wxString GetReportTitle() const;
    
    void  SetReportSubtitle(const wxString &, bool writeDB = true) const;
    wxString GetReportSubtitle() const;

    wxString GetDateFormat() const;
    wxString GetTimeFormat() const;

    wxString GetTournament() const;
    wxString GetPath() const;
    wxString GetDatabase() const {return GetPath();}

    wxString GetBackupPath() const;
    void     SetBackupPath(const wxString &);

    int      GetBackupTime() const;
    void     SetBackupTime(int t);

    bool     GetBackupAppendTimestamp() const;
    void     SetBackupAppendTimestamp(bool);

    bool     GetBackupKeepLast() const;
    void     SetBackupKeepLast(bool);

    int    GetBackupKeepNofItems() const;
    void     SetBackupKeepNofItems(int);
    
    bool     GetPrintBanner() const;
    void     SetPrintBanner(bool) const;

    bool     GetPrintLogo() const;
    void     SetPrintLogo(bool) const;
    
    bool     GetPrintSponsor() const;
    void     SetPrintSponsor(bool) const;

    void  SetDefaultCP(const wxString &) const;
    void  SetDefaultGR(const wxString &) const;
    void  SetDefaultNA(const wxString &) const;

    wxString GetDefaultCP() const;
    wxString GetDefaultGR() const;
    wxString GetDefaultNA() const;
  
    wxString GetPrintLanguage() const;
    void SetPrintLanguage(const wxString &);

    bool GetPrintScaleToPaperSize() const;
    void SetPrintScaleToPaperSize(bool);
    
    wxString GetLanguage() const;
    void SetLanguage(const wxString &);

    bool GetPrintScoreExtras() const;
    void SetPrintScoreExtras(bool, bool writeDB = true);

    bool GetPrintPlayersSignature() const;
    void SetPrintPlayersSignature(bool, bool writeDB = true);

    bool GetPrintScoreStartEnd() const;
    void SetPrintScoreStartEnd(bool, bool writeDB = true);

    bool GetPrintScoreRemarks() const;
    void SetPrintScoreRemarks(bool, bool writeDB = true);

    bool GetPrintScoreCoaches() const;
    void SetPrintScoreCoaches(bool, bool writeDB = true);

    bool GetPrintScoreUmpires() const;
    void SetPrintScoreUmpires(bool, bool writeDB = true);

    bool GetPrintScoreUmpireName() const;
    void SetPrintScoreUmpireName(bool, bool writeDB = true);

    bool GetPrintScoreServiceTimeout() const;
    void SetPrintScoreServiceTimeout(bool, bool writeDB = true);

    void SetOvFgColor(const wxString &which, const wxColor &color);
    wxColor GetOvFgColor(const wxString &which) const;

    void SetOvBkColor(const wxString &which, const wxColor &color);
    wxColor GetOvBkColor(const wxString &which) const;

    wxString GetResourcesPath() const;

    bool IsLicenseValid() const;

  public:
    bool  CreateTournament(const wxString &tour, const wxString &dbse, 
                           const wxString &server, bool windowsAuthentication,
                           const wxString &user, const wxString &passwd,
                           short type, short table);
    bool  OpenTournament(const wxString &tour);
    bool  OpenLastTournament();
    bool  CloseTournament();
    bool  DetachTournament(const wxString &tour, bool deleteDir);
    
    void  BackupDatabase();
    void  RestoreDatabase();

    void  CheckForUpdate();
    void  InstallLicense();

    void  ImportOnlineEntries();

  private:
    void  OnTimer(wxTimerEvent &);
    void  OnProgressBarStep(wxThreadEvent &);
    void  OnProgressBarExit(wxThreadEvent &);

  private:
    bool  CheckLicenseCode(const wxString &name) const;
    bool  HasLicenseExpired(const wxString &name) const;
    void  ReadLicense(const wxString &name);

    wxColor StringToColor(const wxString &name) const;
    wxString ColorToString(const wxColor &color) const;

    void SetMenuBar(const wxString &menBar);

    wxPanel *  OpenViewChildFrame(const wxString &childFrame, const wxString &title, const wxChar *xrcName, va_list vaList);

	private:		  
	  // wxMenuBar *m_noMenuBar;
	  // wxMenuBar *m_ittfMenuBar;
	  wxMDIParentFrame * m_pMainWnd;
	  
	  wxString tournament;

    wxLocale   m_locale;

    wxTimer    m_backupTimer;
};

#endif