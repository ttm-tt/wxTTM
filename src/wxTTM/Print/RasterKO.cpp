/* Copyright (C) 2020 Christoph Theis */

// Aufspaltung von GRRASTER
// Raster fuer KO-Systeme (implemetiert nur fuer Einfach KO)
// 02.09.95 (ChT)
// To Do: RasterKO::PrintMatch: Evt. mal Spielnummer etc auch fuer (2) Freilose drucken
//        Ist im Prinzip eingebaut, aber noch auskommentiert.

#include  "stdafx.h"

#include  "RasterKO.h"   // Modul Header

#include  "TT32App.h"
#include  "CpListStore.h"
#include  "GrListStore.h"
#include  "MtListStore.h"
#include  "PlListStore.h"
#include  "StEntryStore.h"

#include  "IdStore.h"
#include  "SyStore.h"

#include  "res.h"

#include  <stdlib.h>
#include  <sstream>
#include  <iomanip>

#undef _
#define _(a) printer->GetString(a)


// Switch fuer Kurzform der Ergebnisse (12 -12 12)
#define SHORT_RESULTS

#define NEWSTYLE

// Definition von Strings
// #define  STR_FINAL      infoSystem->QryString("RES_STR", STR_FINAL)
// #define  STR_SEMIFINAL  infoSystem->QryString("RES_STR", STR_SEMIFINAL)
// #define  STR_ROUND      infoSystem->QryString("RES_STR", STR_ROUND)
// #define  STR_PLACE			 infoSystem->QryString("RES_STR", STR_PLACE),

#define  STR_FINAL         IDS_FINAL
#define  STR_SEMIFINAL     IDS_SEMIFINAL
#define  STR_ROUND         IDS_ROUND
#define  STR_POS           IDS_POS
#define  STR_QUALIFICATION IDS_LBL_QUALIFICATION

// +---------------------------------------+
// +             RasterKO                 +
// +---------------------------------------+

struct  SettingKO
{
  int  rd;       // Ab Runde
  int  mt;       // und vonMatch; ab hier KO, dh. nur Gewinner
  int  nrrd;     // Zahl der Runden;
  int  nrmt;     // Zahl der Spiele in der erten Runde
  int  maxRound; // Max Anzahl an Runden je Seite
  int  maxMatch; // Max Anzahl an Spielen in erster Runde je Seite
  int  sc;       // Trostrunde; damit koennen Spiele gefunden werden
  int  lastRd;   // Letzte Runde zu drucken

  int  startX;   // Startpunkt in x-Richtung des Rasters
  int  startY;   // Startpunkt in y-Richtung des Rasters
  int  endX;     // Rechter Rand des Rasters (Output)
  int  width;    // Breite des Rasters;
  int  height;   // Hoehe des Rasters; beginnt immer mit 1*height

  int  koMarginWidth;  // Breite des ersten Raster, wenn printKOMargin == true
  int  printKOMargin;  // Drucke am linken Rand die Spieler mit Nation,
                       // ohne Abstand zwischen den Spielen
  int  printNr;        // Auch Matchnr
  int  ignoreByes;     // Byes werden wie Spieler behandelt

  int  inbox;          // Spieler innerhalb vom Raster drucken	

  int  printPosNrs;    // Position am Rand drucken
};


// Druckt Spielergebnis linksbuendig in Region reg
int  RasterKO::PrintResult(MtRec &mt, const CRect &reg, const std::vector<MtSet> &mtSetList)
{
  short  winner = mt.QryWinnerAX();

  // Ergebnisse werden nur nach beendeten Spiel gedruckt
  if (!winner)
    return 0;

  // Hat jemand aufgegeben? In Setzung oder Meldung?
  if (mt.mtWalkOverA || mt.mtWalkOverX ||
    mt.mtInjuredA || mt.mtInjuredX ||
    mt.mtDisqualifiedA || mt.mtDisqualifiedX)
  {
    wxString what;
    if (mt.mtWalkOverA || mt.mtWalkOverX)
      what = _("w/o");
    else if (mt.mtInjuredA || mt.mtInjuredX)
      what = _("Injured");
    else if (mt.mtDisqualifiedA || mt.mtDisqualifiedX)
      what = _("Disqualified");

    int offset = 0;

    wxString tmp = wxString::Format("%d:%d",
      winner == +1 ? mt.mtResA : mt.mtResX,
      winner == +1 ? mt.mtResX : mt.mtResA);

    if (cp.cpType == CP_TEAM)
    {
      PrintString(tmp.wx_str(), reg);

      offset = printer->TextWidth(tmp) + printer->cW;
    }
    else
    {
      short boldFont = printer->DeriveFont(printer->GetFont(), 700, 0);
      short oldFont = printer->SelectFont(boldFont);
      PrintString(tmp, reg);

      offset = printer->TextWidth(tmp) + printer->cW;

      printer->SelectFont(oldFont);
      printer->DeleteFont(boldFont);
    }

    CRect rect = reg;
    rect.left += offset;

    PrintString(wxString::Format("(%s)", what.wx_str()).wx_str(), rect);

    return 0;
  }

  // Formatieren der Ausgabe
  std::tostringstream  str;

  if (cp.cpType == CP_TEAM)
  {
    int tmp[2];

    if (winner == +1)
    {
      tmp[0] = mt.mtResA;
      tmp[1] = mt.mtResX;
    }
    else if (winner == -1)
    {
      tmp[0] = mt.mtResX;
      tmp[1] = mt.mtResA;
    }
    else
      return 0;  // Wurd schon oben abgefangen

    PrintGame(tmp, reg);

    return 0;
  }

  int offset = 0;

  if (true)
  {
    wxChar tmp[16];
    if (winner == +1)
      wxSprintf(tmp, "%d:%d", mt.mtResA, mt.mtResX);
    else if (winner == -1)
      wxSprintf(tmp, "%d:%d", mt.mtResX, mt.mtResA);

    short boldFont = printer->DeriveFont(printer->GetFont(), 700, 0);
    short oldFont = printer->SelectFont(boldFont);
    PrintString(tmp, reg);

    offset = printer->TextWidth(tmp) + printer->cW;

    printer->SelectFont(oldFont);
    printer->DeleteFont(boldFont);
  }

  str << "(";

  for (MtSet mtSet : mtSetList)
  {
    // Summe ueberspringen
    if (mtSet.mtSet == 0)
      continue;

    if (mtSet.mtResA == 0 && mtSet.mtResX == 0)
      break;

# ifdef SHORT_RESULTS
    if (mtSet.mtSet > 1)
      str << "  ";
# endif

    if (winner == +1)     // A hat gewonnen
# ifndef SHORT_RESULTS
      str << " " << mtSet.mtResA
      << " : " << mtSet.mtResX
      << " ";
# else
    {
      if (mtSet.mtResX > mtSet.mtResA)
        str << "-" << mtSet.mtResA;
      else
        str << "" << mtSet.mtResX;
    }
# endif
    else if (winner == -1)  // X hat gewonnen
    {
# ifndef SHORT_RESULTS
      str << " " << mtSet.mtResX
        << " : " << mtSet.mtResA
        << " ";
# else
      if (mtSet.mtResA > mtSet.mtResX)
        str << "-" << mtSet.mtResX;
      else
        str << "" << mtSet.mtResA;
# endif
    }
  }

  str << ")";
  str << std::ends;

  CRect rect = reg;
  rect.left += offset;

  PrintStringWrapped(str.str().data(), rect);
  //printer->Text(reg.left, reg.top, text, textFont, &reg);

  return 0;
}


// Druckt Austragungszeit und Ort rechtsbuendig in reg
int  RasterKO::PrintTime(MtRec &mt, const CRect &reg, SettingKO &ko)
{
  // Formatstring
  wxString strNr, strDate, strTime, strTable, strMatch;

  // Zuerst die Spielnr
  if (ko.printNr && mt.mtNr)
    strNr = wxString::Format("%d ", mt.mtNr);

  tm  tmp;
  memset(&tmp, 0, sizeof(tmp));

  tmp.tm_year = mt.mtPlace.mtDateTime.year - 1900;
  tmp.tm_mon = mt.mtPlace.mtDateTime.month - 1;
  tmp.tm_mday = mt.mtPlace.mtDateTime.day;
  tmp.tm_hour = mt.mtPlace.mtDateTime.hour;
  tmp.tm_min = mt.mtPlace.mtDateTime.minute;
  tmp.tm_sec = 0; // mt.mtPlace.mtDateTime.second;
  tmp.tm_isdst = -1;

  if (mt.mtPlace.mtDateTime.day)
  {
    wxString format = CTT32App::instance()->GetDateFormat();
    strDate = wxDateTime(tmp).Format(format);
  }

  if (mt.mtPlace.mtDateTime.hour)
  {
    wxString format = CTT32App::instance()->GetTimeFormat();
    strTime = wxDateTime(tmp).Format(format);
  }

  if (mt.mtPlace.mtDateTime.second)
  {
    strMatch = " ";
    strMatch += wxString::Format(_("Mt %d"), mt.mtPlace.mtDateTime.second);
  }

  if (mt.mtPlace.mtTable)
    strTable = wxString::Format(" T%d", mt.mtPlace.mtTable);

  if (printer->TextWidth(strNr + strDate + wxString(" ") + strTime + (strTable.IsEmpty() ? wxString() : wxString(" T999")) + strMatch) > reg.GetWidth())
    strTime = wxString("\n") + strTime;
  else
    strTime = wxString(" ") + strTime;

  // Rechtsbuendig formatieren
  // reg.left = reg.right -printer->TextWidth(text, textFont);
  // Linksbuendig formtieren :)

  PrintString(strNr + strDate + strTime + strTable + strMatch, reg);

  return 0;
}

void  RasterKO::PrintCaption()
{
  // Etwas Platz lassen, aber nicht mehr als cH, sonst passen 2-reihige Ergebnisse
  // des letzten Spiels nicht mehr auf die Seite
  offsetY += 1 * printer->cH;
  RasterBase::PrintCaption();
}



