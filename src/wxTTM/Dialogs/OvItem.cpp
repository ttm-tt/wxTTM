/* Copyright (C) 2020 Christoph Theis */

// Anzeige des Spielstatus in der Spieluebersicht

#include  "stdafx.h"

#include  "OvItem.h"

#include  "MtListStore.h"
#include  "MtEntryStore.h"

#include  "Printer.h"

wxColor  OvItem::defaultColors[][2] = {
  {wxColor(0, 0, 0),       wxColor(255, 255, 255)},   // Error
  {wxColor(255, 0, 0),     wxColor(0, 0, 0)},         // Unknown
  {wxColor(255, 255, 0),   wxColor(0, 0, 0)},         // Not printed
  {wxColor(0, 255, 0),     wxColor(0, 0, 0)},         // Printed
  // {wxColor(173, 216, 230), wxColor(0, 0, 0)},         // Finished 
  // {wxColor(121, 188, 255), wxColor(0, 0, 0)},         // Finished 
  {wxColor(0, 255, 255),   wxColor(0, 0, 0)},         // Finished
  {wxColor(255, 255, 255), wxColor(0, 0, 0)}          // Checked
};


wxColor  OvItem::colors[][2] = {
  {wxNullColour, wxNullColour},
  {wxNullColour, wxNullColour},
  {wxNullColour, wxNullColour},
  {wxNullColour, wxNullColour},
  {wxNullColour, wxNullColour},
  {wxNullColour, wxNullColour},
};

static bool defaultShowUmpires = false;



// -----------------------------------------------------------------------
OvItem::OvItem(const wxString & lbl, OvType t)
      : showUmpires(&defaultShowUmpires)
{
  label = lbl;
  ovType  = t;
}

OvItem::OvItem(const MtListRec &rec)
      : ListItem(rec.mtID),
        showUmpires(&defaultShowUmpires)
{
  SetValue(rec);
}


OvItem::OvItem(const MtListRec &rec, OvType t)
      : ListItem(rec.mtID),
        showUmpires(&defaultShowUmpires)
{
  mt = rec;
  ovType = t;
}


OvItem::OvItem(const MtEntry &rec)
      : ListItem(rec.mt.mtID),
        showUmpires(&defaultShowUmpires)
{
  SetValue(rec);
}


OvItem::OvItem(const MtEntry &rec, OvType t)
      : ListItem(rec.mt.mtID),
        showUmpires(&defaultShowUmpires)
{
  SetValue(rec);
  ovType = t;
}      


void  OvItem::SetValue(const MtListRec &rec)
{
  // Tooltip loeschen, wenn sich die Mannschaften aendern
  if (rec.tmA != mt.tmA || rec.tmX != mt.tmX)
  {
    ttText = "";
    tmA = TmEntry();
    tmX = TmEntry();
  }

  mt = rec;

  if (rec.mtPlace.mtTable == 0)
    ovType = OVERROR;
  else if (mt.IsFinished() && mt.mtScoreChecked)     // Freilos, Sieg, Untentschieden im Mannschaftsspiel, doppeltes w/o
    ovType = OVCHECKED;
  else if (mt.IsABye() || mt.IsXBye())
    ovType = OVCHECKED;
  else if (mt.IsFinished() && !mt.mtScoreChecked)
    ovType = OVFINISHED;
  else if (!mt.tmA || !mt.tmX)  // Auch wenn nur XxRec bekannt ist
    ovType = OVUNKNOWN;
  else if (!mt.mtScorePrinted)
    ovType = OVNOTPRINTED;
  else
    ovType = OVPRINTED;
}


void OvItem::SetValue(const MtEntry &rec)
{
  SetValue(rec.mt);
  
  tmA = rec.tmA;
  tmX = rec.tmX;
}


void OvItem::SetShowUmpire(bool *b)
{
  showUmpires = b;  
}
 

// -----------------------------------------------------------------------
wxColor  OvItem::GetBkColor() const
{
  wxASSERT(OVLAST == sizeof(colors) / sizeof(colors[0]));
  wxASSERT(OVLAST == sizeof(defaultColors) / sizeof(defaultColors[0]));

  return GetBkColor(ovType);
}


wxColor  OvItem::GetFgColor() const
{
  wxASSERT(OVLAST == sizeof(colors) / sizeof(colors[0]));
  wxASSERT(OVLAST == sizeof(defaultColors) / sizeof(colors[0]));

  return GetFgColor(ovType);
}


