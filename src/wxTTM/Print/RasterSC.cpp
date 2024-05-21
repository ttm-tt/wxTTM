/* Copyright (C) 2020 Christoph Theis */

// ---------------------------------------------------------------------
//                     RasterScore
//
#include "stdafx.h"

#include "RasterSC.h"

#include "TT32App.h"

#include "MtEntryStore.h"
#include "NmEntryStore.h"
#include "CpStore.h"
#include "GrStore.h"
#include "IdStore.h"
#include "UpListStore.h"
#include "SyListStore.h"
#include "Res.h"

#include  <io.h>
#include  <stdlib.h>

#include  <sstream>
#include  <iomanip>

#undef _
#define _(a) printer->GetString(a)

#define STR_DATE    IDS_DATE
#define STR_TIME    IDS_TIME
#define STR_TABLE   IDS_TABLE

#define STR_SETS    IDS_GAMES
#define STR_RESULT  IDS_RESULT

// Width of entry for 'A' etc
// We need to change it for XTSA
static int  WIDTH_AX;


// Definition von Strings
// Def. von min und max
# ifdef TURBO
  inline int  min(int a, int b) {return (a < b ? a : b);}
  inline int  max(int a, int b) {return (a > b ? a : b);}
# endif




RasterScore::RasterScore(Printer *prt, Connection *ptr)
						: RasterBase(prt, ptr)
{
};


RasterScore::~RasterScore()
{
};


// Score-Sheet Funktionen
// Kopf des Score-Sheets (rechtsbuendig)

/*
																	Tag   Zeit   Tisch
                                                 +-----+------+-------+
     Wettbewerb                                  + Tag | Zeit | Tisch |
     ------------------------------------------- +--------------------+
     Gruppe                                      + Rd  |Match No      +
																								 +--------------------+
*/

int  RasterScore::PrintScoreHeader(const CpRec &cp, const GrRec &gr, const MtEntry &mt)
{
  // Logo, irgendwo. WTTC-Logo.gif ist 296px x 408px
  wxImage logo;
  if (CTT32App::instance()->GetPrintLogo() && IdStore::GetLogoImage(logo))
  {
    // Breite wird entsprechend der Hoehe skaliert  
    wxPoint topLeft(0, 0), bottomRight(-1, -1);

    printer->DrawImage(topLeft, bottomRight, wxSize(2500, 4000), logo);
  }

  MtEntry  mttm = mt;

	short  oldFont = printer->SelectFont(textFont);

	// Breite ist ~ 32 Zeichen
  // int  start = printer->width - sizeof(cp.cpDesc) * printer->cW - 2 * space;
  int start = (printer->width * 3 ) / 4 - 4 * printer->cW;
  int height = 2 * printer->cH;

  // Eine Region fuer alles
  CRect regPlace;
  regPlace.left = start;
  regPlace.right = printer->width;
	regPlace.top = offsetY;
	regPlace.bottom = regPlace.top + height;

	// Regions fuer Tag, Zeit, Tisch
  CRect  regDate = regPlace;
  CRect  regTime = regPlace;
	CRect  regTable = regPlace;

  regDate.right = regDate.left + (regPlace.GetWidth() / 3);
  regTime.left = regDate.right;
  regTime.right = regTime.left + (regPlace.GetWidth() / 3);
  regTable.left = regTime.right;

  // Und einen Rahmen um alles
  printer->Rectangle(regPlace, THICK_FRAME, FALSE);

	// Linien zur Absperrung
	printer->Line(regDate.right, regDate.top, regDate.right, regDate.bottom, THINN_FRAME);
  printer->Line(regTime.right, regTime.top, regTime.right, regTime.bottom, THINN_FRAME);

  // Woerter drucken
  PrintStringCentered(_("Date"), regDate);
	PrintStringCentered(_("Time"), regTime);
  PrintStringCentered(_("Table"), regTable);

  // Eine Zeile weiter
  regPlace.top = regPlace.bottom; regPlace.bottom += height;
  regDate.top = regDate.bottom; regDate.bottom += height;
  regTime.top = regTime.bottom; regTime.bottom += height;
  regTable.top = regTable.bottom; regTable.bottom += height;

  // Und einen Rahmen um alles
	printer->Rectangle(regPlace, THICK_FRAME, FALSE);

  // Linien zur Absperrung
	printer->Line(regDate.right, regDate.top, regDate.right, regDate.bottom, THINN_FRAME);
  printer->Line(regTime.right, regTime.top, regTime.right, regTime.bottom, THINN_FRAME);
  
  regDate.left += space;
  regTime.left += space;

	if (mttm.mt.mtPlace.mtDateTime.day)
		PrintDate(mttm.mt.mtPlace.mtDateTime, regDate, "%d %b", wxALIGN_CENTER);

	if (mttm.mt.mtPlace.mtDateTime.hour)
  {
    if (mttm.mt.mtPlace.mtDateTime.second)
      PrintStringCentered(wxString::Format("%02d:%02d #%d", mttm.mt.mtPlace.mtDateTime.hour, mttm.mt.mtPlace.mtDateTime.minute, mttm.mt.mtPlace.mtDateTime.second), regTime);
    else
		  PrintTime(mttm.mt.mtPlace.mtDateTime, regTime, "%H:%M", wxALIGN_CENTER);
  }

	if (mttm.mt.mtPlace.mtTable)
	{
 		// Der Tisch erscheint groesser
		// PrintInt(mt.mtPlace.mtTable, regTable);
		wxChar  str[17] = {0};
		_itot(mttm.mt.mtPlace.mtTable, str, 10);

		short cpFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_COMP);
		short oldFont = printer->SelectFont(cpFont);

		PrintStringCentered(str, regTable);

		printer->SelectFont(oldFont);
		printer->DeleteFont(cpFont);
	}

	offsetY = regPlace.bottom;

	// Eine dicke Linie auf dieser Hoehe
	printer->Line(offsetX + printer->width / 4, offsetY, 
	              regPlace.left - 2 * space, offsetY, THICK_FRAME);

	// WB obendrueber, Gruppe drunter

	CRect  regCP;
	CRect  regGR;

	short  fontCP;
	short  fontGR;

	fontCP = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_COMP);
	fontGR = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_GROUP);

	regCP.bottom = offsetY;
	regCP.top = offsetY - 1.5 * printer->TextHeight(cp.cpDesc, fontCP);
	regCP.left = offsetX + printer->width / 4;
	regCP.right = offsetX + regPlace.left + 2 * space;

	regGR.top = offsetY;
	regGR.bottom = offsetY + 1.5 * printer->TextHeight(cp.cpDesc, fontCP);
	regGR.left = offsetX + printer->width / 4;
	regGR.right = offsetX + regPlace.left + 2 * space;

	printer->SelectFont(fontCP);
	PrintStringCentered(cp.cpDesc, regCP);

	printer->SelectFont(fontGR);
	PrintStringCentered(gr.grDesc, regGR);

	printer->SelectFont(textFont);
	printer->DeleteFont(fontCP);
	printer->DeleteFont(fontGR);

	// Runde und Spielnummer

	CRect  regRd;
	CRect  regMT;
	regRd.top = offsetY;
	regRd.bottom = offsetY + height;
	regRd.left = regPlace.left;
	regRd.right = regDate.right;
	regMT.top = regRd.top;
	regMT.bottom = regRd.bottom;
	regMT.left = regRd.right;
	regMT.right = regPlace.right;

	// Rahmen drum
	printer->Rectangle(regRd, THICK_FRAME, FALSE);
  printer->Rectangle(regMT, THICK_FRAME, FALSE);

	// Rundeninformation
	wxChar  str[64];
  wxString strRd = _("Rd.");
	wxString strSF = _("SF");
	wxString strQF = _("QF");
  wxString strQual = _("Qu.");

  if (gr.grModus == MOD_RR)
    wxSprintf(str, "%s: %2i", strRd.t_str(), mttm.mt.mtEvent.mtRound);
  else if (gr.grWinner > 1 || gr.grNofRounds > 0 || gr.grNofMatches > 0)
    wxSprintf(str, "%s: %2i", strRd.t_str(), mttm.mt.mtEvent.mtRound);
  else if (gr.NofRounds() == mttm.mt.mtEvent.mtRound && mttm.mt.mtEvent.mtMatch == 1)
	 wxSprintf(str, "%s: F", strRd.t_str());
	else if (gr.NofRounds() == mttm.mt.mtEvent.mtRound + 1 && mttm.mt.mtEvent.mtMatch <= 2)
		wxSprintf(str, "%s: %s", strRd.t_str(), strSF.t_str());
  else if (gr.NofRounds() == mttm.mt.mtEvent.mtRound + 2 && mttm.mt.mtEvent.mtMatch <= 4)
    wxSprintf(str, "%s: %s", strRd.t_str(), strQF.t_str());
  else if (mttm.mt.mtEvent.mtRound <= gr.grQualRounds)
    wxSprintf(str, "%s: %s", strRd.t_str(), strQual.t_str());
	else
    wxSprintf(str, "%s: %2i", strRd.t_str(), mttm.mt.mtEvent.mtRound - gr.grQualRounds);

  regRd.left += space;
	PrintString(str, regRd);

	// Spielnummer, aber vom _richtigen_ Spiel
	wxString strMtNr = _("Match No.");
	wxSprintf(str, "%s:  %li", strMtNr.t_str(), mttm.mt.mtNr);
	
	regMT.left += space;
	PrintString(str, regMT);

  offsetY = regMT.bottom;
  offsetY += printer->cH;

	return 0;
}


// Das Ergebnis fuer ein einzelnes Match drucken (hier zum selber ausfuellen)
// Aufbau
/*
	 +-------------------------------+-------+-------+-------+-------+
	 |    'Player(s)'                |'        Sets          | Result|
	 +-------------------------------+-------+-------+-------+-------+
	 |     Team A                    |       |       |       |       |
	 +-------------------------------+-------+-------+-------+-------+
	 |     Team X                    |       |       |       |       |
	 +-------------------------------+-------+-------+-------+-------+

	 +-------------------------------+
	 + 'Winner                       |
	 +-------------------------------+
	 +                               |
	 +-------------------------------+

	 +-------------------------------+-----------------------------+
	 +                     'Signature of Players'                  |
	 +-------------------------------+-----------------------------+
	 +                               |                             |
	 +-------------------------------+-----------------------------+

	 +-------------------------------+-----------------------------+
	 + 'Umpire'                      |       'Signature'           |
	 +-------------------------------+-----------------------------+
	 +                               |                             |
	 +-------------------------------+-----------------------------+

	 +------+------+------+
	 +  1   |  2   |  3   |
	 +--------------------+
	 +      |      |      |
	 +------+------+------+

*/

