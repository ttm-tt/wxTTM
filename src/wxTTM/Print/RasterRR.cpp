/* Copyright (C) 2020 Christoph Theis */

// Aufspaltung von GRRASTER
// Raster fuer Round Robin
// 02.09.95 (ChT)
// 24.06.96 (ChT)

#include  "stdafx.h"

#include  "RasterRR.h"   // Modul Header

#include  "TbSort.h"     // Sortieren der Spieler in Round Robin

#include  "TbItem.h"

#include  "TT32App.h"
#include  "CpListStore.h"
#include  "GrListStore.h"
#include  "MtListStore.h"
#include  "PlListStore.h"
#include  "StEntryStore.h"
#include  "StListStore.h"

#include  "Res.h"

#include  <stdlib.h>

#include  <strstream>
#include  <iomanip>

#undef _
#define _(a) printer->GetString(a)

// #define  RR_MATCHPTS  (infoSystem->QryString("RES_STR", STR_MATCHPTS))
// #define  RR_GAMES     (infoSystem->QryString("RES_STR", STR_RESULT))
// #define  RR_PLACE     (infoSystem->QryString("RES_STR", STR_PLACE))
// #define  RR_PLAYERS   (infoSystem->QryString("RES_STR", STR_PLAYERS))
// #define  RR_TEAMS     (infoSystem->QryString("RES_STR", STR_TEAMS))

#define RR_MATCHPTS IDS_MTPTS
#define RR_MATCHES  IDS_MATCHES
#define RR_GAMES    IDS_GAMES
#define RR_PLACE    IDS_PLACE
#define RR_PLAYERS  IDS_PLAYERS
#define RR_TEAMS    IDS_TEAMS

#define  PRINT_FULL_CAPTION


// Definition von Strings
// Def. von min und max
# ifdef TURBO
  inline int  min(int a, int b) {return (a < b ? a : b);}
  inline int  max(int a, int b) {return (a > b ? a : b);}
# endif


// +-----------------------------------------------------------+
// +                        RasterRR                          +
// +-----------------------------------------------------------+
// +   Druckt das Raster einer RR-Gruppe                       +
// +-----------------------------------------------------------+

RasterRR::RasterRR(Printer *prt, Connection *ptr)
         : RasterBase(prt, ptr)
{
  // teamDesc = NULL;
}


RasterRR::~RasterRR()
{
}


// externer Aufruf
// Paramter: (in) Gruppe, (in/out) offset x, (in/out) offsetY
int  RasterRR::Print(CpRec &cp_, GrRec &gr_, 
                     PrintRasterOptions &options_,
										 int *ofstX, int *ofstY, int *ppage)
{
  SetupGroup(cp_, gr_);
  
  options = options_;

  rrSchedule = options.rrSchedule;
  rrIgnore   = options.rrIgnoreByes;
  offsetX = *ofstX;
  offsetY = *ofstY;
  page = *ppage;
  cp = cp_;
  gr = gr_;

	// Gruppe einlesen und sortieren
  MtListStore  mt(connPtr);

  mt.SelectByGr(gr);
  while (mt.Next())
    mtList.push_back(mt);

  mt.Close();

  for (const auto &it : stMap)
    tbList.push_back(new TbItem(it.second));

  TbSort::Sort(gr, cp.cpType, tbList, mtList);
  
  bool newPage = false;
  
	// Schriftgroesse und Rasterweite haengen von Gruppengroesse ab
	// War: Helv 10 fuer <= 8 Spieler, Helv 8 fuer sontige
  if (gr.grSize <= 6)
    textFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_NORMAL);
  else if (gr.grSize <= 8)
    textFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_MEDIUM);
	else
    textFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_SMALL);

  if (options.rrNewPage || page == 0)
  {
		NewPage(false);     // Ohne Seitenzahl
		newPage = true;
	}

  short  oldFont = printer->SelectFont(textFont);

  width = printer->cW;
  space = printer->cW / 2;

  // Wenn Ergebnisse auftauchen oder es ein Combined Scoresheet ist
  // groesseres Raster nehmen. Dann passen aber nicht mehr 4 Gruppen 
  // auf eine Seite.
  if (options.rrResults || options.rrCombined)
    height = 3 * printer->cH;
  else
    height = 2.5 * printer->cH;  

