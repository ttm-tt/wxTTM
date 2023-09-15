/* Copyright (C) 2020 Christoph Theis */

// Basisklasse fuer KO- und RR-Raster

#include  "stdafx.h"
#include  "raster.h"

// #include  <tbsort.h>     // Sortieren der Spieler in Round Robin


#include  "TT32App.h"
#include  "CpListStore.h"
#include  "GrListStore.h"
#include  "MdListStore.h"
#include  "MtListStore.h"
#include  "MtEntryStore.h"
#include  "PlListStore.h"

#include  "SyListStore.h"

#include  "IdStore.h"

#include  "res.h"

#include  <stdlib.h>
#include  <io.h>

#include  <iomanip>
#include  <algorithm>

#undef _
#define _(a) printer->GetString(a)

#define  RES_UMPIRE  IDS_UMPIRE
#define  RES_WINNER  IDS_WINNER
#define  RES_BYE     IDS_BYE
#define  RES_GROUP   IDS_GROUP
#define  RES_SCHEDULE  IDS_SCHEDULE
#define  RES_SETS    IDS_GAMES
#define  RES_SET     IDS_GAME
#define  RES_RESULT  IDS_RESULT
#define  RES_MATCH   IDS_MATCH

// Basisklasse RasterBase

RasterBase::RasterBase(Printer *prt, Connection *ptr)
{
  if (ptr)
    connPtr = ptr;
  else
    connPtr = TTDbse::instance()->GetDefaultConnection();

  printer = prt;

  // Init von Variablen
  textFont = 0;
  space = 0;
  offsetX = 0;
  offsetY = 0;

  page = 0;

  nationNameWidth = CTT32App::instance()->GetPrintNationNameWidth();
  nationDescWidth = CTT32App::instance()->GetPrintNationDescWidth();
  nationRegionWidth = CTT32App::instance()->GetPrintNationRegionWidth();
  teamNameWidth = CTT32App::instance()->GetPrintTeamNameWidth();
  startNrWidth    = CTT32App::instance()->GetPrintStartNrWidth();

  // -1 means "auto"
  if (nationNameWidth == -1)
    nationNameWidth = NaStore(connPtr).GetMaxNameLength();

  if (nationDescWidth == -1)
    nationDescWidth = NaStore(connPtr).GetMaxDescLength();

  if (nationRegionWidth == -1)
    nationRegionWidth = NaStore(connPtr).GetMaxRegionLength();

  if (teamNameWidth == -1)
    teamNameWidth = TmStore(connPtr).GetMaxNameLength();

  if (startNrWidth == -1)
  {
    // Calculate digits in highest nr
    int highestNr = PlStore(connPtr).GetHighestNumber();
    int nr = 10;
    startNrWidth = 1;

    while (nr < highestNr)
    {
      nr *= 10;
      ++startNrWidth;
    }
  }
};


RasterBase::~RasterBase()
{
};


// -----------------------------------------------------------------------
// Initialisierung vom Raster
void  RasterBase::SetupGroup(const CpRec &cp_, const GrRec &gr_)
{
  stMap.clear();

  cp = cp_;
  gr = gr_;

  md = MdRec();

  if (gr.grModus == MOD_RR)
  {
    // Besser waere MdListStore, aber dann wird die Spielfolge
    // nicht gelesen. Wuerde aber erst relevant werden, wenn
    // man die Lese- / Schreibrechte auf der DB auf Views 
    // einschraenkt.
    MdStore  md_(connPtr);
    md_.SelectById(gr.mdID);
    if (md_.Next())
      md = md_;

    md_.Close();    
  }

  StEntryStore  st(connPtr);
  st.SelectByGr(gr, cp.cpType);
  while (st.Next())
    stMap.insert(StEntryMap::value_type(st.st.stID, st));
}


// Beginn einer neuen Seite mit Ueberschrift
void  RasterBase::NewPage(bool printPageNr)
{
  if (page > 0)
  {
    printer->EndPage();
  }

  printer->StartPage();

  page++;
  offsetY = 0;

  PrintBanner();

  short  tmFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_TINY);
  short  oldFont = printer->SelectFont(tmFont);

  // Datum und Zeit Rechts oben
  PrintPageTime();

  offsetY += printer->cH;

  // Im RR-Raster dann etwas Abstand waren
  // if (gr.grModus == MOD_RR)
  // offsetY += printer->TextHeight(wxT("m"), textFont);

  // Seitennummer
  if (printPageNr && page > 1)
    PrintPageNr();

  // WB und Gruppe
  PrintCaption();

  PrintLogo();
  PrintSponsor();

  printer->SelectFont(oldFont);
  printer->DeleteFont(tmFont);
}


void RasterBase::PrintBanner()
{
  wxImage banner;
  if (CTT32App::instance()->GetPrintBanner() && IdStore::GetBannerImage(banner))
  {
    short  pgFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_PAGE);
    short  oldFont = printer->SelectFont(pgFont);

    int imgWidth = printer->width;
    int imgHeight = 5 * printer->cH;

    wxPoint topLeft, bottomRight;

    // Center image
    int realWidth = (imgHeight / banner.GetHeight()) * banner.GetWidth();
    int realHeight = imgHeight;
    if (realWidth > printer->width)
    {
      realWidth = printer->width;
      realHeight = (imgWidth / banner.GetWidth()) * banner.GetHeight();
    }

    topLeft = wxPoint((printer->width - realWidth) / 2, offsetY);
    bottomRight = wxPoint((printer->width + realWidth) / 2, offsetY + realHeight);

    printer->DrawImage(topLeft, bottomRight, wxSize(realWidth, realHeight), banner);

    if (offsetY < bottomRight.y)
      offsetY = bottomRight.y + printer->cH;

    printer->SelectFont(oldFont);
    printer->DeleteFont(pgFont);
  }
}


void RasterBase::PrintLogo()
{
  bool combined = cp.cpType != CP_TEAM && gr.grModus == MOD_RR && options.rrCombined;

  wxImage logo;
  if (CTT32App::instance()->GetPrintLogo() && IdStore::GetLogoImage(logo))
  {
    int imgWidth = printer->width / 4;
    int imgHeight = offsetY;

    wxPoint topLeft, bottomRight;

    // Im combined scoresheet linksbuendig, sonst rechtsbuendig.
    // Bei linksbuendig unterhalb vom Datum drucken
    if (combined)
    {
      topLeft = wxPoint(0, printer->cH);
      bottomRight = wxPoint(-1, -1);
    }
    else
    {
      topLeft = wxPoint(-1, 0);
      bottomRight = wxPoint(printer->width, -1);
    }

    printer->DrawImage(topLeft, bottomRight, wxSize(imgWidth, imgHeight), logo);
     
    if (offsetY < bottomRight.y)
      offsetY = bottomRight.y;
  }
}
 

void RasterBase::PrintSponsor()
{
  wxImage sponsor;
  if (CTT32App::instance()->GetPrintSponsor() && IdStore::GetSponsorImage(sponsor))
  {
    int xFooterLogo = printer->width; 
    int yFooterLogo = 800; 

    wxPoint topLeft(-1, -1), bottomRight(printer->width, printer->height);
    printer->DrawImage(topLeft, bottomRight, wxSize(xFooterLogo, yFooterLogo), sponsor);
      
    printer->height -= (bottomRight.y - topLeft.y);
  }
}


// In die linke obere Ecke das Datum und die Zeit
void  RasterBase::PrintPageTime()
{
  short  tmFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_TINY);
  short  oldFont = printer->SelectFont(tmFont);

  wxChar  text[128];

  struct tm *tmp;
  time_t ct;

  time(&ct);
  tmp = localtime(&ct);
  wxStrftime(text, 127, "%#d.%#m.%y %H:%M", tmp);

  int  tmWidth = printer->TextWidth(text, tmFont);
  int  tmStart = 0; // printer->width - tmWidth;
  int  tmHeight = printer->TextHeight(text, tmFont);

  printer->Text(tmStart, offsetY + tmHeight, text);

  printer->SelectFont(oldFont);
  printer->DeleteFont(tmFont);
}


// Druckt auf offsetY die Seitenzahl linksbuendig
void  RasterBase::PrintPageNr()
{
  // Fonst war "Helv" 10pt
  short  pgFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_PAGE);
  short  oldFont = printer->SelectFont(pgFont);

  wxString text, strPage;
  strPage = _("Page");
  text = wxString::Format("%s %i", strPage.t_str(), page);

  int  pgWidth = printer->TextWidth(text, pgFont);
  int  pgStart = 0; // printer->width - pgWidth;
  int  pgHeight = printer->TextHeight(text, pgFont);

  printer->Text(pgStart, offsetY + pgHeight, text);

  printer->SelectFont(oldFont);
  printer->DeleteFont(pgFont);
}


void  RasterBase::PrintEntry(long stID, const CRect &reg, long flags)
{
  StEntry  st = GetTeam(stID);

  if (!st.IsBye())
    PrintEntry(st, reg, flags);
  else   // Freilos
    PrintStringCentered(_("Bye"), reg);
}