// Druckt das Raster eines einzelnen Spieles, mit Spielern,
// Ergebnis / Zeit in reg (Zeit links von reg)
int  RasterKO::PrintMatch(MtRec &mt, const CRect &reg, SettingKO &ko, const std::vector<MtSet> &mtSetList)
{
  // Ein match besteht aus senkrechten Strich am linken Rand
  // und wagrechten Strich in der Mitte

  if (printer->PrinterAborted())
    return -1;

#ifdef NEWSTYLE
  if (!mt.mtEvent.mtChance || (mt.mtEvent.mtRound % 2))
    printer->Line(wxPoint(reg.left, reg.top),
      wxPoint(reg.left, reg.bottom),
      THICK_FRAME);
  else
    printer->Line(wxPoint(reg.left, reg.top),
      wxPoint(reg.left, reg.top + reg.GetHeight() / 2),
      THICK_FRAME);
#else
  if (!mt.mtEvent.mtChance || (mt.mtEvent.mtRound % 2))
    printer->Line(wxPoint(reg.left, reg.top),
      wxPoint(reg.left, reg.bottom));
  else
    printer->Line(wxPoint(reg.left, reg.top),
      wxPoint(reg.left, reg.bottom));
#endif 

  printer->Line(wxPoint(reg.left, reg.top + reg.GetHeight() / 2),
    wxPoint(reg.right, reg.top + reg.GetHeight() / 2));

  // Gedruckt wird der Gewinner, falls er ex.
  // QryWinnerAX liefert aber auch einen Sieger, wenn es zwei Byes sind.
  int  winner = mt.QryWinnerAX();
  int  winnerIsBye = (winner ? !(mt.mtResA || mt.mtResX) : 0);

  // Vorher: DKO und MDK: Spiel-Id auf der Siegerseite drucken
  if (!winner && !ko.sc && mt.mtEvent.mtRound > 1 && (gr.grModus == MOD_DKO || gr.grModus == MOD_MDK))
  {
    CRect regMt = reg;
    regMt.right = regMt.left - printer->cW;
    regMt.left = regMt.left - ko.width;

    wxChar tmp[16];
    wxSprintf(tmp, "(%d/%d)", mt.mtEvent.mtRound, mt.mtEvent.mtMatch);
    printer->Text(regMt, tmp, wxALIGN_RIGHT);
  }

  // Der Sieger wird gedruckt, wenn es kein Bye ist, oder wenn Byes
  // nicht wie Spieler behandelt werden sollen
  if (winner && (!winnerIsBye || !ko.ignoreByes))
  {
    CRect  tmReg = reg;

    if (ko.inbox)
    {
      if (mt.mtEvent.mtRound == ko.rd + ko.maxRound - 1 ||
        mt.mtEvent.mtRound == ko.lastRd ||
        mt.mtEvent.mtRound == gr.NofRounds())
      {
        // Sieger oder letztes Spiel einer Seite:
        // Sieger auf die Linie
        tmReg.bottom = tmReg.top + tmReg.GetHeight() / 2 - 0.1 *printer->cH;
        tmReg.top = tmReg.bottom - printer->cH;
        if (cp.cpType == CP_DOUBLE || cp.cpType == CP_MIXED)
          tmReg.top -= printer->cH;
      }
      else if (mt.mtEvent.mtMatch & 0x01)
      {
        // Ungerade Spiele: Sieger unter die Linie
        tmReg.top = tmReg.top + tmReg.GetHeight() / 2 + 0.1 * printer->cH;
        tmReg.bottom = tmReg.top + printer->cH;
        if (cp.cpType == CP_DOUBLE || cp.cpType == CP_MIXED)
          tmReg.bottom += printer->cH;
      }
      else
      {
        // Gerade Spiele: Sieger ueber die Linie
        tmReg.bottom = tmReg.top + tmReg.GetHeight() / 2 - 0.1 *printer->cH;
        tmReg.top = tmReg.bottom - printer->cH;
        if (cp.cpType == CP_DOUBLE || cp.cpType == CP_MIXED)
          tmReg.top -= printer->cH;
      }
    }
    else
    {
      tmReg.bottom = tmReg.top + tmReg.GetHeight() / 2 - 0.1 *printer->cH;
      tmReg.top = (cp.cpType == CP_DOUBLE || cp.cpType == CP_MIXED ?
        tmReg.bottom - 2 * printer->cH :
        tmReg.bottom - printer->cH);
    }

    tmReg.left += space;
    tmReg.right -= space;

    if (CTT32App::instance()->GetPrintKONamesBold())
      printer->SelectFont(nameFont);
    PrintEntry((winner == 1 ? mt.stA : mt.stX), tmReg, 0);
    if (CTT32App::instance()->GetPrintKONamesBold())
      printer->SelectFont(textFont);

    // Ergebnis
    CRect  regRes = tmReg;

    if (ko.inbox)
    {
      if (mt.mtEvent.mtRound == ko.rd + ko.maxRound - 1 ||
        mt.mtEvent.mtRound == ko.lastRd ||
        mt.mtEvent.mtRound == gr.NofRounds())
      {
        // Sieger oder letztes Spiel einer Seite: 
        // Ergebnis nach unten
        regRes.top = tmReg.bottom + 0.2 * printer->cH;
        regRes.bottom = regRes.top + printer->cH;
      }
      else if (mt.mtEvent.mtMatch & 0x01)
      {
        // Ungrade Spiele: Ergebnis nach oben
        regRes.bottom = tmReg.top - 0.2 * printer->cH;
        regRes.top = regRes.bottom - printer->cH;
      }
      else
      {
        // Grade Spiele: Ergebnis nach unten
        regRes.top = tmReg.bottom + 0.2 * printer->cH;
        regRes.bottom = regRes.top + printer->cH;
      }
    }
    else
    {
      // Ergebnis unter die Linie
      regRes.top = tmReg.bottom + 0.2 * printer->cH;
      regRes.bottom = regRes.top + printer->cH;
    }

#ifdef NEWSTYLE
    // Im INBOX hab ich beliebig Platz nach rechts, allerdins
    // darf ich nicht zweizeilig drucken, da sonst der Name 
    // ueberschrieben wird
    if (ko.inbox && cp.cpType != CP_TEAM)
      regRes.right = printer->width;
#endif      

    if (!winnerIsBye)
      PrintResult(mt, regRes, mtSetList);
  }

  if (ko.inbox || !(winner && (!winnerIsBye || !ko.ignoreByes)))
  {
    CRect  regRes;

    if (ko.inbox)
    {
      // Spielzeit links vom Spiel ?
      regRes.top = (reg.top + reg.GetHeight() / 2 - printer->cH / 2);
      regRes.bottom = regRes.top + printer->cH;
      regRes.right = reg.left - space;
      regRes.left = regRes.right + space - reg.GetWidth() + space;
    }
    else
    {
      // Spielzeit dort, wo das Ergebnis stehen wuerde
      regRes.top = reg.top + reg.GetHeight() / 2 + 0.1 * printer->cH;
      regRes.bottom = regRes.top + 2 * printer->cH;

      regRes.left = reg.left + space;
      regRes.right = reg.right - space;
    }

    PrintTime(mt, regRes, ko);
  }

  return 0;
}



