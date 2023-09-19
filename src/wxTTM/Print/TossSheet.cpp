/* Copyright (C) 2023 Christoph Theis */

// ---------------------------------------------------------------------
//                     TossSheet
//
#include "stdafx.h"

#include "TossSheet.h"

#include "TT32App.h"

#include "MtEntryStore.h"
#include "NmEntryStore.h"
#include "NtEntryStore.h"

#include "LtItem.h"

#include "CpStore.h"
#include "GrStore.h"
#include "SyListStore.h"
#include "Res.h"

// -----------------------------------------------------------------------

TossSheet::TossSheet(Printer *prt, Connection *ptr)
				 : RasterBase(prt, ptr)
{
};


TossSheet::~TossSheet()
{
};


// -----------------------------------------------------------------------
int  TossSheet::Print(const CpRec& cp_, const GrRec& gr_, const MtEntry& mt)
{
  printer->StartPage();

  cp = cp_;
  gr = gr_;

	textFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_NORMAL);
  headerFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_MEDIUMB);

	short  oldFont = printer->SelectFont(textFont);

  CRect top;
  top.left = offsetX;
  top.top = offsetY;
  top.right = top.left + printer->width;
  top.bottom = top.top + printer->height / 2;

  CRect bot = top;
  bot.top = top.bottom + printer->cH;
  bot.bottom = bot.top + printer->height / 2;

  // Print a thick dashed line between
  printer->Line(top.left, top.bottom - printer->topMargin / 2, top.right, top.bottom - printer->topMargin / 2, THICK_FRAME, wxPENSTYLE_SHORT_DASH);

  PrintToss(mt, top, false);
  PrintToss(mt, bot, true);

  printer->SelectFont(oldFont);
  printer->DeleteFont(textFont);

  printer->EndPage();

	return 0;
}


// -----------------------------------------------------------------------
// Print upper / lower half of toss sheet
int TossSheet::PrintToss(const MtEntry& mt, const CRect& rect, bool ax)
{
  // Lookup team system
  GrListStore gr;
  gr.SelectById(mt.mt.mtEvent.grID);
  gr.Next();
  gr.Close();

  SyListStore sy;
  sy.SelectById(gr.syID);
  sy.Next();
  sy.Close();

  // Header with date, time, table
  int headerHeight = 2 * printer->cH;
  CRect header = rect;
  header.bottom = header.top + headerHeight;
  PrintHeader(mt, header);

  CRect teamA = rect;
  teamA.top = header.bottom + printer->cH;
  teamA.bottom = teamA.top + printer->cH + std::max((int) sy.syMatches, 5) * printer->cH;
  teamA.right = teamA.left + printer->width / 3 - 2 * printer->cW;;

  PrintTeam(mt.tmA, teamA);

  CRect teamX = teamA;
  teamX.left = teamA.right + printer->cW;
  teamX.right = teamX.left + printer->width / 3 - 2 * printer->cW;;

  PrintTeam(mt.tmX, teamX);

  CRect syRect = teamX;
  syRect.left = teamX.right + printer->cW;
  syRect.right = teamX.right + printer->width / 3 ;
  syRect.top = teamX.top;
  syRect.bottom = teamX.top + sy.syMatches * printer->cH;

  PrintSystem(sy, syRect);

  CRect nmRect = rect;
  nmRect.top = std::max(std::max(teamA.bottom, teamX.bottom), syRect.bottom) + 2 * printer->cH;
  PrintNomination(sy, nmRect, ax);

  return 0;
}


// -----------------------------------------------------------------------
// Print header with date, time, table
int TossSheet::PrintHeader(const MtEntry &mt, const CRect &rect)
{
  CRect evt = rect;
  CRect date = rect;
  CRect time = rect;
  CRect table = rect;

  short tableFont = printer->DeriveFont(headerFont, 700, 0, 1.9);

  evt.right = evt.left + rect.GetWidth() / 4;
  date.left = evt.right;
  date.right = date.left + rect.GetWidth() / 4;
  time.left = date.right;
  time.right = time.left + rect.GetWidth() / 4;
  table.left = time.right;

  wxString lblEvent = wxT("Event:");
  evt.left += printer->cW;
  PrintString(lblEvent, evt);
  evt.left += printer->TextWidth(lblEvent) + printer->cW;
  printer->SelectFont(headerFont);
  PrintString(mt.mt.cpName, evt);
  printer->SelectFont(textFont);

  wxString lblDate = wxT("Date:");
  PrintString(lblDate, date);
  date.left += printer->TextWidth(lblDate) + printer->cW;
  printer->SelectFont(headerFont);
  PrintDate(mt.mt.mtPlace.mtDateTime, date);
  printer->SelectFont(textFont);

  wxString lblTime = wxT("Time:");
  PrintString(lblTime, time);
  time.left += printer->TextWidth(lblTime) + printer->cW;
  printer->SelectFont(headerFont);
  PrintTime(mt.mt.mtPlace.mtDateTime, time);
  printer->SelectFont(textFont);

  wxString lblTable = wxT("Table:");
  PrintString(lblTable, table);
  table.left += printer->TextWidth(lblTable) + printer->cW;
  printer->SelectFont(tableFont);
  PrintInt(mt.mt.mtPlace.mtTable, table, wxALIGN_LEFT);
  printer->SelectFont(textFont);

  printer->DeleteFont(tableFont);

  // And a border around all
  printer->Rectangle(rect, THICK_FRAME);

  return 0;
}