// Ausgabe eines Strings, linksbuendig mit space, vertikal zentriert
void  RasterBase::PrintString(const wxString &text, const CRect &reg, int fmt)
{
  if (!text)
    return;
    
  // Mehrzeilige Texte
  if (wxStrchr(text, wxT('\n')))
  {
    wxStringTokenizer tokens(text, "\n");

    int count = tokens.CountTokens();
        
    CRect tmpReg = reg;
    tmpReg.bottom = tmpReg.top + tmpReg.GetHeight() / count;
    
    for (int i = 0; i < count; i++)
    {
      PrintString(tokens.GetNextToken(), tmpReg, fmt);
      tmpReg.top = tmpReg.bottom;
      tmpReg.bottom = tmpReg.top + reg.GetHeight() / count;
    }  
        
    return;
  }
  
  // Platz testen
  if (reg.GetWidth() < printer->cW)
    return;

  CRect region = reg;
  region.bottom -= 2 * THICK_FRAME;
  region.top += 2 * THICK_FRAME;
  region.right -= 2 * THICK_FRAME - printer->cW / 10;
  region.left += 2 * THICK_FRAME + printer->cW / 10;

  // printer->Text besorgt Clipping
  printer->Text(region, text, fmt);
}


void  RasterBase::PrintStringCentered(const wxString &text, const CRect &reg)
{
  PrintString(text, reg, wxALIGN_CENTER);
}


void  RasterBase::PrintStringWrapped(const wxString &text, const CRect &reg, int offset)
{
  CRect rect = reg;
  
  rect.left += offset;
  
  if (text.IsEmpty())
    return;

  if (printer->TextWidth(text) < rect.GetWidth() - printer->cW)
  {
    PrintString(text, rect);
    return;
  }

  const wxChar *txt = text.t_str();
  const wxChar *ptr = wxStrrchr(txt, wxT(' '));

  if (!ptr)
  {
    PrintString(text, rect);
    return;
  }

  while (ptr && ptr > txt)
  {
    while (ptr > txt && ptr[-1] == ' ')
      ptr--;

    // Wenn der String nicht umgebrochen werden kann, drucken
    if (ptr == txt)
    {
      PrintString(txt, rect);
      return;
    }

    *const_cast<wxChar *>(ptr) = '\0';
    
    const wxChar *tmp2 = ptr + 1;
    while (*tmp2 && *tmp2 == ' ')
      ++tmp2;
    
    if (wxStrlen(tmp2) > 2 && printer->TextWidth(txt) < rect.GetWidth() - printer->cW)
      break;

    const wxChar *tmp = wxStrrchr(txt, wxT(' '));

    if (!tmp)
      break;

    *const_cast<wxChar *>(ptr) = ' ';
    ptr = tmp;
  }

  // txt ist jetzt ein String, der in das Feld past,
  // irgendwo hinter ptr beginnt der Rest

  PrintString(txt, rect);

  *const_cast<wxChar *>(ptr) = ' ';

  // Naechstes nicht-Blank suchen und von dort weitermachen
  while (*ptr && *ptr == ' ')
    ptr++;

  CRect  regTmp = reg;
  regTmp.top = reg.bottom;
  regTmp.bottom = regTmp.top + reg.GetHeight();

  // Rekursiv den naechsten Teil drucken
  PrintStringWrapped(ptr, regTmp, 0);
}


// Ausgabe eines int, rechtsbuendig mit space, vertikal zentriert
void  RasterBase::PrintInt(int nr, const CRect &reg, int fmt)
{
  wxChar  str[17];
  _itot(nr, str, 10);

  PrintString(str, reg, fmt);
}


// Ausgabe eines long
void  RasterBase::PrintLong(long nr, const CRect &reg, int fmt)
{
  wxChar  str[33];
  _itot(nr, str, 10);

  PrintString(str, reg, fmt);
}


// Zeit / Datumsausgabe
void  RasterBase::PrintDate(const timestamp &ts, const CRect &reg, int fmt)
{
  PrintDate(ts, reg, CTT32App::instance()->GetDateFormat(), fmt);
}


void RasterBase::PrintDate(const timestamp &ts, const CRect &reg, const wxString &dateFmtStr, int fmt)
{
  // Nichts drucken, wenn ts <null> ist
  if (ts.day == 0)
    return;

  tm  tmp;
  memset(&tmp, 0, sizeof(&tmp));
  tmp.tm_year = ts.year - 1900;
  tmp.tm_mon  = ts.month - 1;
  tmp.tm_mday = ts.day;
  tmp.tm_hour = ts.hour;
  tmp.tm_min  = ts.minute;
  tmp.tm_sec  = ts.second;
  tmp.tm_isdst = -1;

  wxString strDate = wxDateTime(tmp).Format(dateFmtStr);

  PrintString(strDate, reg, fmt);
}


void  RasterBase::PrintTime(const timestamp &ts, const CRect &reg, int fmt)
{
  PrintTime(ts, reg, CTT32App::instance()->GetTimeFormat(), fmt);
}


void RasterBase::PrintTime(const timestamp &ts, const CRect &reg, const wxString &timeFmtStr, int fmt)
{
  // Nichts drucken, wenn ts <null> ist oder keine Zeit angegeben ist
  if (ts.hour == 0 && ts.minute == 0)
    return;

  tm  tmp;
  memset(&tmp, 0, sizeof(tmp));
  tmp.tm_year = ts.year - 1900;
  tmp.tm_mon  = ts.month - 1;
  tmp.tm_mday = ts.day;
  tmp.tm_hour = ts.hour;
  tmp.tm_min  = ts.minute;
  tmp.tm_sec  = ts.second;
  tmp.tm_isdst = -1;

  wxString strTime = wxDateTime(tmp).Format(timeFmtStr);
  // wxString strTime = wxDateTime(ts.day, (wxDateTime::Month) (ts.month - 1), ts.year, ts.hour, ts.minute, ts.second, 0).Format(timeFmtStr);

  PrintString(strTime, reg, fmt);
}


// Ausgabe eines Ergebnisses xx : yy
void  RasterBase::PrintGame(int *sets, const CRect &reg, bool revers)
{
  if (!sets || !sets[0] && !sets[1])
    return;

  CRect reg1 = reg, reg2 = reg;
  CRect regSep = reg;
  wxChar  str[5];

  int  tW = printer->TextWidth(wxT(":"));

  reg1.right = reg1.left + reg1.GetWidth() / 2 - 0.5 * space - tW / 2;
  reg2.left = reg1.right + space + tW;
  // reg2.right = reg2.left + printer->TextWidth("99");

  regSep.left = reg1.right;
  regSep.right = reg2.left;

  PrintStringCentered(wxT(":"), regSep);
  PrintInt((revers ? sets[1] : sets[0]), reg1);

  wxSprintf(str, "%i", (revers ? sets[0] : sets[1]));
  PrintString(str, reg2);
  // PrintInt(sets[1], reg2);
}


void  RasterBase::PrintGame(short *sets, const CRect &reg, bool revers)
{
  if (!sets)
    return;

  int tmp[2];
  tmp[0] = sets[0];
  tmp[1] = sets[1];

  PrintGame(tmp, reg, revers);
}


void  RasterBase::PrintGame(const MtSet &mtSet, const CRect &reg, bool revers)
{
  int tmp[2];
  tmp[0] = mtSet.mtResA;
  tmp[1] = mtSet.mtResX;

  PrintGame(tmp, reg, revers);
}


// Ausgabe eines Spielers
void  RasterBase::PrintPlayer(const PlListRec &player, const CRect &reg, long flags)
{
  if (player.plNr == 0 && !*player.psName.psLast)
    return;

  // Startnr
  CRect  regNr = reg;
  if (STARTNR_WIDTH && !(flags & FLAG_PRINT_NOPLNR))
  {
    regNr.right = regNr.left + STARTNR_WIDTH * printer->cW + 3 * space;
    if (player.plNr)
      PrintInt(player.plNr % 10000, regNr);
  }
  else
    regNr.right = regNr.left;

  // Name
  CRect  regName = reg;
  regName.left = regNr.right;  // macht netto 2 space
  if (flags & FLAG_PRINT_NATION)
  {
    if (flags & FLAG_PRINT_NADESC)
      regName.right = reg.right - ASSOCDESC_WIDTH * printer->cW - 2 * space;
    else if (flags & FLAG_PRINT_NAREGION)
      regName.right = reg.right - ASSOCREGION_WIDTH * printer->cW - 2 * space;
    else
      regName.right = reg.right - ASSOCNAME_WIDTH * printer->cW - 2 * space;
  }
  else
    regName.right -= space;

  // Heuristisch: links + 2 space
  regName.left += 2 * space;

  wxChar txt[sizeof(player.psName.psLast) + sizeof(player.psName.psFirst) + 2];

  // Vornamen mit einbeziehen ?
  if (!*player.psName.psFirst || 
      wxStrlen(player.psName.psFirst) == 1 && isspace(*player.psName.psFirst))
    wxSprintf(txt, "%s", player.psName.psLast);
  else if (flags & FLAG_PRINT_FNAME)
    wxSprintf(txt, "%s, %s", player.psName.psLast, player.psName.psFirst);
  else
    wxSprintf(txt, "%c. %s", player.psName.psFirst[0], player.psName.psLast);

  PrintString(txt, regName);

  // Nation
  if (flags & FLAG_PRINT_NATION)
  {
    CRect  regNation = reg;
    regNation.left = regName.right + space;
    regNation.right -= space;
    if (flags & FLAG_PRINT_NADESC)
      PrintString(player.naDesc, regNation);
    else if (flags & FLAG_PRINT_NAREGION)
      PrintString(player.naRegion, regNation);
    else
      PrintString(player.naName, regNation);
  }
}