// Druckt linke Randzeile mit Nation der Spieler,
// ohne Abstand zwischen Spielen
int  RasterKO::PrintKOMargin(SettingKO &ko, long flags)
{
  // Zahl der Spiele (bestimmt durch Zahl der Runden
  int  nrmt = ko.nrmt;

  // height und width bestimmen
  int  height = ko.height;
  int  width = ko.koMarginWidth;
  int  startX = ko.startX;
  int  startY = ko.startY;

#ifdef NEWSTYLE
  int  tH = (cp.cpType == CP_DOUBLE || cp.cpType == CP_MIXED) ?
    2 * printer->cH : printer->cH;
#else
  int  tH = 2 * printer->cH;
#endif            

  // Matches fuer diese Runde lesen
  std::map<short, MtRec> mtMap;

  // Kapselung von MtListStore match
  {
    MtListStore match(connPtr);
    match.mtEvent.grID = gr.grID;
    // Doppelt so viele Runden im DKO
    if (!ko.sc)
      match.mtEvent.mtRound = ko.rd;
    else
      match.mtEvent.mtRound = 2 * ko.rd - 1;

    match.SelectByRound(gr, match.mtEvent.mtRound);
    while (match.Next())
    {
      // Falsche Seite im Doppel KO
      if (match.mtEvent.mtChance != ko.sc)
        continue;

      if (match.mtEvent.mtMatch < ko.mt)
        continue;

      if (match.mtEvent.mtMatch > ko.mt + nrmt)
        continue;  // Evtl. unsortiert

      mtMap[match.mtEvent.mtMatch] = match;
    }

    match.Close();
  }

  for (int mt = 0; mt < nrmt; mt++)
  {
    if (CTT32App::instance()->GetPrintKONamesBold())
      printer->SelectFont(nameFont);

    CRect  reg;
    reg.left = startX;
    reg.right = reg.left + width;
    reg.top = startY + mt * height;
    reg.bottom = reg.top + height;

#if defined (NEWSTYLE)
    if (ko.inbox)
    {
      printer->Line(wxPoint(reg.left, reg.top),
        wxPoint(reg.right, reg.top), THINN_FRAME);
      printer->Line(wxPoint(reg.left, reg.bottom - printer->cH),
        wxPoint(reg.right, reg.bottom - printer->cH), THINN_FRAME);
    }
    else
    {
      printer->Line(wxPoint(reg.left, reg.top + 2 * printer->cH),
        wxPoint(reg.right, reg.top + 2 * printer->cH), THINN_FRAME);
      printer->Line(wxPoint(reg.left, reg.bottom - printer->cH / 2),
        wxPoint(reg.right, reg.bottom - printer->cH / 2), THINN_FRAME);
    }
#else
    printer->Line(wxPoint(reg.left, reg.top),
      wxPoint(reg.right, reg.top), THICK_FRAME);
#endif


    // Obere Mannschaft (ist A)
    MtRec match = mtMap[mt + ko.mt];

    CRect  regTeamA = reg;
#ifdef NEWSTYLE		
    if (ko.inbox)
    {
      regTeamA.top += 0.05 *printer->cH;
      regTeamA.bottom = reg.top + tH;
    }
    else
    {
      regTeamA.bottom = reg.top + 2 * printer->cH - 0.05 * printer->cH;
      regTeamA.top = regTeamA.bottom - tH;
    }
#else
    regTeamA.top += 0.05 *printer->cH;
    regTeamA.bottom = reg.top + tH;
#endif		

    CRect  regTeamX = reg;
#ifdef NEWSTYLE
    if (ko.inbox)
    {
      regTeamX.bottom = reg.bottom - 1.05 * printer->cH;
      regTeamX.top = regTeamX.bottom - tH;
    }
    else
    {
      regTeamX.bottom = reg.bottom - 0.55 * printer->cH;
      regTeamX.top = regTeamX.bottom - tH;
    }
#else		
    regTeamX.top = regTeamX.bottom - tH;
#endif		

    StEntry  stA = GetTeamA(match);
    StEntry  stX = GetTeamX(match);

    if (ko.printPosNrs)
    {
      CRect regPosA = regTeamA;
      CRect regPosX = regTeamX;

      wxChar tmp[16];

      int offset = printer->TextWidth(_itot(gr.grSize, tmp, 10)) + printer->cW;

      regPosA.right = regPosA.left + offset;
      regPosX.right = regPosX.left + offset;

      regTeamA.left = regPosA.right + printer->cW;
      regTeamX.left = regPosX.right + printer->cW;

      wxSprintf(tmp, "%d)", 2 * (mt + ko.mt) - 1);
      printer->Text(regPosA, tmp, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);

      wxSprintf(tmp, "%d)", 2 * (mt + ko.mt));
      printer->Text(regPosX, tmp, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
    }

    // Gedruckt wird per stID, dann loest PrintEntry naemlich die Freilose auf
    // In der Hauptrunde druckt ko.ignoreByes Leerstrings, in der Trostrunde 
    // sollen aber die Verweise gedruckt werden
    if ((!ko.sc && !ko.ignoreByes) || (stA.st.stID && !stA.IsBye()))
      PrintEntry(stA.st.stID, regTeamA, flags);
    else if (ko.sc)
    {
      wxChar tmp[16];
      wxSprintf(tmp, "(%d/%d)", gr.QryFromRound(match, +1), gr.QryFromMatch(match, +1));
      printer->Text(regTeamA, tmp, wxALIGN_CENTER);
    }

    if ((!ko.sc && !ko.ignoreByes) || (stX.st.stID && !stX.IsBye()))
      PrintEntry(stX.st.stID, regTeamX, flags);
    else if (ko.sc)
    {
      wxChar tmp[16];
      wxSprintf(tmp, "(%d/%d)", gr.QryFromRound(match, -1), gr.QryFromMatch(match, -1));
      printer->Text(regTeamX, tmp, wxALIGN_CENTER);
    }

    if (CTT32App::instance()->GetPrintKONamesBold())
      printer->SelectFont(textFont);
  } // for (alle Spiele)

  // Unterste Linie
#if !defined (NEWSTYLE)  
  printer->Line(wxPoint(startX, startY + nrmt * height),
    wxPoint(startX + width, startY + nrmt * height), THICK_FRAME);
#endif								

  return 0;
}



// +----------------------------------------------------------+
// +                     PrintBand                            +
// +----------------------------------------------------------+
// +  Druckt einen Teil eines Rasters, wie er auf eine        +
// +  Seite passt                                             +
// +----------------------------------------------------------+

int  RasterKO::PrintBand(SettingKO &ko, int noHead)
{
  // Kopfzeile drucken
  if (!noHead)
    PrintHeading(ko);

  // Uebernahme einiger Daten aus ko
  int  startX = ko.startX;

  // Der Anfang ist (gelegentlich) eine lange Beschreibung
  // der Spieler: Startnr, Name, Nation, ohne Abstaende
  // zwischen den Spielen.
  // Aenderung: Der Rand wir immer gedruckt, aber nicht immer die Nation
  if (ko.printKOMargin)
  {
    SettingKO  ko1 = ko;

    // Niemals den vollen Namen mit vollen Nation drucken
    // Aber Mannschaften ohne Nation
    if (CTT32App::instance()->GetPrintAssociation() == FLAG_PRINT_ASSOC_REGION)
      PrintKOMargin(ko1, FLAG_PRINT_NATION | FLAG_PRINT_NAREGION | FLAG_PRINT_FNAME);
    else if (cp.cpType == CP_TEAM)
      PrintKOMargin(ko1, FLAG_PRINT_FNAME);
    else if (CTT32App::instance()->GetPrintAssociation() == FLAG_PRINT_ASSOC_DESC)
      PrintKOMargin(ko1, FLAG_PRINT_NATION | FLAG_PRINT_NADESC | FLAG_PRINT_FNAME);
    else
      PrintKOMargin(ko1, FLAG_PRINT_NATION | FLAG_PRINT_FNAME);

    startX += ko1.koMarginWidth;
  }
  else
  {
    SettingKO ko1 = ko;
    ko1.koMarginWidth = ko.width;
    PrintKOMargin(ko1, 0);
    startX += ko.width;
  }

  ko.endX = startX;

  // Ich brauch weiter unten den rechten Rand von reg
  CRect  reg;

  // Schleife ueber alle Runden
  for (int rd = 0, nrmt = ko.nrmt; rd < ko.nrrd; rd++, nrmt /= 2)
  {
    // Hoehe und Abstand der Raster
    double  rH = (rd == 0 ? 1 : (1 << (rd - 1)));   // Hoehe: 1, 1, 2, 4, ...
    double  rA = 1 << rd;                          // Abstand: 1, 2, 4

    // Schleife ueber alle Spiele
    // Im Playoff beginnt jede Runde mit dem gleichen Spiel,
    // jedenfalls, solange das Raster auf eine Seite passt.
    // Im EKO/DKO gibt es einen Rundenabhaengigen Offset je Seite
    int  firstMt = (gr.grModus == MOD_PLO ? ko.mt - 1 : (ko.mt - 1) >> rd);

    // Matches fuer diese Runde lesen
    std::map<short, MtRec> mtMap;

    // Kapselung von MtListStore match
    {
      MtListStore match(connPtr);
      match.mtEvent.grID = gr.grID;
      // Doppelt so viele Runden im DKO
      if (!ko.sc)
        match.mtEvent.mtRound = rd + ko.rd;
      else
        match.mtEvent.mtRound = 2 * (rd + ko.rd) - 1;

      match.SelectByRound(gr, match.mtEvent.mtRound);
      while (match.Next())
      {
        // Falsche Seite im Doppel KO
        if (match.mtEvent.mtChance != ko.sc)
          continue;

        if (match.mtEvent.mtMatch < firstMt + 1)
          continue;

        if (match.mtEvent.mtMatch > firstMt + 1 + nrmt)
          continue;  // Evtl. unsortiert

        mtMap[match.mtEvent.mtMatch] = match;
      }

      match.Close();
    }

    // Satzergebnisse fuer die Spiele
    std::map<short, std::vector<MtSet>> mtSetMap;

    // Kapselung von MtSetStore
    {
      std::set<long> mtIds;
      for (const auto &it : mtMap)
        mtIds.insert(it.second.mtID);

      MtSetStore mtSet(connPtr);
      mtSet.SelectAll(mtIds);
      while (mtSet.Next())
        mtSetMap[mtSet.mtID].push_back(mtSet);
      mtSet.Close();
    }

    for (int mt = 0; mt < nrmt; mt++)
    {
      if (printer->PrinterAborted())
        return -1;

      // Match laden
      MtRec match = mtMap[mt + firstMt + 1];

      // offset waagrechte Linie: 0.5, 1, 2, 4, ...
      reg.top = reg.bottom = ko.startY + rA / 2 * ko.height +
        mt * rA * ko.height;

#ifdef NEWSTYLE
      if (ko.inbox)
      {
        if (rd == 0)
        {
          reg.bottom -= printer->cH;
        }
        else
        {
          reg.top -= printer->cH / 2;
          reg.bottom -= printer->cH / 2;
        }
      }
      else
      {
        reg.top += 0.15 * ko.height;
        reg.bottom += 0.15 * ko.height;

        if (rd == 0)
        {
          reg.top += 0.25 * ko.height;
          reg.bottom -= 0.25 * ko.height;
        }
      }
#endif

      reg.top -= (rH * ko.height) / 2;
      reg.bottom += (rH * ko.height) / 2;

      // Zwischenrunden in DKO
      if (!ko.sc)
        reg.left = startX + rd * ko.width;
      else
        reg.left = startX + 2 * rd * ko.width;
      reg.right = reg.left + ko.width;

      ko.endX = reg.right;

      PrintMatch(match, reg, ko, mtSetMap[match.mtID]);

      if (match.mtEvent.mtChance && match.mtEvent.mtRound + 1 < 2 * ko.lastRd)
      {
        // Spiel in der naechsten Runde
        match.mtEvent.mtRound++;

        MtListStore nextMt(connPtr);

        nextMt.SelectByEvent(match.mtEvent);
        nextMt.Next();

        std::vector<MtSet> nextMtSetList;
        MtSetStore nextMtSet(nextMt, connPtr);
        nextMtSet.SelectAll();
        while (nextMtSet.Next())
          nextMtSetList.push_back(nextMtSet);
        nextMtSet.Close();

        reg.top = reg.top + reg.GetHeight() / 2 - ko.height / 4;
        reg.bottom = reg.top + ko.height / 2;

        CRect regTeam = reg;
        regTeam.bottom = regTeam.top;
        regTeam.top -= (cp.cpType == CP_DOUBLE || cp.cpType == CP_MIXED) ?
          2 * printer->cH : printer->cH;

        // Und eine Linie
        printer->Line(regTeam.left, regTeam.bottom, regTeam.right, regTeam.bottom);

        if (nextMt.tmA)
          PrintEntry(nextMt.stA, regTeam, 0);
        else
        {
          wxChar tmp[32];
          wxSprintf(tmp, "(%d/%d)",
            gr.QryFromRound(nextMt, +1), gr.QryFromMatch(nextMt, +1));
          PrintStringCentered(tmp, regTeam);
        }

        reg.left = reg.right;
        reg.right += ko.width;
        PrintMatch(nextMt, reg, ko, nextMtSetList);
      }
    } // for (alle Spiele)
  } // for (alle Runden)

  ko.endX = reg.right;

  return 0;
}


// +----------------------------------------------------------+
// +                      PrintRaster                         +
// +----------------------------------------------------------+
// +  Druckt ein Teil einer Gruppe, der als EKO-Raster        +
// +    dargestellt werden kann                               +
// +  Raster wird wiederum in Teil aufgeteilt, die auf eine   +
// +    Seite passen                                          +
// +----------------------------------------------------------+

int  RasterKO::PrintRaster(SettingKO &ko)
{
  // Das erste Match ist ko.mt; Nur in PLO ist dies nicht 1!
  // Dann aber der Offset fuer kommende Spiele
  int  firstmt = ko.mt;

  // Zahl der Spiele in Runde am Anfang
  // int  nrmt = (1 << (ko.nrrd-1));
  int  nrmt = ko.nrmt;

  // Anfang des Druckens
  int  startY = ko.startY;
  // Und rechter Rand
  int  endX = ko.endX;

  int  maxRound = ko.maxRound;
  int  maxMatch = ko.maxMatch;

  // Es passen nur maximal 5 Runden (32 Spiele) auf eine Seite
  // (Grosse Ausnahme: 6 Runden)
  // Und bei Spielnummern ist es eine weniger ...
  // for (int rd = 0; rd < ko.nrrd;
  // 		 rd += maxRound, nrmt /= (maxMatch * 2))
  for (int rd = 0; rd < ko.nrrd; rd += maxRound, nrmt /= (1 << maxRound))
  {
    // Das Finale von 64 Spielern kommt auf Seite 1, d.h. nichts mehr auf letzter Seite
    // Ausser, es ist nur eine Runde
    if ((ko.nrrd > 1) && ((ko.nrrd - rd) == 1))
      break;

    for (int mt = 0; mt < nrmt; mt += maxMatch)
    {
      if (printer->PrinterAborted())
        return -1;

      // Kein Heading, wenn nicht grade eine neue Seite angefangen wird
      // Ueberfluessig ??
      int  noHead = (mt ? 1 : 0);

      // Neue Seite, wenn Band nicht auf alte passt (tut sie per Definition nicht)
      // Aber nicht, wenn es der Beginn einer Seite ist. Waehrend ein Raster gedruckt
      // wird, ist startY > offsetY, erst am Ende der Funktion wird offsetY auf den 
      // endgueltigen Wert gesetzt. Ohne das "startY > offsetY" wuerde er auch bei 
      // einer frischen Seite, z.B. Seite 1, eine neue Seite beginnen, wenn wg. Logo
      // und Sponsozeile die Seite kleiner wird als ein 32-er Raster braucht.
      if (startY > offsetY && std::min(maxMatch, nrmt) * ko.height > printer->height - startY)
      {
        NewPage();
        offsetY += printer->cH * CTT32App::instance()->GetPrintCaptionMarginKO();
        startY = offsetY;
        noHead = 0;
      }

      // Aufruf des Banding
      SettingKO  ko1 = ko;
      ko1.startY = startY;
      ko1.rd = rd + ko.rd;
      ko1.mt = mt + firstmt;                  // Erstes Spiel
      // nrmt ist die Anzahl der zu druckenden Spiele,
      // mt wurden bereits gedruckt. Also duerfen auf diese Seite nur
      // max. (nrmt - mt) Spiele kommen.
      ko1.nrmt = std::min(nrmt - mt, maxMatch);      // Anzahl der Spiele auf eier Seite
      ko1.nrrd = std::min(maxRound, ko.nrrd - rd);   // Verbleibende Zahl der Runden,
                                              // hoechstens jedoch 5
      PrintBand(ko1, noHead);

      endX = ko.endX = ko1.endX;
      startY = ko.startY = ko1.startY; // Neuen Wert von startY zurueckgeben

      // Ausnahme: maxRound + 1 Runden; Finale auf 1. Seite
      if ((ko.nrrd - rd) == maxRound + 1)
      {
        if (mt == 0)
        {
          double  top, bot, pos;
          // double  top = 2 << (maxRound - 2);
          // double  bot = top * 2 - 0.5;

          if (maxRound == ko.maxRound)
          {
            top = maxMatch / 2;
            bot = maxMatch - 1.0;
            pos = maxMatch - 1.0;

            // Wenn es ein gueltiges Sponsorbild gibt, dann etwas hoeher aufhoeren
            bool sponsor = CTT32App::instance()->GetPrintSponsor();
            if (sponsor)
            {
              bot -= 1.0;
              pos -= 1.0;
            }
          }
          else
          {
            top = maxMatch / 2;
            bot = top + maxMatch;
            pos = maxMatch;
          }

#ifdef NEWSTYLE
          if (!ko.inbox)
          {
            top += 0.15;
            bot += 0.15;
          }

          top *= ko.height;
          bot *= ko.height;
          pos *= ko.height;

          if (ko.inbox)
          {
            top -= printer->cH;
            bot -= printer->cH;
            pos -= printer->cH;
          }

          // Wenn die beiden Raster untereinander auf eine Seite kommen,
          // z.B. beim Querformat, das Finale in die Mitte drucken
          // (unterer Rand von diesem Raster)
          if (ko.startY + 2 * std::min(maxMatch, nrmt) * ko.height < printer->height)
          {
            pos = ko.startY + std::min(maxMatch, nrmt) * ko.height - startY;
            bot = ko.startY + std::min(maxMatch, nrmt) * ko.height - startY;
          }
#else          
          top *= ko.height;
          bot *= ko.height;
          pos *= ko.height;
#endif

          printer->Line(wxPoint(endX, startY + top),
            wxPoint(endX, startY + bot));

          // Match als waagrechte Linie
          CRect  reg;
          reg.top = reg.bottom = startY + pos;
          reg.right = endX; //printer->width;
          reg.left = reg.right - ko.width;

#ifdef NEWSTYLE
          reg.top += 0.15 * ko.height;
          reg.bottom += 0.15 * ko.height;
#endif

          // Finale
          // Match laden
          MtListStore  match(connPtr);
          match.mtEvent.grID = gr.grID;
          match.mtEvent.mtRound = ko.nrrd - 1 + ko.rd;
          match.mtEvent.mtMatch = mt + ko.mt;
          match.mtEvent.mtChance = 0;

          match.SelectByEvent(match.mtEvent);
          match.Next();

          PrintMatch(match, reg, ko);
        } // obere Haelfte von 6 verbleibende Runden
        else
        {
          double  top, bot;
          if (maxRound == ko.maxRound)
          {
            top = 1.5;
            bot = maxMatch / 2;

#ifdef NEWSTYLE
            top += 0.15;
            bot += 0.15;

            // Siehe oben: Wenn beide Raster auf eine Seite kommen,
            // den Strich bis ganz nach oben von diesem Raster ziehen.
            if (ko.startY <= printer->height)
            {
              top = 0;
            }
#endif

            printer->Line(wxPoint(endX, startY + bot * ko.height),
              wxPoint(endX, startY + top * ko.height));
          }
          // else gibt es eigentlich nie ...
          else
          {
            // top = maxMatch / 2;
            // bot = (3 * maxMatch) / 4;
          }
        } // untere Haelfte
      } // 6 verbleibende Runden

      startY += std::min(maxMatch, nrmt) * ko.height;
    } // jeweils 32 Spiele
  } // jeweils 5 Runden

  // Neuer offsetY ist das untere Ende des letzten Bandes
  offsetY = startY;

  return 0;
}



// +-------------------------------------+
// +   abgeleitete Klasse RasterSKO     +
// +-------------------------------------+


RasterSKO::RasterSKO(Printer *prt, Connection *ptr)
  : RasterKO(prt, ptr)
{
}


// Kopfzeile drucken
int  RasterSKO::PrintHeading(SettingKO &ko)
{
  CRect  reg;
  reg.left = ko.startX;
  reg.top = ko.startY;
  reg.bottom = reg.top + printer->cH;

  int  nrRounds = gr.NofRounds();

  // Eine Runde mehr drucken, als ko.nrrd angibt
  for (int rd = 0; rd <= ko.nrrd; rd++)
  {
    if (ko.lastRd == 1 || rd + ko.rd > ko.lastRd)
      break;

    if (rd + ko.rd > nrRounds)      // aber nicht mehr, als Runden ueberhaupt
      break;

    if (rd == 0 && ko.printKOMargin)
      reg.right = reg.left + ko.koMarginWidth;
    else
      reg.right = reg.left + ko.width;

    wxChar str[64];

    if (rd + ko.rd == nrRounds)     // Finale
    {
      if (gr.grWinner == 1)
      {
        // Finale
        wxString strFinal = _("Final");
        wxStrcpy(str, strFinal.t_str());
      }
      else
      {
        // Pos. X
        wxString strPos = _("Pos.");
        wxSprintf(str, "%s %d", strPos.t_str(), gr.grWinner);
      }
    }
    else if ((rd + ko.rd == nrRounds - 1) && (rd + ko.rd > 1))
    {
      if (gr.grWinner == 1)
      {
        wxString strSemiFinal = _("Semifinal");
        wxStrcpy(str, strSemiFinal.t_str());
      }
      else
      {
        wxString strPos = _("Pos.");
        wxSprintf(str, "%s %d - %d", strPos.t_str(), gr.grWinner, gr.grWinner + 1);
      }
    }
    else
    {
      wxString strRound;

      if (rd + ko.rd <= gr.grQualRounds)
      {
        strRound = _("Qualification");
        wxSprintf(str, "%s", strRound.t_str());
      }
      else
      {
        strRound = _("Round");
        wxSprintf(str, "%i. %s", rd + ko.rd - gr.grQualRounds, strRound.t_str());
      }
    }

    CRect  region = reg;
    // region.left += (reg.Width() -printer->TextWidth(str, textFont)) / 2 - space;
    // region.right -= (reg.Width() -printer->TextWidth(str, textFont)) / 2 + space;

    PrintStringCentered(str, region);

    reg.left = reg.right;
  }

  // Wenn ueberhaupt eine Ueberschrift gedruckt wurde, vorruecken
  if (ko.lastRd > 1 && nrRounds > ko.rd)
    ko.startY += 2 * printer->cH;

  return 0;
}


void RasterSKO::PrintSponsor()
{
  wxImage sponsor;
  if (CTT32App::instance()->GetPrintSponsor() && IdStore::GetSponsorImage(sponsor))
  {
    // Sponsor Bild wird rechts unten eingepasst
    int xFooterLogo = printer->width * 0.45;
    int yFooterLogo = 10 * printer->cH;

    wxPoint topLeft(-1, -1);
    wxPoint bottomRight(printer->width, printer->height);
    printer->DrawImage(topLeft, bottomRight, wxSize(xFooterLogo, yFooterLogo), sponsor);
  }
}


// Aufgabe von Print besteht lediglich, Raster in EKO-Raster
// zu zerlegen; Triviale Aufgabe!
// Daneben noch einige Initialisierungen
int  RasterSKO::Print(const CpRec &cp_, const GrRec &gr_,
  const PrintRasterOptions &options_,
  int *pofstX, int *pofstY, int *ppage)
{
  SetupGroup(cp_, gr_);

  options = options_;

  offsetX = *pofstX;
  offsetY = *pofstY;
  page = *ppage;

  bool newPage = false;

  if (options.koNewPage || page == 0)
  {
    NewPage();
    newPage = true;
  }

  textFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_SMALL);
  nameFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_SMALLB);

  short oldFont = printer->SelectFont(textFont);