// Text ausgeben
void OvItem::DrawItem(wxDC *pDC, wxRect &rect)
{
  wxASSERT(OVLAST == sizeof(colors) / sizeof(colors[0]));

  wxColor  oldBkCol = pDC->GetTextBackground();
  wxColor  oldFgCol = pDC->GetTextForeground();

  pDC->SetTextBackground(GetBkColor());
  pDC->SetTextForeground(GetFgColor());

  if (mt.mtNr)
  {
    wxRect  cpRect = rect;
    wxRect  grRect = rect;
    wxRect  mtRect = rect;

    cpRect.SetBottom(cpRect.GetTop() + rect.GetHeight() / 3);
    grRect.SetTop(cpRect.GetBottom());
    grRect.SetBottom(grRect.GetTop() + rect.GetHeight() / 3);
    mtRect.SetTop(grRect.GetBottom());
    mtRect.SetBottom(rect.GetBottom());

    if (*showUmpires)
    {
      if (tmA.team.cpType == CP_DOUBLE || tmA.team.cpType == CP_MIXED ||
          tmX.team.cpType == CP_DOUBLE || tmX.team.cpType == CP_MIXED)
      {
        wxChar tmp[32];
        wxSprintf(tmp, "%s / %s", 
                tmA.team.pl.naName, 
                tmA.team.bd.naName);
        DrawString(pDC, cpRect, tmp, wxALIGN_CENTER);
        wxSprintf(tmp, "%s / %s", 
                tmX.team.pl.naName,
                tmX.team.bd.naName);
        DrawString(pDC, grRect, tmp, wxALIGN_CENTER);
        //DrawLong(pDC, mtRect, mt.mtUmpire, wxALIGN_CENTER);                                                     
      }
      else if (tmA.team.cpType == CP_SINGLE || tmX.team.cpType == CP_SINGLE)
      {
        DrawString(pDC, cpRect, tmA.team.pl.naName, wxALIGN_CENTER);
        DrawString(pDC, grRect, tmX.team.pl.naName, wxALIGN_CENTER);
        
        //if (mt.mtUmpire > 0)
        //  DrawLong(pDC, mtRect, mt.mtUmpire, wxALIGN_CENTER);                                                     
      }
      else if (tmA.team.cpType == CP_TEAM || tmX.team.cpType == CP_TEAM)
      {
        DrawString(pDC, cpRect, tmA.team.tm.naName, wxALIGN_CENTER);
        DrawString(pDC, grRect, tmX.team.tm.naName, wxALIGN_CENTER);
        //DrawLong(pDC, mtRect, mt.mtUmpire, wxALIGN_CENTER);                                                     
      }
      
      // EYC2006: Auch WB anzeigen
      wxChar cpump[32];
      if (mt.mtUmpire > 0)
      {
        wxSprintf(cpump,"%s - %i", mt.cpName, mt.mtUmpire);
      }
      else
      {
        wxSprintf(cpump,"%s", mt.cpName);
      }
      
      DrawString(pDC, mtRect, cpump, wxALIGN_CENTER);  
    }
    else
    {
      DrawStringCentered(pDC, cpRect, mt.cpName);
      DrawStringCentered(pDC, grRect, mt.grName);
      // DrawLong(pDC, mtRect, mt.mtNr, wxALIGN_CENTER);
      
      wxChar buffer[10];
      // ltoa(mt.mtNr, buffer, 10);
      
      wxString strRd = _("Rd.");
      wxString strSF = _("SF");
      wxString strQF = _("QF");
      wxString strQual = _("Qu.");
          
      switch (mt.grModus)
      {
        case MOD_RR :
          if (mt.mtEvent.mtRound)
            wxSprintf(buffer, "%s %d", strRd.t_str(), mt.mtEvent.mtRound);
          else
            *buffer = 0;
          break;
          
        case MOD_DKO : 
          wxSprintf(buffer, "%s %d", strRd.t_str(), mt.mtEvent.mtRound);
          break;
          
        case MOD_PLO :
        {
          int nof = 
            mt.grNofRounds > 0 ?
            1 << (mt.grNofRounds - mt.mtEvent.mtRound) :
            mt.grSize >> mt.mtEvent.mtRound;
          if (nof == 1 && mt.mtEvent.mtMatch == 1 && mt.grWinner == 1)
            wxSprintf(buffer, "F");
          else if (nof == 2 && mt.mtEvent.mtMatch <= 2 && mt.grWinner == 1)
            wxSprintf(buffer, "%s", strSF.t_str());
          else if (nof == 4 && mt.mtEvent.mtMatch <= 4 && mt.grWinner == 1)
            wxSprintf(buffer, "%s", strQF.t_str());
          else
          {
            short m    = mt.mtEvent.mtMatch - 1;
            short from = (m / nof) * nof ;
            short to   = (from + nof);
            // Ein Spiel geht um 2 Plaetze
            wxSprintf(buffer, "%d-%d", mt.grWinner + 2 * from, mt.grWinner + 2 * to - 1);
          }
          
          break;
        }

        case MOD_SKO :
        {
          int nof = mt.grSize >> mt.mtEvent.mtRound;
          
          if (mt.mtEvent.mtRound <= mt.grQualRounds)
            wxSprintf(buffer, "%s", strQual.t_str());
          else if (mt.grNofRounds > 0 || mt.grNofMatches > 0)
            wxSprintf(buffer, "%s %d", strRd.t_str(), mt.mtEvent.mtRound);
          else if (nof == 1 && mt.mtEvent.mtMatch == 1 && mt.grWinner == 1)
            wxSprintf(buffer, "F");
          else if (nof == 2 && mt.mtEvent.mtMatch <= 2 && mt.grWinner == 1)
            wxSprintf(buffer, "%s", strSF.t_str());
          else if (nof == 4 && mt.mtEvent.mtMatch <= 4 && mt.grWinner == 1)
            wxSprintf(buffer, "%s", strQF.t_str());  
          else
            wxSprintf(buffer, "1/%d", nof);
            
          break;
        }
      }
      
      DrawStringCentered(pDC, mtRect, buffer);      
    }    
  }
  else
  {
    pDC->SetBrush(wxBrush(GetBkColor(), wxBRUSHSTYLE_SOLID));
    
    pDC->DrawRectangle(rect);
    
    wxRect tmpRect = rect;
    // tmpRect.Deflate(1, 1);
    DrawStringCentered(pDC, tmpRect, label.data());
  }

  pDC->SetTextBackground(oldBkCol);
  pDC->SetTextForeground(oldFgCol);
}