/*
  if (cp.cpType == CP_SINGLE)
    height = 2.3 * printer->cH;
  else if (cp.cpType != CP_TEAM)
	  height = 3 * printer->cH;
  else
	  height = (5 * printer->cH) / 2;   // Faktor 2.5
*/
	  
  // height = (cp.cpType == CP_DOUBLE || cp.cpType == CP_MIXED ?
  //           3 * printer->cH : 2 * printer->cH);

	rw = (gr.grSize <= 8 ? height : printer->TextWidth(wxT("10:10"), textFont) + 2 * space);
	
	if (gr.grModus == MOD_RR && options.rrSchedule)
	{
    // Tag ist "Wed, 10 Apr 2013" und damit möglichst breit
    wxString tmp = wxDateTime(10, wxDateTime::Apr, 2013).Format(CTT32App::instance()->GetDateFormat());

  	short  smallFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_SMALL);

	  if (rw < printer->TextWidth(tmp, smallFont) + 2 * space)
	    rw = printer->TextWidth(tmp, smallFont) + 2 * space;
	    
	  printer->DeleteFont(smallFont);
	}
	
	// Hier kenne ich die Hoehe des Rasters
  // Neue Seite starten und Ueberschrift drucken
  // Neue Seite, wenn verlangt, oder zu wenig Platz
  
#if 1 // EYC2006 Sarajevo
  // Auch wenn Ergebnisse gedruckt werden, neue Seite nur wenn noetig.
  int neededSize = 1.0 * height /* + 100 */ + (gr.grSize + 1) * height;

  // Platz fuer Ueberschrift in Combined Score Sheet
  if (options.rrCombined)
    neededSize += height;

  // if (CTT32App::instance()->GetPrintSponsor())
  //   neededSize += 100;  // footer logo (100 px Hoehe) 
    
  if (options.rrResults)
  {
    int nofRounds = options.rrLastResults ? 1 : gr.NofRounds();

    if (cp.cpType == CP_SINGLE)
      neededSize += nofRounds * (gr.NofMatches(1) + 1) * printer->cH;
    else if (cp.cpType == CP_DOUBLE || cp.cpType == CP_MIXED)
      neededSize += nofRounds * (gr.NofMatches(1) + 1) * printer->cH * 2;
    else if (!options.rrTeamDetails)
      neededSize += nofRounds * (gr.NofMatches(1) + 1) * printer->cH;
    else
    {
      // Ich tippe mal auf Einzelspiele, die noch auf die Seite passen sollten.
      neededSize = nofRounds * (gr.NofMatches(1) + 1) * printer->cH * 5;
    }
  }