# ifndef KO_MAX_ROUNDS
  // Wieviele Spiele passen auf eine Seite ?
  int  koMaxMatch = printer->height / (5 * printer->cH);
  int  koMaxRound = 0;
  for (; koMaxMatch & ~(1 << koMaxRound); koMaxRound++)
    koMaxMatch &= ~(1 << koMaxRound);

  // plusFinale
  koMaxRound++;

  // Testweise mal eine groessere Schrift waehlen
  short mediumFont =
    printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_MEDIUM);
  printer->SelectFont(mediumFont);

  if (((koMaxMatch + 1) * 5 * printer->cH) < printer->height)
  {
    printer->DeleteFont(textFont);
    textFont = mediumFont;
    printer->DeleteFont(nameFont);
    nameFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_MEDIUMB);
  }
  else
  {
    printer->SelectFont(textFont);
    printer->DeleteFont(mediumFont);
  }

  // Und eine noch groessere Schrift
  short normalFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_NORMAL);
  printer->SelectFont(normalFont);

  if (((koMaxMatch + 1) * 5 * printer->cH) < printer->height)
  {
    printer->DeleteFont(textFont);
    textFont = normalFont;
    printer->DeleteFont(nameFont);
    nameFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_NORMALB);
  }
  else
  {
    printer->SelectFont(textFont);
    printer->DeleteFont(normalFont);
  }