int  RasterScore::PrintScore(const MtEntry &mt)
{
  short textFont = -1; 
  short oldFont = -1; 

  textFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_MEDIUM);
  oldFont = printer->SelectFont(textFont);     

  // Aufsetzen der einzelnen Bestandteile
  CRect  regTm;         // Teams (1 - 2)
  CRect  regSet;        // Saetze 1 - 3
  CRect  regWinner;     // Sieger der Partie
  CRect  regSets;       // Ergebnis der Saetze
  CRect  regUmpire;     // Schiri und Unterschrift
  CRect  regSignature;  // Unterschrift Spieler

	// Define width of entries	
	WIDTH_AX = (6 * printer->cW);

	bool isDouble = cp.cpType == CP_DOUBLE || cp.cpType == CP_MIXED;
	
	int top = offsetY;
  int left = offsetX;
  int heightTop = 2 * printer->cH;  // Hoehe Ueberschrift
  int heightBot = 3 * printer->cH;  // Hoehe Eintraege

  // Insgesamt gibt es vier Gruppen
  CRect  regGroup;
  regGroup.left = left;
  regGroup.right = left + printer->width / 2;
  regGroup.top = offsetY;
  regGroup.bottom = regGroup.top + heightTop + 2 * heightBot;

  // Die rechte Haelfte der Seite enthaelt die Saetze und das Ergebnis
  double width = (1. * printer->width - left) / (2 * (mt.mt.mtBestOf + 1));

  // Erste Gruppe: Spieler und einzelne Saetze
  regGroup.right = printer->width;

  // Dicken Rahmen drum
  printer->Rectangle(regGroup, THICK_FRAME, FALSE);

  // Und Abteilung: Team A
  printer->Line(regGroup.left, regGroup.top + heightTop,
					 regGroup.right, regGroup.top + heightTop);

  // und Team X
  printer->Line(regGroup.left, regGroup.top + heightTop + heightBot,
					 regGroup.right, regGroup.top + heightTop + heightBot);

  // Eintraege: Spieler
  // Ueberschrift: Players
  regTm = regGroup;
  regTm.bottom = regTm.top + heightTop;
  regTm.right = regTm.left + printer->width / 2;

  if (CTT32App::instance()->GetPrintScoreServiceTimeout())
    regTm.right -= 2 * WIDTH_AX;
	if (CTT32App::instance()->GetPrintScoreCards())
		regTm.right -= 3 * WIDTH_AX;

	PrintStringCentered(_("Players"), regTm);
	printer->Line(regTm.right, regTm.top, regTm.right, regGroup.bottom);

	CRect regYR = regGroup;  // Service / Timeout / Cards
	
	regYR.bottom = regTm.bottom;

  regYR.left = regTm.right;
  regYR.right = regYR.left + WIDTH_AX;

  if (CTT32App::instance()->GetPrintScoreServiceTimeout())
  {
    PrintStringCentered(_("S"), regYR);

		// Seperating lines for service/return per player
		if (isDouble)
		{
			printer->Line(
				regYR.left, regGroup.top + heightTop + heightBot / 2,
				regYR.right, regGroup.top + heightTop + heightBot / 2,
				THINN_FRAME, wxPENSTYLE_SHORT_DASH);
			printer->Line(
				regYR.left, regGroup.bottom - heightBot / 2,
				regYR.right, regGroup.bottom - heightBot / 2,
				THINN_FRAME, wxPENSTYLE_SHORT_DASH);
		}

	  printer->Line(regYR.right, regYR.top, regYR.right, regGroup.bottom);

    regYR.left = regYR.right;
    regYR.right = regYR.left + WIDTH_AX;

	  PrintStringCentered(_("T"), regYR);
	  printer->Line(regYR.right, regYR.top, regYR.right, regGroup.bottom);

    regYR.left = regYR.right;
    regYR.right = regYR.left + WIDTH_AX;
  }

	if (CTT32App::instance()->GetPrintScoreCards())
	{
		PrintStringCentered(_("Y"), regYR);
		printer->Line(regYR.right, regYR.top, regYR.right, regGroup.bottom);
		if (isDouble)
		{
			printer->Line(
				regYR.left, regGroup.top + heightTop + heightBot / 2,
				regYR.right, regGroup.top + heightTop + heightBot / 2,
				THINN_FRAME, wxPENSTYLE_SHORT_DASH);
			printer->Line(
				regYR.left, regGroup.bottom - heightBot / 2,
				regYR.right, regGroup.bottom - heightBot / 2,
				THINN_FRAME, wxPENSTYLE_SHORT_DASH);
		}

		regYR.left = regYR.right;
		regYR.right += WIDTH_AX;
		PrintStringCentered(_("1P"), regYR);
		printer->Line(regYR.right, regYR.top, regYR.right, regGroup.bottom);
		if (isDouble)
		{
			printer->Line(
				regYR.left, regGroup.top + heightTop + heightBot / 2,
				regYR.right, regGroup.top + heightTop + heightBot / 2,
				THINN_FRAME, wxPENSTYLE_SHORT_DASH);
			printer->Line(
				regYR.left, regGroup.bottom - heightBot / 2,
				regYR.right, regGroup.bottom - heightBot / 2,
				THINN_FRAME, wxPENSTYLE_SHORT_DASH);
		}

		regYR.left = regYR.right;
		regYR.right += WIDTH_AX;
		PrintStringCentered(_("2P"), regYR);
		printer->Line(regYR.right, regYR.top, regYR.right, regGroup.bottom, THICK_FRAME);
		if (isDouble)
		{
			printer->Line(
				regYR.left, regGroup.top + heightTop + heightBot / 2,
				regYR.right, regGroup.top + heightTop + heightBot / 2,
				THINN_FRAME, wxPENSTYLE_SHORT_DASH);
			printer->Line(
				regYR.left, regGroup.bottom - heightBot / 2,
				regYR.right, regGroup.bottom - heightBot / 2,
				THINN_FRAME, wxPENSTYLE_SHORT_DASH);
		}
	}

	TmEntry  tmA = mt.tmA, tmX = mt.tmX;

  int flags = FLAG_PRINT_FNAME;
  int what = (cp.cpType == CP_TEAM ? CTT32App::instance()->GetPrintAssociationTeam() : CTT32App::instance()->GetPrintAssociation());
  if (what == FLAG_PRINT_ASSOC_NAME)
    flags |= FLAG_PRINT_NATION;
  else if (what == FLAG_PRINT_ASSOC_DESC)
    flags |= FLAG_PRINT_NATION | FLAG_PRINT_NADESC;
  else if (what == FLAG_PRINT_ASSOC_REGION)
    flags |= FLAG_PRINT_NATION | FLAG_PRINT_NAREGION;

	regTm.top = regTm.bottom;
	regTm.bottom = regTm.top + heightBot;
	// PrintTeam(TM_ENTRY(tma), regTm);
	PrintEntry(tmA, regTm, flags);

	regTm.top = regTm.bottom;
	regTm.bottom = regGroup.bottom;
	// PrintTeam(TM_ENTRY(tmX), regTm);
	PrintEntry(tmX, regTm, flags);

	// Die anderen CRect verlassen sich auf regTm.right in der Mitten
	regTm.right = regTm.left + regGroup.GetWidth() / 2;

	// Drucke Kaesten fuer die Saetze 1 - n
	regSet.top = regGroup.top;
	regSet.bottom = regSet.top + heightTop;
	regSet.left = regTm.right;
	regSet.right = regSet.left + mt.mt.mtBestOf * width;

	PrintStringCentered(_("Games"), regSet);

	// 'Result'
	regSet.left = regSet.right;
	regSet.right += width;

	PrintStringCentered(_("Result"), regSet);
	printer->Line(regSet.left, regSet.top, regSet.left, regSet.bottom, THICK_FRAME);

	// Die Abgrenzungen
	for (int i = 0; i < (mt.mt.mtBestOf + 1); i++)
	{
		regSet.top = regGroup.top + heightTop;
		regSet.bottom = regSet.top + heightTop;
		regSet.left = regTm.right + i * width;
		regSet.right = regSet.left + width;

    // Letzte Linie dick
    if (i == mt.mt.mtBestOf)
      printer->Line(regSet.left, regSet.top, regSet.left, regGroup.bottom, THICK_FRAME);
    else
		  printer->Line(regSet.left, regSet.top, regSet.left, regGroup.bottom);
	}
	
	// Ergebnisse, soweit bekannt
	if (mt.mt.mtResA || mt.mt.mtResX)
	{
	  short res[2] = {0};
	  
	  MtSetStore  mtSet(mt.mt, connPtr);
	  mtSet.SelectAll();
	  
	  while (mtSet.Next())
	  {
	    if (mtSet.mtSet == 0)
	      continue;
	      
	    if (!mtSet.mtResA && !mtSet.mtResX)
	      continue;

      short win = (mtSet.mtSet == mt.mt.mtBestOf ? cp.cpPtsToWinLast : cp.cpPtsToWin);
	      
	    if ( (mtSet.mtResA > mtSet.mtResX) && (mtSet.mtResA >= win) )
	      res[0]++;
	    else if ( (mtSet.mtResX > mtSet.mtResA) && (mtSet.mtResX >= win) )
	      res[1]++;
	      
	    wxChar tmp[10] = {0};
	    
	    CRect  regRes;
	    regRes.top = regGroup.top + heightTop;
	    regRes.bottom = regRes.top + heightBot;	  
	    regRes.left = regTm.right + (mtSet.mtSet - 1) * width;
	    regRes.right = regRes.left + width;
	    
	    _itot(mtSet.mtResA, tmp, 10);
	    PrintStringCentered(tmp, regRes);
	    
	    regRes.top = regRes.bottom;
	    regRes.bottom = regRes.top + heightBot;
	    
	    _itot(mtSet.mtResX, tmp, 10);
	    PrintStringCentered(tmp, regRes);
	  }
	  
	  if (res[0] || res[1])
	  {
	    wxChar tmp[10] = {0};
  	  
	    CRect  regRes;
	    regRes.top = regGroup.top + heightTop;
	    regRes.bottom = regRes.top + heightBot;	  
	    regRes.left = regTm.right + mt.mt.mtBestOf * width;
	    regRes.right = regRes.left + width;
  	  
	    _itot(res[0], tmp, 10);
	    PrintStringCentered(tmp, regRes);
  	  
	    regRes.top = regRes.bottom;
	    regRes.bottom = regRes.top + heightBot;
  	  
	    _itot(res[1], tmp, 10);
	    PrintStringCentered(tmp, regRes);
	  }
	}
	
	offsetY = regGroup.bottom;

	// 2. Sieger und Ergebnishaeuschen
  int winnerHeight = 1;
  
  if (CTT32App::instance()->GetPrintPlayersSignature() && (mt.mt.cpType == CP_DOUBLE || mt.mt.cpType == CP_MIXED))
    winnerHeight = 2;

	regGroup.top = offsetY + 0.5 * heightTop;
	regGroup.bottom = regGroup.top + heightTop + winnerHeight * heightBot;

	regWinner = regGroup;
	regWinner.right = regWinner.left + printer->width / 2;

	// Dicken Rahmen drum und Strich drunter
	printer->Rectangle(regWinner, THICK_FRAME, FALSE);
	printer->Line(regWinner.left, regWinner.top + heightTop,
					regWinner.right, regWinner.top + heightTop, THINN_FRAME);

	// Sieger
	regWinner.bottom = regWinner.top + heightTop;

	// 'Winner'
	PrintStringCentered(_("Winner"), regWinner);

	if (mt.mt.QryWinnerAX())
	{
		CRect regTmp = regWinner;
		regTmp.top = regWinner.bottom;
		regTmp.bottom = regGroup.bottom;
		
    int flags = FLAG_PRINT_FNAME;
    int what = (cp.cpType == CP_TEAM ? CTT32App::instance()->GetPrintAssociationTeam() : CTT32App::instance()->GetPrintAssociation());
    if (what == FLAG_PRINT_ASSOC_NAME)
      flags |= FLAG_PRINT_NATION;
    else if (what == FLAG_PRINT_ASSOC_DESC)
      flags |= FLAG_PRINT_NATION | FLAG_PRINT_NADESC;
    else if (what == FLAG_PRINT_ASSOC_REGION)
      flags |= FLAG_PRINT_NATION | FLAG_PRINT_NAREGION;

		PrintEntry(mt.mt.QryWinnerAX() == +1 ? tmA : tmX, regTmp, flags);
	}

  if (CTT32App::instance()->GetPrintPlayersSignature())
  {
		regSignature = regGroup;
		regSignature.left = regSignature.left + printer->width / 2;

    // Dicken Rahmen drum und Strich drunter
    printer->Rectangle(regSignature, THICK_FRAME, FALSE);
    printer->Line(regSignature.left, regSignature.top + heightTop,
			regSignature.right, regSignature.top + heightTop, THINN_FRAME);

    // Sieger
		regSignature.bottom = regSignature.top + heightTop;

    // 'Signature'
    // Eindhoven
    if (CTT32App::instance()->GetType() == TT_SCI)
      PrintStringCentered(_("Signature of Player(s)"), regSignature);
    else
      PrintStringCentered(_("Signature of Player(s) / Coach(es)"), regSignature);

    // Und eine duenne Linie, wenn Doppel
    if (mt.mt.cpType == CP_DOUBLE || mt.mt.cpType == CP_MIXED)
    {
      printer->Line(regSignature.left, regSignature.top + heightTop + heightBot,
				regSignature.right, regSignature.top + heightTop + heightBot, THINN_FRAME, wxPENSTYLE_SHORT_DASH);
    }
  }

	offsetY = regGroup.bottom;

  if (CTT32App::instance()->GetPrintScoreCoaches())
  {
    // 3. Feld fuer die Coaches
    regGroup.top = offsetY + 0.5 * heightTop;
    regGroup.bottom = regGroup.top + heightTop + heightBot;

    // Im Doppel kann es 2 Coaches geben
    if (cp.cpType == CP_DOUBLE || cp.cpType == CP_MIXED)
      regGroup.bottom += heightBot;

    // Dicken Rahmen
    printer->Rectangle(regGroup, THICK_FRAME, FALSE);
    printer->Line(regGroup.left, regGroup.top + heightTop, regGroup.right, regGroup.top + heightTop, THINN_FRAME);

    int hcenter = regGroup.left + regGroup.GetWidth() / 2;

    // Mitte mit dicken Strich abtrennten
    printer->Line(hcenter, regGroup.top, hcenter, regGroup.bottom, THICK_FRAME);

    // Im Doppel / Mixed rote / gelbe Karten nochmal unterteilen
    if (CTT32App::instance()->GetPrintScoreCards()&& (cp.cpType == CP_DOUBLE || cp.cpType == CP_MIXED))
    {
      printer->Line(
        hcenter, regGroup.top + heightTop + heightBot, 
        hcenter, regGroup.top + heightTop + heightBot, 
        THINN_FRAME, wxPENSTYLE_SHORT_DASH);

      printer->Line(
        regGroup.right, regGroup.top + heightTop + heightBot, 
        regGroup.right, regGroup.top + heightTop + heightBot, 
        THINN_FRAME, wxPENSTYLE_SHORT_DASH);
    }

    // Trennlinie fuer die beiden Spieler im Doppel. Verwarnungen werden einem Spieler ausgesprochen,
    // auch wenn sie für das Paar gelten.
    if (CTT32App::instance()->GetPrintScoreCards() && (cp.cpType == CP_DOUBLE || cp.cpType == CP_MIXED))
      printer->Line(regTm.right, regTm.top + regTm.GetHeight() / 2, 
					regYR.right, regTm.top + regTm.GetHeight() / 2, THINN_FRAME, wxPENSTYLE_SHORT_DASH);
	
    // Ueberschriften
    regSignature = regGroup;
    regSignature.bottom = regSignature.top + heightTop;
  
    regSignature.right = hcenter;
		if (CTT32App::instance()->GetPrintScoreSides())
			regSignature.right -= WIDTH_AX;  // "Sd"
		if (CTT32App::instance()->GetPrintScoreCards())
			regSignature.right -= 2 * WIDTH_AX; // "YR"
		
		printer->Line(regSignature.right, regGroup.top, regSignature.right, regGroup.bottom, THINN_FRAME);

		PrintStringCentered(_("Coach(es)"), regSignature);

		if (CTT32App::instance()->GetPrintScoreSides())
		{
		  regSignature.left = regSignature.right;
			regSignature.right += WIDTH_AX;
			printer->Line(regSignature.right, regGroup.top, regSignature.right, regGroup.bottom, THINN_FRAME);

			PrintStringCentered(_("Sd"), regSignature);

			regSignature.left = regSignature.right;
		}

		if (CTT32App::instance()->GetPrintScoreCards())
		{
			regSignature.left = regSignature.right;
			regSignature.right += WIDTH_AX;
			printer->Line(regSignature.right, regGroup.top, regSignature.right, regGroup.bottom, THINN_FRAME);

			PrintStringCentered(_("Y"), regSignature);
			
			regSignature.left = regSignature.right;

			regSignature.left = regSignature.right;
			regSignature.right += WIDTH_AX;
			printer->Line(regSignature.right, regGroup.top, regSignature.right, regGroup.bottom, THINN_FRAME);

			PrintStringCentered(_("R"), regSignature);

			regSignature.left = regSignature.right;
		}

		regSignature.left = hcenter;
		regSignature.right = regGroup.right;

		if (CTT32App::instance()->GetPrintScoreSides())
			regSignature.right -= WIDTH_AX;  // "Sd"
		if (CTT32App::instance()->GetPrintScoreCards())
			regSignature.right -= 2 * WIDTH_AX; // "YR"

		printer->Line(regSignature.right, regGroup.top, regSignature.right, regGroup.bottom, THINN_FRAME);

		PrintStringCentered(_("Coach(es)"), regSignature);

		if (CTT32App::instance()->GetPrintScoreSides())
		{
			regSignature.left = regSignature.right;
			regSignature.right += WIDTH_AX;
			printer->Line(regSignature.right, regGroup.top, regSignature.right, regGroup.bottom, THINN_FRAME);

			PrintStringCentered(_("Sd"), regSignature);

			regSignature.left = regSignature.right;
		}

		if (CTT32App::instance()->GetPrintScoreCards())
		{
			regSignature.left = regSignature.right;
			regSignature.right += WIDTH_AX;
			printer->Line(regSignature.right, regGroup.top, regSignature.right, regGroup.bottom, THINN_FRAME);

			PrintStringCentered(_("Y"), regSignature);

			regSignature.left = regSignature.right;

			regSignature.left = regSignature.right;
			regSignature.right += WIDTH_AX;
			printer->Line(regSignature.right, regGroup.top, regSignature.right, regGroup.bottom, THINN_FRAME);

			PrintStringCentered(_("R"), regSignature);

			regSignature.left = regSignature.right;
		}

    offsetY = regGroup.bottom;
  }

  if (CTT32App::instance()->GetPrintScoreUmpires())
  {
	  // 5. Gruppe Schiedsrichter
	  regGroup.top = offsetY + 0.5 * heightTop;
	  // Einhoven: Doppelte Hoehe
	  regGroup.bottom = regGroup.top + heightTop + 2 *  heightBot;

	  // Rahmen drum und Strich drunter
	  printer->Rectangle(regGroup, THICK_FRAME, FALSE);
	  printer->Line(regGroup.left, regGroup.top + heightTop,
					   regGroup.right, regGroup.top + heightTop, THICK_FRAME);

	  // Eindhoven: Abteilen
	  printer->Line(regGroup.left, regGroup.top + heightTop + heightBot,
					  regGroup.right, regGroup.top + heightTop + heightBot);

	  // Links 'Umpire'
	  regUmpire = regGroup;
	  regUmpire.right = regUmpire.left + regUmpire.GetWidth() / 2;

	  // Von Unterschrift abtrennen
	  printer->Line(regUmpire.right, regGroup.top,
					   regUmpire.right, regGroup.bottom, THINN_FRAME);

	  regUmpire.bottom = regUmpire.top + heightTop;
	  PrintStringCentered(_("Umpire(s)"), regUmpire);

		if (CTT32App::instance()->GetPrintScoreUmpireName())
		{
			CRect regSRNr(regGroup.left + printer->cW, regGroup.top + heightTop, 
							regGroup.left + regGroup.GetWidth() / 2 - printer->cW, regGroup.bottom);

			char tmpSRNr[64], tmpSR2Nr[64];
			_itoa(mt.mt.mtUmpire, tmpSRNr, 10);
			_itoa(mt.mt.mtUmpire2, tmpSR2Nr, 10);

			UpListStore sr(connPtr), sr2(connPtr);

			if (mt.mt.mtUmpire)
			{
				sr.SelectByNr(mt.mt.mtUmpire);
				sr.Next();
				sr.Close();
			}

			if (mt.mt.mtUmpire2)
			{
				sr2.SelectByNr(mt.mt.mtUmpire2);
				sr2.Next();
				sr2.Close();
			}

			CRect regSR1Nr = regSRNr, regSR2Nr = regSRNr;
			regSR1Nr.bottom = regSR1Nr.top + regSR1Nr.GetHeight() / 2;
			regSR2Nr.top = regSR1Nr.bottom;

			if (sr.WasOK())
				PrintUmpire(sr, regSR1Nr, FLAG_PRINT_NOPLNR | FLAG_PRINT_NATION | FLAG_PRINT_FNAME);
			else if (mt.mt.mtUmpire)
				PrintString(tmpSRNr, regSR1Nr);

			if (sr2.WasOK())
				PrintUmpire(sr2, regSR2Nr, FLAG_PRINT_NOPLNR | FLAG_PRINT_NATION | FLAG_PRINT_FNAME);
			else if (mt.mt.mtUmpire2)
				PrintString(tmpSR2Nr, regSR2Nr);
    }

	  // rechts "Signature of Umpire"
	  regUmpire.left = regUmpire.right;
	  regUmpire.right = regGroup.right;
	  PrintStringCentered(_("Signature of Umpire(s)"), regUmpire);

	  offsetY = regGroup.bottom;
  }

  printer->SelectFont(oldFont);
  printer->DeleteFont(textFont);

	return 0;
}


