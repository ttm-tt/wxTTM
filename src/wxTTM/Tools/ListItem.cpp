/* Copyright (C) 2020 Christoph Theis */

// Basisklasse fuer Items in ListCtrl(Ex)

#include  "stdafx.h"

#include  "ListItem.h"

#include  "TmEntryStore.h"
#include  "Rec.h"


// Abstand zwischen den Raendern des Rechtecks zum Text
unsigned  ListItem::offset = 4;

// naNameWidth startet mit 2, bei jedem Listeneintrag wird aber ermittelt, 
// ob ein Nationenkuerzel nicht breiter ist. Irgendwann ist naNameWidth
// dann die Mindestbreite, die erforderlich ist, um alle Nationenkuerzel
// ohne kuerzen darstellen zu muessen. 
// Wechselt man da Turnier, bliebe naNameWidth erhalten, aber das kann man
// in Kauf nehmen. Ebenso, dass evtl. am Anfang einmal falsch gezeichnet wird.
unsigned  ListItem::naNameWidth = 2;

unsigned  ListItem::plNrWidth = 2;

unsigned  ListItem::tmNameWidth = 2;

int ListItem::CompareFunction(const ListItem *item1, const ListItem *item2, int col)
{
  if (!item1)
    return (item2 ? +1 : 0);
  
  if (!item2)
    return (item1 ? -1 : 0);
    
  return item1->Compare(item2, col);
}


ListItem::~ListItem()
{
}


// -----------------------------------------------------------------------
// Vergleich zweiter Items
int  ListItem::Compare(const ListItem *itemPtr, int col) const
{
  return m_id - itemPtr->m_id;
}


// -----------------------------------------------------------------------
// Inkrementelle Suche
bool  ListItem::HasString(const wxString &str) const
{
  return false;
}


// -----------------------------------------------------------------------
void  ListItem::DrawString(wxDC *pDC, const wxRect &rect, const wxString &str, unsigned fmt)
{
  if (rect.GetLeft() >= rect.GetRight())
    return;
    
  // fmt |=  DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP | DT_VCENTER;

  wxArrayString tmp = wxStringTokenize(str, "\n");

  if (tmp.GetCount() == 0)
    tmp.Add("");

  int lH = rect.GetHeight() / tmp.GetCount();

  for (unsigned i = 0; i < tmp.GetCount(); i++)
  {
    wxRect line = rect;
    line.SetTop (rect.GetTop() + i * lH);
    line.SetHeight(lH);

    wxString shortStr = MakeShortString(pDC, tmp[i], line);

    pDC->DrawLabel(shortStr, line, fmt | wxALIGN_CENTER_VERTICAL);
  }
}


// -----------------------------------------------------------------------
void ListItem::DrawImage(wxDC *pDC, const wxRect &rect, const wxBitmap &img)
{
  pDC->DrawLabel(wxEmptyString, img, rect, wxALIGN_CENTER);
}


