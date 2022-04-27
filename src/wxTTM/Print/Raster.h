/* Copyright (C) 2020 Christoph Theis */

// Aufteilung von grraster.h
// Basis RASTER_BASE: Allgemeine Funktionalitaet
// 02.09.95 (ChT)

# ifndef  RASTER_BASE_H
# define  RASTER_BASE_H  

#include  "Printer.h"

#include  "CpListStore.h"
#include  "GrListStore.h"
#include  "PlListStore.h"
#include  "MtListStore.h"
#include  "TmEntryStore.h"   // TmEntry
#include  "StEntryStore.h"   // StEntry
#include  "MtEntryStore.h"   // MtEntry
#include  "MtStore.h"        // MtSet
#include  "MdStore.h"
#include  "UpStore.h"
#include  "PoStore.h"

#include  <map>

// #include  <ranking.h>   // Zum Sortieren von Tabellen (Round Robin)

// Dicker und duenner Rahmen
constexpr int THINN_FRAME = 1;
constexpr int THICK_FRAME = 4;

// Breite des Namenfeldes
constexpr int RR_NAME_WIDTH = 40;
constexpr int KO_NAME_WIDTH = 35;


// Maximale Zahl der KO-Runden und KO_Spiele auf einer Seite
// #define   KO_MAX_ROUNDS      5
// #define   KO_MAX_MATCHES    16

// Breiten von Startnr und Nation
// Factor for upper case to lower case
#define   U2L_FACTOR         (((double) printer->TextWidth("X")) / printer->cW)

#define   STARTNR_WIDTH      (startNrWidth ? std::ceil(startNrWidth) : 0)
#define   TEAMNAME_WIDTH     (teamNameWidth ? std::ceil(teamNameWidth * U2L_FACTOR + 0) : 0)

// ASSOC_* are at the right and may need a bit more space
#define   ASSOCNAME_WIDTH    (nationNameWidth ? std::ceil(nationNameWidth * U2L_FACTOR + 1) : 0)
#define   ASSOCDESC_WIDTH    (nationDescWidth ? std::ceil(nationDescWidth * U2L_FACTOR + 1) : 0)
#define   ASSOCREGION_WIDTH  (nationRegionWidth ? std::ceil(nationRegionWidth * U2L_FACTOR + 1) : 0)

// Mehreinheiten fuer 1 Spalte bei vollem Nationennamen
// Das ist die Differenz zwischen ASSOCNAME- und ASSOCDESC-WIDTH
#define   ASSOCDESC_OFFST    (nationDescWidth > nationNameWidth ? std::ceil((nationDescWidth - nationNameWidth)  * U2L_FACTOR + 1) : 0)


struct CRect
{
  int left;
  int top;
  int right;
  int bottom;
  
  CRect() : left(0), top(0), right(0), bottom(0) {}
  CRect(int l, int t, int r, int b) : left(l), top(t), right(r), bottom(b) {}
  CRect(const wxRect &rect) : left(rect.GetLeft()), top(rect.GetTop()), right(rect.GetRight()), bottom(rect.GetBottom()) {}
  
  
  int GetLeft() const {return left;}
  int GetRight() const {return right;}
  int GetTop() const {return top;}
  int GetBottom() const {return bottom;}
  int GetWidth() const {return right - left;}
  int GetHeight() const {return bottom - top;}
  
  operator wxRect () const {return wxRect(left, top, right - left, bottom - top);}
};


// +----------------------------------------------+
// +        Basisklasse  RasterBase               +
// +----------------------------------------------+


class  RasterBase
{
  typedef  std::map<long, StEntry, std::less<long> >  StEntryMap;

  public:
    enum {
      FLAG_PRINT_ASSOC_NONE = 0,
      FLAG_PRINT_ASSOC_NAME = 1,
      FLAG_PRINT_ASSOC_DESC = 2,
      FLAG_PRINT_ASSOC_REGION = 3
    };

    enum {
      FLAG_PRINT_NONE      = 0x0,   // Print Nothing
      FLAG_PRINT_NATION    = 0x01,  // Print Association name
      FLAG_PRINT_NADESC    = 0x02,  // Print Association desc
      FLAG_PRINT_NAREGION  = 0x04,  // Print Region          
      FLAG_PRINT_FNAME     = 0x08,  // Print first name
      FLAG_PRINT_CONDENSED = 0x10,  // Print player / buddy
      FLAG_PRINT_NOPLNR    = 0x20   // Keine Startnummer
    };