void  RasterBase::PrintPlayer(const TmPlayer &player, const CRect &reg, long flags)
{
  if (player.plNr == 0 && !*player.psName.psLast)
    return;

  // Startnr
  CRect  regNr = reg;
  if (STARTNR_WIDTH)
  {
    regNr.right = regNr.left + STARTNR_WIDTH * printer->cW + 3 * space;
    if (player.plNr)
      PrintInt(player.plNr % 10000, regNr);
  }
  else
    regNr.right = regNr.left;

  // Name
  CRect  regName = reg;
  regName.left = regNr.right;  // macht netto 2 space
  if (flags & FLAG_PRINT_NATION)
  {
    if (flags & FLAG_PRINT_NADESC)
      regName.right = reg.right - ASSOCDESC_WIDTH * printer->cW - 2 * space;
    else if (flags & FLAG_PRINT_NAREGION)
      regName.right = reg.right - ASSOCREGION_WIDTH * printer->cW - 2 * space;
    else
      regName.right = reg.right - ASSOCNAME_WIDTH * printer->cW - 2 * space;
  }
  else
    regName.right -= space;

  // Heuristisch: links + 2 space
  regName.left += 2 * space;

  wxChar txt[sizeof(player.psName.psLast) + sizeof(player.psName.psFirst) + 2];

  // Vornamen mit einbeziehen ?
  if (!*player.psName.psFirst || 
      wxStrlen(player.psName.psFirst) == 1 && isspace(*player.psName.psFirst))
    wxSprintf(txt, "%s", player.psName.psLast);
  else if (flags & FLAG_PRINT_FNAME)
    wxSprintf(txt, "%s, %s", player.psName.psLast, player.psName.psFirst);
  else
    wxSprintf(txt, "%c. %s", player.psName.psFirst[0], player.psName.psLast);

  PrintString(txt, regName);

  // Nation
  if (flags & FLAG_PRINT_NATION)
  {
    CRect  regNation = reg;
    regNation.left = regName.right + space;
    regNation.right -= space;
    if (flags & FLAG_PRINT_NADESC)
      PrintString(player.naDesc, regNation);
    else if (flags & FLAG_PRINT_NAREGION)
      PrintString(player.naRegion, regNation);
    else
      PrintString(player.naName, regNation);
  }
}


// Ausgabeeines Schiedsrichters
void  RasterBase::PrintUmpire(const UpRec &ump, const CRect &reg, long flags)
{
  if (ump.upNr == 0 && !*ump.psName.psLast)
    return;

  // Startnr
  CRect  regNr = reg;
  if (STARTNR_WIDTH && !(flags & FLAG_PRINT_NOPLNR))
  {
    regNr.right = regNr.left + STARTNR_WIDTH * printer->cW + 3 * space;
    if (ump.upNr)
      PrintInt(ump.upNr % 10000, regNr);
  }
  else
    regNr.right = regNr.left;

  // Name
  CRect  regName = reg;
  regName.left = regNr.right;  // macht netto 2 space
  if (flags & FLAG_PRINT_NATION)
  {
    if (flags & FLAG_PRINT_NADESC)
      regName.right = reg.right - ASSOCDESC_WIDTH * printer->cW - 2 * space;
    else if (flags & FLAG_PRINT_NAREGION)
      regName.right = reg.right - ASSOCREGION_WIDTH * printer->cW - 2 * space;
    else
      regName.right = reg.right - ASSOCNAME_WIDTH * printer->cW - 2 * space;
  }
  else
    regName.right -= space;

  // Heuristisch: links + 2 space
  regName.left += 2 * space;

  wxChar txt[sizeof(ump.psName.psLast) + sizeof(ump.psName.psFirst) + 2];

  // Vornamen mit einbeziehen ?
  if (!*ump.psName.psFirst || 
      wxStrlen(ump.psName.psFirst) == 1 && isspace(*ump.psName.psFirst))
    wxSprintf(txt, "%s", ump.psName.psLast);
  else if (flags & FLAG_PRINT_FNAME)
    wxSprintf(txt, "%s, %s", ump.psName.psLast, ump.psName.psFirst);
  else
    wxSprintf(txt, "%c. %s", ump.psName.psFirst[0], ump.psName.psLast);

  PrintString(txt, regName);

  // Nation
  if (flags & FLAG_PRINT_NATION)
  {
    CRect  regNation = reg;
    regNation.left = regName.right + space;
    regNation.right -= space;
    if (flags & FLAG_PRINT_NADESC)
      PrintString(ump.naDesc, regNation);
    else if (flags & FLAG_PRINT_NAREGION)
      PrintString(ump.naRegion, regNation);
    else
      PrintString(ump.naName, regNation);
  }
}


// Ausgabe einer Mannschaft
void  RasterBase::PrintTeam(const TmTeam &team, const CRect &reg, long flags)
{
  if (!*team.tmName)
    return;

  int xxx = ASSOCNAME_WIDTH;
  double yyy = ((double)printer->TextWidth("GER")) / printer->cW;

  // Kuerzel
  CRect  regName = reg;
  if (TEAMNAME_WIDTH)
  {
    regName.left += space;
    regName.right = regName.left + TEAMNAME_WIDTH * printer->cW + 2 * space;
  }
  else
    regName.right = regName.left;

  PrintString(team.tmName, regName);

  // Name
  CRect  regDesc = reg;
  regDesc.left = regName.right;  // macht netto 2 space

  if (flags & FLAG_PRINT_NATION)
  {
    if (flags & FLAG_PRINT_NADESC)
      regDesc.right = reg.right - ASSOCDESC_WIDTH * printer->cW - 2 * space;
    else if (flags & FLAG_PRINT_NAREGION)
      regDesc.right = reg.right - ASSOCREGION_WIDTH * printer->cW - 2 * space;
    else
      regDesc.right = reg.right - ASSOCNAME_WIDTH * printer->cW - 2 * space;
  }
  else
    regDesc.right -= space;

  // Heuristisch: links + 2 space
  regDesc.left += 2 * space;

  PrintString(team.tmDesc, regDesc);

  if (flags & FLAG_PRINT_NATION)
  {
    CRect  regNation = reg;
    regNation.left = regDesc.right + space;
    regNation.right -= space;

    if (flags & FLAG_PRINT_NAREGION)
      PrintString(team.naRegion, regNation);
    else if (flags & FLAG_PRINT_NADESC)
      PrintString(team.naDesc, regNation);
    else
      PrintString(team.naName, regNation);
  }
}


void  RasterBase::PrintGroup(const TmGroup &group, const CRect &reg, long flags)
{
  if (!*group.grName)
    return;

  // Beschreibung: Allen verfuegbaren Platz verwenden, einschliesslich Startnr und Verband
  CRect  regDesc = reg;

  // regDesc.right = reg.right - ASSOCNAME_WIDTH * printer->cW - 2 * space;
  regDesc.right = reg.right - 3 * printer->cW - 2 * space;

  // Heuristisch: links + 2 space
  regDesc.left += 2 * space;

  PrintString(group.grDesc, regDesc);

  // Position
  CRect  regPos = reg;
  regPos.left = regDesc.right + space;
  regPos.right -= space;
  PrintInt(group.grPos, regPos);
}


void  RasterBase::PrintEntry(const TmEntry &tm, const CRect &reg, long flags)
{
  // Hier geht's los:
  switch (tm.team.cpType)
  {
  case  CP_SINGLE :
    PrintPlayer(tm.team.pl, reg, flags);
    break;

  case  CP_DOUBLE :
  case  CP_MIXED :
    {
      // Aufteilen der Region in zwei Haelften
      CRect  regA = reg;
      CRect  regX = reg;

      if (flags & FLAG_PRINT_CONDENSED)
      {
        regA.right = regA.left + reg.GetWidth() / 2 - printer->cW;
        regX.left  = regX.right - reg.GetWidth() / 2 + printer->cW;
      }
      else
      {
        regA.bottom = regA.top + reg.GetHeight() / 2;
        regX.top = regX.bottom - reg.GetHeight() / 2;
      }

      PrintPlayer(tm.team.pl, regA, flags);
      PrintPlayer(tm.team.bd, regX, flags);

      if (flags & FLAG_PRINT_CONDENSED)
      {
        CRect regDash = reg;
        regDash.left = regA.right;
        regDash.right = regX.left;

        PrintStringCentered(wxT("/"), regDash);
      }

      break;
    }

  case  CP_TEAM :
    {
      PrintTeam(tm.team.tm, reg, flags);
      break;
    }

  case CP_GROUP :
    {
      PrintGroup(tm.team.gr, reg, flags);
      break;
    }
  }
}