// Print list of players in teeam
int TossSheet::PrintTeam(const TmEntry& tm, const CRect& rect)
{
  printer->SelectFont(headerFont);

  CRect header = rect;
  header.bottom = header.top + printer->cH;
  PrintStringCentered(tm.team.tm.tmDesc, header);
  printer->Line(header.left, header.top + printer->cH, header.right, header.top + printer->cH, THICK_FRAME);

  wxRect entry(header.left, header.top + printer->cH, header.GetWidth(), printer->cH);
  entry.Offset(0, -printer->cH);

  printer->SelectFont(textFont);

  NtEntryStore nt;
  nt.SelectByTm(tm);
  while (nt.Next() && entry.GetBottom() <= rect.bottom)
  {
    entry.Offset(0, printer->cH);

    LtItem(nt, false).DrawItem(printer->GetDC(), entry);
    printer->Line(entry.GetBottomLeft(), entry.GetBottomRight());
  }

  printer->Rectangle(header.left, header.top, entry.GetRight(), std::max(rect.GetBottom(), entry.GetBottom()), THICK_FRAME);

  return 0;
}


int TossSheet::PrintSystem(const SyListRec & sy, const CRect & rect)
{
  // Print match order
  // First a thick frame around it and lines cut-off description and matches
  // First line is caption, so the matches are aligned with the players (somewhat)
  printer->SelectFont(headerFont);
  CRect header = rect;
  header.bottom = header.top + printer->cH;
  PrintStringCentered(wxT("Matches"), header);

  printer->Rectangle(header, THICK_FRAME);

  printer->SelectFont(textFont);

  CRect matches = rect;
  matches.top = header.bottom;
  matches.bottom = matches.top + sy.syMatches * printer->cH;

  printer->Rectangle(matches, THICK_FRAME);
  printer->Line(rect.left + rect.GetWidth() / 3, matches.top, matches.left + matches.GetWidth() / 3, matches.bottom);
  printer->Line(rect.right - rect.GetWidth() / 3, matches.top, matches.right - matches.GetWidth() / 3, matches.top + sy.syMatches * printer->cH);

  // Now the matches
  CRect match = matches;

  // We increment at the start of the for-loop, so top is cH above
  match.top = matches.top - printer->cH;
  match.bottom = matches.top;

  wxString lblMatch = _("Match %d");
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
	const wxChar *xtsa[][4] = 
	{
		{wxT("A-G1"), wxT("A-B1"), wxT("A-G2"), wxT("A-B2")},
		{wxT("X-G1"), wxT("X-B1"), wxT("X-G2"), wxT("X-B2")}
	};

  for (int i = 0; i < sy.syMatches; i++)
  {
    match.top += printer->cH;
    match.bottom = match.top + printer->cH;

    printer->Line(match.left, match.bottom, match.right, match.bottom);

    CRect col = match;
    col.right = col.left + match.GetWidth() / 3;

    wxString str = wxString::Format(lblMatch, i+1);
    printer->Text(col, str);

    col.left = col.right;
    col.right += match.GetWidth() / 3;

    wxString strA;
    wxString strX;

    if (sy.syList[i].syType == CP_SINGLE)
    {
      if (wxStrcmp(sy.syName, "ETS") == 0)
      {
        if (sy.syList[i].syPlayerA < 4)
        {
          strA = wxString::Format("A-%d", sy.syList[i].syPlayerA);
          strX = wxString::Format("X-%d", sy.syList[i].syPlayerX);
        }
        else
        {
          strA = wxString::Format("A-%d/A-4", i == 3 ? 1 : 2);
          strX = wxString::Format("X-%d/X-4", i == 3 ? 1 : 2);
        }
      }
      else if (wxStrcmp(sy.syName, "XTS") == 0)
      {
        strA = xts[0][sy.syList[i].syPlayerA-1];
        strX = xts[1][sy.syList[i].syPlayerX-1];
      }
      else if (wxStrcmp(sy.syName, "XTSA") == 0)
      {
        strA = xtsa[0][sy.syList[i].syPlayerA-1];
        strX = xtsa[1][sy.syList[i].syPlayerX-1];
      }
      else if (wxStrcmp(sy.syName, "YSTA") == 0)
      {
        if (i == 0)
        {
          strA = wxString::Format("A-1");
          strX = wxString::Format("X-2");
        }
        else if (i == 1)
        {
          strA = wxString::Format("A-2");
          strX = wxString::Format("X-1");
        }
        else if (i == 3)
        {
          strA = wxString::Format("A-2/A-3/A-4");
          strX = wxString::Format("X-2/X-3/X-4");
        }
        else if (i == 4)
        {
          strA = wxString::Format("A-1/A-3/A-4");
          strX = wxString::Format("X-1/X-3/X-4");
        }
      }
      else if (sy.sySingles < 4)
      {
        strA = alpha[0][sy.syList[i].syPlayerA-1];
        strX = alpha[1][sy.syList[i].syPlayerX-1];
      }
      else
      {
        strA = wxString::Format("A-%d", sy.syList[i].syPlayerA);
        strX = wxString::Format("X-%d", sy.syList[i].syPlayerX);
      }
    }
    else if (sy.syList[i].syType == CP_DOUBLE)
    {
      if (wxStrcmp(sy.syName, "XTS") == 0)
      {
        strA = "A-B/A-G";
        strX = "X-B/X-G";
      }
      else if (wxStrcmp(sy.syName, "XTSA") == 0)
      {
        strA = "A-B/A-G";
        strX = "X-B/X-G";

      }
      else if (sy.syDoubles > 1)
      {
        strA = wxString::Format("DA-%d", sy.syList[i].syPlayerA);  // plA is the nof double
        strX = wxString::Format("DX-%d", sy.syList[i].syPlayerX);  // plX is the nof double
      }
      else
      {
        strA = "DA";
        strX = "DX";
      }
    }

    printer->Text(col, strA);

    col.left = col.right;
    col.right = match.right;

    printer->Text(col, strX);
  }

  return 0;
}