#endif

  if (newPage)
  {
    // Nothing		
  }
  else if (offsetY > printer->height - neededSize)
  {
		NewPage();     
		// offsetY += 0.7 * height;
		
		printer->SelectFont(oldFont);
		printer->SelectFont(textFont);
	}
	else                  // dann wenigstens Ueberschrift
	{
		if (options.rrCombined)
		{
			offsetY += 0.5 * height;
			PrintCaption();            // Wird auf height geschaetzt
		}
		else if (false)     // Gruppe steht im Raster
		{
			// Eindhoven: Statt die komplette Caption nur die Gruppe
			offsetY += 0.7 * height;

		  short  oldFont, groupFont;
		  groupFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_GROUP);
		  oldFont = printer->SelectFont(groupFont);

		  CRect  reg;
		  reg.left = offsetX;
		  reg.right = printer->width;
		  reg.top = offsetY;
		  reg.bottom = offsetY + height;

		  PrintStringCentered(gr.grDesc, reg);

		  offsetY += 0.7 * height;
		  printer->SelectFont(oldFont);
		  printer->DeleteFont(groupFont);
		}
	}

  // Add Bookmark fuer aktuelle Position (Caption)
  printer->Bookmark(gr.grDesc);

	if (options.rrCombined)
		offsetY += 0.5 * height;
	else
		offsetY += 1.0 * height;
	
  // Zelle Nummern
  number.top    = offsetY;
  number.bottom = offsetY + height;
  number.left   = offsetX;
  number.right  = offsetX + rw;

	// Zelle Namen
  names.top = offsetY;
  names.bottom = offsetY + height;
  names.left = number.right;
	names.right = number.right + RR_NAME_WIDTH * width;

  // Zellen Raster
  raster.top = offsetY;
  raster.bottom = offsetY + height;
  raster.left = names.right;
	raster.right = names.right + gr.grSize * rw;

  // Kapselung von tmp
  {
    wxString  tmp;
    // Der Rest ist so breit wie der Eintrag in der Titelzeile
    // Zelle Match
    tmp = _("Mt.Pts.");
    match.top = offsetY;
    match.bottom = offsetY + height;
	  match.left = raster.right;
    match.right = raster.right + printer->TextWidth(tmp, textFont) + 2*space;

	  // Zelle Games
	  if (cp.cpType == CP_TEAM)
	    tmp = _("Matches");
	  else
	    tmp = _("Games");
	  games.top = offsetY;
    games.bottom = offsetY + height;
    games.left = match.right;
    games.right = match.right + printer->TextWidth(tmp, textFont) + 4*space;

    // Zelle Place
    tmp = _("Place");
    place.top = offsetY;
    place.bottom = offsetY + height;
    place.left = games.right;
    place.right = games.right + printer->TextWidth(tmp, textFont) + 2*space;
  }

  // Gesamtbreite ausrechnen und Namensfeld strecken
	// if (place.right > printer->columns)    // 10mm Rand rechts
  {
		int ofst = place.right - printer->width;
    names.right -= ofst;

    // den Rest korrigieren
    raster.left  -= ofst;
    raster.right -= ofst;

    match.left  -= ofst;
    match.right -= ofst;

    games.left  -= ofst;
    games.right -= ofst;
    
    place.left  -= ofst;
		place.right -= ofst;
	}

  // Und einen grossen Rahmen um das Ganze
  CRect  tmp(number.left, number.top,	place.right, place.bottom + gr.grSize * height);
  printer->Rectangle(tmp,	THICK_FRAME, FALSE);

	// Untere Kante vom Text
	baseLine = offsetY + (height + printer->cH) / 2;

  // Aufbau des Gitters
  PrintNames();
  PrintRaster();
  PrintResults();

	// Eintrag der Texte
	WriteNames();
	WriteRaster();
	WriteResults();

	offsetY += (gr.grSize + 1) * height;

  printer->SelectFont(oldFont);
  printer->DeleteFont(textFont);

  for (std::vector<TbItem *>::iterator tbIt = tbList.begin();
       tbIt != tbList.end(); tbIt++)
    delete *tbIt;

  // *ofstY += height + gr.grSize * height;
  *ofstY = offsetY;
  *ppage = page;

  return 0;
}


//               
//                                    +------+------+-------+
// Wettbewerb                         | Date | Time | Table |
// -------------------------------    +------+------+-------+
// Gruppe                             |      |      |       |
//                                    +------+------+-------+

void  RasterRR::PrintCaption()
{
	// Rechts sitzt Event
	short  oldFont = printer->SelectFont(textFont);
	short  cpFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_COMP);
	short  grFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_GROUP);

	int  height = 2 * printer->cH;

	// Eindhoven Spezial: CP / GR centered
	// Laenge der Eintraege im CP-Font berechnen
	unsigned  startX, length;
	printer->SelectFont(cpFont);

  // Breite fuer CP ausreichend fuer cpDesc, aber mindestens SIZEOF_CPDESC / 2
	length = printer->TextWidth(cp.cpDesc, cpFont);
	if (length < (sizeof(cp.cpDesc) / sizeof(wxChar)) * printer->cW / 2)
		length = (sizeof(cp.cpDesc) / sizeof(wxChar)) * printer->cW / 2;
	length += 2 * space;

	startX = offsetX + (printer->width - offsetX - length) / 2;

	printer->SelectFont(textFont);

	// Region dafuer
	CRect regPlace;
	regPlace.right = printer->width;
	regPlace.top = offsetY;

	regPlace.left = regPlace.right -
									(printer->width - offsetX) / 4;
	regPlace.bottom = regPlace.top + 2 * height;

	if (options.rrCombined)
	 PrintEvent(regPlace);

	// Eine dicke Linie auf halber Hoehe, daruber WB, darunter GR
	offsetY += height;

	// Eine dicke Linie auf dieser Hoehe
  printer->Line(wxPoint(startX, offsetY), wxPoint(startX + length, offsetY), THICK_FRAME);

	// WB und Gruppe sitzen zentriert links davon

	// Wettbewerb
	printer->SelectFont(cpFont);

	CRect  regCP;
	regCP.left = startX;
	regCP.right = startX + length;
	regCP.bottom = offsetY;
	regCP.top = regCP.bottom - 1.5 * printer->TextHeight(cp.cpDesc, cpFont);

	PrintStringCentered(cp.cpDesc, regCP);
	
	offsetY = regCP.bottom;

  if (options.rrCombined)
  {
	  // Gruppe in 12pt Helv
	  printer->SelectFont(grFont);

	  CRect  regGR;
	  regGR = regCP;

	  regGR.top = regGR.bottom;
	  regGR.bottom = regGR.top + 1.5 * printer->TextHeight(gr.grDesc, grFont);

    PrintStringCentered(gr.grDesc, regGR);

	  offsetY = regGR.bottom; //  + printer->cH;
	}

	printer->SelectFont(oldFont);
	printer->DeleteFont(cpFont);
	printer->DeleteFont(grFont);
}