void  RasterBase::PrintCaption()
{
  // Wettbewerb in 14pt Helv, Gruppe in 12pt Helv, Seite in 10pt Helv
  short  cpFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_COMP);
  // printer->CreateFont("Helv", 14);
  short  grFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_GROUP);
  // printer->CreateFont("Helv", 12);

  // Wettbewerb
  short  oldFont = printer->SelectFont(cpFont);

  int  cpWidth  = printer->TextWidth(cp.cpDesc, cpFont);
  int  cpHeight = printer->TextHeight(cp.cpDesc, cpFont);

  unsigned length = wxMax((unsigned) cpWidth, ((sizeof(cp.cpDesc) / sizeof(wxChar)) * printer->cW) / 2);

  int startX = (printer->width - offsetX - length) / 2;

  printer->Text(startX + length / 2 - cpWidth / 2, offsetY, cp.cpDesc);

  // Drunter eine dicke Linie, mindestens SIZEOF_CPDESC * cW / 2 lang
  printer->Line(wxPoint(startX, offsetY), wxPoint(startX + length, offsetY), THICK_FRAME);

  // neuer Offset fuer Gruppe
  offsetY += cpHeight;

  // Gruppe in 12pt Helv
  printer->SelectFont(grFont);

  int  grWidth = printer->TextWidth(gr.grDesc, grFont);
  int  grHeight = printer->TextHeight(gr.grDesc, grFont);

  printer->Text(startX + length / 2 - grWidth / 2, offsetY, gr.grDesc);

  offsetY += grHeight;
  // offsetY ist schon baseline und kann bleiben

  printer->SelectFont(oldFont);
  printer->DeleteFont(cpFont);
  printer->DeleteFont(grFont);
}



// Einzelne Spiele untereinander drucken
// Steht in Raster_Base, weil es auch von KO-Rastern verwendet werden kann

struct  CompareMatchFunctional
{
  bool  operator()(MtEntry *i1, MtEntry *i2) 
  {
    if (i1->mt.mtEvent.mtRound > i2->mt.mtEvent.mtRound)
      return true;
    else if (i1->mt.mtEvent.mtRound < i2->mt.mtEvent.mtRound)
      return false;
    else if (i1->mt.mtEvent.mtMatch < i2->mt.mtEvent.mtMatch)
      return true;
    else
      return false;
  }
}; 

void  RasterBase::PrintMatches(const CpRec &cp_, const GrRec &gr_, 
                               const PrintRasterOptions &options_,
                               int *pofstX, int *pofstY, int *ppage)
{
  if (printer->PrinterAborted())
    return;
    
  // SetupGroup(cp_, gr_);

  bool combined = cp_.cpType != CP_TEAM && gr_.grModus == MOD_RR && options_.rrCombined;

  cp = cp_;
  gr = gr_;

  options = options_;

  offsetX = *pofstX;
  offsetY = *pofstY;
  page    = *ppage;

  SyListStore  sy(connPtr);

  if (gr.syID)
  {
    if (!sy.SelectById(gr.syID) || !sy.Next())
      return;

    sy.Close();
  }

  if (gr.grSize <= 4)
    textFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_MEDIUM);
  else if (gr.grSize <= 8)
    textFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_MEDIUM);
  else
    textFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_SMALL);

  short  oldFont = printer->SelectFont(textFont);

  // Vorerst mal height festlegen, wird noch in PrintMatch korrigiert
  double height = 2 * printer->cH;

  offsetY += height;

  short lastPlayedRound = 0;

  if (gr.grModus == MOD_RR && options.rrLastResults || gr.grModus != MOD_RR && options.koLastResults)
  {
    MtStore::MtEvent mtEvent;
    memset(&mtEvent, 0, sizeof(MtStore::MtEvent));
    mtEvent.grID = gr.grID;
    lastPlayedRound = MtStore(connPtr).GetLastPlayedRound(mtEvent);
  }

  MtEntryStore  mt(connPtr);
  mt.SelectByGr(gr, 0, cp.cpType);

  std::vector<MtEntry *> mtList;

  if (!mt.Next())
    return;

  bool matchPlayed = false;

  do
  {
    matchPlayed = (mt.mt.mtResA || mt.mt.mtResX) || matchPlayed;
    mtList.push_back(new MtEntry(mt));
  } while (mt.Next());
  
  if (true && gr.grModus != MOD_RR && !options.koLastRounds)
    std::sort(mtList.begin(), mtList.end(), CompareMatchFunctional());
  
  // Eigentlich wurden Zeitplaene nur gedruckt, wenn auch Spiele fertig waren.
  // Warum auch immer. Soll aber nicht so sein, wer Zeitplaene haben will,
  // bekommt auch welche.

  // if ( !(matchPlayed || combined) )
  //   return;
  
  // Im KO hab ich den Zeitplan aber im Raster
  if (!matchPlayed && gr.grModus != MOD_RR)
    return;

  // Testen ob Ergebnis noch auf eine Seite passt
  // In Mannschaften zumindest ein kompletter Durchgang,
  int  neededSpace;

  // Nur eine Schaetzung bei Mannschaften, da die SyListe keine Auflistung
  // der Spiele hat. Alternativ durch SyMatch laufen.
  if (cp.cpType == CP_TEAM)
  {
    // Mindestens ein Spiel
    neededSpace = mt.mt.mtMatches + sy.syDoubles;
  }
  else
  {
    // Die ganze Gruppe
    neededSpace = gr.grSize;
    if (cp.cpType == CP_DOUBLE || cp.cpType == CP_MIXED)
      neededSpace *= 2;
  }

  if (combined)
    neededSpace += 2;

  neededSpace *= height;

  // Praeventiver Seitenumbruch.
  // if (offsetY + neededSpace > printer->height)
  // Quick fix fuer WTTC damit Footer Logo mit y=1470/3 nicht ueberdruckt wird
  // int yFooterLogo = 1470/3;
  // Fix fuer EVC2006
  int yFooterLogo = CTT32App::instance()->GetPrintSponsor() ? 1470/3 : 0;

  if (offsetY + neededSpace > printer->height - yFooterLogo)
  {
    NewPage();
    offsetY += height;
  }
  else if (gr.grModus != MOD_RR && offsetY > 2 * printer->height / 3)
  {
    NewPage();
    offsetY += height;
  }

  PrintMatchHeader((*mtList.begin())->mt);
  
  short lastRound = 0;

  for (std::vector<MtEntry *>::iterator it = mtList.begin();
    it != mtList.end(); it++)
  {
    MtEntry mt = *(*it);

    // Gleich loeschen, spart die Arbeit spaeter
    delete (*it);

    if (printer->PrinterAborted())
      break;

    // Im KO-Raster gibt es die Spielansetzung schon im Raster.
    // Ungespielte Spiele brauchen also nicht aufgefuehrt werden
    if ( gr.grModus != MOD_RR && !(mt.mt.mtResA || mt.mt.mtResX) )
      continue;

    // Die Runden und Spiele beachten
    if ( options.rrLastResults && (gr.grModus == MOD_RR) )
    {
      // Nach den letzten Spielen kommt der Zeitplan der naechsten Spiele
      if (mt.mt.mtEvent.mtRound < lastPlayedRound)
        continue;
    }
    
    if ( options.koLastResults && (gr.grModus != MOD_RR) )
    { 
      if (mt.mt.mtEvent.mtRound != lastPlayedRound)
        continue;
    }
    
    if ( options.koSlctRound && (gr.grModus != MOD_RR) )
    {
      if ( (mt.mt.mtEvent.mtRound < options.koFromRound) ||
        (mt.mt.mtEvent.mtRound > options.koToRound) )
        continue;
    }
    
    if ( options.koSlctMatch && (gr.grModus != MOD_RR) )
    {
      if ( (mt.mt.mtEvent.mtMatch < options.koFromMatch) ||
        (mt.mt.mtEvent.mtMatch > options.koToMatch) )
        continue;
    }
    
    if ( options.rrSlctRound && (gr.grModus == MOD_RR) )
    {
      if ( (mt.mt.mtEvent.mtRound < options.rrFromRound) ||
        (mt.mt.mtEvent.mtRound > options.rrToRound) )
        continue;
    }

    // Nur eine Schaetzung bei Mannschaften, da die SyListe keine Auflistung
    // der Spiele hat. Alternativ durch SyMatch laufen.
    if (cp.cpType == CP_TEAM)
      neededSpace = mt.mt.mtResA + mt.mt.mtResX + sy.syDoubles;
    else if (cp.cpType == CP_DOUBLE || cp.cpType == CP_MIXED)
      neededSpace = 2;
    else
      neededSpace = 1; 

    // In RR eine ganze Runde drucken. Ausser Team, dort nur ein Spiel
    if (cp.cpType != CP_TEAM && gr.grModus == MOD_RR && lastRound != mt.mt.mtEvent.mtRound)
      neededSpace *= gr.NofMatches(1);

    if (combined)
      neededSpace += 1;

    // Das ganze jetzt mal der Hoehe
    neededSpace *= height;

    // Zusaetzlicher Rundenabstand im RR-Raster, 
    // wenn nicht Combined Scoresheet
    if ( !combined && lastRound && lastRound != mt.mt.mtEvent.mtRound )
    {
      offsetY += 0.25 * height;
    }
        
    // Im KO-Raster eine Rundenueberschrift ueber die einzelnen 
    // Spiele drucken
    wxChar strRoundHeader[64];
    *strRoundHeader = 0;
    
    if ( gr.grModus != MOD_RR || 
         cp.cpType == CP_TEAM && options.rrTeamDetails)
    {
      short mtRound = mt.mt.mtEvent.mtRound;
      short mtMatch = mt.mt.mtEvent.mtMatch;
      
      if ( (gr.grModus != MOD_PLO || mtRound == 1) && 
           lastRound != mtRound )
      {
        wxString strRound = _("Round");
			  wxSprintf(strRoundHeader, "%i. %s", mtRound, strRound.t_str());
			}
			else if ( gr.grModus == MOD_PLO && 
			          ( (mtMatch - 1) % (1 << (gr.NofRounds() - mtRound)) ) == 0 )
			{
			  wxString strPos = _("Pos.");
			  
			  // Sprung der Endpositionen
			  short diff = ((gr.grSize / 2) >> (mtRound - 1));
			  wxSprintf(strRoundHeader, "%s %i - %i", strPos.t_str(), 
                  gr.grWinner + 2 * (mtMatch - 1),
							    gr.grWinner + 2 * (mtMatch + diff - 2) + 1 );			  							
			}
    }
    
    lastRound = mt.mt.mtEvent.mtRound;
    
    // Wenn ein Rundenheader gedruckt werden soll, 
    // den Platz berucksichtigen. Der Platz wird weiter unten verbraucht.
    if (*strRoundHeader)
      neededSpace += printer->cH + 0.75 * height;

    //if ( offsetY + neededSpace > printer->height)
    // Quick fix fuer WTTC damit Footer Logo mit y=1470/3 nicht ueberdruckt wird
    if ( offsetY + neededSpace > printer->height - yFooterLogo)
    {
      NewPage();
      offsetY += height;

      PrintMatchHeader(mt.mt);
    }    
    
    if (*strRoundHeader)
    {
      offsetY += 0.5 * height;
      CRect rect(0, offsetY, printer->width, offsetY + printer->cH);
      PrintStringCentered(strRoundHeader, rect);
    
      offsetY += printer->cH + 0.25 * height;
    }

    PrintMatch(mt);

    if ( cp.cpType == CP_TEAM && mt.mt.mtEvent.mtMS == 0 &&
        (mt.mt.mtResA || mt.mt.mtResX) && !mt.mt.mtWalkOverA && !mt.mt.mtWalkOverX &&
        ((gr.grModus == MOD_RR && options.rrTeamDetails) ||
        (gr.grModus != MOD_RR && options.koTeamDetails)) )
    {
      PrintMatchDetails(mt);
      offsetY += 0.1 * height;
    }
  };

  // Oldies: In RR-Spielen auch noch Participate Consolation
  if (combined && options.rrConsolation)
  {
    offsetY += (2 * height) / 3;

    CRect  regParticipate;
    regParticipate.left  = offsetX;
    regParticipate.top   = offsetY;
    regParticipate.right = printer->width;
    regParticipate.bottom = regParticipate.top + 4 * height;

    CRect  regPartHeader = regParticipate;
    CRect  regPartThird  = regParticipate;
    CRect  regPartForth  = regParticipate;

    regPartHeader.bottom = regPartHeader.top + height;
    regPartThird.top     = regPartHeader.bottom;
    regPartThird.bottom  = regPartThird.top + (3 * height) / 2;
    regPartForth.top     = regPartThird.bottom;

    // Rahmen um alles
    printer->Rectangle(regParticipate, THICK_FRAME);

    // Trennlinie Header / Dritter
    printer->Line(regPartHeader.left, regPartHeader.bottom,
      regPartHeader.right, regPartHeader.bottom);
    // Trennlinie Dritter / Vierter
    printer->Line(regPartThird.left, regPartThird.bottom,
      regPartThird.right, regPartThird.bottom);

    // Vertikale Trennlinien berechnen
    wxChar  strThird[32], strForth[32];
    wxString  strPlace = _("Place");
    wxSprintf(strThird, "%s 3", strPlace.t_str());
    wxSprintf(strForth, "%s 4", strPlace.t_str());

    int  placeWidth = printer->TextWidth(strThird) + 2 * printer->cW;
    printer->Line(regParticipate.left + placeWidth, regParticipate.top,
      regParticipate.left + placeWidth, regParticipate.bottom);

    // regPartX fuer die Texte verkuerzen
    regPartThird.right = regPartThird.left + placeWidth;
    regPartForth.right = regPartThird.right;

    // Texte
    PrintString(strThird, regPartThird);
    PrintString(strForth, regPartForth);

#if 1
    // Header verkuerzen
    regPartHeader.left = regPartThird.right;
    regPartHeader.right = regPartHeader.left + regPartHeader.GetWidth() / 2;

    // Trennlinie
    printer->Line(regPartHeader.right, regParticipate.top,
      regPartHeader.right, regParticipate.bottom);

    // YES-Text
    PrintStringCentered(_("YES, I want to play Consolation"), regPartHeader);

    // NO-Text
    regPartHeader.left = regPartHeader.right;
    regPartHeader.right = regParticipate.right;
#endif
    PrintStringCentered(_("NO, I don't want to play Consolation"), regPartHeader);

    // offset weiterschreiben
    offsetY = regParticipate.bottom;
  }

  *pofstX = offsetX;
  *pofstY = offsetY;
  *ppage  = page;

  printer->SelectFont(oldFont);
  printer->DeleteFont(textFont);
}