// ------------------------------------------------------------------
// ScoreSheet fuer Mannschaften

/*
	+-------------------------------+---------------------------+
	|      Team A                               Team X          |
	+-+-----------------------------+-+-------------------------+
	|A|                             |X|                         |
	+-+-----------------------------+-+-------------------------+

	+-------------------------------+---------------------------+
	|                               |                           |
	+-------------------------------+---------------------------+
	|                               |                           |
	+-------------------------------+---------------------------+

	+--+---------------+---------------+----+----+----+-----+---+
	|       Players    |    Players    |1. G|2. G|3. G|Games|MSc|
	+--+---------------+---------------+----+----+----+-----+---+
	|  |               |  |            |    |    |    |     |   |
	+--+---------------+--+------------+----+----+----+-----+---+
	|  |               |  |            |    |    |    |     |   |
	+--+---------------+--+------------+----+----+----+-----+---+
	|  |               |  |            |    |    |    |     |   |
	+--+---------------+--+------------+----+----+----+-----+---+
	| Winner                                          |     |   |
	+-----------------------------------------------------------+

	+-------------------------------+---------------------------+
	|                    Signatures of Team Captains            |
	+-+-----------------------------+-+-------------------------+
	|A|                             |X|                         |
	+-+-----------------------------+-+-------------------------+

	+-------------------------------+---------------------------+
	| Umpires                       | Signature                 |
	+-------------------------------+---------------------------+
	|                               |                           |
	+-------------------------------+---------------------------+

  +------+------+------+
  +  1   |  2   |  3   |
	+--------------------+
	+      |      |      |
	+------+------+------+
	
*/

