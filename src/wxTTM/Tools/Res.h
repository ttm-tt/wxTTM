/* Copyright (C) 2020 Christoph Theis */

// RES.H
// Definition der Resourcennamen

#ifndef  RES_H
#define  RES_H


// Spezielle WM_COMMAND
const unsigned  IDC_LIST_ADD   = 0x8001;
const unsigned  IDC_LIST_EDIT  = 0x8002;
const unsigned  IDC_LIST_DEL   = 0x8003;

#define  UWM_PROGRESS_STEP  (WM_APP + 101)
#define  UWM_UPDATEVIEW     (WM_APP + 102)
#define  UWM_ENDDOC         (WM_APP + 102)
#define  UWM_ABORTDOC       (WM_APP + 103)

// Privates Profile
#define  TT_PROFILE          wxT("tt32.ini")

// Weitere Files. Kein wxT(...), weil sie der Praeproztessor verkettet
#define  TT_XRCFILE          "ttm.xrc"
#define  TT_MOFILE           "ttm.mo"

// In TT.INI
#define  PRF_TURNIER         wxT("Tournaments")
#define  PRF_OPEN            wxT("Open")
#define  PRF_LAST            wxT("Last")
#define  PRF_LASTUSED        wxT("Last Used")
#define  PRF_REPORTS         wxT("Reports")

#define  PRF_DEFAULTS        CTT32App::instance()->GetTournament().wx_str()
#define  PRF_DEFCP           wxT("Default CP")
#define  PRF_DEFGR           wxT("Default GR")
#define  PRF_DEFNA           wxT("Default NA")
#define  PRF_SCORE_DATE      wxT("Scoresheet Date")
#define  PRF_SCORE_TIME      wxT("Scoresheet Time")
#define  PRF_SCORE_FROMTABLE wxT("Scoresheet from Table")
#define  PRF_SCORE_TOTABLE   wxT("Scoresheet to Table")
#define  PRF_DEFTIMEOUT      wxT("Refresh Time")
#define  PRF_PATH            wxT("Path")

#define  PRF_OUTPUT      	   wxT("Output")
#define  PRF_OUTPUT_PREVIEW  wxT("Preview")

#define  PRF_QLIST       		 wxT("QRF")

#define  PRF_QUERY       		 wxT("Query")
#define  PRF_QUERY_HEADER    wxT("Header")
#define  PRF_QUERY_DETAIL    wxT("Detail")
#define  PRF_QUERY_BREAK     wxT("Break")
#define  PRF_QUERY_FOOTER    wxT("Footer")

#define  PRF_RASTER          wxT("Raster")
#define  PRF_RASTER_TINY     wxT("Tiny")
#define  PRF_RASTER_SMALL    wxT("Small")
#define  PRF_RASTER_SMALLB   wxT("Small-Bold")
#define  PRF_RASTER_MEDIUM   wxT("Medium")
#define  PRF_RASTER_MEDIUMB  wxT("Medium-Bold")
#define  PRF_RASTER_NORMAL   wxT("Normal")
#define  PRF_RASTER_NORMALB  wxT("Normal-Bold")
#define  PRF_RASTER_GROUP    wxT("Group")
#define  PRF_RASTER_COMP     wxT("Comp")
#define  PRF_RASTER_PAGE     wxT("Page")

#define  PRF_STRTITLE        wxT("Title")
#define  PRF_STRSUBTITLE     wxT("Subtitle")

#define  PRF_SETTINGS               wxT("Settings")
#define  PRF_GLOBAL_SETTINGS        wxT("Settings")
#define  PRF_LOCAL_SETTINGS         CTT32App::instance()->GetTournament().wx_str()
#define  PRF_SETTINGS_MTWINNER      wxT("MTWINNER")
#define  PRF_SETTINGS_LOCALE        wxT("Locale")
#define  PRF_SETTINGS_PREVIEW       wxT("Preview")
#define  PRF_SETTINGS_PDF           wxT("Pdf")
#define  PRF_SETTINGS_DATESTRING    wxT("Date")
#define  PRF_SETTINGS_TIMESTRING    wxT("Time")
#define  PRF_SETTINGS_TYPE          wxT("Type")
#define  PRF_SETTINGS_TABLE         wxT("TableMode")
#define  PRF_SETTINGS_LANGUAGE      wxT("Language")
#define  PRF_SETTINGS_PRINTBANNER   wxT("Banner")
#define  PRF_SETTINGS_PRINTLOGO     wxT("Logo")
#define  PRF_SETTINGS_PRINTSPONSOR  wxT("Sponsor")
#define  PRF_SETTINGS_PRINTLANG     wxT("PrintLanguage")
#define  PRF_SETTINGS_SCOREEXTRAS   wxT("ScoreExtras")
#define  PRF_SETTINGS_SCOREPLSIG    wxT("ScorePlayersSignature")
#define  PRF_SETTINGS_SCOREREMARKS  wxT("PrintScoreRemarks")
#define  PRF_SETTINGS_SCORECOACHES  wxT("PrintScoreCoaches")
#define  PRF_SETTINGS_SCOREUMPIRES  wxT("PrintScoreUmpires")
#define  PRF_SETTINGS_SCOREUMPIRENAME  wxT("PrintScoreUmpireName")
#define  PRF_SETTINGS_SCORESERVICETIMEOUT wxT("PrintScoreServiceTimeout")
#define  PRF_SETTINGS_PRINTSCALEPAPER wxT("PrintScaleToPaperSize")
#define  PRF_SETTINGS_DATABASEPATH  wxT("DatabasePath")