// Das Ergebnis fuer ein einzelnes Match drucken (hier zum selber ausfuellen)
// Aufbau
/*
+-------------------------------------------------------------------+
| 123456 || Team A - Team X || 21 : 12 | 12 : 21 | 21 : 12 || 2 : 1 |
+-------------------------------------------------------------------+
| SchiRi                    | Sieger                                |
+-------------------------------------------------------------------+
*/

// Das Ergebnis fuer ein einzelnes Match drucken (hier zum selber ausfuellen)
// Kopf des Score-Sheets (rechtsbuendig)

/*
+-----+------+-------+
Score Sheet             | Tag | Zeit | Tisch |
------------------------------ +-----+------+-------+
*/

// No.              Games                     Result
void  RasterBase::PrintMatchHeader(const MtRec &mt)
{
  bool  combined = cp.cpType != CP_TEAM && gr.grModus == MOD_RR && options.rrCombined;

  short smallFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_SMALL);
  
  // Normale Schrift
  short  oldFont = printer->SelectFont(smallFont);  

  wxString  str;
  wxChar  text[32];
  int   left, right;

  // Spielnr (abgekuerzt Spiel)
  wxString strMatch = _("Match");
  left = 0.2 * printer->cW;
  wxSprintf(text, strMatch.t_str());

  printer->Text(left, offsetY, text);

  // short nofMatches = (cp.cpType == CP_TEAM ? mt.mtMatches : mt.mtBestOf);
  double setWidth = printer->width / (2 * (mt.mtBestOf + 1));

  // Schedule / Games zentriert ausgeben
  if (!combined)
  {
    left = printer->width / 2;
    right = left + mt.mtBestOf * setWidth;

    // Muss es so machen und nicht per sprintf, da infoSystem einen
    // Pointer auf 'ne statische Variable zurueckgibt.
    str = _("Schedule");
    wxStrcpy(text, str.t_str());
    wxStrcat(text, wxT(" / "));
    str = _("Games");
    wxStrcat(text, str.t_str());

    printer->Text((left + right - printer->TextWidth(text)) / 2, offsetY, text);
  }
  else
  {
    str = _("Game");

    // Die einzelnen Spiele
    for (int i = 0; i < mt.mtBestOf; i++)
    {
      left = printer->width - (mt.mtBestOf - i + 1) * setWidth;
      left += 0.2 * printer->cW;
      wxSprintf(text, "%s %1i", str.t_str(), i+1);

      printer->Text(left + space, offsetY, text);
    }
  }

  str = _("Result");

  left = printer->width - setWidth;
  left += 0.2 * printer->cW;

  printer->Text(left + space, offsetY, str);

  offsetY += 0.25 * printer->cH;

  printer->SelectFont(oldFont);
  
  printer->DeleteFont(smallFont);
}


// Das Ergebnis fuer ein einzelnes Match drucken (hier zum selber ausfuellen)
// Aufbau
/*
+-------------------------------------------------------------------+
| 123456 || Team A - Team X || 21 : 12 | 12 : 21 | 21 : 12 || 2 : 1 |
+-------------------------------------------------------------------+
| SchiRi                    | Sieger                                |
+-------------------------------------------------------------------+
*/