int  RasterScore::PrintScoreTM(const MtEntry &mt)
{
  // Spielsystem
  SyListStore  sy(connPtr);  
  sy.SelectByGr(gr);
  sy.Next();
  sy.Close();   
  
  // Correction for special systems
  int sySingles = sy.sySingles;
  int syDoubles = sy.syDoubles;

	if (wxStrcmp(sy.syName, wxT("OTS")) == 0)
		sySingles = 3;
	else if (wxStrcmp(sy.syName, wxT("ETS")) == 0)
		sySingles = 4;
	else if (wxStrcmp(sy.syName, wxT("XTS")) == 0)
		sySingles = 2;
	else if (wxStrcmp(sy.syName, wxT("XTSA")) == 0)
		sySingles = 5;
	else if (wxStrcmp(sy.syName, wxT("YSTA")) == 0)
		sySingles = 4;
  
  short textFont =  sy.syMatches + sySingles + syDoubles > 12 ?
                    printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_SMALL) :
                    printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_MEDIUM);
                    
  short oldFont = printer->SelectFont(textFont);  

	// Define width of entries	
	WIDTH_AX = (6 * printer->cW);

	int top = offsetY;
	int left = offsetX;
	int heightTop = 2 * printer->cH;  // Hoehe Ueberschrift
	int heightBot = 2 * printer->cH;  // Hoehe Eintraege
	int cW = printer->cW;

	double width = 1. * printer->width / (2 * (mt.mt.mtBestOf +1));
	const wxChar *alpha[][3] = 
	{
		{wxT("A"), wxT("B"), wxT("C")},
		{wxT("X"), wxT("Y"), wxT("Z")}
	};
	const wxChar *xts[][4] = 
	{
		{wxT("A-G2"), wxT("A-B2"), wxT("A-G1"), wxT("A-B1")},
		{wxT("X-G2"), wxT("X-B2"), wxT("X-G1"), wxT("X-B1")}
	};
	const wxChar *xtsa[][10] = 
	{
		{wxT("A-G1"), wxT("A-B1"), wxT("A-G2"), wxT("A-B2"), wxT("A-Res"),
		 wxT("A-G1/Res"), wxT("A-B1/Res"), wxT("A-G2/Res"), wxT("A-B2/Res")},
		{wxT("X-G1"), wxT("X-B1"), wxT("X-G2"), wxT("X-B2"), wxT("X-Res"),
		 wxT("X-G1/Res"), wxT("X-B1/Res"), wxT("X-G2/Res"), wxT("X-B2/Res")}
	};
	const wxChar *ysta[][6] =
	{
		{wxT("A1"), wxT("A2"), wxT("A3"), wxT("A4"), wxT("A1/Res"), wxT("A2/Res")},
		{wxT("X1"), wxT("X2"), wxT("X3"), wxT("X4"), wxT("X1/Res"), wxT("X2/Res")}
	};

	// Strings are longer in XTSA and YSTA
	if (wxStrcmp(sy.syName, wxT("XTSA")) == 0)
		WIDTH_AX += 6 * printer->cW;
	else if (wxStrcmp(sy.syName, wxT("YSTA")) == 0)
		WIDTH_AX += 2 * printer->cW;

	// Insgesamt gibt es ??? Gruppen
	CRect  regGroup;

  // Ueberschrift: Mannschaften, wie sie spielen
  regGroup.left  = left;
  regGroup.right = left + printer->width;
  regGroup.top   = offsetY;
  regGroup.bottom = regGroup.top + heightTop;

	int hcenter = regGroup.left + regGroup.GetWidth() / 2;

  if (mt.tmA.tmID || mt.tmX.tmID)
  {
    short boldFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_COMP);
    short oldFont = printer->SelectFont(boldFont);
    
    wxString strA, strX;
    strA = mt.tmA.team.tm.tmName;
    strA += "    ";
    strA += mt.tmA.team.tm.tmDesc;
    
    strX = mt.tmX.team.tm.tmName;
    strX += "    ";
    strX += mt.tmX.team.tm.tmDesc;    
    
    CRect regTmA = regGroup;
    CRect regTmX = regGroup;
    CRect regDash = regGroup;
    
    regTmA.right = regTmA.left + (regTmA.right - regTmA.left) / 2 - printer->cW;
    regTmX.left = regTmA.right + 2 * printer->cW;

    regDash.left  = regTmA.right;
    regDash.right = regTmX.left;

    // regTmA.left += (regTmA.right - regTmA.left) / 3;
    // regTmX.left += (regTmX.right - regTmX.left) / 3;

    // PrintEntry(mt.tmA, regTmA, 0);
    // PrintEntry(mt.tmX, regTmX, 0);
    PrintStringCentered(mt.mt.mtReverse ? strX : strA, regTmA);
    PrintStringCentered(mt.mt.mtReverse ? strA : strX, regTmX);
    PrintStringCentered(wxT("-"), regDash);

    printer->SelectFont(oldFont);
    printer->DeleteFont(boldFont);
  }

  offsetY = regGroup.bottom + heightBot;

	// --- Erste Gruppe: Mannschaftsnamen ---
	regGroup.left = left;
	regGroup.right = left + printer->width;
	regGroup.top = offsetY;
	regGroup.bottom = regGroup.top + heightTop + 1 * heightBot;

	// Dicken Rahmen
	printer->Rectangle(regGroup, THICK_FRAME, FALSE);
	
	// Mitte abteilen
	printer->Line(hcenter, regGroup.top, hcenter, regGroup.bottom, THICK_FRAME);
	
	CRect  regTeamNames = regGroup;
	regTeamNames.bottom = regTeamNames.top + heightTop;
	
  // Nomination existiert
  bool nmExists = NmEntryStore(connPtr).ExistsForMt(mt.mt);
	{
	  // Geklammert, damit keine Konfusionen wg. der Variablennamen entstehen
	  wxString  strTeam = nmExists ? _("Coach(es)") :  _("Team");

    // Team A	
	  CRect reg = regTeamNames;
		reg.right = hcenter;
		if (CTT32App::instance()->GetPrintScoreCards())
			reg.right -= 2 * WIDTH_AX;
    	
	  PrintStringCentered(strTeam + " A", reg);
		printer->Line(reg.right, regGroup.top, reg.right, regGroup.bottom);

		// "Y", "R" (Gelbe / Rote Karte)
		if (CTT32App::instance()->GetPrintScoreCards())
		{
		  reg.left = reg.right;
			reg.right += WIDTH_AX;
			PrintStringCentered(_("Y"), reg);
			printer->Line(reg.right, regGroup.top, reg.right, regGroup.bottom);

			reg.left = reg.right;
			reg.right += WIDTH_AX;
			PrintStringCentered(_("R"), reg);
			printer->Line(reg.right, regGroup.top, reg.right, regGroup.bottom);
		}
	  
	  // Team X
	  reg.left = hcenter;
	  reg.right = regGroup.right;
		if (CTT32App::instance()->GetPrintScoreCards())
			reg.right -= 2 * WIDTH_AX;

		PrintStringCentered(strTeam + " X", reg);
		printer->Line(reg.right, regGroup.top, reg.right, regGroup.bottom);

		// "Y", "R" (Gelbe / Rote Karte)
		if (CTT32App::instance()->GetPrintScoreCards())
		{
			reg.left = reg.right;
			reg.right += WIDTH_AX;
			PrintStringCentered(_("Y"), reg);
			printer->Line(reg.right, regGroup.top, reg.right, regGroup.bottom);

			reg.left = reg.right;
			reg.right += WIDTH_AX;
			PrintStringCentered(_("R"), reg);
			printer->Line(reg.right, regGroup.top, reg.right, regGroup.bottom);
		}
	}

	// Duenne Linie unter den Text
	printer->Line(regTeamNames.left, regTeamNames.bottom,
					 regTeamNames.right, regTeamNames.bottom, THINN_FRAME);

	// Jetzt die Felder fuer die Namen
	regTeamNames.top = regTeamNames.bottom;
	regTeamNames.bottom += heightBot;

	// Und ein A, X
	CRect  regA = regTeamNames;
	regA.right = regA.left + WIDTH_AX;
	// PrintStringCentered("A", regA);

	CRect  regTmNameA = regTeamNames;
	regTmNameA.left = regA.right;
	regTmNameA.right = hcenter;

	CRect regX = regTeamNames;
	regX.left = regTmNameA.right;
	regX.right = regX.left + WIDTH_AX;
	// PrintStringCentered("X", regX);

	CRect  regTmNameX;
	regTmNameX = regTeamNames;
	regTmNameX.left = regX.right;
	
	offsetY = regGroup.bottom;

	// ---- 2. Gruppe: Mannschaftsaufstellung  ---
	regGroup.top = offsetY + 0.5 * heightTop;
	regGroup.bottom = regGroup.top + heightTop;

	CRect  regNomination = regGroup;
	regNomination.right = hcenter;
	if (CTT32App::instance()->GetPrintScoreCards())
		regNomination.right -= 3 * WIDTH_AX;

	PrintStringCentered(_("Nomination"), regNomination);
	printer->Line(regNomination.right, regGroup.top, regNomination.right, regGroup.bottom);

	if (CTT32App::instance()->GetPrintScoreCards())
	{
		// "Y", "1P", "2P" (Gelbe / Rot-Gelbe / Rote Karte)
		regNomination.left = regNomination.right;
		regNomination.right += WIDTH_AX;
	
		PrintStringCentered(_("Y"), regNomination);
	
		regNomination.left = regNomination.right;
		regNomination.right += WIDTH_AX;

		PrintStringCentered(_("1P"), regNomination);
	
		regNomination.left = regNomination.right;
		regNomination.right += WIDTH_AX;

		PrintStringCentered(_("2P"), regNomination);
	}
	
	regNomination.left = hcenter;
	regNomination.right = regGroup.right;

	if (CTT32App::instance()->GetPrintScoreCards())
		regNomination.right -= 3 * WIDTH_AX;

	PrintStringCentered(_("Nomination"), regNomination);
	printer->Line(regNomination.right, regGroup.top, regNomination.right, regGroup.bottom);

	if (CTT32App::instance()->GetPrintScoreCards())
	{
		// "Y", "1P", "2P" (Gelbe / Rot-Gelbe / Rote Karte)
		regNomination.left = regNomination.right;
		regNomination.right += WIDTH_AX;

		PrintStringCentered(_("Y"), regNomination);

		regNomination.left = regNomination.right;
		regNomination.right += WIDTH_AX;

		PrintStringCentered(_("1P"), regNomination);

		regNomination.left = regNomination.right;
		regNomination.right += WIDTH_AX;

		PrintStringCentered(_("2P"), regNomination);
	}

	// Waagrechte Trennlinie
	printer->Line(regGroup.left, regGroup.bottom,
								regGroup.right, regGroup.bottom);
	
  // Jetzt den Rahmen drucken
	// If the first match is a double we print doubles first
	bool wantsDoublesFirst = sy.syList[0].syType == CP_DOUBLE;
	for (int singles = 0; singles < sySingles; ++singles)
	{
	  wxString str;

		CRect reg = regGroup;
		reg.top = regGroup.bottom + ((wantsDoublesFirst ? syDoubles : 0) * 1.7 * heightBot) + (singles * heightBot);
		reg.bottom = reg.top + heightBot;
    
		CRect regA = reg;
		regA.right = regA.left + WIDTH_AX;
    
		CRect regNameA = reg;
		regNameA.left = regA.right;
		regNameA.right = reg.left + reg.GetWidth() / 2;
    
		CRect regX = reg;
		regX.left = regNameA.right;
		regX.right = regX.left + WIDTH_AX;
    
		CRect regNameX = reg;
		regNameX.left = regX.right;
    
		// Die horizontale Trennlinie
		printer->Line(reg.left, reg.bottom, reg.right, reg.bottom);
    
		// Special considerations for XTS(A), but YSTA is printed normally
		if (wxStrcmp(sy.syName, "XTS") == 0)
			str = xts[0][singles];
		else if (wxStrcmp(sy.syName, "XTSA") == 0)
			str = xtsa[0][singles];
		else if (sySingles < 4)
			str = alpha[0][singles];
		else
			str = wxString::Format("A%i", singles+1);

		PrintStringCentered(str, regA);

		// Special considerations for XTS(A), but YSTA is printed normally
		if (wxStrcmp(sy.syName, "XTS") == 0)
			str = xts[1][singles];
		else if (wxStrcmp(sy.syName, "XTSA") == 0)
			str = xtsa[1][singles];
		else if (sySingles < 4)
			str = alpha[1][singles];
		else
			str = wxString::Format("X%i", singles+1);

		PrintStringCentered(str, regX);
	}
	
	for (int doubles = 0; doubles < syDoubles; ++doubles)
	{
		wxString str;
    
		CRect reg = regGroup;
		reg.top = regGroup.bottom + ((wantsDoublesFirst ? 0 : sySingles) * heightBot) + (doubles * 1.7 * heightBot);
		reg.bottom = reg.top + 1.7 * heightBot;
    
		CRect regA = reg;
		regA.right = regA.left + WIDTH_AX;
    
		CRect regNameA = reg;
		regNameA.left = regA.right;
		regNameA.right = reg.left + reg.GetWidth() / 2;
    
		CRect regX = reg;
		regX.left = regNameA.right;
		regX.right = regX.left + WIDTH_AX;
    
		CRect regNameX = reg;
		regNameX.left = regX.right;
    
		// Die horizontale Trennlinie
		printer->Line(reg.left, reg.bottom, reg.right, reg.bottom);
    
 		if (wxStrcmp(sy.syName, "OTS") == 0)
 			str = "C\nA/B";
		else if (wxStrcmp(sy.syName, "XTS") == 0)
			str = "A-B1\nA-G1";
		else if (wxStrcmp(sy.syName, "XTSA") == 0)
			str = "A-B\nA-G";
		else if (syDoubles > 1)
 			str = wxString::Format("DA%i", doubles+1);
 		else
 			str = "DA";

		PrintStringCentered(str, regA);

 		if (wxStrcmp(sy.syName, "OTS") == 0)
 			str = "Z\nX/Y";
		else if (wxStrcmp(sy.syName, "XTS") == 0)
			str = "X-B1\nX-G1";
		else if (wxStrcmp(sy.syName, "XTSA") == 0)
			str = "X-B\nX-G";
		else if (syDoubles > 1)
			str = wxString::Format("DX%i", doubles+1);
		else
			str = "DX";

		PrintStringCentered(str, regX);     
	}

	// Jetzt jede einzelne Meldung drucken.
	NmEntryStore nmA(connPtr);
	nmA.SelectByMtTm(mt.mt, mt.mt.mtReverse ? mt.tmX :  mt.tmA);
	while (nmA.Next())
	{
	  if (nmA.team.cpType == CP_SINGLE && nmA.nmNr > sySingles ||
	      nmA.team.cpType == CP_DOUBLE && nmA.nmNr > syDoubles)
	    continue;
	    
	  if (nmA.team.cpType ==  CP_SINGLE)
	  {
	    CRect reg = regGroup;
	    reg.top = regGroup.bottom + (wantsDoublesFirst ? syDoubles : 0) * 1.7 * heightBot + ((nmA.nmNr - 1) * heightBot);
	    reg.bottom = reg.top + heightBot;

		  CRect regA = reg;
		  regA.right = regA.left + WIDTH_AX;

		  CRect  regNameA = reg;
		  regNameA.left = regA.right;
		  regNameA.right = reg.left + reg.GetWidth() / 2;

		  CRect  regX = reg;
		  regX.left = regNameA.right;
		  regX.right = regX.left + WIDTH_AX;

		  CRect  regNameX = reg;
		  regNameX.left = regX.right;
		  
		  PrintEntry(nmA, regNameA, FLAG_PRINT_FNAME);
		}
		else
		{
	    CRect reg = regGroup;
	    reg.top = regGroup.bottom + (wantsDoublesFirst ? 0 : sySingles) * heightBot + (nmA.nmNr-1) * 1.7 * heightBot;
	    reg.bottom = reg.top + 1.7 * heightBot;

		  CRect regA = reg;
		  regA.right = regA.left + WIDTH_AX;

		  CRect  regNameA = reg;
		  regNameA.left = regA.right;
		  regNameA.right = reg.left + reg.GetWidth() / 2;

		  CRect  regX = reg;
		  regX.left = regNameA.right;
		  regX.right = regX.left + WIDTH_AX;

		  CRect  regNameX = reg;
		  regNameX.left = regX.right;
		  
		  PrintEntry(nmA, regNameA, FLAG_PRINT_FNAME);
		}
	}
	
	nmA.Close();

	NmEntryStore nmX(connPtr);
	nmX.SelectByMtTm(mt.mt, mt.mt.mtReverse ? mt.tmA : mt.tmX);
	while (nmX.Next())
	{
	  if (nmX.team.cpType == CP_SINGLE && nmX.nmNr > sySingles ||
	      nmX.team.cpType == CP_DOUBLE && nmX.nmNr > syDoubles)
	    continue;
	    
	  if (nmX.team.cpType == CP_SINGLE)
	  {
	    CRect reg = regGroup;
	    reg.top = regGroup.bottom + (wantsDoublesFirst ? syDoubles : 0) * 1.7 * heightBot + ((nmX.nmNr - 1) * heightBot);
	    reg.bottom = reg.top + heightBot;

		  CRect regA = reg;
		  regA.right = regA.left + WIDTH_AX;

		  CRect  regNameA = reg;
		  regNameA.left = regA.right;
		  regNameA.right = reg.left + reg.GetWidth() / 2;

		  CRect  regX = reg;
		  regX.left = regNameA.right;
		  regX.right = regX.left + WIDTH_AX;

		  CRect  regNameX = reg;
		  regNameX.left = regX.right;
		  
		  PrintEntry(nmX, regNameX, FLAG_PRINT_FNAME);
		}
		else
		{
	    CRect reg = regGroup;
	    reg.top = regGroup.bottom + (wantsDoublesFirst ? 0 : sySingles) * heightBot + (nmX.nmNr-1) * 1.7 * heightBot;
	    reg.bottom = reg.top + 1.7 * heightBot;

		  CRect regA = reg;
		  regA.right = regA.left + WIDTH_AX;

		  CRect  regNameA = reg;
		  regNameA.left = regA.right;
		  regNameA.right = reg.left + reg.GetWidth() / 2;

		  CRect  regX = reg;
		  regX.left = regNameA.right;
		  regX.right = regX.left + WIDTH_AX;

		  CRect  regNameX = reg;
		  regNameX.left = regX.right;
		  
		  PrintEntry(nmX, regNameX, FLAG_PRINT_FNAME);
		}
	}
	
	nmX.Close();
	
	regGroup.bottom += sySingles * heightBot + 
	                   1.7 * syDoubles * heightBot;

	// Jetzt noch der Dicke Rahmen aussenrum und die senkrechten Linien
	printer->Rectangle(regGroup, THICK_FRAME, FALSE);
	
	printer->Line(regGroup.left + WIDTH_AX, regGroup.top + heightTop,
								regGroup.left + WIDTH_AX, regGroup.bottom);
								
	if (CTT32App::instance()->GetPrintScoreCards())
	{
		printer->Line(hcenter - 3 * WIDTH_AX, regGroup.top, 
									hcenter - 3 * WIDTH_AX, regGroup.bottom);							
		printer->Line(hcenter - 2 * WIDTH_AX, regGroup.top, 
									hcenter - 2 * WIDTH_AX, regGroup.bottom);							
		printer->Line(hcenter - 1 * WIDTH_AX, regGroup.top, 
									hcenter - 1 * WIDTH_AX, regGroup.bottom);					
	}
                
	printer->Line(hcenter, regGroup.top,
								hcenter, regGroup.bottom, THICK_FRAME);
								
	printer->Line(hcenter + WIDTH_AX, regGroup.top + heightTop,
								hcenter + WIDTH_AX, regGroup.bottom);
								
	if (CTT32App::instance()->GetPrintScoreCards())
	{
		printer->Line(regGroup.right - 3 * WIDTH_AX, regGroup.top, 
									regGroup.right - 3 * WIDTH_AX, regGroup.bottom);							
		printer->Line(regGroup.right - 2 * WIDTH_AX, regGroup.top, 
									regGroup.right - 2 * WIDTH_AX, regGroup.bottom);							
		printer->Line(regGroup.right - 1 * WIDTH_AX, regGroup.top, 
									regGroup.right - 1 * WIDTH_AX, regGroup.bottom);							
	}
	
	offsetY = regGroup.bottom;

	// Undo the correction, in XTS(A) and YSTA we need all of them
	if (wxStrcmp(sy.syName, wxT("XTS")) == 0)
		sySingles = sy.sySingles;
	else if (wxStrcmp(sy.syName, wxT("XTSA")) == 0)
		sySingles = sy.sySingles;
	else if (wxStrcmp(sy.syName, wxT("YSTA")) == 0)
		sySingles = sy.sySingles;

	// --- Naechste Gruppe: Liste der Spiele ---
	regGroup.top = offsetY + 0.5 * heightTop;
	regGroup.bottom = regGroup.top + heightTop;

	// int  gameWidth = printer->TextWidth("5. Game");
	int  gameWidth = (printer->width / 2) / (mt.mt.mtBestOf + 2);

	// Players
	CRect  regPlayers = regGroup;
	regPlayers.right = regGroup.right - (mt.mt.mtBestOf + 2) * gameWidth;

	CRect  regPlayerA = regPlayers;
	CRect  regPlayerX = regPlayers;
	regPlayerA.right = regPlayers.left + regPlayers.GetWidth() / 2;
	regPlayerX.left = regPlayerA.right;

	PrintStringCentered(_("Players"), regPlayers);

	// 5 Saetze, Ergebnis und 'Score'
	CRect *regSet = (CRect *) _alloca(sizeof(CRect) * (mt.mt.mtBestOf + 2));
	CRect regSets;
	CRect regMatch;

	// Erst die Saetze
	for (int i = 0; i < mt.mt.mtBestOf; i++)
	{
		regSet[i].left = (i ? regSet[i-1].right :
								regPlayerX.right + i * gameWidth);
		regSet[i].right = regSet[i].left + gameWidth;
		regSet[i].bottom = regPlayerX.top;
		regSet[i].top = regPlayerX.bottom;
	}

	// "Games" drucken
	regSets = regSet[0];
	regSets.right = regSet[mt.mt.mtBestOf-1].right;

	PrintStringCentered(_("Games"), regSets);

	// Jetzt die endgueltige Position
	regSets.left   = regGroup.right - 2 * gameWidth;
	regSets.right  = regSets.left + gameWidth;
	regSets.top    = regGroup.top;
	regSets.bottom = regGroup.bottom;

	regMatch.left = regSets.right;
	regMatch.right = regGroup.right;
	regMatch.top = regSets.top;
	regMatch.bottom = regSets.bottom;

	PrintStringCentered(_("Games"), regSets);
	PrintStringCentered(_("Score"), regMatch);

	// Horizontale Linie
	printer->Line(regGroup.left, regGroup.bottom,
								regGroup.right, regGroup.bottom);

	// Platz fuer regNrA/X schaffen
	CRect  regNrA = regPlayerA;
	CRect  regNrX = regPlayerX;

	regPlayerA.left += WIDTH_AX;
	regPlayerX.left += WIDTH_AX;

	regNrA.right = regPlayerA.left;
	regNrX.right = regPlayerX.left;

	// Jetzt wieder durch die Spiele gehen...
	short  matchFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_SMALL);
  short  oldMatchFont = printer->SelectFont(matchFont);
  int  heightMatches = (sy.syMatches > 8 ? 2 * printer->cH : heightBot);
  
  std::vector<MtEntry *>  mtEntryList;
  
  MtEntryStore mtmt(connPtr);
  mtmt.SelectByMS(mt.mt);
  while (mtmt.Next())  
    mtEntryList.push_back(new MtEntry(mtmt));
    
  mtmt.Close();
  
  short  mtTotalGames[2] = {0};
  
  short  mtTotalPoints[2] = {0};
  
  for (auto it = mtEntryList.begin(); it != mtEntryList.end(); it++)
	{
	  // Geloescht wird am Ende, also Vorsicht bei continue
	  MtEntry &mtmt = *(*it);	  
	  
		// Die Ausgaben vorbereiten...
		wxString  strA;
		wxString  strX;

		if (mtmt.tmA.team.cpType == CP_SINGLE)
		{
			regGroup.bottom += heightMatches;

			if (mtmt.nmAnmNr > sySingles) 
			{
				if (wxStrcmp(sy.syName, "OTS") == 0) 
			    strA = "A/B"; // wxSprintf(strA, "A/B\nn.i.D.");
				else if (wxStrcmp(sy.syName, "ETS") == 0)
				  strA = wxString::Format("A%d/A4", mtmt.nmAnmNr == 5 ? 1 : 2);
				else
				  strA = "";
			}
			else if (wxStrcmp(sy.syName, "XTS") == 0)
			{
				strA = xts[0][mtmt.nmAnmNr - 1];
			}
			else if (wxStrcmp(sy.syName, "XTSA") == 0)
			{
				strA = xtsa[0][mtmt.nmAnmNr - 1];
			}
			else if (wxStrcmp(sy.syName, "YSTA") == 0)
			{
				strA = ysta[0][mtmt.nmAnmNr - 1];
			}
			else if (sySingles < 4)
			{
        if (mtmt.nmAnmNr > 0)
				  strA = alpha[0][mtmt.nmAnmNr-1];
        else
          strA = "";
			}
			else
			{
				strA = wxString::Format("A%i", mtmt.nmAnmNr);
			}

			if (mtmt.nmXnmNr > sySingles)
			{
				if (wxStrcmp(sy.syName, "OTS") == 0) 
          strX = "X/Y"; // wxSprintf(strX, "X/Y\nn.i.D.");
				else if (wxStrcmp(sy.syName, "ETS") == 0)
				  strX = wxString::Format("X%d/X4", mtmt.nmXnmNr == 5 ? 1 : 2);
				else
				  strX = "";
			}
			else if (wxStrcmp(sy.syName, "XTS") == 0)
			{
				strX = xts[1][mtmt.nmXnmNr - 1];
			}
			else if (wxStrcmp(sy.syName, "XTSA") == 0)
			{
				strX = xtsa[1][mtmt.nmXnmNr - 1];
			}
			else if (wxStrcmp(sy.syName, "YSTA") == 0)
			{
				strX = ysta[1][mtmt.nmXnmNr - 1];
			}
			else if (sySingles < 4)
			{
        if (mtmt.nmXnmNr > 0)
				  strX = alpha[1][mtmt.nmXnmNr - 1];
        else
          strX = "";
			}
			else
			{
				strX = wxString::Format("X%i", mtmt.nmXnmNr);
			}
		}
		else
		{
			regGroup.bottom += 1.7 * heightMatches;

			if (wxStrcmp(sy.syName, "OTS") == 0)
			{
			  strA = "C\nA/B";
			  strX = "Z\nX/Y";
			}
			else if (wxStrcmp(sy.syName, "XTS") == 0)
			{
				strA = "A-B1\nA-G1";
				strX = "X-B1\nX-G1";
			}
			else if (wxStrcmp(sy.syName, "XTSA") == 0)
			{
				strA = "A-B\nA-G";
				strX = "X-B\nX-G";
			}
      else if (syDoubles > 1)
      {
			  strA = wxString::Format("DA%i", mtmt.nmAnmNr);
			  strX = wxString::Format("DX%i", mtmt.nmXnmNr);
			}
			else
			{
			  strA = "DA";
			  strX = "DX";
			}
		}

		// Region fuer den Eintrag
		CRect  reg = regGroup;
		reg.top = reg.bottom - (mtmt.tmA.team.cpType == CP_SINGLE ?
							              heightMatches : 1.7 * heightMatches);

		// ... die Regionen verschieben ...
		regNrA.top = reg.top;
		regNrA.bottom = reg.bottom;
		regPlayerA.top = reg.top;
		regPlayerA.bottom = reg.bottom;
		regNrX.top = reg.top;
		regNrX.bottom = reg.bottom;
		regPlayerX.top = reg.top;
		regPlayerX.bottom = reg.bottom;

		for (int i = 0; i < mt.mt.mtBestOf + 1; i++)
		{
			regSet[i].top = reg.top;
			regSet[i].bottom = reg.bottom;
		}

		regSets.top = reg.top;
		regSets.bottom = reg.bottom;
		regMatch.top = reg.top;
		regMatch.bottom = reg.bottom;

		// ... und drucken
		PrintStringCentered(strA, regNrA);
		PrintEntry(mt.mt.mtReverse ? mtmt.tmX : mtmt.tmA, regPlayerA, 0);
		PrintStringCentered(strX, regNrX);
		PrintEntry(mt.mt.mtReverse ? mtmt.tmA : mtmt.tmX, regPlayerX, 0);
		
		// Ergebniss ausgeben, soweit bekannt
		if (mtmt.mt.mtResA || mtmt.mt.mtResX)
		{
		  MtSetStore mtSet(mtmt.mt, connPtr);
		  mtSet.SelectAll(mtmt.mt.mtEvent.mtMS);
		  
		  short res[2] = {0};
		  
		  while (mtSet.Next())
		  {
		    if (mtSet.mtSet > 0)
		    {
          short win = (mtSet.mtSet == mt.mt.mtBestOf ? cp.cpPtsToWinLast : cp.cpPtsToWin);

		      // Der Einfachheit mitzaehlen und nicht mtMatch abfragen
		      if ( (mtSet.mtResA > mtSet.mtResX) && (mtSet.mtResA >= win) )
		        res[0]++;
		      else if ( (mtSet.mtResX > mtSet.mtResA) && (mtSet.mtResX >= win) )
		        res[1]++;
		      
          PrintGame(mtSet, regSet[mtSet.mtSet-1], mt.mt.mtReverse ? true : false);
		    }
		  }		  
		  
		  mtSet.Close();
		  
		  PrintGame(res, regSets, mt.mt.mtReverse ? true : false);
		  
		  if (mtmt.QryWinnerAX() == +1)
		  {
		    mtTotalPoints[0]++;
		    PrintStringCentered(mt.mt.mtReverse ? wxT("0 : 1") : wxT("1 : 0"), regMatch);
		  }
		  else if (mtmt.QryWinnerAX() == -1)
		  {
		    mtTotalPoints[1]++;
		    PrintStringCentered(mt.mt.mtReverse ? wxT("1 : 0") : wxT("0 : 1"), regMatch);
		  }
		  
		  mtTotalGames[0] += res[0];
		  mtTotalGames[1] += res[1];
		}

		// Waagrechte Linie
		printer->Line(reg.left, reg.bottom, reg.right, reg.bottom);
		
		delete (*it);
	}
	
	mtEntryList.clear();

	// Dicker Frame
	printer->Rectangle(regGroup, THICK_FRAME, FALSE);

	// Senkrechte Linien
	printer->Line(regNrA.right, regGroup.top + heightTop,
	 							regNrA.right, regGroup.bottom);

	printer->Line(regPlayerA.right, regGroup.top + heightTop,
								regPlayerA.right, regGroup.bottom);

	printer->Line(regNrX.right, regGroup.top + heightTop,
								regNrX.right, regGroup.bottom);

	printer->Line(regPlayerX.right, regGroup.top,
								regPlayerX.right, regGroup.bottom, THICK_FRAME);

	for (int i = 0; i < mt.mt.mtBestOf - 1; i++)
	{
		printer->Line(regSet[i].right, regGroup.top+heightTop,
							regSet[i].right, regGroup.bottom);
	}

	// Rechte Abgrenzung des letzten Satzes ist auch erste von regSets
	printer->Line(regSets.left, regGroup.top,
                regSets.left, regGroup.bottom, THICK_FRAME);

	printer->Line(regSets.right, regGroup.top,
	 							regSets.right, regGroup.bottom, THICK_FRAME);

	printer->SelectFont(oldMatchFont);
	printer->DeleteFont(matchFont);

	// Sieger druntersetzen
	CRect  regWinner = regGroup;
	regWinner.top = regGroup.bottom;
	regWinner.bottom = regWinner.top + heightBot;

  wxString strWinner = _("Winner");
  strWinner = " " + strWinner + ":";
	PrintString(strWinner, regWinner);

	CRect  regWinnerTM = regWinner;
	regWinnerTM.left += printer->TextWidth(strWinner);
	regWinnerTM.right = regPlayerX.right;

	// Unter Team A kommt der Sieger, falls bekannt
	switch (mt.QryWinnerAX())
	{
		case +1 :
		  PrintEntry(mt.tmA, regWinnerTM, 0);
			break;
		case -1 :
		  PrintEntry(mt.tmX, regWinnerTM, 0);
			// PrintString(tmX.tmDesc, regPlayerA);
			break;
		case 0 :
		  if (mt.IsFinished())
		  {
		    PrintStringCentered(_("Tie"), regWinnerTM);
		  }
	}

	// Dicken Rahmen ausenrum
	printer->Rectangle(regWinner, THICK_FRAME);

	// Ergebnis, wenn Spiel fertig ist
	regSets.top = regMatch.top = regWinner.top;
	regSets.bottom = regMatch.bottom = regWinner.bottom;

	if (mt.IsFinished())
	{
		PrintGame(mtTotalGames, regSets, mt.mt.mtReverse ? true : false);
		PrintGame(mtTotalPoints, regMatch, mt.mt.mtReverse ? true : false);
	}

	// Abgrenzende Linien
	printer->Line(regMatch.left, regMatch.top,
								regMatch.left, regMatch.bottom, THICK_FRAME);
	printer->Line(regSets.left, regSets.top,
								regSets.left, regSets.bottom, THICK_FRAME);

	offsetY = regWinner.bottom;

	// --- 4.te Gruppe: Unterschriften ---
	regGroup.top = offsetY + 0.5 * heightTop;
	regGroup.bottom = regGroup.top + heightTop + 2 * heightBot;

	CRect  regSignature = regGroup;
	regSignature.bottom = regSignature.top + heightTop;

	printer->Line(regSignature.left, regSignature.bottom,
								regSignature.right, regSignature.bottom);

	PrintStringCentered(_("Signature of Team Captains"), regSignature);

	CRect regSigA = regGroup;
	CRect regSigX = regGroup;

	regSigA.top = regSignature.bottom;
	regSigA.right = regSigA.left + WIDTH_AX;
	PrintStringCentered(wxT("A"), regSigA);

	regSigX.top = regSignature.bottom;
	regSigX.left = regGroup.left + regGroup.GetWidth() / 2;
	regSigX.right = regSigX.left + WIDTH_AX;
	PrintStringCentered(wxT("X"), regSigX);

	// Und Linien in der Mitte
	printer->Line(regSigA.right, regSigA.top, regSigA.right, regSigA.bottom);
	printer->Line(regSigX.left, regSigX.top, regSigX.left, regSigX.bottom);
	printer->Line(regSigX.right, regSigX.top, regSigX.right, regSigX.bottom);

	// Und ein Rahmen aussenrum
	printer->Rectangle(regGroup, THICK_FRAME, FALSE);

	offsetY = regGroup.bottom;

  if (CTT32App::instance()->GetPrintScoreUmpires())
  {
	  // --- 5.te Gruppe: Schiedsrichter ---
	  // Schiedsrichter
	  regGroup.top = offsetY + 0.5 * heightTop;
	  // Eindhoven: Doppelte Hoehe
	  // regGroup.bottom = regGroup.top + heightTop + heightBot;
	  regGroup.bottom = regGroup.top + heightTop + 2 * heightBot;
	  regGroup.right = printer->width;

	  // Rahmen drum und Strich drunter
	  printer->Rectangle(regGroup, THICK_FRAME, FALSE);
	  printer->Line(regGroup.left, regGroup.top + heightTop,
					   regGroup.right, regGroup.top + heightTop, THICK_FRAME);

    if (CTT32App::instance()->GetPrintScoreUmpireName() && mt.mt.mtUmpire)
    {
      CRect regSRNr(regGroup.left + printer->cW, regGroup.top + heightTop, 
             regGroup.left + regGroup.GetWidth() / 2 - printer->cW, regGroup.bottom);

      // Doha: SR-Nr drucken
      wxChar tmpSRNr[20] = {0}, tmpSR2Nr[20] = {0};
      _itot(mt.mt.mtUmpire, tmpSRNr, 10);
      _itot(mt.mt.mtUmpire2, tmpSR2Nr, 10);

      UpListStore sr(connPtr), sr2(connPtr);

      CRect regSR = regGroup;
      regSR.top += heightTop;
      regSR.right = regSR.left + regSR.GetWidth() / 2;
      regSR.left += printer->cW;
  
      sr.SelectByNr(mt.mt.mtUmpire);
      sr.Next();
      sr.Close();

      if (mt.mt.mtUmpire2)
      {
        sr2.SelectByNr(mt.mt.mtUmpire2);
        sr2.Next();
        sr2.Close();
      }

      if (mt.mt.mtUmpire2 == 0)
      {
        // Nur ein SR: Wenn Name bekannt, dann drucken, sonst die Nummer
        if (sr.WasOK())
          PrintUmpire(sr, regSR, FLAG_PRINT_NOPLNR | FLAG_PRINT_NATION | FLAG_PRINT_FNAME);
        else
          PrintString(tmpSRNr, regSR);
      }
      else 
      {
        CRect regSR1Nr = regSR, regSR2Nr = regSR;
        regSR1Nr.bottom = regSR1Nr.top + regSR1Nr.GetHeight() / 2;
        regSR2Nr.top = regSR1Nr.bottom;

        if (sr.WasOK())
          PrintUmpire(sr, regSR1Nr, FLAG_PRINT_NOPLNR | FLAG_PRINT_NATION | FLAG_PRINT_FNAME);
        else
          PrintString(tmpSRNr, regSR1Nr);

        if (sr2.WasOK())
          PrintUmpire(sr2, regSR2Nr, FLAG_PRINT_NOPLNR | FLAG_PRINT_NATION | FLAG_PRINT_FNAME);
        else
          PrintString(tmpSR2Nr, regSR2Nr);
      }
    }
  
  
	  // Eindhoven: Abteilen
    printer->Line(regGroup.left, regGroup.top + heightTop + heightBot,
					   regGroup.right, regGroup.top + heightTop + heightBot);

	  // Links 'Umpire'
	  CRect  regUmpire = regGroup;
	  regUmpire.right = regUmpire.left + regUmpire.GetWidth() / 2;

	  // Von Unterschrift abtrennen
	  printer->Line(regUmpire.right, regGroup.top,
					   regUmpire.right, regGroup.bottom, THINN_FRAME);

	  regUmpire.bottom = regUmpire.top + heightTop;
	  PrintStringCentered(_("Umpire(s)"), regUmpire);

	  // rechts "Signature of Umpire"
	  regUmpire.left = regUmpire.right;
	  regUmpire.right = regGroup.right;
	  PrintStringCentered(_("Signature of Umpire(s)"), regUmpire);

	  offsetY = regGroup.bottom;
  }

	printer->SelectFont(oldFont);
	printer->DeleteFont(textFont);

	return 0;
}