void OvItem::PrintItem(Printer *printer, wxRect &rect)
{
  if (mt.mtNr)
  {
    wxRect  cpRect = rect;
    wxRect  grRect = rect;
    wxRect  mtRect = rect;

    cpRect.SetBottom(rect.GetTop() + cpRect.GetHeight() / 3);
    grRect.SetTop(cpRect.GetBottom());
    grRect.SetBottom(grRect.GetTop() + cpRect.GetHeight());
    mtRect.SetTop(grRect.GetBottom());
    mtRect.SetBottom(rect.GetBottom());

    if (*showUmpires)
    {
      if (tmA.team.cpType == CP_DOUBLE || tmA.team.cpType == CP_MIXED ||
          tmX.team.cpType == CP_DOUBLE || tmX.team.cpType == CP_MIXED)
      {
        wxChar tmp[32];
        wxSprintf(tmp, "%s / %s", 
                tmA.team.pl.naName, 
                tmA.team.bd.naName);
        printer->Text(cpRect, tmp, wxALIGN_CENTER);
        wxSprintf(tmp, "%s / %s", 
                tmX.team.pl.naName,
                tmX.team.bd.naName);
        printer->Text(grRect, tmp, wxALIGN_CENTER);
      }
      else if (tmA.team.cpType == CP_SINGLE || tmX.team.cpType == CP_SINGLE)
      {
        printer->Text(cpRect, tmA.team.pl.naName, wxALIGN_CENTER);
        printer->Text(grRect, tmX.team.pl.naName, wxALIGN_CENTER);
      }
      else if (tmA.team.cpType == CP_TEAM || tmX.team.cpType == CP_TEAM)
      {
        printer->Text(cpRect, tmA.team.tm.naName, wxALIGN_CENTER);
        printer->Text(grRect, tmX.team.tm.naName, wxALIGN_CENTER);
      }
      
      // EYC2006: WB auch anzeigen
      wxChar buffer[32];
      if (mt.mtUmpire <= 0)
        wxSprintf(buffer, "%s", mt.cpName);
      else
        wxSprintf(buffer, "%s - %i", mt.cpName, mt.mtUmpire);
        
      printer->Text(mtRect, buffer, wxALIGN_CENTER);

    }
    else
    {
      wxString strRd = _("Rd.");
      wxString strSF = _("SF");
      wxString strQF = _("QF");
      wxString strQual = _("Qu.");

      wxChar buffer[10];
      // ltoa(mt.mtNr, buffer, 10);
      
      switch (mt.grModus)
      {
        case MOD_RR :
          wxSprintf(buffer, "%s %d", strRd.t_str(), mt.mtEvent.mtRound);
          break;
          
        case MOD_DKO : 
          wxSprintf(buffer, "%s %d", strRd.t_str(), mt.mtEvent.mtRound);
          break;
          
        case MOD_PLO :
        {
          int nof = mt.grSize >> mt.mtEvent.mtRound;
          if (nof == 1 && mt.mtEvent.mtMatch == 1 && mt.grWinner == 1)
            wxSprintf(buffer, "F");
          else if (nof == 2 && mt.mtEvent.mtMatch <= 2 && mt.grWinner == 1)
            wxSprintf(buffer, "%s", strSF.t_str());
          else if (nof == 4 && mt.mtEvent.mtMatch <= 4 && mt.grWinner == 1)
            wxSprintf(buffer, "%s", strQF.t_str());
          else
          {
            short m    = mt.mtEvent.mtMatch - 1;
            short from = (m / nof) * nof ;
            short to   = (from + nof);
            // Ein Spiel geht um 2 Plaetze
            wxSprintf(buffer, "%d-%d", mt.grWinner + 2 * from, mt.grWinner + 2 * to - 1);
          }
          
          break;
        }

        case MOD_SKO :
        {
          int nof = mt.grSize >> mt.mtEvent.mtRound;
          
          if (mt.mtEvent.mtRound <= mt.grQualRounds)
            wxSprintf(buffer, "%s", strQual.t_str());
          else if (mt.grNofRounds > 0 || mt.grNofMatches > 0)
            wxSprintf(buffer, "%s %d", strRd.t_str(), mt.mtEvent.mtRound);
          else if (nof == 1 && mt.mtEvent.mtMatch == 1 && mt.grWinner == 1)
            wxSprintf(buffer, "F");
          else if (nof == 2 && mt.mtEvent.mtMatch <= 2 && mt.grWinner == 1)
            wxSprintf(buffer, "%s", strSF.t_str());
          else if (nof == 4 && mt.mtEvent.mtMatch <= 4 && mt.grWinner == 1)
            wxSprintf(buffer, "%s", strQF.t_str());
          else
            wxSprintf(buffer, "1/%d", nof);
            
          break;
        }
      }
      
      printer->Text(cpRect, mt.cpName, wxALIGN_CENTER);
      printer->Text(grRect, mt.grName, wxALIGN_CENTER);
      printer->Text(mtRect, buffer, wxALIGN_CENTER);
    }
  }
  else
  {
    if (label.IsEmpty())
      return;

    wxArrayString tmp = wxStringTokenize(label, "\n");
    if (tmp.GetCount() == 0)
      return;

    int lH = rect.GetHeight() / tmp.GetCount();

    for (unsigned i = 0; i < tmp.GetCount(); i++)
    {
      wxRect line = rect;
      line.SetTop(rect.GetTop() + i * lH);
      line.SetHeight(lH);

      printer->Text(line, tmp[i], wxALIGN_CENTER);
    }
  }
}