# else
  int  koMaxMatch = KO_MAX_MATCHES;
  int  koMaxRound = KO_MAX_ROUNDS;
# endif

  int  maxRound = (options.koNr ? koMaxRound - 1 : koMaxRound);
  int  maxMatch = (options.koNr ? koMaxMatch / 2 : koMaxMatch);

  // Wenn der volle Nationenname gedruckt wird, nochmals verringern
  // Ebenso bei Doppeln in Internationalen WB
  if ( (cp.cpType != CP_TEAM && CTT32App::instance()->GetPrintAssociation() == FLAG_PRINT_ASSOC_DESC) ||
       (cp.cpType == CP_TEAM && CTT32App::instance()->GetPrintAssociationTeam() == FLAG_PRINT_ASSOC_DESC) )
  {
    if ((maxRound * KO_NAME_WIDTH + ASSOCDESC_OFFST) * printer->cW > printer->width)
    {
      maxRound--;
      maxMatch /= 2;
    }
  }
  else if (options.koInbox)
  {
    if (cp.cpType == CP_DOUBLE || cp.cpType == CP_MIXED)
    {
      maxRound--;
      maxMatch /= 2;
    }
  }

  int  nrrd = std::min(options.koToRound - options.koFromRound + 1,
    gr.NofRounds() - options.koFromRound + 1);

  // Schriftgroesse in Abhaengigkeit von der Zahl der Runden
  // Abstand Texte von Raendern
  space = 0.5 *printer->cW;

  // Gruppe auf neuer Seite anfangen lassen, wenn gefordert, erste Seite
  // oder Gruppe nicht mehr auf Seite passt.
  if (newPage)
    offsetY += printer->cH * CTT32App::instance()->GetPrintCaptionMarginKO();
  else if ((printer->height - offsetY) <
    (gr.NofMatches(options.koFromRound) + 2) * 5 * printer->cH)
  {
    NewPage();
    offsetY += printer->cH * CTT32App::instance()->GetPrintCaptionMarginKO();
  }
  else
  {
    offsetY += 5 * printer->cH;
    /*
        PrintCaption();
        offsetY += printer->cH * CTT32App::instance()->GetPrintCaptionMarginKO();
    */
    // Eindhoven: Statt die komplette Caption nur die Gruppe
    short  oldFont, groupFont;
    groupFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_GROUP);
    oldFont = printer->SelectFont(groupFont);

    int height = printer->cH;

    CRect  reg;
    reg.left = offsetX;
    reg.right = printer->width;
    reg.top = offsetY;
    reg.bottom = offsetY + height;

    PrintStringCentered(gr.grDesc, reg);

    offsetY += height;
    printer->SelectFont(oldFont);
    printer->DeleteFont(groupFont);

    offsetY += 0.7 * height;
  }

  // Add Bookmark fuer aktuelle Position (Caption)
  printer->Bookmark(gr.grDesc);

  // Unter das Heading muessen 32 Spiele passen, das Heading ist 2 Zeilen hoch
  // Das Raster muss jedoch mindestens 5 Zeilen hoch sein
  // int height = (printer->height - offsetY - 2 *printer->cH) / KO_MAX_MATCHES;
  // height = max(height, 5 *printer->cH);
  int  height = 5 * printer->cH;                        // 2 Doppel + Zeiten
  // int height = (printer->height - offsetY) / KO_MAX_MATCHES;

  if (options.koInbox)
  {
    if (cp.cpType == CP_DOUBLE || cp.cpType == CP_MIXED)
      height = 7 * printer->cH;
    else
      height = 4.5 * printer->cH;
  }

  // Die Breite ist fuer maxRound Felder,
  // maximal jedoch KO_NAME_WIDTH Zeichen breit
  int  width = KO_NAME_WIDTH * printer->cW;
  /*
    int  rw = (printer->width - offsetX - width) /
              min(maxRound, nrrd);
    width = min(width, rw);
  */
  // Berechnung der Rasterbreite
  int  rw = printer->width - offsetX - width;
  if ((cp.cpType != CP_TEAM && CTT32App::instance()->GetPrintAssociation() == FLAG_PRINT_ASSOC_DESC) ||
      (cp.cpType == CP_TEAM && CTT32App::instance()->GetPrintAssociationTeam() == FLAG_PRINT_ASSOC_DESC))
    rw -= ASSOCDESC_OFFST * printer->cW;
  rw /= std::min(maxRound, (int)gr.NofRounds());

  width = std::min(width, rw);

  // Aufsetzen von ko
  SettingKO  ko;
  ko.maxRound = maxRound;
  ko.maxMatch = maxMatch;
  ko.sc = 0;
  int fromRound = options.koFromRound;
  int toRound = options.koToRound;

  ko.rd = fromRound;
  ko.lastRd = toRound;

  ko.nrrd = std::min(toRound - fromRound + 1, gr.NofRounds() - fromRound + 1);

  int fromMatch = options.koFromMatch;
  int toMatch = options.koToMatch;

  ko.mt = fromMatch;

  ko.nrmt = std::min(toMatch - fromMatch + 1, (int)gr.NofMatches(ko.rd));

  // Max. soviele Runden wie die Spiele erfordern  
  while ((1 << (ko.nrrd - 1)) > ko.nrmt)
    --ko.nrrd;

  ko.maxMatch = maxMatch;
  ko.maxRound = maxRound;
  ko.width = width;
  ko.height = height;
  ko.startX = offsetX;
  ko.startY = offsetY;
  ko.endX = ko.startX;
  ko.printKOMargin = TRUE;
  ko.printNr = options.koNr;
  ko.ignoreByes = options.koIgnoreByes;
  ko.inbox = options.koInbox;
  ko.printPosNrs = options.koPrintPosNrs;

  ko.koMarginWidth = std::max(ko.width, (int)(KO_NAME_WIDTH * printer->cW));
  if (ko.printPosNrs)
  {
    wxChar tmp[16];
    wxSprintf(tmp, "%d)", 2 * (ko.mt + ko.nrmt));
    ko.koMarginWidth += printer->TextWidth(tmp);
  }

  // Bei dem vollen Nationennamen verbreitert sich der Rand noch
  if ((cp.cpType != CP_TEAM && CTT32App::instance()->GetPrintAssociation() == FLAG_PRINT_ASSOC_DESC) ||
      (cp.cpType == CP_TEAM && CTT32App::instance()->GetPrintAssociationTeam() == FLAG_PRINT_ASSOC_DESC))
    ko.koMarginWidth += (ASSOCDESC_OFFST *printer->cW);

  if (ko.koMarginWidth + ko.nrrd * ko.width < printer->width - offsetX)
  {
    ko.koMarginWidth = std::min(ko.koMarginWidth + ko.koMarginWidth / 4, (int)(printer->width - offsetX - ko.nrrd * width));
    ko.width = std::min(ko.width + ko.width / 4, (int)(printer->width - offsetX - ko.koMarginWidth) / std::max(ko.nrrd, 1));
  }

  PrintRaster(ko);

  *pofstY = offsetY;
  *ppage = page;

  // Clean up
  printer->SelectFont(oldFont);
  printer->DeleteFont(textFont);
  printer->DeleteFont(nameFont);

  return 0;
}



// +-------------------------------------+
// +   abgeleitete Klasse RasterDKO     +
// +-------------------------------------+

RasterDKO::RasterDKO(Printer *prt, Connection *ptr)
  : RasterKO(prt, ptr)
{
}


// Kopfzeile drucken
int  RasterDKO::PrintHeading(SettingKO &ko)
{
  CRect  reg;
  reg.left = ko.startX;
  reg.top = ko.startY;
  reg.bottom = reg.top + printer->cH;

  int  nrRounds = gr.NofRounds();

  // Eine Runde mehr drucken, als ko.nrrd angibt
  // Die Verliererseite hat doppelt so viele Runden
  for (int rd = 0; rd <= (ko.sc ? 2 * ko.nrrd : ko.nrrd); rd++)
  {
    if (rd + ko.rd > nrRounds)      // aber nicht mehr, als Runden ueberhaupt
      break;

    if (rd == 0 && ko.printKOMargin)
      reg.right = reg.left + ko.koMarginWidth;
    else
      reg.right = reg.left + ko.width;

    wxChar str[64];

    if (ko.sc)
    {
      wxString strRound;
      strRound = _("Round");
      wxSprintf(str, "%i. %s", rd + 2 * ko.rd - 1, strRound.t_str());
    }
    else if (rd + ko.rd == nrRounds)     // Finale
    {
      if (gr.grWinner == 1)
      {
        // Finale
        wxString strFinal = _("Final");
        wxStrcpy(str, strFinal.t_str());
      }
      else
      {
        // Pos. X
        wxString strPos = _("Pos.");
        wxSprintf(str, "%s %d", strPos.t_str(), gr.grWinner);
      }
    }
    else if ((rd + ko.rd == nrRounds - 1) && (rd + ko.rd > 1))
    {
      if (gr.grWinner == 1)
      {
        wxString strSemiFinal = _("Semifinal");
        wxStrcpy(str, strSemiFinal.t_str());
      }
      else
      {
        wxString strPos = _("Pos.");
        wxSprintf(str, "%s %d - %d", strPos.t_str(), gr.grWinner, gr.grWinner + 1);
      }
    }
    else
    {
      wxString strRound = _("Round");
      wxSprintf(str, "%i. %s", rd + ko.rd, strRound.t_str());
    }

    CRect  region = reg;
    // region.left += (reg.Width() -printer->TextWidth(str, textFont)) / 2 - space;
    // region.right -= (reg.Width() -printer->TextWidth(str, textFont)) / 2 + space;

    PrintStringCentered(str, region);

    reg.left = reg.right;
  }

  ko.startY += 2 * printer->cH;

  return 0;
}


int  RasterDKO::Print(CpRec &cp_, GrRec &gr_,
  PrintRasterOptions &options_,
  int *pofstX, int *pofstY, int *ppage)
{
  SetupGroup(cp_, gr_);

  options = options_;

  offsetX = *pofstX;
  offsetY = *pofstY;
  page = *ppage;

  bool newPage = false;

  if (options.koNewPage || page == 0)
  {
    NewPage();
    newPage = true;
  }

  textFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_SMALL);
  nameFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_SMALLB);

  short oldFont = printer->SelectFont(textFont);