// -----------------------------------------------------------------------
void  ListItem::DrawPlayer(wxDC *pDC, const wxRect &rect, const PlRec &pl, bool showNaName)
{
  if (!pl.plNr)
    return;
  
  wxRect rcNr    = rect; // StartNr
  wxRect rcName  = rect; // Spielername
  wxRect rcAssoc = rect; // Verband

  unsigned cW = pDC->GetTextExtent("M").GetWidth();
  unsigned needed;

  needed = (unsigned) pDC->GetTextExtent(wxString::Format("%d", pl.plNr)).GetWidth();
  needed += 2 * offset;

  plNrWidth = std::max(plNrWidth, (needed + cW - 1) / cW);

  rcNr.SetRight(rcNr.GetLeft() + plNrWidth * cW);
  rcName.SetLeft(rcNr.GetRight());

  if (showNaName)
  {
    if (*pl.naName)
    {
      // Die minimale Breite (in Einheiten von cw) ermitteln, die fuer den Verband noetig ist.
      needed = (unsigned) pDC->GetTextExtent(pl.naName).GetWidth();
      needed += 2 * offset;
            
      naNameWidth = std::max(naNameWidth, (needed + cW - 1) / cW);
    }
      
    rcAssoc.SetLeft(rcAssoc.GetRight() - naNameWidth * cW);
    rcName.SetRight(rcAssoc.GetLeft());
  }

  rcNr.SetLeft(rcNr.GetLeft() + offset);
  rcNr.SetRight(rcNr.GetRight() - offset);
  rcName.SetLeft(rcName.GetLeft() + offset);
  rcName.SetRight(rcName.GetRight() - offset);
  rcAssoc.SetLeft(rcAssoc.GetLeft() + offset);
  rcAssoc.SetRight(rcAssoc.GetRight() - offset);

  wxChar  buf[130];
  if (!*pl.psName.psFirst || 
      wxStrlen(pl.psName.psFirst) == 1 && wxIsspace(*pl.psName.psFirst))
    wxSprintf(buf, "%s", pl.psName.psLast);
  else
    wxSprintf(buf, "%s, %s", pl.psName.psLast, pl.psName.psFirst);
  
  if (pl.plNr)
    DrawLong(pDC, rcNr, pl.plNr);
  if (*pl.psName.psLast)
  {
    DrawString(pDC, rcName, buf);
    if (showNaName)
      DrawString(pDC, rcAssoc, pl.naName);
  }
}


void  ListItem::DrawTeam(wxDC *pDC, const wxRect &rect, const TmTeam &tm, bool showNaName)
{
  if (!*tm.tmName && !*tm.tmDesc)
    return;
  
  wxRect rcName  = rect; // Abkuerzung
  wxRect rcDesc  = rect; // Name
  wxRect rcAssoc = rect; // Verband

  unsigned cW = pDC->GetTextExtent("M").GetWidth();

  // Die minimale Breite (in Einheiten von cw) ermitteln, die fuer den Verband noetig ist.
  if (*tm.naName)
  {
    int needed = pDC->GetTextExtent(tm.naName).GetWidth();
    needed += 2 * offset;
    
    naNameWidth = std::max(naNameWidth, (needed + cW - 1) / cW);
  }

  // Breite fuer das Mannschaftskuerzel berechnen
  if (*tm.tmName)
  {
    int needed = pDC->GetTextExtent(tm.tmName).GetWidth();
    needed += 2 * offset;
    
    tmNameWidth = std::max(tmNameWidth, (needed + cW - 1) / cW);
  }

  // Die gleiche Breite fuer das Mannschaftskuerzel annehmen.
  rcName.SetRight(rcName.GetLeft() + tmNameWidth * cW);
  rcDesc.SetLeft(rcName.GetRight());

  if (showNaName)
  {
    rcAssoc.SetLeft(rcAssoc.GetRight() - naNameWidth * cW);
    rcAssoc.SetRight(rect.GetRight());

    rcDesc.SetRight(rcAssoc.GetLeft());
  }

  rcName.SetLeft(rcName.GetLeft() + offset);
  rcName.SetRight(rcName.GetRight() - offset);
  rcDesc.SetLeft(rcDesc.GetLeft() + offset);
  rcDesc.SetRight(rcDesc.GetRight() - offset);
  rcAssoc.SetLeft(rcAssoc.GetLeft() + offset);
  rcAssoc.SetRight(rcAssoc.GetRight() - offset);

  DrawString(pDC, rcName, tm.tmName);
  DrawString(pDC, rcDesc, tm.tmDesc);
  if (showNaName)
    DrawString(pDC, rcAssoc, tm.naName);
}