// -----------------------------------------------------------------------
wxSize OvItem::GetToolTipSize(wxDC *pDC)
{
  int cW = pDC->GetTextExtent("M").GetWidth();

  wxSize size = pDC->GetTextExtent("BYE - BYE");
  
  if (!ttText.IsEmpty())
  {
    unsigned len = 0;
    wxArrayString tmp = wxStringTokenize(ttText, "\n");
    for (unsigned i = 0; i < tmp.GetCount(); i++)
      len = wxMax(len, cW * tmp[i].Length());

    size.SetWidth(len);
    size.SetHeight(size.GetHeight() * tmp.GetCount());

    return size;
  }

  if (!mt.IsABye() && !tmA.tmID ||
      !mt.IsXBye() && !tmX.tmID )
  {
    MtEntryStore  mtEntry;
    mtEntry.SelectById(mt.mtID, mt.cpType);
    mtEntry.Next();

    tmA = mtEntry.tmA;
    tmX = mtEntry.tmX;
  }

  if ( (tmA.team.cpType == CP_DOUBLE) || (tmA.team.cpType == CP_MIXED) ||
       (tmX.team.cpType == CP_DOUBLE) || (tmX.team.cpType == CP_MIXED) )
    size.SetHeight(size.GetHeight() * 2);

  int cxA, cxX;

  cxA = GetTextWidth(pDC, tmA);
  cxX = GetTextWidth(pDC, tmX);

  // size.GetWidth() = 2 * max(cxA, cxX) + 8 * pDC->GetTextExtent("M").GetWidth();
  size.SetWidth(cxA + cxX + 2 * cW);  // Space (2)
  
  if (!mt.IsABye() && !mt.IsXBye())
    size.SetWidth(size.GetWidth() + 6 * cW);

  return size;
}