# ifndef KO_MAX_ROUNDS
  // Wieviele Spiele passen auf eine Seite ?
  int  koMaxMatch = printer->height / (5 * printer->cH);
  int  koMaxRound = 0;
  for (; koMaxMatch & ~(1 << koMaxRound); koMaxRound++)
    koMaxMatch &= ~(1 << koMaxRound);

  // plusFinale
  koMaxRound++;

  // Testweise mal eine groessere Schrift waehlen
  short mediumFont =
    printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_MEDIUM);
  printer->SelectFont(mediumFont);

  if (((koMaxMatch + 1) * 5 * printer->cH) < printer->height)
  {
    printer->DeleteFont(textFont);
    textFont = mediumFont;
    printer->DeleteFont(nameFont);
    nameFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_MEDIUMB);
  }
  else
  {
    printer->SelectFont(textFont);
    printer->DeleteFont(mediumFont);
  }

  // Und eine noch groessere Schrift
  short normalFont =
    printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_NORMAL);
  printer->SelectFont(normalFont);

  if (((koMaxMatch + 1) * 5 * printer->cH) < printer->height)
  {
    printer->DeleteFont(textFont);
    textFont = normalFont;
    printer->DeleteFont(nameFont);
    nameFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_NORMALB);
  }
  else
  {
    printer->SelectFont(textFont);
    printer->DeleteFont(normalFont);
  }

# else
  int  koMaxMatch = KO_MAX_MATCHES;
  int  koMaxRound = KO_MAX_ROUNDS;
# endif

  int  maxRound = (options.koNr ? koMaxRound - 1 : koMaxRound);
  int  maxMatch = (options.koNr ? koMaxMatch / 2 : koMaxMatch);

  // Wenn der volle Nationenname gedruckt wird, nochmals verringern
  if ((cp.cpType != CP_TEAM && CTT32App::instance()->GetPrintAssociation() == FLAG_PRINT_ASSOC_DESC) ||
      (cp.cpType == CP_TEAM && CTT32App::instance()->GetPrintAssociationTeam() == FLAG_PRINT_ASSOC_DESC))
  {
    if ((maxRound * KO_NAME_WIDTH + ASSOCDESC_OFFST) * printer->cW > printer->width)
    {
      maxRound--;
      maxMatch /= 2;
    }
  }
  else if (options.koInbox)
  {
    if (cp.cpType == CP_DOUBLE || cp.cpType == CP_MIXED)
    {
      maxRound--;
      maxMatch /= 2;
    }
  }

  int  nrrd = std::min(options.koToRound - options.koFromRound + 1,
    gr.NofRounds() - options.koFromRound + 1);

  // Abstand Texte von Raendern
  space = 0.5 *printer->cW;

  // Gruppe auf neuer Seite anfangen lassen, wenn gefordert, erste Seite
  // oder Gruppe nicht mehr auf Seite passt.
  if (newPage)
    ; // Nothing
  else if ((printer->height - offsetY) <
    (gr.NofMatches(options.koFromRound) + 2) * 5 * printer->cH)
  {
    NewPage();
    offsetY += printer->cH * CTT32App::instance()->GetPrintCaptionMarginKO();
  }
  else
  {
    offsetY += 5 * printer->cH;
    /*
        PrintCaption();
        offsetY += printer->cH * CTT32App::instance()->GetPrintCaptionMarginKO();
    */
    // Eindhoven: Statt die komplette Caption nur die Gruppe
    short  oldFont, groupFont;
    groupFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_GROUP);
    oldFont = printer->SelectFont(groupFont);

    int height = printer->cH;

    CRect  reg;
    reg.left = offsetX;
    reg.right = printer->width;
    reg.top = offsetY;
    reg.bottom = offsetY + height;

    PrintStringCentered(gr.grDesc, reg);

    offsetY += height;
    printer->SelectFont(oldFont);
    printer->DeleteFont(groupFont);

    offsetY += 0.7 * height;
  }

  // Add Bookmark fuer aktuelle Position (Caption)
  printer->Bookmark(gr.grDesc);

  // Unter das Heading muessen 32 Spiele passen, das Heading ist 2 Zeilen hoch
  // Das Raster muss jedoch mindestens 5 Zeilen hoch sein
  // int height = (printer->height - offsetY - 2 *printer->cH) / KO_MAX_MATCHES;
  // height = max(height, 5 *printer->cH);
  int  height = 5 * printer->cH;                        // 2 Doppel + Zeiten
  // int height = (printer->height - offsetY) / KO_MAX_MATCHES;

  if (options.koInbox)
  {
    if (cp.cpType == CP_DOUBLE || cp.cpType == CP_MIXED)
      height = 7 * printer->cH;
    else
      height = 4.5 * printer->cH;
  }

  // Die Breite ist fuer maxRound Felder,
  // maximal jedoch KO_NAME_WIDTH Zeichen breit
  int  width = KO_NAME_WIDTH * printer->cW;
  /*
    int  rw = (printer->width - offsetX - width) /
              min(maxRound, nrrd);
    width = min(width, rw);
  */
  // Berechnung der Rasterbreite
  int  rw = printer->width - offsetX - width;
  if ((cp.cpType != CP_TEAM && CTT32App::instance()->GetPrintAssociation() == FLAG_PRINT_ASSOC_DESC) ||
      (cp.cpType == CP_TEAM && CTT32App::instance()->GetPrintAssociationTeam() == FLAG_PRINT_ASSOC_DESC))
    rw -= ASSOCDESC_OFFST * printer->cW;
  rw /= std::min(maxRound, gr.NofRounds() - 1);

  width = std::min(width, rw);

  // Aufsetzen von ko
  SettingKO  ko;
  ko.maxRound = maxRound;
  ko.maxMatch = maxMatch;
  ko.sc = 0;
  if (!options.koSlctRound)
  {
    ko.rd = 1;
    ko.lastRd = ko.nrrd = gr.NofRounds() - 1;
  }
  else
  {
    ko.rd = options.koFromRound;
    ko.nrrd = std::min(options.koToRound - options.koFromRound + 1,
      gr.NofRounds() - 1 - options.koFromRound + 1);
    ko.lastRd = options.koToRound;
  }
  if (!options.koSlctMatch)
  {
    ko.mt = 1;
    ko.nrmt = gr.NofMatches(ko.rd);
  }
  else
  {
    ko.mt = options.koFromMatch;
    ko.nrmt = std::min(options.koToMatch - options.koFromMatch + 1, (int)gr.NofMatches(ko.rd));
  }

  ko.maxMatch = maxMatch;
  ko.maxRound = maxRound;
  ko.width = width;
  ko.height = height;
  ko.startX = offsetX;
  ko.startY = offsetY;
  ko.endX = ko.startX;
  ko.printKOMargin = TRUE;
  ko.printNr = options.koNr;
  ko.ignoreByes = options.koIgnoreByes;
  ko.inbox = options.koInbox;
  ko.printPosNrs = options.koPrintPosNrs;

  ko.koMarginWidth = std::max(ko.width, (int)(KO_NAME_WIDTH * printer->cW));
  if (ko.printPosNrs)
  {
    wxChar tmp[16];
    wxSprintf(tmp, "%d)", 2 * (ko.mt + ko.nrmt));
    ko.koMarginWidth += printer->TextWidth(tmp);
  }

  // Bei dem vollen Nationennamen verbreitert sich der Rand noch
  if ((cp.cpType != CP_TEAM && CTT32App::instance()->GetPrintAssociation() == FLAG_PRINT_ASSOC_DESC) ||
      (cp.cpType == CP_TEAM && CTT32App::instance()->GetPrintAssociationTeam() == FLAG_PRINT_ASSOC_DESC))
    ko.koMarginWidth += (ASSOCDESC_OFFST *printer->cW);

  PrintRaster(ko);

  // Jetzt das Verliererraster
  if (ko.nrrd > 1)
  {
    NewPage();
    ko.sc = 1;
    ko.height *= 2;
    // ko.maxMatch /= 2;
    // Die zusaetzlichen Runden werden nicht mitgezaehlt
    ko.maxRound /= 2;
    ko.nrmt /= 2;
    ko.startX = offsetX;
    ko.startY = offsetY;
    ko.endX = ko.startX;

    PrintRaster(ko);
  }

  // TODO: Die beiden Endspiele

  *pofstY = offsetY;
  *ppage = page;

  // Clean up
  printer->SelectFont(oldFont);
  printer->DeleteFont(textFont);
  printer->DeleteFont(nameFont);

  return 0;
}




// +-------------------------------------+
// +   abgeleitete Klasse RasterPLO     +
// +-------------------------------------+

RasterPLO::RasterPLO(Printer *prt, Connection *ptr)
  : RasterKO(prt, ptr)
{
}


// Kopfzeile drucken
int  RasterPLO::PrintHeading(SettingKO &ko)
{
  CRect  reg;
  reg.left = ko.startX;
  reg.top = ko.startY;
  reg.bottom = reg.top + printer->cH;

  for (int rd = 0; rd < ko.nrrd; rd++)
  {
    if (rd == 0 && ko.printKOMargin)
      reg.right = reg.left + ko.koMarginWidth;
    else
      reg.right = reg.left + ko.width;

    wxChar str[64];

    if (gr.grNofRounds == 1)             // Sonderregelung nur eine Runde
    {
      wxString strPos = _("Pos");
      wxSprintf(str, "%s %i - %i", strPos.t_str(), gr.grWinner + 2 * (ko.mt - 1), gr.grWinner + 2 * (ko.mt - 1) + 1);
    }
    else if (rd + ko.rd == gr.NofRounds())     // Finale
    {
      if (gr.grWinner == 1 && ko.mt == 1)
      {
        // Finale
        wxString strFinal = _("Final");
        wxStrcpy(str, strFinal.t_str());
      }
      else
      {
        wxString strPos = _("Pos");
        wxSprintf(str, "%s %i - %i", strPos.t_str(),
          gr.grWinner + 2 * (ko.mt - 1),
          gr.grWinner + 2 * (ko.mt + (ko.nrmt >> rd) - 2) + 1);
      }
    }
    else if ((rd + ko.rd == gr.NofRounds() - 1) && (rd + ko.rd > 1))
    {
      if (gr.grWinner == 1 && ko.mt <= 2)
      {
        wxString strSemiFinal = _("Semifinal");
        wxStrcpy(str, strSemiFinal.t_str());
      }
      else
      {
        wxString strPos = _("Pos");
        wxSprintf(str, "%s %i - %i", strPos.t_str(),
          gr.grWinner + 2 * (ko.mt - 1),
          gr.grWinner + 2 * (ko.mt + (ko.nrmt >> rd) - 2) + 1);
      }
    }
    else if ((rd + ko.rd) == 1)         // 1. Runde
    {
      wxString strRound = _("Round");
      wxSprintf(str, "%i. %s", rd + ko.rd, strRound.t_str());
    }
    else if (gr.grOnlyThirdPlace)
    {
      wxString strRound = _("Round");
      wxSprintf(str, "%i. %s", rd + ko.rd, strRound.t_str());
    }
    else
    {
      wxString strPos = _("Pos");
      wxSprintf(str, "%s %i - %i", strPos.t_str(),
        gr.grWinner + 2 * (ko.mt - 1),
        gr.grWinner + 2 * (ko.mt + (ko.nrmt >> rd) - 2) + 1);
    }

    PrintStringCentered(str, reg);

    reg.left = reg.right;
  }

  ko.startY += 2 * printer->cH;

  return 0;
}