// Rechtsbuendig auf die Seite Datum/Zeit/Tisch
//
// +------+------+-------+
// | Date | Time | Table |
// +------+------+-------+
// |      |      |       |
// +------+----- +-------+
//
void  RasterRR::PrintEvent(CRect &reg)
{
  // Daten werden bis auf weiteres vom ersten Spiel genommen
  MtRec  mt = (*mtList.begin());

  short  oldFont = printer->SelectFont(textFont);

  // Eine Region fuer alles
  CRect regPlace = reg;

	// Und einen Rahmen um alles
  printer->Rectangle(regPlace, THICK_FRAME, FALSE);

  // Region Halbieren, oben der Kopf, unten die Daten, Linie ziehen
  regPlace.bottom = regPlace.top + regPlace.GetHeight() / 2;
  printer->Line(wxPoint(regPlace.left, regPlace.bottom), 
                wxPoint(regPlace.right, regPlace.bottom), 
                THINN_FRAME);

  // Regions fuer Tag, Zeit, Tisch
  CRect  regDate = regPlace;
  CRect  regTime = regPlace;
  CRect  regTable = regPlace;

  regDate.right = regDate.left + (regPlace.GetWidth() / 3);
  regTime.left = regDate.right;
  regTime.right = regTime.left + (regPlace.GetWidth() / 3);
  regTable.left = regTime.right;

	// Linien zur Absperrung
  printer->Line(wxPoint(regDate.right, regDate.top), 
                wxPoint(regDate.right, regDate.bottom), 
                THINN_FRAME);
  printer->Line(wxPoint(regTime.right, regTime.top), 
                wxPoint(regTime.right, regTime.bottom), 
                THINN_FRAME);

  // Und die Eintraege
  PrintString(_("Date"), regDate);
  PrintString(_("Time"), regTime);
  PrintString(_("Table"), regTable);

  // Region eins tiefer
  int tmp = regPlace.GetHeight();
  regPlace.top = regPlace.bottom;
  regPlace.bottom += tmp;

  regDate.top = regDate.bottom;
	regDate.bottom += tmp;

  regTime.top = regTime.bottom;
  regTime.bottom += tmp;

  regTable.top = regTable.bottom;
  regTable.bottom += tmp;

  // Absperrungen
  printer->Line(wxPoint(regDate.right, regDate.top), 
                wxPoint(regDate.right, regDate.bottom), 
                THINN_FRAME);
  printer->Line(wxPoint(regTime.right, regTime.top), 
                wxPoint(regTime.right, regTime.bottom), 
                THINN_FRAME);

  if (mt.mtPlace.mtDateTime.day)
    PrintDate(mt.mtPlace.mtDateTime, regDate, "%d %b", wxALIGN_CENTER);

	if (mt.mtPlace.mtDateTime.hour)
		PrintTime(mt.mtPlace.mtDateTime, regTime, "%H:%M", wxALIGN_CENTER);

  if (mt.mtPlace.mtTable)
  {
    // Der Tisch erscheint groesser
    wxChar  str[17];
    _itot(mt.mtPlace.mtTable, str, 10);

    short cpFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_COMP);
    short oldFont = printer->SelectFont(cpFont);

		PrintStringCentered(str, regTable);

    printer->SelectFont(oldFont);
		printer->DeleteFont(cpFont);
  }

  printer->SelectFont(oldFont);
}