void  OvItem::DrawToolTip(wxDC *pDC, const wxRect &rect)
{
  unsigned cW = pDC->GetTextExtent("M").GetWidth();

  if (!ttText.IsEmpty())
  {
    DrawString(pDC, rect, ttText, wxALIGN_LEFT);
    return;
  }

  wxRect rcA = rect;
  wxRect rcX = rect;
  wxRect rcSep = rect;
  
  int cxA = GetTextWidth(pDC, tmA);
  int cxX = GetTextWidth(pDC, tmX);

  // rcSep.left = rcA.right = rect.left + (rect.right - rect.left) / 2 - cW;
  // rcSep.right = rcX.left = rcA.right + 2 * cW;
  rcA.SetRight(rect.GetLeft() + cxA);
  rcSep.SetLeft(rcA.GetRight());

  rcX.SetLeft(rcA.GetRight() + 2 * cW);
  rcSep.SetRight(rcX.GetLeft());

  rcX.SetRight(rcX.GetLeft() + cxX);

  if (mt.IsABye())
  {
    rcA.SetLeft(rcA.GetLeft() + cW);
    rcA.SetRight(rcA.GetRight() - cW);
  }
  
  if (mt.IsABye())
    DrawBye(pDC, rcA);
  else
    DrawEntry(pDC, rcA, tmA);

  DrawString(pDC, rcSep, wxT("-"), wxALIGN_CENTER);

  if (mt.IsXBye())
    DrawBye(pDC, rcX);
  else
    DrawEntry(pDC, rcX, tmX);
    
  if (!mt.IsABye() && !mt.IsXBye())
  {    
    wxRect rcRes = rect;
    rcRes.SetLeft(rcX.GetRight());
    rcRes.SetRight(rect.GetRight());
    
    DrawStringCentered(pDC, rcRes, wxString::Format("%d : %d", mt.mtResA, mt.mtResX));
  }
}


int  OvItem::GetTextWidth(wxDC *pDC, const TmEntry &tm)
{
  int  cxA, cxB;

  int cW = pDC->GetTextExtent("M").GetWidth();

  if (!tm.tmID && tm.team.cpType != CP_GROUP)
    return pDC->GetTextExtent("BYE").GetWidth() + cW;

  switch (tm.team.cpType)
  {
    case CP_SINGLE :
      cxA = pDC->GetTextExtent(tm.team.pl.psName.psFirst).GetWidth() + 
            pDC->GetTextExtent(tm.team.pl.psName.psLast).GetWidth() +
            pDC->GetTextExtent(tm.team.pl.naName).GetWidth();

      cxB = 0;

      break;

    case CP_DOUBLE :
    case CP_MIXED :
      cxA = pDC->GetTextExtent(tm.team.pl.psName.psFirst).GetWidth() +
            pDC->GetTextExtent(tm.team.pl.psName.psLast).GetWidth() +
            pDC->GetTextExtent(tm.team.pl.naName).GetWidth();

      cxB = pDC->GetTextExtent(tm.team.bd.psName.psFirst).GetWidth() +
            pDC->GetTextExtent(tm.team.bd.psName.psLast).GetWidth() +
            pDC->GetTextExtent(tm.team.bd.naName).GetWidth();

      break;

    case CP_TEAM :
      cxA = pDC->GetTextExtent(tm.team.tm.tmName).GetWidth() +
            pDC->GetTextExtent(tm.team.tm.tmDesc).GetWidth() +
            pDC->GetTextExtent(tm.team.tm.naName).GetWidth();

      cxB = 0;

      break;

    case CP_GROUP :

      if (!tm.team.gr.grID)
      {
        cxA = pDC->GetTextExtent(_("T.B.D.")).GetWidth();
        cxB = 0;
      } 
      else
      {
        cxA = pDC->GetTextExtent(tm.team.gr.grName).GetWidth() +
              pDC->GetTextExtent(tm.team.gr.grDesc).GetWidth() +
              pDC->GetTextExtent("999").GetWidth();

        cxB = 0;
      }

      break;

    default :
      return 0;
  }

  return std::max(cxA, cxB) + 6 * cW + 6 * offset;  // ", " (2) + StartNr (4) + Offsets
}