	// Konstruktor
	public:
		RasterBase(Printer *, Connection *connPtr = 0);
	 ~RasterBase();

	// Variablen
	protected:
		Printer   *printer = nullptr;
		// TEAMLIST  teamList;

    long   width = 0;     // Einheitsweite
		short  textFont = 0;  // Font der Ausgabe

		long   offsetX = 0;   // Abstand vom linken Rand
		long   offsetY = 0;   // Abstand vom oberen Rand

		long   space = 0;     // Abstand Schrift zum Rand

		short  page = 0;      // Aktuelle Seitenzahl

	// Einzelne Ergebnisse drucken
	public:
    // Initialisierung
    void  SetupGroup(const CpRec &, const GrRec &);

    // Spiele drucken
		void  PrintMatches(const CpRec &, const GrRec &, const PrintRasterOptions &,
		                   int *pofstX, int *pofstY, int *ppage);   

    // Kopf des Schiedsrichterzettels
		void  PrintMatchHeader(const MtRec &);  
    // Ergebniszeilen
		void  PrintMatch(const MtEntry &);  
    // Einzelergebnisse in Mannschaftsspielen
    void  PrintMatchDetails(const MtEntry &);      
    // Neue Seite und Ueberschrift (Flag: Print Pagenr)
		void  NewPage(bool printPageNr = true);     
    // Anmerkung drucken
    void  PrintNotes(GrListStore &, const PrintRasterOptions &, int *pofstX, int *pofstY, int *ppage);

	// protected Methoden
	protected:
		virtual void  PrintCaption();  // Ueberschrift
    virtual void  PrintBanner();   // Banner oben
    virtual void  PrintLogo();     // Logo oben
    virtual void  PrintSponsor();  // Sponsor unten
		void  PrintPageNr();           // Seitenzahl
    void  PrintPageTime();         // Datum des Ausdrucks

		// Drucke Spieler / Mannschaft von Gruppe in Region
		// Neue Methoden fuer die Ausgabe von Teams, etc
		void  PrintString(const wxString &, const CRect &, int fmt = wxALIGN_LEFT);
		// void  PrintString(const wxString &, const CRect &, int fmt = DT_LEFT | DT_END_ELLIPSIS);
		void  PrintStringCentered(const wxString &, const CRect &);
    void  PrintStringWrapped(const wxString &, const CRect &, int offset = 0);
		void  PrintInt(int, const CRect &);
		void  PrintLong(long, const CRect &);
		
		void  PrintDate(const timestamp &, const CRect &, int fmt = wxALIGN_LEFT);
		void  PrintTime(const timestamp &, const CRect &, int fmt = wxALIGN_LEFT);

		void  PrintDate(const timestamp &, const CRect &, const wxString &dateFmtStr, int fmt = wxALIGN_LEFT);
		void  PrintTime(const timestamp &, const CRect &, const wxString &timeFmtStr, int fmt = wxALIGN_LEFT);

		void  PrintGame(int *, const CRect &, bool revers = false);
		void  PrintGame(short *, const CRect &, bool revers = false);
    void  PrintGame(const MtSet &, const CRect &, bool revers = false);

		void  PrintPlayer(const TmPlayer &, const CRect &, long flag);
    void  PrintPlayer(const PlListRec &, const CRect &, long flag);
    void  PrintUmpire(const UpRec &, const CRect &, long flag);
		void  PrintTeam(const TmTeam &, const CRect &, long flag);
    void  PrintGroup(const TmGroup &, const CRect &, long flag);

		void  PrintEntry(const TmEntry &, const CRect &, long flag);

		void  PrintEntry(long stID, const CRect &reg, long flags);

	// protected Members
	protected:
	  // Druckoptionen
	  PrintRasterOptions options;
	  
    // Connection
    Connection *connPtr;
    // Aktueller WB und Gruppe
		CpRec  cp;
    GrRec  gr;
    MdRec  md;

    // Liste der Spieler (StEntry)
    StEntryMap  stMap;

    StEntry  GetTeamA(const MtRec &);
    StEntry  GetTeamX(const MtRec &);
    StEntry  GetTeam(long stID);
    StEntry  GetTeamByNr(short stNr);
    
    long     GetUmpire(const MtRec &);
    short    GetUmpireCandidate(const MtRec &);

    // Allgemeine Einstellungen
    int  nationNameWidth;
    int  nationDescWidth;
    int  nationRegionWidth;
    int  teamNameWidth;
    int  startNrWidth;
};


# endif