int RasterScore::PrintScoreExtras(const MtEntry &mt)
{
  // Extras
  short textFont =  printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_MEDIUM);                    
  short oldFont = printer->SelectFont(textFont);                    

  int top = 1.0 * printer->cH;
  int bot = 2.0 * printer->cH;

  if (cp.cpType == CP_DOUBLE || cp.cpType == CP_MIXED)
    bot *= 2;

  CRect regExtras(offsetX, offsetY, printer->width, offsetY);
  regExtras.bottom += 2 * top + 2 * bot;

  // Dicken Rahmen
  printer->Rectangle(regExtras, THICK_FRAME);

  CRect regPl = regExtras;
  regPl.right = regPl.left + regPl.GetWidth() / 2;

  CRect regColor = regExtras;
  regColor.left = regPl.right;
  regColor.right = regColor.left + (2 * regPl.GetWidth() / 7);

  CRect regBack = regExtras;
  regBack.left = regColor.right;
  regBack.right = regBack.left + regColor.GetWidth() / 2;

  CRect regRacket = regExtras;
  regRacket.left = regBack.right;
  regRacket.right = regRacket.left + regColor.GetWidth();

  CRect regBall = regExtras;
  regBall.left = regRacket.right;

  // Linien
  printer->Line(regPl.right, regPl.top, regPl.right, regPl.bottom);
  printer->Line(regColor.right, regColor.top, regColor.right, regColor.bottom);
  printer->Line(regBack.right, regBack.top, regBack.right, regBack.bottom);
  printer->Line(regRacket.right, regRacket.top, regRacket.right, regRacket.bottom);

  printer->Line(regRacket.left + regRacket.GetWidth() / 2, regRacket.top + top, regRacket.left + regRacket.GetWidth() / 2, regRacket.bottom);
  printer->Line(regBall.left + regBall.GetWidth() / 2, regBall.top + top, regBall.left + regBall.GetWidth() / 2, regBall.bottom);

  regPl.bottom = regPl.top + top;
  regColor.bottom = regColor.top + top;
  regBack.bottom = regColor.top + top;
  regRacket.bottom = regColor.top + top;
  regBall.bottom = regBall.top + top;

  PrintStringCentered(_("Color of Shirt"), regColor);
  PrintStringCentered(_("Back N."), regBack);
  PrintStringCentered(_("Racket Control"), regRacket);
  PrintStringCentered(_("Ball Selection"), regBall);

  regPl.top = regPl.bottom;
  regPl.bottom = regPl.top + top;
  regColor.top = regColor.bottom;
  regColor.bottom = regColor.top + top;
  regBack.top = regBack.bottom;
  regBack.bottom = regBack.top + top;
  regRacket.top = regRacket.bottom;
  regRacket.bottom = regRacket.top + top;
  regBall.top = regBall.bottom;
  regBall.bottom = regBall.top + top;

  CRect regRacketBefore = regRacket;
  regRacketBefore.right = regRacketBefore.left + regRacketBefore.GetWidth() / 2;
  CRect regRacketAfter = regRacket;
  regRacketAfter.left = regRacketBefore.right;

  PrintStringCentered(_("Before"), regRacketBefore);
  PrintStringCentered(_("After"), regRacketAfter);

  CRect regBallYes = regBall;
  regBallYes.right = regBall.left + regBall.GetWidth() / 2;
  CRect regBallNo = regBall;
  regBallNo.left = regBallYes.right;

  PrintStringCentered(_("Yes"), regBallYes);
  PrintStringCentered(_("No"), regBallNo);

  printer->Line(regExtras.left, regExtras.top + 2 * top, regExtras.right, regExtras.top + 2 * top);

  regPl.top = regPl.bottom;
  regPl.bottom = regPl.top + bot;

  printer->Line(regExtras.left, regPl.bottom, regExtras.right, regPl.bottom);

  regPl.left += printer->cW;
  PrintEntry(mt.mt.mtReverse ? mt.tmX : mt.tmA, regPl, 0);

  regPl.top = regPl.bottom;
  regPl.bottom = regExtras.bottom;

  PrintEntry(mt.mt.mtReverse ? mt.tmA : mt.tmX, regPl, 0);

  offsetY = regExtras.bottom;

  printer->SelectFont(oldFont);
  printer->DeleteFont(textFont);

  return 0;
}


