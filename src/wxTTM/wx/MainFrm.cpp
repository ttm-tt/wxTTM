/* Copyright (C) 2020 Christoph Theis */

#include "stdafx.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "FormViewEx.h"

#include "TT32App.h"
#include "Rec.h"
#include "Printer.h"
#include "Profile.h"

#include "Dialogs/TTNewDlg.h"
#include "Dialogs/TTOpenDlg.h"
#include "Dialogs/TTDetachDlg.h"

// Import / Export
#include "CpStore.h"
#include "GrStore.h"
#include "NaStore.h"
#include "PlStore.h"
#include "RpStore.h"
#include "LtStore.h"
#include "NmStore.h"
#include "StStore.h"
#include "MtStore.h"
#include "UpStore.h"

#include <fstream>


IMPLEMENT_DYNAMIC_CLASS(CMainFrame, wxMDIParentFrame)

BEGIN_EVENT_TABLE(CMainFrame, wxMDIParentFrame)
  EVT_INIT_DIALOG(CMainFrame::OnInitDialog)
  EVT_MENU(XRCID("TournamentNewMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("TournamentOpenMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("TournamentCloseMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("TournamentDetachMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("TournamentSettingsMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("TournamentBackupMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("TournamentRestoreMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("TournamentCancelDoubleMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("TournamentSQLEditorMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(wxID_EXIT, CMainFrame::OnMenuCommand)

  EVT_MENU(XRCID("TournamentImportEventsMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("TournamentImportGroupsMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("TournamentImportAssocMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("TournamentImportUmpiresMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("TournamentImportPlayersMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("TournamentImportRankingpointsMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("TournamentImportEntriesMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("TournamentImportPositionsMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("TournamentImportSchedulesMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("TournamentImportLineupsMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("TournamentImportResultsMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("TournamentImportOnlineEntriesMenuItem"), CMainFrame::OnMenuCommand)
  
  EVT_MENU(XRCID("TournamentExportEventsMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("TournamentExportGroupsMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("TournamentExportAssocMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("TournamentExportUmpiresMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("TournamentExportPlayersMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("TournamentExportRankingpointsMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("TournamentExportEntriesMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("TournamentExportPositionsMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("TournamentExportSchedulesMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("TournamentExportLineupsMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("TournamentExportResultsMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("TournamentExportForRankingMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("TournamentExportForTTMMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("TournamentExportForITTFMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("TournamentExportForETTUMenuItem"), CMainFrame::OnMenuCommand)

  // EVT_MENU(XRCID("TournamentRemoveDoublesMenuItem"), CMainFrame::OnMenuCommand)

  EVT_MENU(XRCID("CompetitionTeamSystemMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("CompetitionGroupModusMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("CompetitionMatchPointsMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("CompetitionEventMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("CompetitionGroupMenuItem"), CMainFrame::OnMenuCommand)
  
  EVT_MENU(XRCID("RegistrationAssociationsMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("RegistrationTeamsMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("RegistrationPlayersMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("RegistrationUmpiresMenuItem"), CMainFrame::OnMenuCommand)
  
  EVT_MENU(XRCID("DrawSeedingMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("DrawRankingMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("DrawQualificationMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("DrawChampionshipMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("DrawConsolationMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("DrawFromQualificationMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("DrawGroupsFromQualificationMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("DrawFromKOMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("DrawCSDMenuItem"), CMainFrame::OnMenuCommand)
  
  EVT_MENU(XRCID("MatchTimeMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("MatchResultMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("MatchNumberMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("MatchOverviewMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("MatchUnscheduledMenuItem"), CMainFrame::OnMenuCommand)
  
  EVT_MENU(XRCID("PrintPreviewMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("PrintPdfMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("PrintReportsMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("PrintResultsFormsMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("PrintFinishedMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("PrintManageOptionsMenuItem"), CMainFrame::OnMenuCommand)

  EVT_MENU(XRCID("HelpInstallLicenseMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("HelpCheckForUpdateMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("HelpShortcutsMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(XRCID("HelpThirdPartyMenuItem"), CMainFrame::OnMenuCommand)
  EVT_MENU(wxID_ABOUT, CMainFrame::OnMenuCommand)
  
  EVT_UPDATE_UI(XRCID("PrintPreviewMenuItem"), CMainFrame::OnUpdateUI)  
  EVT_UPDATE_UI(XRCID("PrintPdfMenuItem"), CMainFrame::OnUpdateUI)  

  // <ESC> kann ich nicht im wxMDIChild abfangen, der KeyboardHook
  // arbeitet nur im wxMIDParent. Darum hier den Hook setzen und 
  // an das child weitergeben.
END_EVENT_TABLE()


// -----------------------------------------------------------------------
//
CMainFrame::CMainFrame() : wxMDIParentFrame(), m_statusBar(0)
{
}

// -----------------------------------------------------------------------
void CMainFrame::SetFindString(const wxString &str)
{
  if (str.Length())
    m_statusBar->SetStatusText(_("Search text:") + " " + str);
  else
    m_statusBar->SetStatusText(_("Ready"));
}


// -----------------------------------------------------------------------
void CMainFrame::OnInitDialog(wxInitDialogEvent &evt)
{
  wxMDIParentFrame::OnInitDialog(evt);
  
  wxScreenDC dc;
  wxFont font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
  dc.SetFont(font);
  int cW = dc.GetTextExtent("M").GetWidth();
  
  // Init status bar
  
  static const int widths[] = {-1, 5 * cW, 5 * cW, 5 * cW};  
  m_statusBar = (wxStatusBar *) FindWindow("Statusbar"); 
  if (m_statusBar == NULL)
    SetStatusBar(m_statusBar = new wxStatusBar(this));
  m_statusBar->SetFieldsCount(4, widths);
  m_statusBar->SetStatusText(_("Ready"));
}


void CMainFrame::OnMenuCommand(wxCommandEvent &evt)
{
  if (false)
    ;
  else if (evt.GetId() == XRCID("TournamentNewMenuItem"))
  {
    CTTNewDlg *dlg = (CTTNewDlg *)wxXmlResource::Get()->LoadDialog(this, "NewTournament");
    if (dlg->ShowModal() == wxID_OK)
      CTT32App::instance()->CreateTournament(
        dlg->GetTournament(), dlg->GetDatabase(), dlg->GetServer(), dlg->GetWindowsAuthentication(),
        dlg->GetUser(), dlg->GetPassword(), dlg->GetType(), dlg->GetTable());

    delete dlg;
  }
  else if (evt.GetId() == XRCID("TournamentOpenMenuItem"))
  {
    CTTOpenDlg * dlg = (CTTOpenDlg *)wxXmlResource::Get()->LoadDialog(this, "OpenTournament");
    if (dlg->ShowModal() == wxID_OK)
      CTT32App::instance()->OpenTournament(dlg->GetTournament());
    delete dlg;
  }
  else if (evt.GetId() == XRCID("TournamentCloseMenuItem"))
    CTT32App::instance()->CloseTournament();
  else if (evt.GetId() == XRCID("TournamentDetachMenuItem"))
  {
    CTTDetachDlg *dlg = (CTTDetachDlg *)wxXmlResource::Get()->LoadDialog(this, "DetachTournament");
    if (dlg->ShowModal() == wxID_OK)
      CTT32App::instance()->DetachTournament(dlg->GetTournament(), dlg->GetDeleteDirectory());

    delete dlg;
  }
  else if (evt.GetId() == XRCID("TournamentBackupMenuItem"))
    CTT32App::instance()->BackupDatabase();
  else if (evt.GetId() == XRCID("TournamentRestoreMenuItem"))
    CTT32App::instance()->RestoreDatabase();
  else if (evt.GetId() == XRCID("TournamentCancelDoubleMenuItem"))
    Import(_("Remove From Doubles"), wxEmptyString, &LtStore::RemoveFromDoubles);
  else if (evt.GetId() == XRCID("TournamentSQLEditorMenuItem"))
    CTT32App::instance()->OpenView(_("SQL Editor"), wxT("SQLEditorBook"));
  else if (evt.GetId() == XRCID("TournamentSettingsMenuItem"))
    CTT32App::instance()->OpenView(_("Tournament Settings"), wxT("Settings"));
  else if (evt.GetId() == wxID_EXIT)
    Close();

  else if (evt.GetId() == XRCID("TournamentImportEventsMenuItem"))
    Import(_("Import Events"), "cp.csv", &CpStore::Import);
  else if (evt.GetId() == XRCID("TournamentImportGroupsMenuItem"))
    Import(_("Import Groups"), "gr.csv", &GrStore::Import);
  else if (evt.GetId() == XRCID("TournamentImportAssocMenuItem"))
    Import(_("Import Associations"), "na.csv", &NaStore::Import);
  else if (evt.GetId() == XRCID("TournamentImportUmpiresMenuItem"))
    Import(_("Import Umpires"), "up.csv", &UpStore::Import);
  else if (evt.GetId() == XRCID("TournamentImportPlayersMenuItem"))
    Import(_("Import Players"), "pl.csv", &PlStore::Import);
  else if (evt.GetId() == XRCID("TournamentImportRankingpointsMenuItem"))
    Import(_("Import Ranking Points"), "rp.csv", &RpStore::Import);
  else if (evt.GetId() == XRCID("TournamentImportEntriesMenuItem"))
    Import(_("Import Entries"), "lt.csv", &LtStore::Import);
  else if (evt.GetId() == XRCID("TournamentImportPositionsMenuItem"))
    Import(_("Import Positions"), "st.csv", &StStore::Import);
  else if (evt.GetId() == XRCID("TournamentImportSchedulesMenuItem"))
    Import(_("Import Schedules"), "sc.csv", &MtStore::ImportSchedule);
  else if (evt.GetId() == XRCID("TournamentImportLineupsMenuItem"))
    Import(_("Import Lineups"), "lu.csv", &NmStore::Import);
  else if (evt.GetId() == XRCID("TournamentImportResultsMenuItem"))
    Import(_("Import Results"), "mt.csv", &MtStore::ImportResults);
  else if (evt.GetId() == XRCID("TournamentImportOnlineEntriesMenuItem"))
    CTT32App::instance()->ImportOnlineEntries();

  else if (evt.GetId() == XRCID("TournamentExportEventsMenuItem"))
    Export(_("Export Events"), "cp.csv", &CpStore::Export, CpStore::GetMaxSupportedExportVersion());
  else if (evt.GetId() == XRCID("TournamentExportGroupsMenuItem"))
    CTT32App::instance()->OpenView(_("Export Groups"), wxT("GrExport"), &GrStore::Export, GrStore::GetMaxSupportedExportVersion());
  else if (evt.GetId() == XRCID("TournamentExportAssocMenuItem"))
    Export(_("Export Associations"), "na.csv", &NaStore::Export);
  else if (evt.GetId() == XRCID("TournamentExportUmpiresMenuItem"))
    Export(_("Export Umpires"), "up.csv", &UpStore::Export);
  else if (evt.GetId() == XRCID("TournamentExportPlayersMenuItem"))
    Export(_("Export Players"), "pl.csv", &PlStore::Export);
  else if (evt.GetId() == XRCID("TournamentExportRankingpointsMenuItem"))
    Export(_("Export Ranking Points"), "rp.csv", &RpStore::Export);
  else if (evt.GetId() == XRCID("TournamentExportEntriesMenuItem"))
    Export(_("Export Entries"), "lt.csv", &LtStore::Export);
  else if (evt.GetId() == XRCID("TournamentExportPositionsMenuItem"))
    CTT32App::instance()->OpenView(_("Export Positions"), wxT("GrExport"), &StStore::Export, StStore::GetMaxSupportedExportVersion());
  else if (evt.GetId() == XRCID("TournamentExportSchedulesMenuItem"))
    CTT32App::instance()->OpenView(_("Export Schedules"), wxT("GrExport"), &MtStore::ExportSchedule, MtStore::GetMaxSupportedExportVersion());
  else if (evt.GetId() == XRCID("TournamentExportLineupsMenuItem"))
    CTT32App::instance()->OpenView(_("Export Lineups"), wxT("GrExport"), &NmStore::Export, NmStore::GetMaxSupportedExportVersion());
  else if (evt.GetId() == XRCID("TournamentExportResultsMenuItem"))
    CTT32App::instance()->OpenView(_("Export Results"), wxT("GrExport"), &MtStore::ExportResults, MtStore::GetMaxSupportedExportVersion());
  else if (evt.GetId() == XRCID("TournamentExportForRankingMenuItem"))
    CTT32App::instance()->OpenView(_("Export for Ranking"), wxT("GrExport"), &MtStore::ExportForRanking, MtStore::GetMaxSupportedExportVersion());
  else if (evt.GetId() == XRCID("TournamentExportForTTMMenuItem"))
    CTT32App::instance()->OpenView(_("Export for Ranking TTM"), wxT("GrExport"), &MtStore::ExportForRankingTTM, MtStore::GetMaxSupportedExportVersion());
  else if (evt.GetId() == XRCID("TournamentExportForITTFMenuItem"))
    CTT32App::instance()->OpenView(_("Export for Ranking ITTF"), wxT("GrExport"), &MtStore::ExportForRankingITTF, MtStore::GetMaxSupportedExportVersion());
  else if (evt.GetId() == XRCID("TournamentExportForETTUMenuItem"))
    CTT32App::instance()->OpenView(_("Export for Ranking ETTU"), wxT("GrExport"), &MtStore::ExportForRankingETTU, MtStore::GetMaxSupportedExportVersion());

  // else if (evt.GetId() == XRCID("TournamentRemoveDoublesMenuItem"))
  //   Import(_("Remove From Doubles"), "", &LtStore::RemoveFromDoubles);

  else if (evt.GetId() == XRCID("CompetitionTeamSystemMenuItem"))
    CTT32App::instance()->OpenView(_("List of Team Systems"), wxT("SyListView"));
  else if (evt.GetId() == XRCID("CompetitionGroupModusMenuItem"))
    CTT32App::instance()->OpenView(_("List of Group Modi"), wxT("MdListView"));
  else if (evt.GetId() == XRCID("CompetitionMatchPointsMenuItem"))
    CTT32App::instance()->OpenView(_("List of Match Points"), wxT("MpListView"));
  else if (evt.GetId() == XRCID("CompetitionEventMenuItem"))
    CTT32App::instance()->OpenView(_("List of Events"), wxT("CpListView"));
  else if (evt.GetId() == XRCID("CompetitionEventMenuItem"))
    CTT32App::instance()->OpenView(_("List of Events"), wxT("CpListView"));
  else if (evt.GetId() == XRCID("CompetitionGroupMenuItem"))
    CTT32App::instance()->OpenView(_("List of Groups"), wxT("GrListView"));
    
  else if (evt.GetId() == XRCID("RegistrationAssociationsMenuItem"))
    CTT32App::instance()->OpenView(_("List of Associations"), wxT("NaListView"));
  else if (evt.GetId() == XRCID("RegistrationTeamsMenuItem"))
    CTT32App::instance()->OpenView(_("List of Teams"), wxT("TmListView"), (long) 0);
  else if (evt.GetId() == XRCID("RegistrationPlayersMenuItem"))  
    CTT32App::instance()->OpenView(_("List of Players"), wxT("PlListView"), (long) 0);
  else if (evt.GetId() == XRCID("RegistrationUmpiresMenuItem"))  
    CTT32App::instance()->OpenView(_("List of Umpires"), wxT("UpListView"));
    
  else if (evt.GetId() == XRCID("DrawSeedingMenuItem"))
    CTT32App::instance()->OpenView(_("Group Positioning"), wxT("StListView"), 0);
  else if (evt.GetId() == XRCID("DrawRankingMenuItem"))
    CTT32App::instance()->OpenView(_("Ranking List"), wxT("RkListView"));
  else if (evt.GetId() == XRCID("DrawQualificationMenuItem"))
    CTT32App::instance()->OpenView(_("DrawQualification"), wxT("DrawRR"));
  else if (evt.GetId() == XRCID("DrawChampionshipMenuItem"))
    CTT32App::instance()->OpenView(_("Draw Championship"), wxT("DrawKO"), 0);
  else if (evt.GetId() == XRCID("DrawConsolationMenuItem"))
    CTT32App::instance()->OpenView(_("Draw Consolation"), wxT("DrawKO"), 1);
  else if (evt.GetId() == XRCID("DrawFromQualificationMenuItem"))
    CTT32App::instance()->OpenView(_("Draw From Qualification"), wxT("DrawKO"), 3);
  else if (evt.GetId() == XRCID("DrawFromKOMenuItem"))
    CTT32App::instance()->OpenView(_("Draw Consolation From Knock-Out"), wxT("DrawKO"), 4);
  else if (evt.GetId() == XRCID("DrawGroupsFromQualificationMenuItem"))
    CTT32App::instance()->OpenView(_("Draw Groups From Qualification"), wxT("DrawKO"), 5);
  else if (evt.GetId() == XRCID("DrawCSDMenuItem"))
    CTT32App::instance()->OpenView(_("CSD"), wxT("DrawKO"), 2);
    
  else if (evt.GetId() == XRCID("MatchTimeMenuItem"))
    CTT32App::instance()->OpenView(_("Match Time"), wxT("MtListView"), 0, true);
  else if (evt.GetId() == XRCID("MatchResultMenuItem"))
    CTT32App::instance()->OpenView(_("Match Result"), wxT("MtListView"), 0, false);
  else if (evt.GetId() == XRCID("MatchNumberMenuItem"))
    CTT32App::instance()->OpenViewNoResize(_("Select Match"), wxT("MtSelect"));
  else if (evt.GetId() == XRCID("MatchOverviewMenuItem"))
    CTT32App::instance()->OpenView(_("Match Overview"), wxT("OvListBook"));
  else if (evt.GetId() == XRCID("MatchUnscheduledMenuItem"))
    CTT32App::instance()->OpenView(_("Unscheduled Matches"), wxT("MtUnscheduled"));  // MtUnscheduled
    
  else if (evt.GetId() == XRCID("PrintPreviewMenuItem"))
  {
    CTT32App::instance()->SetPrintPreview(evt.IsChecked());
    if (evt.IsChecked())
      GetMenuBar()->FindItem(XRCID("PrintPdfMenuItem"))->Check(false);
  }
  else if (evt.GetId() == XRCID("PrintPdfMenuItem"))
  {
    CTT32App::instance()->SetPrintPdf(evt.IsChecked());
    if (evt.IsChecked())
      GetMenuBar()->FindItem(XRCID("PrintPreviewMenuItem"))->Check(false);
  }
  else if (evt.GetId() == XRCID("PrintReportsMenuItem"))
    CTT32App::instance()->OpenView(_("Print Reports"), wxT("ReportListView"));  
  else if (evt.GetId() == XRCID("PrintResultsFormsMenuItem"))
    CTT32App::instance()->OpenView(_("Print Result Forms"), wxT("GrPrint"));  
  else if (evt.GetId() == XRCID("PrintFinishedMenuItem"))
    CTT32App::instance()->OpenView(_("Print Played Groups"), wxT("GrFinished"));
  else if (evt.GetId() == XRCID("PrintManageOptionsMenuItem"))
    CTT32App::instance()->OpenView(_("Manage Options"), wxT("GrOptionsList"));

  else if (evt.GetId() == XRCID("HelpInstallLicenseMenuItem"))
    CTT32App::instance()->InstallLicense();
  else if (evt.GetId() == XRCID("HelpCheckForUpdateMenuItem"))
    CTT32App::instance()->CheckForUpdate();
  else if (evt.GetId() == XRCID("HelpShortcutsMenuItem"))
    CTT32App::instance()->ShowHtmlDialog("shortcuts.html");
  else if (evt.GetId() == XRCID("HelpThirdPartyMenuItem"))
    CTT32App::instance()->ShowHtmlDialog("3rdparty.html");
  else if (evt.GetId() == wxID_ABOUT)
    CTT32App::instance()->ShowAboutDialog();

  else
    evt.Skip();  
}


void CMainFrame::OnUpdateUI(wxUpdateUIEvent &evt)
{
  if (false)
    ;
  else if (evt.GetId() == XRCID("PrintPreviewMenuItem"))
    evt.Check(CTT32App::instance()->GetPrintPreview());
  else if (evt.GetId() == XRCID("PrintPdfMenuItem"))
    evt.Check(CTT32App::instance()->GetPrintPdf());
  else
    evt.Skip();  
}


// -----------------------------------------------------------------------
void CMainFrame::Import(const wxString &title, const wxString &defaultName, bool (*func)(wxTextBuffer &))
{
  wxString fileName;
  wxFileDialog dlg(this, title, CTT32App::instance()->GetPath(), defaultName, 
    _("CSV Files (*.csv)|*.csv|All Files (*.*)|*.*"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);

  if (dlg.ShowModal() != wxID_OK)
    return;

  fileName = dlg.GetPath();

  struct ImportHelper
  {
    wxString fileName;
    bool (*func)(wxTextBuffer &);

    static unsigned Run(void *arg)
    {
      ImportHelper *ih = (ImportHelper *) arg;
      wxTextFile is;
      if (!is.Open(ih->fileName))
        return 1;

      bool ret = (*ih->func)(is);
      is.Close();

      delete ih;
      return ret ? 0 : 1;
    }
  };

  ImportHelper *ih = new ImportHelper;
  ih->fileName = fileName;
  ih->func = func;

  wxTextFile tf(fileName);
  tf.Open();
  long count = tf.GetLineCount();
  tf.Close();

  CTT32App::instance()->ProgressBarThread(ImportHelper::Run, ih, title, count);
}


void CMainFrame::Export(const wxString &title, const wxString &defaultName, bool (*func)(wxTextBuffer &, long), long maxVersion)
{
  wxString fileName;
  int version = maxVersion;

  // Initial path with default file, 
  wxFileName fn(CTT32App::instance()->GetPath(), defaultName);
  // Normalize so we have the absolute path
  fn.Normalize();

  wxDialog *dlg = wxXmlResource::Get()->LoadDialog(this, "ExportDialog");

  dlg->SetTitle(title);
  XRCCTRL(*dlg, "File", wxFilePickerCtrl)->SetFileName(fn);

  // Fill combobox with all possible versions in descending order
  wxArrayString as;
  for (int v = maxVersion; v; v--)
    as.Add(wxString::Format("%d", v));
    
  XRCCTRL(*dlg, "Version", wxComboBox)->Set(as);
  // And select the first (highest) one as default
  XRCCTRL(*dlg, "Version", wxComboBox)->SetSelection(0);

  // Disable version selection if there is only one
  // Hiding the ctrl doesn't look good because the dlg has to have a mininum size 
  // (or we don't see the buttons)
  if (maxVersion == 1)
    dlg->FindWindow("Version")->Disable();

  if (dlg->ShowModal() != wxID_OK)
    return;

  fileName = XRCCTRL(*dlg, "File", wxFilePickerCtrl)->GetPath();
  // Versions are in descending order so selection 0 is equal to maxVersion, and so on
  version = maxVersion - XRCCTRL(*dlg, "Version", wxComboBox)->GetSelection();

  delete dlg;

  // We read into a buffer where each line is an entry so this is the only place
  // where we have to care about the encoding
  wxMemoryText buf;
  if (!(*func)(buf, version))
    return;

  std::ofstream  ofs(fileName.mb_str(), std::ios::out);

  // Excel needs a BOM to read UTF-8 (with ';') correctly. Strange ...
  const wxString bom(wxChar(0xFEFF));
  ofs << bom.ToUTF8();

  for (wxString str = buf.GetFirstLine(); !buf.Eof(); str = buf.GetNextLine())
    ofs << str.ToUTF8() << std::endl;

  ofs.close();
}


void CMainFrame::SetMenuBar(wxMenuBar *menuBar)
{
  if (GetMenuBar() == menuBar)
    return;

  bool isAttached = m_windowMenu && m_windowMenu->IsAttached();
  if (isAttached)
    m_windowMenu->Detach();

  wxMDIParentFrame::SetMenuBar(menuBar);

  if (isAttached && !m_windowMenu->IsAttached())
    m_windowMenu->Attach(GetMenuBar());
}