void  RasterRR::PrintNames()
{
  int  totalHeight = (gr.grSize + 1) * height;

	// Namensteil
  CRect  tmp(number.left, number.top, names.right, number.top + totalHeight);
  printer->Rectangle(tmp, THINN_FRAME);
  
  for (int i = 0; i < gr.grSize; i++)
		printer->Line(wxPoint(number.left, +(number.top + (i+1) * height)),
                  wxPoint(names.right, +(number.top + (i+1) * height)),
                  THINN_FRAME);

  // Dicke Linie zwischen Namen und Raster
  printer->Line(wxPoint(names.right - 1, names.top),
                wxPoint(names.right - 1, names.top + totalHeight),
                THICK_FRAME);

  // Abtrennen der Nummern im Namen
  printer->Line(wxPoint(number.right, (number.top + height)),
								wxPoint(number.right, (number.top + totalHeight)),
								THINN_FRAME);

}


void  RasterRR::PrintRaster()
{
  // Raster
  int startX = raster.left;
  int startY = offsetY;
  int totalHeight = (gr.grSize + 1) * height;

  // Raster: Umrandung
  CRect tmp(raster.left, startY, raster.right, startY + totalHeight);
  printer->Rectangle(tmp, THINN_FRAME);

  // Senkrechte und waagrechte Linien im Raster selbst
  for (int i = 1; i < gr.grSize; i++)
    printer->Line(wxPoint(raster.left + i * rw, +startY),
									wxPoint(raster.left + i * rw, +(startY + totalHeight)),
                  THINN_FRAME);

  for (int i = 0; i < gr.grSize; i++)
    printer->Line(wxPoint(raster.left, +(startY + height + i * height)),
                  wxPoint(raster.right, +(startY + height + i * height)),
                  THINN_FRAME);

	// grosse 'X'
  for (int i = 0; i < gr.grSize; i++)
	{
		printer->Line(wxPoint(raster.left + i * rw, +(startY + height + i * height)),
                  wxPoint(raster.left + (i+1) * rw, +(startY + height + (i+1) * height)),
                  THINN_FRAME);

    printer->Line(wxPoint(raster.left + (i+1) * rw, +(startY + height + i * height)),
									wxPoint(raster.left + i * rw, +(startY + height + (i+1) * height)),
                  THINN_FRAME);
  }
}


void  RasterRR::PrintResults()
{
	// Ergebnisteil
	int startX = match.left;
  int startY = offsetY;
	int totalHeight = (gr.grSize + 1) * height;

  // Dicke Linie zwischen Raster und Ergebnisteil
  printer->Line(wxPoint(match.left, startY),
					      wxPoint(match.left, startY + totalHeight), 
                THICK_FRAME);

  // Rechteck um alles
  CRect tmp(match.left, startY, place.right, startY + totalHeight);
	printer->Rectangle(tmp, THINN_FRAME);

  // horizontale Linien
  for (int i = 0; i < gr.grSize; i++)
		printer->Line(wxPoint(match.left, +(startY + (i+1) * height)),
                  wxPoint(place.right, +(startY + (i+1) * height)),
                  THINN_FRAME);

  // Linien zwischen Matches - Games - Place
  startX = games.left;
  printer->Line(wxPoint(games.left, startY),
								wxPoint(games.left, startY + totalHeight),
								THINN_FRAME);

  startX = place.left;
	printer->Line(wxPoint(place.left, +startY),
                wxPoint(place.left, +(startY + totalHeight)),
                THINN_FRAME);

}


void  RasterRR::WriteNames()
{
	// Text der Ueberschrift
	int startX = names.left;
	int startY = offsetY;

	// Namen der Spieler / Mannschaften drucken
	CRect  reg = names;
	wxString tmp;
	
	if (cp.cpType == CP_TEAM)
	  tmp = _("Teams");
	else
	  tmp = _("Players");
	
	if (options.rrCombined)	
    PrintStringCentered(tmp, reg);
  else
  {
	  short grFont = printer->DeriveFont(textFont, 700, 0);
  	// short  grFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_GROUP);  
	  short oldFont = printer->SelectFont(grFont);
	  
	  PrintStringCentered(gr.grDesc, reg);
	  
	  printer->SelectFont(oldFont);
	  printer->DeleteFont(grFont);
  }
  
  for (std::vector<TbItem *>::iterator it = tbList.begin(); it != tbList.end(); it++)
	{
    CRect regName = reg;
    regName.top += (*it)->st.stNr * height;
    regName.bottom = regName.top + height;

		// PrintTeam(TM_ENTRY(tm), reg);
    long flags = FLAG_PRINT_FNAME;
    int what = (cp.cpType == CP_TEAM ? CTT32App::instance()->GetPrintAssociationTeam() : CTT32App::instance()->GetPrintAssociation());
    if (what == FLAG_PRINT_ASSOC_NAME)
      flags |= FLAG_PRINT_NATION;
    else if (what == FLAG_PRINT_ASSOC_DESC)
      flags |= FLAG_PRINT_NATION | FLAG_PRINT_NADESC;
    else if (what == FLAG_PRINT_ASSOC_REGION)
      flags |= FLAG_PRINT_NATION | FLAG_PRINT_NAREGION;

		PrintEntry((*it)->entry, regName, flags);
	}
}