void  RasterBase::PrintMatch(const MtEntry &mtEntry)
{
  if (printer->PrinterAborted())
    return;

  // Abkuerzungen
  const MtListRec &mt  = mtEntry.mt;
  const TmEntry   &tmA = mtEntry.tmA;
  const TmEntry   &tmX = mtEntry.tmX;

  bool teamDetails = gr.grModus == MOD_RR && options.rrTeamDetails || gr.grModus != MOD_RR && options.koTeamDetails;
  bool ignoreByes  = gr.grModus == MOD_RR && options.rrIgnoreByes  || gr.grModus != MOD_RR && options.koIgnoreByes;
  bool combined    = cp.cpType != CP_TEAM && gr.grModus == MOD_RR && options.rrCombined;

  // Top-Levelspiele nur beruecksichtigen, wenn beide Teams bekannt sind
  if (!ignoreByes && (!tmA.tmID && tmA.team.cpType != CP_GROUP || !tmX.tmID && tmX.team.cpType != CP_GROUP))
    return;

  // Individual matches werden kleiner gedruckt
  short  smallFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_SMALL);
  short  boldFont;
  short  oldFont;

  if (gr.grSize <= 4)
    boldFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_MEDIUMB);
  else if (gr.grSize <= 8)
    boldFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_MEDIUMB);
  else
    boldFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_SMALLB);

  // Nur drucken, wenn kein Bye dabei ist
  // StEntry sta = GetTeamA(mt);
  // StEntry stx = GetTeamX(mt);

  oldFont = printer->SelectFont(textFont);
  if (mt.cpType == CP_INDIVIDUAL)
  {
    oldFont = printer->SelectFont(smallFont);
  }
  else if ( mt.cpType == CP_TEAM && teamDetails )
  {
    oldFont = printer->SelectFont(boldFont);
  }
  else
  {
    oldFont = printer->SelectFont(textFont);
  }

  // Aufsetzen der einzelnen Bestandteile
  CRect  reg;           // Alles
  CRect  regMtNr;       // Match Nummer (Match Nr ...)
  CRect  regTm;         // Teams (1 - 2)
  CRect  regSet;        // Saetze 1 - 3
  CRect  regSets;       // Ergebnis der Saetze
  CRect  regWinner;     // Sieger der Partie
  CRect  regSchiri;     // SchiRi

  wxChar  str[32];        // Allgemeiner Ausgabestring

  double height   = 1.5 * (double) printer->cH;
  if (combined)
    height *= 2;
  else if ( mt.cpType == CP_DOUBLE || mt.cpType == CP_MIXED || 
           (mt.cpType == CP_INDIVIDUAL && tmA.team.cpType == CP_DOUBLE))
    height *= 2;
  double setWidth = printer->width / (2 * (mt.mtBestOf + 1));

  // Passt das Raster noch auf eine Seite?
  if (combined)
  {
    if ((offsetY + 2 * height) > printer->height)
    {
      NewPage( 0);
      offsetY += height;
    }
  }
  else
  {
    if ((offsetY + height) > printer->height)
    {
      NewPage( 0);
      offsetY += height;
    }
  }

  int top = offsetY;
  int left = offsetX;

  reg.top = top;
  reg.left = left;
  reg.right = printer->width;
  if (combined)
    reg.bottom = top + 2 * height;
  else
    reg.bottom = top + 1 * height;  // 2 * height

  printer->Rectangle(reg, THICK_FRAME);

  // Umpire / Winner abtrennen
  if (combined)
    printer->Line(reg.left, reg.top + height, reg.right, reg.top + height);

  // Matchnummer
  regMtNr = reg;
  regMtNr.bottom = regMtNr.top + height;
  if (combined)
    regMtNr.right = regMtNr.left + height;
  else
    regMtNr.right = regMtNr.left;

  if (combined)
  {
    printer->Line(regMtNr.right, regMtNr.top, regMtNr.right, regMtNr.bottom);

    regMtNr.right -= printer->cW;
    PrintLong(mt.mtNr, regMtNr);
    regMtNr.right += printer->cW;
  }

  // Spieler
  regTm = reg;
  regTm.left   = regMtNr.right;
  regTm.right  = printer->width / 2;
  regTm.bottom = regMtNr.bottom;

  printer->Line(regTm.right, regTm.top, regTm.right, regTm.bottom, THICK_FRAME);

  // Zwei Region fuer die Spieler
  CRect  regTmA = regTm;
  CRect  regTmX = regTm;
  CRect  regDash = regTm;

  regTmA.right = regTmA.left + (regTmA.right - regTmA.left) / 2 - printer->cW;
  regTmX.left = regTmA.right + 2 * printer->cW;

  regDash.left = regTmA.right;
  regDash.right = regTmX.left;

  PrintStringCentered(wxT("-"), regDash);

  // Print Teams
  // In den Einzelspielen die Mannschaften wieder zurueckdrehen
  // Dann steht zwar u.U. X links und A rechts, aber ich muss nicht soviel
  // Ergebnisse vertauschen, und die Mannschaft selbst steht genauso oben
  if (mt.cpType == CP_INDIVIDUAL)
  {
    // PrintEntry(tmA, mt.mtReverse ? regTmX : regTmA, 0);
    // PrintEntry(tmX, mt.mtReverse ? regTmA : regTmX, 0);
    // MtIndividualList liefert die Spalten wie die Aufstellung
    PrintEntry(mt.mtReverse ? tmX : tmA, regTmA, FLAG_PRINT_FNAME);
    PrintEntry(mt.mtReverse ? tmA : tmX, regTmX, FLAG_PRINT_FNAME);
  }
  else
  {
    StEntry stA = GetTeamA(mt);
    StEntry stX = GetTeamX(mt);
    
    if (tmA.tmID == 0 && tmA.team.cpType != CP_GROUP && 
        gr.grModus == MOD_RR && stA.st.stPos != 0)
    {
      wxChar tmp[16];
      wxSprintf(tmp, "Pos. %d", stA.st.stPos);
      
      regTmA.left += printer->cW;
      PrintString(tmp, regTmA);
      regTmA.left -= printer->cW;
    }
    else
    {
      PrintEntry(mt.mtReverse ? tmX : tmA, regTmA, 0);
    }
    
    if (tmX.tmID == 0 && tmX.team.cpType != CP_GROUP &&
        gr.grModus == MOD_RR && stX.st.stPos != 0)
    {
      wxChar tmp[16];
      wxSprintf(tmp, "Pos. %d", stX.st.stPos);
      
      regTmX.left += printer->cW;
      PrintString(tmp, regTmX);
      regTmX.left -= printer->cW;
    }
    else
    {
      PrintEntry(mt.mtReverse ? tmA : tmX, regTmX, 0);
    }
  }

  // 3 / 5 Saetze oder Schedule
  // Ob ich den Zeitplan hier drucke, haengt ab, ob der schon
  // oben gedruckt wurde.
  if ( combined || mt.mtResA || mt.mtResX || mt.mtWalkOverA || mt.mtWalkOverX )
  {
    // Mannschaftsspiele drucken prinzipiell keine Saetze
    regSet = regTm;

    // TODO: MtSetStore auch direkt von einem MtRec initialisieren
    MtStore  tmp(connPtr);
    tmp.MtRec::operator=(mt);

    if (mt.cpType != CP_TEAM)
    {
      if (mt.mtWalkOverA || mt.mtWalkOverX)
      {
        regSet.left = regTm.right;
        regSet.right = regSet.left + mt.mtBestOf * setWidth;
        printer->Text(regSet, wxT("w/o"), wxALIGN_CENTER);
      }
      else if (mt.mtInjuredA || mt.mtInjuredX)
      {
        regSet.left = regTm.right;
        regSet.right = regSet.left + mt.mtBestOf * setWidth;
        printer->Text(regSet, wxT("Injured"), wxALIGN_CENTER);
      }
      else if (mt.mtDisqualifiedA || mt.mtDisqualifiedX)
      {
        regSet.left = regTm.right;
        regSet.right = regSet.left + mt.mtBestOf * setWidth;
        printer->Text(regSet, wxT("Disqualified"), wxALIGN_CENTER);
      }
      else if (mt.mtResA || mt.mtResX)
      {
        MtSetStore  mtSet(tmp, connPtr);
        mtSet.SelectAll(mtEntry.mt.mtEvent.mtMS);

        while (mtSet.Next())
        {
          // Satz Nr 0 enthaelt die Summe: Wird nicht gebraucht
          if (mtSet.mtSet == 0)
            continue;
          // Mannschaftsspiele kennen keine Saetze, aber ich kann auf
          // diese Art die Position berechnen
          short pos = mtSet.mtSet;
          regSet.left = regTm.right + (pos - 1) * setWidth;
          regSet.right = regTm.right + pos * setWidth;

          // Senkrechte Absperrung
          printer->Line(regSet.right, regSet.top, regSet.right, regSet.bottom);

          // Ergebnis
          if (mtSet.mtResA || mtSet.mtResX)
            PrintGame(mtSet, regSet, mt.mtReverse != 0); 
        }        
      }
      else
      {
        for (short pos = 1; pos <= mt.mtBestOf; pos++)
        {
          regSet.right = regTm.right + pos * setWidth;

          // Senkrechte Absperrung
          printer->Line(regSet.right, regSet.top, regSet.right, regSet.bottom);
        }
      }
    }
    else
    {
      if (mt.mtResA || mt.mtResX || mt.mtWalkOverA || mt.mtWalkOverX)
      {
        if ( gr.grModus == MOD_RR && options.rrTeamDetails ||
             gr.grModus != MOD_RR && options.koTeamDetails )
        {
          // Einzelergebisse
          regSet.right = regTm.right + mt.mtBestOf * setWidth;
          regSet.left  = regSet.right - setWidth;

          printer->Line(regSet.right, regSet.top, regSet.right, regSet.bottom);

          // Wenn w/o im Feld fuer die Saetze "w/o" drucken
          if (mt.mtWalkOverA || mt.mtWalkOverX)
          {
            CRect regWO = regSet;
            regWO.left = regTm.right;
            PrintStringCentered(_("w/o"), regWO);
          }
          else if (mt.mtInjuredA || mt.mtInjuredX)
          {
            CRect regWO = regSet;
            regWO.left = regTm.right;
            PrintStringCentered(_("Injured"), regWO);
          }
          else if (mt.mtDisqualifiedA || mt.mtDisqualifiedX)
          {
            CRect regWO = regSet;
            regWO.left = regTm.right;
            PrintStringCentered(_("Disqualified"), regWO);
          }
        }
      }
      else
      {
        for (short pos = 1; pos <= mt.mtBestOf; pos++)
        {
          regSet.left = regTm.right + (pos - 1) * setWidth;
          regSet.right = regTm.right + pos * setWidth;

          // Senkrechte Absperrung
          printer->Line(regSet.right, regSet.top, regSet.right, regSet.bottom);
        }
      }
    }

    // Saetze
    regSets = regTm;
    regSets.left = regTm.right + mt.mtBestOf * setWidth;
    regSets.right = reg.right;

    // Eine dicke Linie als Abgrenzung der Saetze von oben
    printer->Line(regSets.left, regSets.top, regSets.left, regSets.bottom, THICK_FRAME);

    if (mt.mtResA || mt.mtResX)
    {
      short res[2];
      res[0] = mt.mtResA;
      res[1] = mt.mtResX;
      PrintGame(res, regSets, mt.mtReverse != 0);
    }

    // Und eine Linie dazwischen
    printer->Line(regTmA.right, regTmA.bottom, regSets.right, regTmA.bottom);
  }
  else
  {
    CRect  regSch;
    regSch.left = regTm.right + 0.25 * setWidth;
    regSch.right = regSch.left + 0.75 * setWidth;
    regSch.top = regTm.top;
    regSch.bottom = regTm.bottom;

    if (mt.mtPlace.mtTable)
    {
      wxSprintf(str, "T %i", mt.mtPlace.mtTable);
      PrintString(str, regSch);
    }

    regSch.left = regSch.right;
    regSch.right = regSch.left + 2 * setWidth;

    PrintDate(mt.mtPlace.mtDateTime, regSch);

    regSch.left = regSch.right;
    regSch.right = regSch.left + 2 * setWidth;

    PrintTime(mt.mtPlace.mtDateTime, regSch);

    // Aber noch die Linie zeichnen
    printer->Line(regTm.right + (mt.mtBestOf + 1) * setWidth, regTm.top,
      regTm.right + (mt.mtBestOf + 1) * setWidth, regTm.bottom);
  }

  if (combined)
  {
    // SchiRi
    regSchiri.top = regTmX.bottom;
    regSchiri.bottom = reg.bottom;
    regSchiri.left = regMtNr.left;
    regSchiri.right = regTm.right;

    // Abtrennung
    printer->Line(regTm.right, regSchiri.top, regTm.right, regSchiri.bottom, THICK_FRAME);

    // SchiRi
    regSchiri.left += printer->cW;
    PrintString(_("Umpire"), regSchiri);
    regSchiri.left -= printer->cW;

    // Gibt es den SchiRi
    long umpire = GetUmpire(mt);
    if (umpire != 0)   // Ein SchiRi wurde angegeben
    {
      short oldFont = printer->SelectFont(textFont);
      PlListStore  pl(connPtr);
      pl.SelectByNr(umpire);
      pl.Next();

      if (pl.plNr)
      {
        regSchiri.left  = regSchiri.left + 15 * width;
        regSchiri.right = regTm.right;

        regSchiri.left += printer->cW;
        PrintPlayer(pl, regSchiri, 0);
        regSchiri.left -= printer->cW;
      }

      printer->SelectFont(oldFont);
    }

    // Sieger
    regWinner.top = regSchiri.top;
    regWinner.bottom = regSchiri.bottom;
    regWinner.left = regTm.right;
    regWinner.right = reg.right;

    // Unterschrift
    regWinner.left += printer->cW;
    PrintString(_("Winner"), regWinner);
    regWinner.left -= printer->cW;
    
    if (mt.QryWinnerAX())
    {
      short oldFont = printer->SelectFont(textFont);
  
      regWinner.left = regWinner.left + 15 * width;
      if (mt.QryWinnerAX() == +1)
        PrintEntry(tmA, regWinner, 0);
      else if (mt.QryWinnerAX() == -1)
        PrintEntry(tmX, regWinner, 0);
        
      printer->SelectFont(oldFont);    
    }
  }

  offsetY = reg.bottom;

  if (combined)
  {
    // Abstand zum naechsten Spiel hier, da das naechste Freilose haben koennte
    offsetY += 0.125 * height;
  }

  printer->SelectFont(oldFont);
  printer->DeleteFont(smallFont);
  printer->DeleteFont(boldFont);
}