int RasterScore::PrintScoreStartEnd(const MtEntry&)
{
  int top = offsetY;
  int left = offsetX;
  int heightTop = 2 * printer->cH;
  // Bottom on top of the boxes
  int bot = printer->height - 7 * printer->cH - 0.5 * printer->cH;

  if (CTT32App::instance()->GetPrintSponsor())
    bot -= 1470 / 3;

  if (bot > top + 5 * printer->cH)
  {
    // Thick line around box
    printer->Rectangle(offsetX, top, printer->width, top + heightTop, THICK_FRAME);

    // Line below caption
    printer->Line(offsetX, top + heightTop, printer->width, top + heightTop, THICK_FRAME);

    // "Start At", "End At"
    printer->Text(CRect(offsetX + printer->cH, top, (printer->width - offsetX) / 2, top + heightTop), _("Start at:"));
    printer->Text(CRect((printer->width - offsetX) / 2 + printer->cH, top, printer->width - offsetX, top + heightTop), _("End at:"));

    offsetY = top + heightTop;
  }

  return 0;
}


int  RasterScore::PrintScoreRemarks(const MtEntry &)
{
  int top = offsetY;
  int left = offsetX;
  int heightTop = 2 * printer->cH;
  // Bottom on top of the boxes
  int bot = printer->height - 5 * printer->cH - printer->cH;

  if (CTT32App::instance()->GetPrintSponsor())
    bot -= 1470 / 3;

  if (bot > top + 3 * printer->cH)
  {
    // Thick line around box
    printer->Rectangle(offsetX, top, printer->width, bot, THICK_FRAME);

    // "Remarks"
    printer->Text(offsetX + printer->cH, top + 1.5 * printer->cH, _("Remarks:"));

    offsetY = bot;
  }

  return 0;
}