void  ListItem::DrawGroup(wxDC *pDC, const wxRect &rect, const TmGroup &gr)
{
  if (!gr.grID)
  {
    DrawStringCentered(pDC, rect, _("T.B.D."));
    return;
  }

  wxRect rcName = rect; // Group name
  wxRect rcDesc = rect; // Group desc
  wxRect rcPos  = rect; // Position

  unsigned cW = pDC->GetTextExtent("M").GetWidth();
  
  // Die minimale Breite (in Einheiten von cw) ermitteln, die fuer den Verband noetig ist.
  if (*gr.grName)
  {
    int needed = pDC->GetTextExtent(gr.grName).GetWidth();
    needed += 2 * offset;
    
    naNameWidth = std::max(naNameWidth, (needed + cW - 1) / cW);
  }
  
  rcDesc.SetLeft(rcName.GetLeft() + naNameWidth * cW);
  rcName.SetRight(rcDesc.GetLeft());

  rcPos.SetLeft(rcPos.GetRight() - naNameWidth * cW);
  rcDesc.SetRight(rcPos.GetLeft());

  rcName.SetLeft(rcName.GetLeft() + offset);
  rcName.SetRight(rcName.GetRight() - offset);
  rcDesc.SetLeft(rcDesc.GetLeft() + offset);
  rcDesc.SetRight(rcDesc.GetRight() - offset);
  rcPos.SetLeft(rcPos.GetLeft() + offset);
  rcPos.SetRight(rcPos.GetRight() - offset);

  DrawString(pDC, rcName, gr.grName);
  DrawString(pDC, rcDesc, gr.grDesc);

  if (gr.grPos)
    DrawLong(pDC, rcPos, gr.grPos, wxALIGN_LEFT);
}


void  ListItem::DrawEntry(wxDC *pDC, const wxRect &rect, const TmEntry &tm, bool showNaName)
{
  switch (tm.team.cpType)
  {
    case CP_SINGLE :
      DrawPlayer(pDC, rect, tm.team.pl, showNaName);
      break;

    case CP_DOUBLE :
    case CP_MIXED :
    {
      wxRect  tmp = rect;
      tmp.SetBottom(rect.GetTop() + rect.GetHeight() / 2);
      DrawPlayer(pDC, tmp, tm.team.pl, showNaName);
      tmp.SetTop(tmp.GetBottom());
      tmp.SetBottom(rect.GetBottom());
      DrawPlayer(pDC, tmp, tm.team.bd, showNaName);
      break;
    }

    case CP_TEAM :
      DrawTeam(pDC, rect, tm.team.tm, showNaName);
      break;

    case CP_GROUP :
      DrawGroup(pDC, rect, tm.team.gr);
      break;

    default :
      break;
  }
}


void  ListItem::DrawResult(wxDC *pDC, const wxRect &rect, short resA, short resX)
{
  wxRect  rc = rect;
  rc.SetLeft(rc.GetLeft() + offset);
  rc.SetRight(rc.GetRight() - offset);

  wxChar  buf[10];
  wxSprintf(buf, "%2i : %2i", resA, resX);
  DrawString(pDC, rc, buf);
}


// -----------------------------------------------------------------------
// String verkuerzen, bis er in das Rechteck passt
wxString ListItem::MakeShortString(wxDC *pDC, const wxString &str, const wxRect &rect)
{
	static const wxChar strDots[] = wxT("...");
  static wxChar strShort[MAX_PATH];
  static wxChar strNull[] = wxT("");
  
  if (rect.GetLeft() >= rect.GetRight())
    return strNull;

  int len = wxStrlen(str);

	if( len == 0 || pDC->GetTextExtent(str).GetWidth() <= rect.GetWidth() )
	{
		return str;
	}

	wxStrncpy(strShort, str, MAX_PATH);
	strShort[MAX_PATH-1] = 0;
	if (len > MAX_PATH-1)
	  len = MAX_PATH-1;
	
	int nAddLen = pDC->GetTextExtent(strDots).GetWidth();

	for(int i = len - 1; i > 0; i--)
	{
		strShort[i] = wxChar();
		if ( (pDC->GetTextExtent(strShort).GetWidth() + nAddLen) <= rect.GetWidth() )
		{
			break;
		}
	}

	wxStrcat(strShort, strDots);
	return strShort;
}


// -----------------------------------------------------------------------
void ListItem::MeasureItem(LPMEASUREITEMSTRUCT lpMIS) 
{
  return;
}


bool ListItem::DrawSeparatingLine()
{
  return false;
}


// -----------------------------------------------------------------------
wxString ListItem::BeginEditColumn(int col) const
{
  return wxString();
}


void ListItem::EndEditColumn(int col, const wxString &value)
{
  // Nothing
}