int  RasterPLO::Print(CpRec &cp_, GrRec &gr_,
  PrintRasterOptions &options_,
  int *pofstX, int *pofstY, int *ppage)
{
  SetupGroup(cp_, gr_);

  options = options_;

  offsetX = *pofstX;
  offsetY = *pofstY;
  page = *ppage;

  bool newPage = false;

  if (options.koNewPage || page == 0 || gr.grSize > 8)
  {
    NewPage();
    newPage = true;
  }


  // Von der Schriftart haengt die Zahl der Spiele je Seite ab.
  // Die Groesse haengt nicht (mehr) von der Gruppengroesse ab.
  textFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_SMALL);
  nameFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_SMALLB);
  short oldFont = printer->SelectFont(textFont);

# ifndef KO_MAX_ROUNDS

  // Wieviele Spiele passen auf eine Seite ?
  int  koMaxMatch = printer->height / (5 * printer->cH);
  int  koMaxRound = 0;
  for (; koMaxMatch & ~(1 << koMaxRound); koMaxRound++)
    koMaxMatch &= ~(1 << koMaxRound);

  // Plus Finale
  koMaxRound++;

  // Testweise mal eine groessere Schrift waehlen
  short mediumFont =
    printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_MEDIUM);
  printer->SelectFont(mediumFont);

  if (((koMaxMatch + 1) * 5 * printer->cH) < printer->height)
  {
    printer->DeleteFont(textFont);
    textFont = mediumFont;
    printer->DeleteFont(nameFont);
    nameFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_MEDIUMB);
  }
  else
  {
    printer->SelectFont(textFont);
    printer->DeleteFont(mediumFont);
  }

  // Und eine noch groessere Schrift
  short normalFont =
    printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_NORMAL);
  printer->SelectFont(normalFont);

  if (((koMaxMatch + 1) * 5 * printer->cH) < printer->height)
  {
    printer->DeleteFont(textFont);
    textFont = normalFont;
    printer->DeleteFont(nameFont);
    nameFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_NORMALB);
  }
  else
  {
    printer->SelectFont(textFont);
    printer->DeleteFont(normalFont);
  }

# else
  int  koMaxMatch = KO_MAX_MATCHES;
  int  koMaxRound = KO_MAX_ROUNDS;