int  RasterScore::PrintScoreFooter(const MtEntry &)
{
	int top = offsetY;
	int left = offsetX;
	int heightTop = 2 * printer->cH;  // Hoehe Ueberschrift
	int heightBot = 3 * printer->cH;  // Hoehe Eintraege

	if ( (printer->height - offsetY) < (heightTop + heightBot) )
		return 0;

	offsetY = printer->height - heightTop - heightBot;

	int  genWidth = (printer->width - offsetX) / 6;

#if 1 // EYC2006
  // Ab hier neu
  wxImage sponsor;
  
  if (CTT32App::instance()->GetPrintSponsor() && IdStore::GetSponsorImage(sponsor))
  {
    // footer logo (1196x147 px)
    int xFooterLogo = 11960 / 3;
    int yFooterLogo = 1470 / 3;
    
    // Platz fuer Kaestchen und Footer
    if (printer->height - offsetY > 1.5 * heightTop + heightBot + yFooterLogo)
    {
      wxPoint topLeft(-1, -1), bottomRight(printer->width, printer->height);
        
      printer->DrawImage(topLeft, bottomRight, wxSize(xFooterLogo, yFooterLogo), sponsor);

      offsetY -= yFooterLogo;
    }
    else
    {
    	genWidth = (printer->width - offsetX) / 9;
    	
      wxPoint topLeft(-1, -1), bottomRight(printer->width, offsetY + heightTop + heightBot);
      printer->DrawImage(topLeft, bottomRight, wxSize(genWidth * 6, heightTop + heightBot), sponsor);
    }
  }

// Und dann wie gehabt weiter
#endif // EYC2006

	CRect  regGeneral;
	regGeneral.top = offsetY;
	regGeneral.bottom = regGeneral.top + heightTop + heightBot;
	regGeneral.left = offsetX;
	regGeneral.right = regGeneral.left + 3 * genWidth;

	printer->Rectangle(regGeneral, THICK_FRAME, FALSE);

	for (int k = 1; k < 3; k++)
	 printer->Line(regGeneral.left + k * genWidth, regGeneral.top,
						regGeneral.left + k * genWidth, regGeneral.bottom);

	offsetY = regGeneral.bottom;

	// Oberen Teil abgrenzen und Eintraege rein
	regGeneral.bottom = regGeneral.top + heightTop;
	printer->Line(regGeneral.left, regGeneral.bottom,
					 regGeneral.right, regGeneral.bottom);

	for (int k = 1; k <= 3; k++)
	{
	 wxChar str[16] = {0};
	 _itot(k, str, 10);

	 CRect  reg;
	 reg = regGeneral;
	 reg.left += (k-1) * genWidth;
	 reg.right = reg.left + genWidth;

	 PrintStringCentered(str, reg);
	}

	return 0;
}