#define  PRF_SETTINGS_BACKUPPATH    wxT("BackupPath")
#define  PRF_SETTINGS_BACKUPTIME    wxT("BackupTime")
#define  PRF_SETTINGS_BACKUPAPPENDTIMESTAMP wxT("BackupAppendTimestmap")
#define  PRF_SETTINGS_BACKUPKEEPLAST  wxT("BackupKeepLast")
#define  PRF_SETTINGS_BACKUPKEEPNOFITEMS wxT("BackupKeepNofItems")

#define  PRF_PRINT_CAPTIONMARGINKO  wxT("PrintCaptionMarginKO")

#define  PRF_WINDOWPOS              wxT("Window Position")

#define  PRF_PROPTIONS              PRF_LOCAL_SETTINGS
#define  PRF_PROPTIONS_LASTUSED     wxT("poLastUsed")
#define  PRF_PROPTIONS_RRSLCTRD     wxT("rrSlctRound")
#define  PRF_PROPTIONS_RRFROMRD     wxT("rrFromRound")
#define  PRF_PROPTIONS_RRTORD       wxT("rrToRound")
#define  PRF_PROPTIONS_RRRESULTS    wxT("rrResults")
#define  PRF_PROPTIONS_RRSIGNATURE  wxT("rrSignatur")
#define  PRF_PROPTIONS_RRTMDETAILS  wxT("rrTmDetails")
#define  PRF_PROPTIONS_RRNEWPAGE    wxT("rrNewPage")
#define  PRF_PROPTIONS_RRIGNOREBYES wxT("rrIgnoreByes")
#define  PRF_PROPTIONS_RRSCHEDULE   wxT("rrSchedule")
#define  PRF_PROPTIONS_RRCOMBINED   wxT("rrCombined")
#define  PRF_PROPTIONS_RRCONSOLATION wxT("rrConsolation")
#define  PRF_PROPTIONS_RRLASTRESULTS wxT("rrLastResults")
#define  PRF_PROPTIONS_RRPRINTNOTES  wxT("rrPrintNotes")
#define  PRF_PROPTIONS_KOSLCTRD     wxT("koSlctRound")
#define  PRF_PROPTIONS_KONOQUROUNDS wxT("koNoQualiRounds")
#define  PRF_PROPTIONS_KOFROMRD     wxT("koFromRound")
#define  PRF_PROPTIONS_KOTORD       wxT("koToRound")
#define  PRF_PROPTIONS_KOLASTRD     wxT("koLastRounds")
#define  PRF_PROPTIONS_KOSLCTMT     wxT("koSlctMatch")
#define  PRF_PROPTIONS_KOFROMMT     wxT("koFromMatch")
#define  PRF_PROPTIONS_KOTOMT       wxT("koToMatch")
#define  PRF_PROPTIONS_KOLASTMT     wxT("koLastMatches")
#define  PRF_PROPTIONS_KONR         wxT("koNr")
#define  PRF_PROPTIONS_KOTMDETAILS  wxT("koTmDetails")
#define  PRF_PROPTIONS_KOIGNOREBYES wxT("koIgnoreByes")
#define  PRF_PROPTIONS_KONEWPAGE    wxT("koNewPage")
#define  PRF_PROPTIONS_KOLASTRESULTS wxT("koLastResults")
#define  PRF_PROPTIONS_KOINBOX      wxT("koInbox")
#define  PRF_PROPTIONS_KOTHIRDPLACE wxT("koNoThirdPlace")
#define  PRF_PROPTIONS_KOFIRSTRDQUAL wxT("koFirstRdQualification")
#define  PRF_PROPTIONS_KOPRINTPOSNRS wxT("koPrintPositionNumbers")
#define  PRF_PROPTIONS_KOPRINTNOTES  wxT("koPrintNotes")

// Steuerungen der einzelnen Turniere
// Event Angaben im RR-Raster
// #define  PRF_PRINT_RASTER_EVENT  "PrintRasterEvent"
// Voller Nationenname auf den Ausdrucken
#define  PRF_PRINT_NATION              wxT("PrintNation")
#define  PRF_PRINT_NATION_TEAM         wxT("PrintNationTeam")
// Raster mit Details als Combined Scoresheet
#define  PRF_PRINT_COMBINED_SCORESHEET wxT("PrintCombinedScoresheet")

// Breite der Felder Nationen-Name / Desc
#define  PRF_PRINT_NATION_NAME_WIDTH   wxT("PrintNationNameWidth")
#define  PRF_PRINT_NATION_DESC_WIDTH   wxT("PrintNationDescWidth")
#define  PRF_PRINT_NATION_REGION_WIDTH wxT("PrintNationRegionWidth")
#define  PRF_PRINT_TEAM_NAME_WIDTH     wxT("PrintTeamNameWidth")
#define  PRF_PRINT_PLNR_WIDTH          wxT("PrintPlNrWidth")

// Font und Breite von OvList
#define  PRF_OVLIST_FONTSIZE          wxT("OvListFontSize")
#define  PRF_OVLIST_CELLSIZE          wxT("OvListCellSize")
#define  PRF_OVLIST_COLORS            wxT("OvListColors")
#define  PRF_OVLIST_SETTINGS          wxT("OvListSettings")


# endif