void  RasterRR::WriteRaster()
{
	// Durchnummerieren
	for (int i = 0; i < gr.grSize; i++)
	{
		wxChar str[4];
		_itot(i+1, str, 10);

		int textWidth = printer->TextWidth(str, textFont);
		int textHeight = printer->TextHeight(str, textFont);

		CRect  reg = number;

		// 1. Spalte

		reg.top += (i+1) * height;
		reg.bottom += (i+1) * height;

		PrintStringCentered(str, reg);

		// Kopfzeile
		reg.left = raster.left + i * rw;
		reg.right = reg.left + rw;
		reg.top = raster.top;
		reg.bottom = reg.top + height;

		PrintStringCentered(str, reg);
	} // for (durchnummerieren)

	short  smallFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_SMALL);

  for (std::vector<MtRec>::iterator it = mtList.begin(); it != mtList.end(); it++)
	{
    MtRec  mt = (*it);
    StRec  stA, stX;

    for (std::vector<TbItem *>::iterator stIt = tbList.begin(); stIt != tbList.end(); stIt++)
    {
      if ((*stIt)->st.stID == mt.stA)
        stA = (*stIt)->st;
      else if ((*stIt)->st.stID == mt.stX)
        stX = (*stIt)->st;
    }
		// Eingetragen werden nur beendete Spiele
		bool  finished = mt.IsFinished();
    bool  isBye = mt.IsABye() || mt.IsXBye();

		if (!finished && !rrSchedule && !rrIgnore)
			continue;

		int  teamA = stA.stNr;
		int  teamX = stX.stNr;

		// Sicherheit: sollte nie vorkommen,
		// kann aber passieren, dass ein Spieler noch nicht bekannt ist
		if (!rrIgnore && (!teamA || !teamX))
			continue;

		CRect regAX;   // Clipping-Reg im Raster
		regAX.left = raster.left + (teamX-1) * rw;
		regAX.right = regAX.left + rw;
		regAX.top = raster.top + height + (teamA-1) * height;
		regAX.bottom = regAX.top + height;

		CRect regXA;   // Clipping-Reg im Raster
		regXA.left = raster.left + (teamA-1) * rw;
		regXA.right = regXA.left + rw;
		regXA.top = raster.top + height + (teamX-1) * height;
		regXA.bottom = regXA.top + height;

		wxChar str[8];

		// Eventuell Zeiten einzeichnen
		if ( rrSchedule && (!finished || (isBye && rrIgnore)) )
		{
			short  oldFont   = printer->SelectFont(smallFont);

			int  height = regAX.GetHeight() / 3;
			regAX.bottom = regAX.top + height;
			regXA.bottom = regXA.top + height;

			if (mt.mtPlace.mtDateTime.day)
			{
        PrintDate(mt.mtPlace.mtDateTime, regAX, wxALIGN_CENTER);
        PrintDate(mt.mtPlace.mtDateTime, regXA, wxALIGN_CENTER);
			}

			regAX.top = regAX.bottom;
			regAX.bottom = regAX.top + height;
			regXA.top = regXA.bottom;
			regXA.bottom = regXA.top + height;

			if (mt.mtPlace.mtDateTime.hour)
			{
        PrintTime(mt.mtPlace.mtDateTime, regAX, wxALIGN_CENTER);
        PrintTime(mt.mtPlace.mtDateTime, regXA, wxALIGN_CENTER);
			}

			regAX.top = regAX.bottom;
			regAX.bottom = regAX.top + height;
			regXA.top = regXA.bottom;
			regXA.bottom = regXA.top + height;

			if (mt.mtPlace.mtTable)
			{
				wxSprintf(str, "T%d", mt.mtPlace.mtTable);

				PrintStringCentered(str, regAX);
				PrintStringCentered(str, regXA);
			}

			printer->SelectFont(oldFont);

			continue;
		}

		if (isBye)
			continue;

		// A - X
    if (mt.mtWalkOverA) // || mt.mtWalkOverX)
      wxSprintf(str, _("w/o"));
    else if (mt.mtInjuredA)
      wxSprintf(str, _("Inj."));
    else if (mt.mtDisqualifiedA)
      wxSprintf(str, _("Disqu."));
    else if (mt.mtResA || mt.mtResX)  // 0 : 0 unterdruecken
      wxSprintf(str, "%1i : %1i", mt.mtResA, mt.mtResX);
    else
      *str = '\0';

		PrintStringCentered(str, regAX);

	  // X - A
    if (mt.mtWalkOverX) //  || mt.mtWalkOverA)
      wxSprintf(str, _("w/o"));
    else if (mt.mtInjuredX)
      wxSprintf(str, _("Inj."));
    else if (mt.mtDisqualifiedX)
      
      wxSprintf(str, _("Disqu."));
    else if (mt.mtResA || mt.mtResX)  // 0 : 0 unterdruecken
      wxSprintf(str, "%1i : %1i", mt.mtResX, mt.mtResA);
    else
      *str = '\0';

		PrintStringCentered(str, regXA);
	} // for (alle Spiele)

	printer->DeleteFont(smallFont);
}