void  RasterBase::PrintMatchDetails(const MtEntry &mtEntry)
{
  MtEntryStore mtmt(connPtr);
  std::vector<MtEntry *> mtList;

  mtmt.SelectByMS(mtEntry.mt);

  if (!mtmt.Next())
    return;

  do
  {
    mtList.push_back(new MtEntry(mtmt));
  } while (mtmt.Next());

  for (std::vector<MtEntry *>::iterator it = mtList.begin();
    it != mtList.end(); it++)
  {
    if ( (*it)->mt.mtResA > 0 || (*it)->mt.mtResX > 0)
      PrintMatch(*(*it));
    delete (*it);
  }

  mtList.clear();
}


// -----------------------------------------------------------------------
void RasterBase::PrintNotes(GrListStore &gr, const PrintRasterOptions &, int *pofstX, int *pofstY, int *ppage)
{
  int offsetY = *pofstY;

  if (gr.grHasNotes)
  {
    // Remove all comments: anything starting with '#'
    static wxRegEx privateNotes("#[^\n]*[\n]?");

    wxString note = gr.GetNote();

    privateNotes.Replace(&note, wxEmptyString);

    if (note.Trim().IsEmpty())
      return;

    wxStringTokenizer noteLines(note, "\r\n");
    short lines = noteLines.CountTokens();
    short textFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_SMALL);
    short boldFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_SMALLB);
    short oldFont = printer->SelectFont(textFont);
    if (printer->height - offsetY > (lines + 2) * printer->cH)
    {
      while (noteLines.HasMoreTokens())
      {
        wxString text = noteLines.GetNextToken();
        if (text.StartsWith("*"))
        {
          printer->SelectFont(boldFont);
          printer->Text(0, offsetY + printer->cH, text.Mid(1).Trim());
          printer->SelectFont(textFont);
        }
        else
        {
          printer->Text(0, offsetY + printer->cH, text);
        }

        offsetY += printer->cH;
      }
      // Darunter noch eine Zeile Abstand
      offsetY += printer->cH;
    }
    else if (gr.grModus == MOD_SKO)
    {
      // Rechts muesste noch Platz sein: am Seitenende anfangen
      offsetY = printer->height - (lines - 1) * printer->cH;

      while (noteLines.HasMoreTokens())
      {
        wxString text = noteLines.GetNextToken();
        if (text.StartsWith("*"))
        {
          printer->SelectFont(boldFont);
          printer->Text(printer->width - printer->width / 3, offsetY + printer->cH, text.Mid(1).Trim());
          printer->SelectFont(textFont);
        }
        else
        {
          printer->Text(printer->width - printer->width / 3, offsetY + printer->cH, text);
        }
      }

      // Und mehr Platz ist jetzt nicht mehr
      offsetY = printer->height;
    }

    printer->SelectFont(oldFont);
    printer->DeleteFont(textFont);
    printer->DeleteFont(boldFont);
  }

  *pofstY = offsetY;
}

// -----------------------------------------------------------------------
StEntry  RasterBase::GetTeamA(const MtRec &mt)
{
  return GetTeam(mt.stA);
}


StEntry  RasterBase::GetTeamX(const MtRec &mt)
{
  return GetTeam(mt.stX);
}


StEntry  RasterBase::GetTeam(long stID)
{
  StEntryMap::iterator it = stMap.find(stID);
  if (it == stMap.end())
    return StEntry();
  else
    return (*it).second;
}


StEntry  RasterBase::GetTeamByNr(short stNr)
{
  for (StEntryMap::iterator it = stMap.begin(); it != stMap.end(); it++)
  {
    if ( (*it).second.st.stNr == stNr)
      return (*it).second;
  }

  return StEntry();
}


long  RasterBase::GetUmpire(const MtRec &mt)
{
  // Ueberhaupt keiner angegeben
  if (mt.mtUmpire == 0)
    return 0;

  // Macht keinen Sinn fuer Mannschaften
  if (cp.cpType == CP_TEAM)
    return 0;

  // Wenn ein Spieler angegeben wurde, diesen nehmen
  if (mt.mtUmpire > 0)
    return mt.mtUmpire;

  // Derzeit nur RR-Spiele
  if (gr.grModus != MOD_RR)
    return 0;

  // Keine Moeglichkeit fuer 2-er Gruppen  
  if (md.mdSize == 2)
    return 0;

  // Freilos? dann auf 0 setzen
  if (GetTeamA(mt).tmID == 0 || GetTeamX(mt).tmID == 0)
    return 0;

  // Wer spielt= wichtig, dass SR nicht spielt!
  short match = mt.mtEvent.mtMatch;
  short round = mt.mtEvent.mtRound;

  short  playerA = md.GetPlayerA(round, match);
  short  playerX = md.GetPlayerX(round, match);

  short  srStart = GetUmpireCandidate(mt);
  if (srStart == 0)
    return 0;

  // srStart enhaelt die Anzahl vorhergehender Einsaetze
  int count = ((srStart & 0xFF00) >> 8);
  srStart &= 0xFF;

#if 0
  StEntry  stUmpire;

  // Von hier ab einen existierenden Spieler suchen
  // Das i ist noetig, damit die Schleife abbricht
  for (short i = 0; i < md.mdSize; i++)
  {
    // Start ist der Spieler, 
    // Position vom Kandidaten in Gruppe
    short srPos = (srStart + i - 1) % md.mdSize + 1;

    // Kandidaten, die spielen, fallen raus
    if (srPos == playerA || srPos == playerX)
      continue;

    // Freilose fallen raus
    stUmpire = GetTeamByNr(srPos);
    if (stUmpire.tmID == 0)
      continue;

    // Kandidat gefunden
    break;
  }

  if (!stUmpire.tmID)
    return 0;
#else
  StEntry stUmpire = GetTeamByNr(srStart);
#endif

  if (cp.cpType == CP_DOUBLE || cp.cpType == CP_MIXED)
  {
    if ((count % 2) == 0)
      return stUmpire.team.bd.plNr;
    else
      return stUmpire.team.pl.plNr;
  }
  else
    return stUmpire.team.pl.plNr;   
}