int TossSheet::PrintNomination(const SyListRec& sy, const CRect& rect, bool ax)
{
  short captionFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_COMP);
  short entryFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_GROUP);

  printer->SelectFont(captionFont);

  const int space = printer->cW;

  CRect caption = rect;
  caption.left += space;
  caption.bottom = caption.top + 2 * printer->cH;
  printer->Line(caption.left - space, caption.bottom, caption.right, caption.bottom);
  printer->Text(caption, ax ? "Team \"X\":" : "Team \"A\":");

  printer->SelectFont(entryFont);
  CRect entry = caption;
  entry.top = caption.bottom;
  entry.bottom = entry.top + 2 * printer->cH;
  entry.left += space;

  printer->Text(entry, "Team Captain:");
  printer->Line(entry.left - space, entry.bottom, entry.right, entry.bottom);

  int syDoubles = sy.syDoubles;
  int sySingles = sy.sySingles;

  if (wxStrcmp(sy.syName, "ETS") == 0)
    sySingles = 4;
	else if (wxStrcmp(sy.syName, wxT("XTS")) == 0)
		sySingles = 2;
	else if (wxStrcmp(sy.syName, wxT("XTSA")) == 0)
		sySingles = 2;
  else if (wxStrcmp(sy.syName, wxT("YSTA")) == 0)
    sySingles = 4;

  // Start with doubles only if the first match is a double
  if (sy.syList[0].syType == CP_DOUBLE)
  {
    for (int i = 0; i < syDoubles; i++)
    {  
      entry.top = entry.bottom;
      entry.bottom = entry.top + 4 * printer->cH;

      wxString str;
      if (wxStrcmp(sy.syName, "XTS") == 0)
        str = ax ? "X-B1\nX-G1" : "A-B1\nA-G1";
      else if (wxStrcmp(sy.syName, "XTSA") == 0)
        str = ax ? "X-B1\nX-G1" : "A-B1\nA-G1";
      else if (syDoubles > 1)
        str = wxString::Format(ax ? "DX-%d" : "DA-%d");
      else
        str = ax ? "DX" : "DA";

      PrintString(str, entry, wxALIGN_CENTRE_VERTICAL | wxALIGN_LEFT);

      printer->Line(entry.left - space, entry.bottom, entry.right, entry.bottom);
    }
  }

  for (int i = 0; i < sySingles; i++)
  {
    // In XTS/XTSA we add only the 2 players not playing doubles
    wxString str;
    if (wxStrcmp(sy.syName, "XTS") == 0)
    {
      if (i == 0)
        str = ax ? "X-G2" : "A-G2";
      else if (i == 1)
        str = ax ? "X-B2" : "A-B2";
      else
        str = wxString::Format(ax ? "X-%d" : "A-%d", (i+1));
    }
    else if (wxStrcmp(sy.syName, "XTSA") == 0)
    {
      if (i == 0)
        str = ax ? "X-G2" : "A-G2";
      else if (i == 1)
        str = ax ? "X-B2" : "A-B2";
      else
        str = wxString::Format(ax ? "X-%d" : "A-%d", (i+1));
    }
    else if (sy.sySingles < 4)
    {
      const wxChar * alpha[][3] =
      {
        {wxT("A"), wxT("B"), wxT("C")},
        {wxT("X"), wxT("Y"), wxT("Z")}
      };

      str = ax ?
        alpha[1][sy.syList[i].syPlayerX - 1] :
        alpha[0][sy.syList[i].syPlayerA - 1];
    }
    else
    {
      str = wxString::Format(ax ? "X-%d" : "A-%d", (i+1));
    }

    entry.top = entry.bottom;
    entry.bottom = entry.top + 2 * printer->cH;

    printer->Text(entry, str);

    printer->Line(entry.left - space, entry.bottom, entry.right, entry.bottom);
  }

  // And then double(s) if the first match is a single
  if (sy.syList[0].syType == CP_SINGLE)
  {
    for (int i = 0; i < syDoubles; i++)
    {  
      entry.top = entry.bottom;
      entry.bottom = entry.top + 4 * printer->cH;

      wxString str;

      if (syDoubles > 1)
        str = wxString::Format(ax ? "DX-%d" : "DA-%d");
      else
        str = ax ? "DX" : "DA";

      PrintString(str, entry);

      printer->Line(entry.left - space, entry.bottom, entry.right, entry.bottom);
    }
  }

  printer->Rectangle(caption.left - space, caption.top, entry.right, entry.bottom, THICK_FRAME);

  CRect footer = rect;
  footer.top = entry.bottom + printer->cH;
  footer.bottom = footer.top + printer->cH;

  if (wxStrcmp(sy.syName, "ETS") == 0)
  {
    wxString lbl = wxString::Format(wxT("Player %s replaces"), ax ? "X-4" : "A-4");
    wxString val = 
      wxString::Format(wxT("[  ]  None")) + "\n" + 
      wxString::Format(wxT("[  ]  Player %s"), ax? "X-1" : "A-1") + "\n" +
      wxString::Format(wxT("[  ]  Player %s"), ax? "X-2" : "A-2")
    ;
    footer.bottom = footer.top + 2 * printer->cH;
    PrintString(lbl, footer);
    footer.left += printer->TextWidth(lbl) + 4 * printer->cW;
    footer.bottom += 4 * printer->cH;
    PrintString(val, footer);
  }
  else if (wxStrcmp(sy.syName, "YSTA") == 0)
  {
    wxString lbl = wxString::Format(wxT("Changes"));
    wxString vals[] = 
      {
        wxString::Format(wxT("[  ]  None")),
        wxString::Format(wxT("[  ]  Player %s instead of %s"), ax ? "X-3" : "A-3", ax ? "X-1" : "A-1") + "\n" +
        wxString::Format(wxT("[  ]  Player %s instead of %s"), ax ? "X-3" : "A-3", ax ? "X-2" : "A-2"),
        wxString::Format(wxT("[  ]  Player %s instead of %s"), ax ? "X-4" : "A-4", ax ? "X-1" : "A-1") + "\n" +
        wxString::Format(wxT("[  ]  Player %s instead of %s"), ax ? "X-4" : "A-4", ax ? "X-2" : "A-2")
      }
    ;

    footer.bottom = footer.top + 2 * printer->cH;
    PrintString(lbl, footer);
    footer.left += printer->TextWidth(lbl) + 4 * printer->cW;
    size_t count = sizeof(vals) / sizeof(vals[0]);
    // 1st string ("None") is shorter than the others, give it less weight
    int width = (footer.right - footer.left) / (2 * count - 1);

    for (int idx = 0; idx < count; ++idx)
    {
      footer.right = footer.left + (idx == 0 ? width : 2 * width);

      PrintString(vals[idx], footer);

      footer.left = footer.right;
    }
  }

  printer->SelectFont(textFont);

  return 0;
}