void  RasterRR::WriteResults()
{
	// Spiele, Punkte, Platz
	wxString  tmp;
	
	PrintStringCentered(_("Mt. Pts."), match);
	if (cp.cpType == CP_TEAM)
	  PrintStringCentered(_("Matches"), games);
	else
	  PrintStringCentered(_("Games"), games);
	PrintStringCentered(_("Place"), place);

	for (unsigned i = 0; i < tbList.size(); i++)
	{
    // String zum Aufnehmen der Zahlen
		wxChar  str[16];
    int     tW;          // Breite des Strings
		int     tH;          // Hoehe des Strings
    int     stNr;        // Nummer in teamDesc

		stNr = tbList[i]->st.stNr;

		// Weiter, wenn stNr == 0 (Freilose)
    if (stNr == 0)
      continue;

    if (CTT32App::instance()->GetTable() == TT_ITTF)
    {
	    // Wenn es weder MatchPts noch Saetze gibt, und Platz ist 1, dann weiter
		  if (!tbList[i]->result.matchPoints &&
			 	  !tbList[i]->result.sets[0] && !tbList[i]->result.sets[1] &&
			 	  tbList[i]->result.pos == 1)
		    continue;
	  }
	  else
	  {
		  if (!tbList[i]->result.points[0] && !tbList[i]->result.points[1] &&
		  		tbList[i]->result.pos == 1)
			  continue;
    }

		// Print Match Points
    if (CTT32App::instance()->GetTable() == TT_ITTF)
    {
		  wxSprintf(str, "%i", tbList[i]->result.matchPoints);
		}
    else
    {
		  wxSprintf(str, "%2i : %2i", 
              tbList[i]->result.points[0],
						  tbList[i]->result.points[1]);
		}

		tW = printer->TextWidth(str, textFont);
		tH = printer->TextHeight(str, textFont);

    CRect regMPts = match;
    regMPts.top = baseLine + (stNr-1) * height + height-tH;
    regMPts.bottom = regMPts.top + tH;

    PrintStringCentered(str, regMPts);

	  // Print games (Saetze)
	  CRect  regSets = games;
	  regSets.top = baseLine + stNr * height - tH;
	  regSets.bottom = regSets.top + tH;
 
 	  // In Team-Events Spiele drucken, ansonsten die Saetze 
	  if (cp.cpType == CP_TEAM)
      wxSprintf(str, "%2i : %2i", 
              tbList[i]->result.matches[0], tbList[i]->result.matches[1]);
	  else
      wxSprintf(str, "%2i : %2i", 
              tbList[i]->result.sets[0], tbList[i]->result.sets[1]);

    PrintStringCentered(str, regSets);

	  // Print Position
    CRect regPos = place;
    regPos.top = baseLine + stNr * height - tH;
    regPos.bottom = regPos.top + tH;

    wxSprintf(str, "%i", tbList[i]->result.pos);

    PrintStringCentered(str, regPos);
  }; // for (i)
}