# endif

  int  maxRound = (options.koNr ? koMaxRound - 1 : koMaxRound);
  int  maxMatch = (options.koNr ? koMaxMatch / 2 : koMaxMatch);

  // Wenn der volle Nationenname gedruckt wird, nochmals verringern
  if ((cp.cpType != CP_TEAM && CTT32App::instance()->GetPrintAssociation() == FLAG_PRINT_ASSOC_DESC) ||
      (cp.cpType == CP_TEAM && CTT32App::instance()->GetPrintAssociationTeam() == FLAG_PRINT_ASSOC_DESC))
  {
    if ((maxRound * KO_NAME_WIDTH + ASSOCDESC_OFFST) * printer->cW > printer->width)
    {
      maxRound--;
      maxMatch /= 2;
    }
  }
  else if (options.koInbox)
  {
    if (cp.cpType == CP_DOUBLE || cp.cpType == CP_MIXED)
    {
      maxRound--;
      maxMatch /= 2;
    }
  }

  int  nrrd = std::min(options.koToRound - options.koFromRound + 1,
    gr.NofRounds() - options.koFromRound + 1);

  // Abstand Texte von Raendern
  space = 0.5 * printer->cW;

  // Unter das Heading muessen 32 Spiele passen, das Heading ist 2 Zeilen hoch
  // Das Raster muss jedoch mindestens 5 Zeilen hoch sein
  int  height = 5 * printer->cH;                        // 2 Doppel + Zeiten

  if (options.koInbox)
  {
    if (cp.cpType == CP_DOUBLE || cp.cpType == CP_MIXED)
      height = 7 * printer->cH;
    else
      height = 4.5 * printer->cH;
  }

  // Gruppe auf neuer Seite anfangen lassen
  // Gruppen > 8 passen nie
  // Gruppen mit 8 brauchen 10.5, mit 4 brauchen 4, mit 2 brauchen 2 'Hoehen' 
  // (wenn sie enger gedruckt werden, aber das tu ich nur noch mit Logo / Sponsorzeile)
  // Dazu kommt noch der Header
  int  spaceLeft = printer->height - offsetY - height;

  if (newPage)
  {
    offsetY += printer->cH * CTT32App::instance()->GetPrintCaptionMarginKO();
  }
  else if ((gr.grSize == 8) && (spaceLeft < 12 * height) ||
    (gr.grSize == 4) && (spaceLeft < 5 * height) ||
    (gr.grSize == 2) && (spaceLeft < 2 * height))
  {
    NewPage();
    offsetY += printer->cH * CTT32App::instance()->GetPrintCaptionMarginKO();
  }
  else
  {
    offsetY += 5 * printer->cH;
    /*
        PrintCaption();
        offsetY += printer->cH * CTT32App::instance()->GetPrintCaptionMarginKO();
    */
    // Eindhoven: Statt die komplette Caption nur die Gruppe
    short  oldFont, groupFont;
    groupFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_GROUP);
    oldFont = printer->SelectFont(groupFont);

    int height = printer->cH;  // war 3 * printer->cH;

    CRect  reg;
    reg.left = offsetX;
    reg.right = printer->width;
    reg.top = offsetY;
    reg.bottom = offsetY + height;

    PrintStringCentered(gr.grDesc, reg);

    offsetY += height;
    printer->SelectFont(oldFont);
    printer->DeleteFont(groupFont);

    offsetY += 0.7 * height;
  }

  // Add Bookmark fuer aktuelle Position (Caption)
  printer->Bookmark(gr.grDesc);

  // Feststellen, wieviele Freilose es gibt.
  // Davon haengt ab, welche Raster gedruckt werden muessen.
  // Raster, die nur noch aus Freilosen bestehen (z.B. bei 12 Spieler
  // in einem 16-er Feld die Plaetze 13 - 16) brauchen nicht mehr
  // gedruckt werden.
  int nofByes = 0;
  if (!options.koIgnoreByes)
  {
    for (StEntryMap::iterator it = stMap.begin(); it != stMap.end(); ++it)
    {
      if ((*it).second.st.tmID == 0 && (*it).second.team.cpType != CP_GROUP)
        ++nofByes;
    }
  }

  // Vorraussetzung hier ist, dass das ganze Raster auf eine
  // Seite passt.
  // Gedruckt wird das Raster als Folge von EKO-Raster:
  // Ganzes Raster, Endspiel wiederholen,
  // Semifinale, Endspiel wiederholen
  // Achtelfinale, Endspiel, Semifinale, Endspiel wiederholen

  // Die Breite ist fuer maxRound Felder,
  // maximal jedoch KO_NAME_WIDTH Zeichen breit
  int  cW = printer->cW;
  int  width = KO_NAME_WIDTH * cW;

  // Berechnung der Rasterbreite
  int  rw = printer->width - offsetX - width;
  if ((cp.cpType != CP_TEAM && CTT32App::instance()->GetPrintAssociation() == FLAG_PRINT_ASSOC_DESC) ||
      (cp.cpType == CP_TEAM && CTT32App::instance()->GetPrintAssociationTeam() == FLAG_PRINT_ASSOC_DESC))
    rw -= ASSOCDESC_OFFST * printer->cW;
  rw /= std::min(maxRound, (int)gr.NofRounds());

  width = std::min(width, rw);

  // Aufsetzen von ko
  SettingKO  ko;
  ko.maxRound = maxRound;
  ko.maxMatch = maxMatch;
  ko.sc = 0;
  if (!options.koSlctRound)
  {
    ko.rd = 1;
    ko.nrrd = gr.NofRounds();
    ko.lastRd = gr.NofRounds();
  }
  else
  {
    ko.rd = options.koFromRound;
    ko.nrrd = std::min(options.koToRound - options.koFromRound + 1,
      gr.NofRounds() - options.koFromRound + 1);
    ko.lastRd = options.koToRound;
  }

  if (!options.koSlctMatch)
  {
    ko.mt = 1;
    ko.nrmt = gr.NofMatches(ko.rd);
  }
  else
  {
    ko.mt = options.koFromMatch;
    ko.nrmt = std::min(options.koToMatch - options.koFromMatch + 1,
      (int)gr.NofMatches(ko.rd));
  }
  ko.width = width;
  ko.height = height;
  ko.startX = offsetX;
  ko.startY = offsetY;
  ko.printKOMargin = TRUE;
  ko.printNr = options.koNr;
  ko.ignoreByes = options.koIgnoreByes;
  ko.inbox = options.koInbox;
  ko.printPosNrs = options.koPrintPosNrs;

  ko.koMarginWidth = std::max(ko.width, (int)(KO_NAME_WIDTH * printer->cW));
  if (ko.printPosNrs)
  {
    wxChar tmp[16];
    wxSprintf(tmp, "%d)", 2 * (ko.mt + ko.nrmt));
    ko.koMarginWidth += printer->TextWidth(tmp);
  }

  // Bei dem vollen Nationennamen verbreitert sich der Rand noch
  if ((cp.cpType != CP_TEAM && CTT32App::instance()->GetPrintAssociation() == FLAG_PRINT_ASSOC_DESC) ||
      (cp.cpType == CP_TEAM && CTT32App::instance()->GetPrintAssociationTeam() == FLAG_PRINT_ASSOC_DESC))
    ko.koMarginWidth += (ASSOCDESC_OFFST *printer->cW);

  // Abstand zweier Raster
  int distance = height - 2 * printer->cH;

  // Sonderregelung nur eine Runde
  if (gr.grNofRounds == 1)
  {
    SettingKO ko1 = ko;
    ko1.rd = 1;
    ko1.nrrd = 1;
    ko1.nrmt = 1;

    ko1.maxRound = 1;
    ko1.maxMatch = 1;

    int maxMatches = std::min(ko.nrmt, (int)(gr.grNofMatches ? gr.grNofMatches : gr.NofMatches(1)));

    for (ko1.mt = 1; ko1.mt <= maxMatches; ko1.mt++)
    {
      PrintRaster(ko1);
      offsetY += distance;
      ko1.startY = offsetY;
    }

    *pofstY = offsetY;
    *ppage = page;

    // Clean up
    printer->SelectFont(oldFont);
    printer->DeleteFont(textFont);
    printer->DeleteFont(nameFont);

    return 0;
  }

  PrintRaster(ko);

  // Das war der erste Streich, 
  // Bei mehr als 1 Runde das Finale wiederholen
  // Ausser, es gibt kein kleines Finale.
  if (gr.NofRounds() > 1 && !gr.grNoThirdPlace && ko.mt <= 2 && ko.mt + ko.nrmt >= 2)
  {
    SettingKO  ko1 = ko;
    ko1.startX = offsetX + ko.koMarginWidth + (ko.nrrd - 2) * width;
    ko1.startY = offsetY + distance;
    // Das Spiel kann bei 8-er Gruppe noch ein wenig hoeher
    if (CTT32App::instance()->GetPrintLogo() || CTT32App::instance()->GetPrintSponsor())
    {
      if (ko1.nrrd == 3)
        ko1.startY -= (ko1.nrrd - 3 + 0.5) * height;
      else if (ko1.nrrd == 4)
        ko1.startY -= (ko1.nrrd - 3 + 0.5) * height;
      else if (ko1.nrrd == 5)
        ko1.startY -= (ko1.nrrd - 3) * height;
    }
    ko1.printKOMargin = 0;
    ko1.mt = 2;
    ko1.nrmt = std::min(1, ko.mt + ko.nrmt - 2);
    ko1.rd = gr.NofRounds();
    ko1.nrrd = 1;

    // Platz 3 - 4: Drucken, wenn es mehr als 2 Spieler gibt.
    if (gr.grSize - nofByes > 2)
      PrintRaster(ko1);
  }

  // Bei mehr als 2 Runden jetzt ab Halbfinale wiederholen
  if (gr.NofRounds() > 2 && !gr.grOnlyThirdPlace && ko.mt <= 3 && ko.mt + ko.nrmt >= 3)
  {
    // Halbfinale
    SettingKO ko2 = ko;
    ko2.startX = offsetX + ko.koMarginWidth + (ko.nrrd - 3) * width;
    ko2.startY = offsetY + distance;
    ko2.printKOMargin = 0;
    ko2.mt = 3;
    ko2.nrmt = std::min(2, ko.mt + ko.nrmt - 3);
    ko2.rd = gr.NofRounds() - 1;
    ko2.nrrd = 2;

    // Platz 5 - 8: Drucken, wenn mehr als 4 Spieler
    if (gr.grSize - nofByes > 4)
      PrintRaster(ko2);

    // Finale
    ko2.startX += width;
    ko2.startY = offsetY + distance;
    ko2.mt = 4;
    ko2.nrmt = std::min(1, ko.mt + ko.nrmt - 4);
    ko2.rd = gr.NofRounds();
    ko2.nrrd = 1;

    // Platz 7 - 8: Drucken, wenn mehr als 6 Spieler
    if (gr.grSize - nofByes > 6)
      PrintRaster(ko2);
  }

  // Bei mehr als 3 Runden ab Viertelfinale wiederholen
  if (gr.NofRounds() > 3 && !gr.grOnlyThirdPlace && ko.mt <= 5 && ko.mt + ko.nrmt >= 5)
  {
    // Viertelfinale
    SettingKO ko2 = ko;
    ko2.startX = offsetX + ko.koMarginWidth + (ko.nrrd - 4) * width;
    ko2.startY = offsetY + distance;
    ko2.printKOMargin = 0;
    ko2.mt = 5;
    ko2.nrmt = std::min(4, ko.mt + ko.nrmt - 5);
    ko2.rd = gr.NofRounds() - 2;
    ko2.nrrd = 3;

    // Platz 9 - 16: Drucken, wenn mehr als 8 Spieler
    if (gr.grSize - nofByes > 8)
      PrintRaster(ko2);

    // Finale
    ko2.startX += 2 * width;
    ko2.startY = offsetY + distance;
    // Das Spiel kann bei 8-er Gruppe noch ein wenig hoeher
    if (CTT32App::instance()->GetPrintLogo() || CTT32App::instance()->GetPrintSponsor())
    {
      if (ko2.nrrd == 3)
        ko2.startY -= (ko2.nrrd - 3 + 0.5) * height;
      else if (ko2.nrrd == 4)
        ko2.startY -= (ko2.nrrd - 3 + 0.5) * height;
      else if (ko2.nrrd == 5)
        ko2.startY -= (ko2.nrrd - 3) * height;
    }
    ko2.mt = 6;
    ko2.nrmt = std::min(1, ko.mt + ko.nrmt - 6);
    ko2.rd = gr.NofRounds();
    ko2.nrrd = 1;

    // Platz 11 - 12: Drucken, wenn es mehr als 10 Spieler gibt.
    if (gr.grSize - nofByes > 10)
      PrintRaster(ko2);

    // Halbfinale
    ko2.startX -= width;
    ko2.startY = offsetY + distance;
    ko2.mt = 7;
    ko2.nrmt = std::min(2, ko.mt + ko.nrmt - 7);
    ko2.rd = gr.NofRounds() - 1;
    ko2.nrrd = 2;

    // Platz 13 - 16: Drucken, wenn es mehr als 12 Spieler gibt.
    if (gr.grSize - nofByes > 12)
      PrintRaster(ko2);

    // Finale
    ko2.startX += width;
    ko2.startY = offsetY + distance;
    ko2.mt = 8;
    ko2.nrmt = std::min(1, ko.mt + ko.nrmt - 8);
    ko2.rd = gr.NofRounds();
    ko2.nrrd = 1;

    // Platz 15 - 16: Drucken, wenn es mehr als 14 Spieler gibt.
    if (gr.grSize - nofByes > 14)
      PrintRaster(ko2);
  }

  // Bei mehr als 4 Runden ab Achtelfinale wiederholen
  if (gr.NofRounds() > 4 && !gr.grOnlyThirdPlace && ko.mt <= 9 && ko.mt + ko.nrmt >= 9)
  {
    SettingKO ko2 = ko;
    ko2.startX = offsetX + ko.koMarginWidth + (ko.nrrd - 5) * width;
    ko2.startY = offsetY + distance;
    ko2.printKOMargin = 0;
    ko2.mt = 9;
    ko2.nrmt = std::min(8, ko.mt + ko.nrmt - 9);
    ko2.rd = gr.NofRounds() - 3;
    ko2.nrrd = 4;

    // Platz 17 - 32: Drucken, wenn es mehr als 16 Spieler gibt.
    if (gr.grSize - nofByes > 16)
      PrintRaster(ko2);

    // Finale
    ko2.startX += 3 * width;
    ko2.startY = offsetY + distance;
    // Das Spiel kann bei 8-er Gruppe noch ein wenig hoeher
    if (CTT32App::instance()->GetPrintLogo() || CTT32App::instance()->GetPrintSponsor())
    {
      if (ko2.nrrd == 3)
        ko2.startY -= (ko2.nrrd - 3 + 0.5) * height;
      else if (ko2.nrrd == 4)
        ko2.startY -= (ko2.nrrd - 3 + 0.5) * height;
      else if (ko2.nrrd == 5)
        ko2.startY -= (ko2.nrrd - 3) * height;
    }
    ko2.mt = 10;
    ko2.nrmt = std::min(1, ko.mt + ko.nrmt - 10);
    ko2.rd = gr.NofRounds();
    ko2.nrrd = 1;

    // etc. ...
    if (gr.grSize - nofByes > 18)
      PrintRaster(ko2);

    // Halbfinale
    ko2.startX -= width;
    ko2.startY = offsetY + distance;
    ko2.mt = 11;
    ko2.nrmt = std::min(2, ko.mt + ko.nrmt - 11);
    ko2.rd = gr.NofRounds() - 1;
    ko2.nrrd = 2;

    if (gr.grSize - nofByes > 20)
      PrintRaster(ko2);

    // Finale
    ko2.startX += width;
    ko2.startY = offsetY + distance;
    // Das Spiel kann bei 8-er Gruppe noch ein wenig hoeher
    if (CTT32App::instance()->GetPrintLogo() || CTT32App::instance()->GetPrintSponsor())
    {
      if (ko2.nrrd == 3)
        ko2.startY -= (ko2.nrrd - 3 + 0.5) * height;
      else if (ko2.nrrd == 4)
        ko2.startY -= (ko2.nrrd - 3 + 0.5) * height;
      else if (ko2.nrrd == 5)
        ko2.startY -= (ko2.nrrd - 3) * height;
    }
    ko2.mt = 12;
    ko2.nrmt = std::min(1, ko.mt + ko.nrmt - 12);
    ko2.rd = gr.NofRounds();
    ko2.nrrd = 1;

    if (gr.grSize - nofByes > 22)
      PrintRaster(ko2);

    // Viertelfinale
    ko2.startX -= 2 * width;
    ko2.startY = offsetY + distance;
    ko2.mt = 13;
    ko2.nrmt = std::min(4, ko.mt + ko.nrmt - 13);
    ko2.rd = gr.NofRounds() - 2;
    ko2.nrrd = 3;

    if (gr.grSize - nofByes > 24)
      PrintRaster(ko2);

    // Finale
    ko2.startX += 2 * width;
    ko2.startY = offsetY + distance;
    ko2.mt = 14;
    ko2.nrmt = std::min(1, ko.mt + ko.nrmt - 14);
    ko2.rd = gr.NofRounds();
    ko2.nrrd = 1;

    if (gr.grSize - nofByes > 26)
      PrintRaster(ko2);

    // Halbfinale
    ko2.startX -= width;
    ko2.startY = offsetY + distance;
    ko2.mt = 15;
    ko2.nrmt = std::min(2, ko.mt + ko.nrmt - 15);
    ko2.rd = gr.NofRounds() - 1;
    ko2.nrrd = 2;

    if (gr.grSize - nofByes > 28)
      PrintRaster(ko2);

    // Finale
    ko2.startX += width;
    ko2.startY = offsetY + distance;
    ko2.mt = 16;
    ko2.nrmt = std::min(1, ko.mt + ko.nrmt - 16);
    ko2.rd = gr.NofRounds();
    ko2.nrrd = 1;

    if (gr.grSize - nofByes > 30)
      PrintRaster(ko2);
  }

  *pofstY = offsetY;
  *ppage = page;

  // Clean up
  printer->SelectFont(oldFont);
  printer->DeleteFont(textFont);
  printer->DeleteFont(nameFont);

  return 0;
}