short  RasterBase::GetUmpireCandidate(const MtRec &mt)
{
  if (md.mdSize == 2)
    return 0;

  int match = mt.mtEvent.mtMatch;
  int round = mt.mtEvent.mtRound;

  int  playerA = md.GetPlayerA(round, match);
  int  playerX = md.GetPlayerX(round, match);
  int  mdSize  = md.mdSize;

  // Sind es gerade 3 Spieler?
  if (mdSize == 3)
  {
    for (short i = 1; i < 4; i++)
      if (i != playerA && i != playerX)
        return i;

    return 0;
  }

#if 0
  // Wer ist ein Kandidat
  int  srA, srX;

  if (match == 1 && round == 1)
  {
    // Fuer erstes Spiel aus naechstem Spiel
    srA = md.GetPlayerA(1, 2);
    srX = md.GetPlayerX(1, 2);
  } 
  else if (match == 1)
  {
    // Erstes Spiel einer Runde: Aus letztem Spiel der vorigen Runde
    srA = md.GetPlayerA(round - 1, md.Matches());
    srX = md.GetPlayerX(round - 1, md.Matches());
  }
  else
  {
    // Aus vorigem Spiel
    srA = md.GetPlayerA(round, match - 1);
    srX = md.GetPlayerX(round, match - 1);
  }
  
  // Belegte Spieler ausnehmen
  if (srA == playerA || srA == playerX)
    return srX;
  else if (srX == playerA || srX == playerX)
    return srA;
  
  // Freilose ausnehmen
  if (GetTeamByNr(srA).tmID == 0)
    return srX;
  
  if (GetTeamByNr(srX).tmID == 0)
    return srA;
    
  // Ansonsten aus graden Runden srX, aus ungraden srA
  if ( (round % 2) == 0 )
    return srX;
  else
    return srA;
 
#else
  // Count, wie of jemand schon SR ist
  int *counts = (int *) _alloca(md.mdSize * sizeof(int));
  memset(counts, 0, md.mdSize * sizeof(int));

  // Und wer welches Spiel macht
  short *who = (short *) _alloca(md.Rounds() * md.Matches() * sizeof(short));
  memset(who, 0, md.Rounds() * md.Matches() * sizeof(short));

  // Und die Spieler, die ich nicht entscheiden kann.
  // Wenn man spaeter die Wahl zwischen 2 Spielern hat, dann nach Moeglichkeit
  // einen nehmen, der in einem T.B.D. Spiel vorkommt. Andernfalls laeuft man
  // Gefahr, dass ausgerechnet ein solcher die wenigsten Spiele hat.
  short *later = (short *) _alloca(md.mdSize * sizeof(short));
  memset(later, 0, md.mdSize * sizeof(short));

  // Keine Freilose
  for (int i = 1; i <= md.mdSize; i++)
  {
    if (GetTeamByNr(i).tmID == 0)
      counts[i-1] = 0xFFFF;
  }

  // Spieler kommen aus dem vorigen Spiel.
  // Wenn ein Spieler 2-mal hintereinander spielt, dann haben wir nur eine Wahl
  // -2 ist ein Marker, einen beliebigen Spieler am Ende zu suchen.
  // Das ist z.B. der Fall beim ersten Spiel, wenn es kein Freilos hat
  for (int i = 0; i < md.Rounds() * md.Matches(); i++)
  {
    int rd = 1 + (i / md.Matches());
    int mt = 1 + (i % md.Matches());

    short na = md.GetPlayerA(rd, mt);
    short nx = md.GetPlayerX(rd, mt);

    short *ptr = who + (rd-1) * md.Matches() + (mt-1);

    // Wenn eines ein Freilos ist, gibt es kein SR
    if (GetTeamByNr(na).tmID == 0)
    {
      *ptr = -1;
      continue;
    }

    if (GetTeamByNr(nx).tmID == 0)
    {
      *ptr = -1;
      continue;
    }

    // Voriges Spiel ohne Freilos suchen
    short pa = 0, px = 0;

    for (int j = i; j--; )
    {
      short a = md.GetPlayerA(1 + (j / md.Matches()), 1 + (j % md.Matches()));
      short x = md.GetPlayerX(1 + (j / md.Matches()), 1 + (j % md.Matches()));;

      if (GetTeamByNr(a).tmID != 0 && GetTeamByNr(x).tmID != 0)
      {        
        pa = a;
        px = x;

        break;
      }
    }

    // Wenn es kein voriges Spiel gab (dann sind beide 0. Ansonsten beide ungleich 0)
    if (pa == 0 || px == 0)
    {
      *ptr = -2;
      later[na-1]++;
      later[nx-1]++;

      continue;
    }

    if (pa == na || pa == nx)
    {
      if (GetTeamByNr(px).tmID == 0)
      {
        *ptr = -2;

        // Markierung setzen
        later[na-1]++;
        later[nx-1]++;

        continue;
      }

      counts[px-1]++;
      *ptr = px;

      continue;
    }

    if (px == na || px == nx)
    {
      if (GetTeamByNr(pa).tmID == 0)
      {
        *ptr = -2;

        // Markierung setzen
        later[na-1]++;
        later[nx-1]++;

        continue;
      }

      counts[pa-1]++;
      *ptr = pa;
    }
  }

  // Anzahl Spiele
  int last = md.Rounds() * md.Matches() - 1;

  // Solange iterieren, bis alle Spiele einen SR haben
  while (last > 0)
  {
    if (who[last])
    {
      last--;
      continue;
    }

    bool found = false;

    // Minimum von counts suchen
    int min = 0xFFFF;
    for (int i = 0; i < md.mdSize; i++)
    {
      if (min > counts[i])
        min = counts[i];
    }

    // Freie Spiele auffuellen
    for (int i = 1; i <= last; i++)
    {
      if (who[i])
        continue;

      int rd = 1 + (i / md.Matches());
      int mt = 1 + (i % md.Matches());

      short na = md.GetPlayerA(rd, mt);
      short nx = md.GetPlayerX(rd, mt);

      // Voriges Spiel ohne Freilos suchen (es gibt eines, sonst waere die Markierung gesetzt)
      short pa = 0, px = 0;

      for (int j = i; j--; )
      {
        short a = md.GetPlayerA(1 + (j / md.Matches()), 1 + (j % md.Matches()));
        short x = md.GetPlayerX(1 + (j / md.Matches()), 1 + (j % md.Matches()));;

        if (GetTeamByNr(a).tmID != 0 && GetTeamByNr(x).tmID != 0)
        {
          pa = a;
          px = x;

          break;
        }
      }

      // Wenn beide gleich viel haben, Entscheidung vertagen, falls eine gefallen ist
      if (counts[pa-1] == counts[px-1])
      {
        if (i == last && !found)
        {
          // Nach Moeglichkeit einen Spieler nehmen, der in einem "t.b.d." Spiel ist
          if (later[pa-1] > later[px-1])
          {
            who[i] = pa;
            counts[pa-1]++;
          }
          else
          {
            who[i] = px;
            counts[px-1]++;
          }

          found = true;
        }

        continue;
      }

      // Versuchen, nur Spielere mit Minimumanzahl zu verwenden.
      // Ausser im letzten Spiel, wenn noch keiner gefunden wurde.
      if (counts[pa-1] > min && counts[px-1] > min)
      {
        if (i < last || found)
          continue;
      }

      if (counts[pa-1] < counts[px-1])
      {
        counts[pa-1]++;
        who[i] = pa;

        found = true;
      }
      else if (counts[px-1] < counts[pa-1])
      {
        counts[px-1]++;
        who[i] = px;

        found = true;
      }
    }
  }

  // Jetzt alle Spiele, die noch einen SR brauchen.
  for (int i = 0; i < md.Rounds() * md.Matches(); i++)
  {
    if (who[i] != -2)
      continue;

    int rd = 1 + (i / md.Matches());
    int mt = 1 + (i % md.Matches());

    short na = md.GetPlayerA(rd, mt);
    short nx = md.GetPlayerX(rd, mt);

    // Minimum suchen
    int min = 0xFFFF;
    for (int j = 0; j < md.mdSize; j++)
    {
      if (min > counts[j])
        min = counts[j];
    }

    // Spieler suchen, der die wenigsten Spiele hat.
    // Wenn es keinen gibt, einen suchen, der ein Spiel mehr hat. Etc.
    for (bool done = false; !done; min++)
    {
      for (int j = 1; j <= md.mdSize; j++)
      {
        // Ein Spieler mit Minimalanzahl, der frei ist
        if (counts[j-1] == min && j != na && j != nx)
        {
          who[i] = j;
          counts[j-1]++;

          done = true;

          break;
        }
      }
    }
  }

  // Ach ja, wen wollte ich noch?
  short sr = who[(round-1) * md.Matches() + (match-1)];

  if (sr <= 0)
    return 0;

  for (int i = (round-1) * md.Matches() + (match-1) + 1; i--; )
  {
    if (who[i] == (sr & 0xFF))
      sr += 0x100;
  }
    

  return sr;

  return 0;
#endif  
}