int  RasterScore::Print(const CpRec &cp_, const GrRec &gr_, const MtEntry &mt)
{
  cp = cp_;
  gr = gr_;
  
	textFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_NORMAL);

	short  oldFont = printer->SelectFont(textFont);

	space = 0.5 * printer->cW;

  wxString title = CTT32App::instance()->GetReportTitle();
  wxString subTitle = CTT32App::instance()->GetReportSubtitle();

  if (!title.IsEmpty())
  {
	  // Hot Fix fuer Eindhoven ...
	  short bigFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_COMP);
	  short mediumFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_GROUP);

	  printer->SelectFont(bigFont);

	  CRect  reg;
	  reg.left = offsetX;
	  reg.right = printer->width;
	  reg.top = offsetY + 1 * printer->cH;
	  reg.bottom = reg.top + printer->TextHeight(wxT("QP"));

	  PrintStringCentered(title, reg);

	  printer->SelectFont(mediumFont);
	  reg.top = reg.bottom;
	  reg.bottom = reg.top + printer->TextHeight(wxT("QP"));

	  PrintStringCentered(subTitle, reg);

	  printer->SelectFont(textFont);
	  printer->DeleteFont(bigFont);
	  printer->DeleteFont(mediumFont);

    offsetY = reg.bottom;
    offsetY += printer->cH;
  }

	offsetY += printer->cH;

  PrintScoreHeader(cp, gr, mt);

  offsetY += printer->cH;

  if (cp.cpType == CP_TEAM && mt.mt.mtEvent.mtMS == 0)
    PrintScoreTM(mt);
  else
    PrintScore(mt);

  if (CTT32App::instance()->GetPrintScoreExtras())
  {
    offsetY += printer->cH;
    PrintScoreExtras(mt);
  }

  if (CTT32App::instance()->GetPrintScoreStartEnd())
  {
    offsetY += printer->cH;
    PrintScoreStartEnd(mt);
  }

  if (CTT32App::instance()->GetPrintScoreRemarks())
  {
		offsetY += printer->cH;
    PrintScoreRemarks(mt);
  }

	offsetY += printer->cH;
	PrintScoreFooter(mt);

  printer->SelectFont(oldFont);
  printer->DeleteFont(textFont);

  return 0;
}



