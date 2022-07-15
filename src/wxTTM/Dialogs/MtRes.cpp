/* Copyright (C) 2020 Christoph Theis */

// MtRes.cpp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "MtRes.h"

#include "StEntryStore.h"
#include "MtEntryStore.h"

#include "CpItem.h"
#include "GrItem.h"
#include "TmItem.h"

#include "InfoSystem.h"

#include  <sstream>


// -----------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS(CMtRes, CFormViewEx)

BEGIN_EVENT_TABLE(CMtRes, CFormViewEx)
  EVT_CHECKBOX(XRCID("WalkOverA"), CMtRes::OnWalkOverA)
  EVT_CHECKBOX(XRCID("InjuredA"), CMtRes::OnInjuredA)
  EVT_CHECKBOX(XRCID("DisqualifiedA"), CMtRes::OnDisqualifiedA)
  EVT_CHECKBOX(XRCID("WalkOverX"), CMtRes::OnWalkOverX)
  EVT_CHECKBOX(XRCID("InjuredX"), CMtRes::OnInjuredX)
  EVT_CHECKBOX(XRCID("DisqualifiedX"), CMtRes::OnDisqualifiedX)
  EVT_BUTTON(XRCID("Swap"), CMtRes::OnSwap)
END_EVENT_TABLE()

// -----------------------------------------------------------------------
#define MAX_SET 9

long CMtRes::idcList[MAX_SET] = {wxID_ANY, wxID_ANY, wxID_ANY, wxID_ANY, wxID_ANY, wxID_ANY, wxID_ANY, wxID_ANY, wxID_ANY};

// =======================================================================

class ResultListItem : public ListItem
{
  public:
    void DrawItem(wxDC *pDC, wxRect &rect) {DrawStringCentered(pDC, rect, m_label);}
};


// void AFXAPI DDX_Set(CDataExchange* pDX, int nIDC, short &resa, short &resx);

class CSetValidator : public wxValidator
{
  public:
    CSetValidator() : wxValidator(), m_mtResA(NULL), m_mtResX(NULL), m_ptsToWin(11), m_ptsAhead(2) {}
    CSetValidator(short *mtResA, short *mtResX, short win, short ahead) : wxValidator(), m_mtResA(mtResA), m_mtResX(mtResX), m_ptsToWin(win), m_ptsAhead(ahead) {}
    CSetValidator(const CSetValidator &val) : wxValidator(), m_mtResA(val.m_mtResA), m_mtResX(val.m_mtResX), m_ptsToWin(val.m_ptsToWin), m_ptsAhead(val.m_ptsAhead) {}
    
  public:
    wxObject *Clone() const {return new CSetValidator(*this);}
    
    bool TransferFromWindow()
    {
      if ( m_validatorWindow->IsKindOf(CLASSINFO(CItemCtrl)) )
        return true;
      else if ( !m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)) )
        return false;
        
      wxString text;
      text = ((wxTextCtrl *) m_validatorWindow)->GetValue();
        
      if ( !ConvertFromText(text, *m_mtResA, *m_mtResX, m_ptsToWin, m_ptsAhead) ||
           (std::max(*m_mtResA, *m_mtResX) > m_ptsToWin) && (abs(*m_mtResA - *m_mtResX) > m_ptsAhead)
         )
      { 
        if (*m_mtResA || *m_mtResX)
        {
          if (!IsSilent())
            infoSystem.Error(_("Wrong result"));
        }
        
        *m_mtResA = *m_mtResX = 0;     
        
        return false;
      } 
         
      return true;
    }
    
    bool TransferToWindow()
    {
      wxString str = wxString::Format("%hd : %hd", *m_mtResA, *m_mtResX);
      
      
      if ( m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)) )
        ((wxTextCtrl *) m_validatorWindow)->SetValue(str);
      else if ( m_validatorWindow->IsKindOf(CLASSINFO(CItemCtrl)) )
      {
        CItemCtrl *itemCtrl = (CItemCtrl *) m_validatorWindow;
        itemCtrl->GetListItem()->SetLabel(str);
        itemCtrl->Refresh();
      }
      
      return true;
    }
    
    bool Validate(wxWindow *)
    {
      return true;
    }    
    
  private:
    short *m_mtResA;
    short *m_mtResX;
    short  m_ptsToWin;  // Points needed to win
    short  m_ptsAhead;  // Points ahead of opponent to win
    
  private:
  // ---------------------------------------------------------------------------------------
  // --  Hilfen, um aus Eingabe ein Ergebnis zu berechnen --

  // Alles, was nicht isdigit ist, ueberlesen
  // Return: 0, falls Ziffer gefunden (ist aktuelle Position)
  static int  SkipNonDigit(std::tistringstream &istr)
  {
    istr >> std::ws;           // Whitespace abtrennen

    while (istr.good())
    {
      wxChar c = istr.peek();
      if (wxIsdigit(c))
        return 0;

      istr >> c >> std::ws;    // Naechstes Zeichen und folg. ws lesen
    }

    // Keine Ziffer gefunden
    return 1;
  }


  // Aus einer einzelnen Zahl ein vollstaendiges Ergebnis berechnen
  // Parm:   Array aus zwei int
  // return: immer 0 (bislang)
  // verwendet #define MINUS_NULL
  static void  CalcOther(short &resA, short &resX, short win, short ahead)
  {
    const short MINUS_NULL = (short) 0x8000;

    if (resA == MINUS_NULL)      // Spieler A hat zu 0 verloren
    {
      resA = 0;
	    resX = win;
    }  
    else if (resA < 0)           // Player A hat verloren
    {
      resA = -resA;
      resX = std::max((short) (resA + ahead), win);
    }
    else                         // Player A hat gewonnen
    {
	    resX = resA;
      resA = std::max((short) (resX + ahead), win);
    }
  }


  // Import Funktion
  // Erlaubte Eingabe: Ein Ergebnis
  //                   Eine Zahl:    > 0: Spieler A hat gewonnen
  //                                 < 0: Spieler A hat verloren
  //                                  +0: Spieler A hat zu 0 gewonnen
  //                                  -0: Spieler A hat zu 0 verloren
  //                   leerer String:     Ergebnis ist 0 : 0
  // Im Fehlerfall bleibt das alte Ergebnis bestehen (?)
  static bool  ConvertFromText(const wxString &text, short &resA, short &resX, short win, short ahead)
  {
    const short MINUS_NULL = (short) 0x8000;

    short set[] = {0,0};       // Ergebnisse

    if (text.Length() == 0)            // leerer String
    {
      resA = resX = 0;
      return true;
    }

    std::tistringstream  istr(text.t_str());    // Stream aufsetzen

    istr >> std::ws;

    if (!istr)
      return false;

    // Zur Unterscheidung von +0 und -0: Naechstes Zeichen lesen
    // Naechstes Zeichen ein '-' ?
    int minus = (istr.peek() == '-');

    // In jedem Fall soll jetzt eine Zahl kommen
    istr >> std::dec >> set[0];

    if (istr.fail())
    {
      if (!IsSilent())
        wxBell();

      return false;
    }

    // set[0] auf MINUS_NULL, wenn == 0
    if (set[0] == 0 && minus)
      set[0] = MINUS_NULL;

    // Alles ueberlesen, bis eine Ziffer kommt;
    // d.h. Keine Fehlerprüfung auf dem Rest
    SkipNonDigit(istr);

    if (!istr.good())              // Kein weiteres Ergebnis
      CalcOther(set[0], set[1], win, ahead);   // Zweites Ergebnis berechnen 
    else if ( !minus && isdigit(istr.peek()) )
      istr >> std::dec >> set[1];   // gab weiteres Ergebnis
    else
      return false;

    resA = set[0];
    resX = set[1];

    return true;
  }



  // Export: wandelt Ergbnis (zwei int) in String um
  static void  ConvertToText(wxString &text, short resA, short resX)
  {
    text = wxString::Format("%2hd : %2hd", resA, resX);
  }
};


// -----------------------------------------------------------------------
// CMtRes

CMtRes::CMtRes() : CFormViewEx(), cpItem(0), grItem(0), tmAItem(0), tmXItem(0), tmWinnerItem(0), mtSetsItem(0)
{
  mtSetList = new MtSet[MAX_SET];
}

CMtRes::~CMtRes()
{
  delete[] mtSetList;
}


bool  CMtRes::Edit(va_list vaList)
{
  long id    = va_arg(vaList, long);
  short mtMS = va_arg(vaList, short);

  mt.SelectById(id);
  mt.Next();
  mt.Close();

  gr.SelectById(mt.mtEvent.grID);
  gr.Next();

  cp.SelectById(gr.cpID);
  cp.Next();

  for (long idx = 0; idx < MAX_SET; idx++)
  {
    mtSetList[idx].mtID  = mt.mtID;
    mtSetList[idx].mtMS  = mtMS;
    mtSetList[idx].mtSet = idx+1;
    mtSetList[idx].mtResA = mtSetList[idx].mtResX = 0;
  }

  // Enable / Disable der Eingaben
  for (int i = 0; i < sizeof(idcList) / sizeof(idcList[0]); i++)
    FindWindow(idcList[i])->Enable(mt.mtBestOf > i);
    
  if (mtMS != 0)
  {
    MtEntryStore  mtEntry;
    mtEntry.SelectByMS(mt, mtMS);
    mtEntry.Next();

    tmA = (true && mtEntry.mt.mtReverse ? mtEntry.tmX : mtEntry.tmA);
    tmX = (true && mtEntry.mt.mtReverse ? mtEntry.tmA : mtEntry.tmX);
  }
  else
  {
    TmEntryStore tm;

    tm.SelectTeamById(mt.tmA, cp.cpType);
    tm.Next();
    tmA = tm;

    tm.SelectTeamById(mt.tmX, cp.cpType);
    tm.Next();
    tmX = tm;
  }

  // Warnung, wenn Spieler nicht bekannt sind
  if (tmA.team.pl.PlRec::plNr == 0 || tmX.team.pl.PlRec::plNr == 0)
  {
    infoSystem.Information(_("Players are unknown or byes"));
  }

  MtMatchStore  tmpMatch(mt, mt.GetConnectionPtr());
  tmpMatch.SelectAll(mtMS);
  tmpMatch.Next();
  mtMatch = tmpMatch;

  MtSetStore tmpSet(mt, mt.GetConnectionPtr());
  tmpSet.SelectAll(mtMS);

  while (tmpSet.Next())
  {
    if (tmpSet.mtSet > 0 && tmpSet.mtSet <= mt.mtBestOf)
      mtSetList[tmpSet.mtSet - 1] = tmpSet;
  }

  cpItem->SetListItem(new CpItem(cp));
  grItem->SetListItem(new GrItem(gr));
  tmAItem->SetListItem(new TmItem(tmA));
  tmXItem->SetListItem(new TmItem(tmX));
  
  // Siegereintrag setzen
  if (mtMatch.mtResA > mt.mtBestOf / 2)
    tmWinnerItem->SetListItem(new TmItem(mt.mtReverse ? tmX : tmA));
  else if (mtMatch.mtResX > mt.mtBestOf / 2)
    tmWinnerItem->SetListItem(new TmItem(mt.mtReverse ? tmA : tmX));
  else
    tmWinnerItem->SetListItem(new TmItem());

  // Validator haengt von mt.mtReverse ab
  for (int i = 0; i < MAX_SET; i++)
  {
    short win = mt.mtBestOf > 1 && i == mt.mtBestOf - 1 ? cp.cpPtsToWinLast : cp.cpPtsToWin;
    short ahd = mt.mtBestOf > 1 && i == mt.mtBestOf - 1 ? cp.cpPtsAheadLast : cp.cpPtsAhead;

    if (mt.mtReverse)
      FindWindow(idcList[i])->SetValidator(CSetValidator(&mtSetList[i].mtResX, &mtSetList[i].mtResA, win, ahd));
    else
      FindWindow(idcList[i])->SetValidator(CSetValidator(&mtSetList[i].mtResA, &mtSetList[i].mtResX, win, ahd));
  }
  
  if (mt.mtReverse)  
    FindWindow("Result")->SetValidator(CSetValidator(&mtMatch.mtResX, &mtMatch.mtResA, 0, 0));
  else
    FindWindow("Result")->SetValidator(CSetValidator(&mtMatch.mtResA, &mtMatch.mtResX, 0, 0));
  
  FindWindow("WalkOverA")->SetValidator(CEnumValidator(mt.mtReverse ? &mtMatch.mtWalkOverX : &mtMatch.mtWalkOverA, 1));
  FindWindow("WalkOverX")->SetValidator(CEnumValidator(mt.mtReverse ? &mtMatch.mtWalkOverA : &mtMatch.mtWalkOverX, 1));
  
  FindWindow("InjuredA")->SetValidator(CEnumValidator(mt.mtReverse ? &mtMatch.mtInjuredX : &mtMatch.mtInjuredA, 1));
  FindWindow("InjuredX")->SetValidator(CEnumValidator(mt.mtReverse ? &mtMatch.mtInjuredA : &mtMatch.mtInjuredX, 1));
  
  FindWindow("DisqualifiedA")->SetValidator(CEnumValidator(mt.mtReverse ? &mtMatch.mtDisqualifiedX : &mtMatch.mtDisqualifiedA, 1));
  FindWindow("DisqualifiedX")->SetValidator(CEnumValidator(mt.mtReverse ? &mtMatch.mtDisqualifiedA : &mtMatch.mtDisqualifiedX, 1));
  
  TransferDataToWindow();

  // Focus auf ersten Satz, wenn Spiel fertig, sonst auf ersten unvollstaendigen Satz
  int idx = (2 * mtMatch.mtResA  > mt.mtBestOf || 2 * mt.mtResX > mt.mtBestOf ? 0 : mtMatch.mtResA + mtMatch.mtResX);

  FindWindow(idcList[idx])->SetFocus();
  ((wxTextCtrl *) FindWindow(idcList[idx]))->SetSelection(0, -1);  

  return true;
}


void  CMtRes::OnOK()
{
  TransferDataFromWindow();

  bool finished = 
    mtMatch.mtResA > mt.mtBestOf / 2 ||
    mtMatch.mtResX > mt.mtBestOf / 2 ||
    mtMatch.mtWalkOverA || mtMatch.mtWalkOverX ||
    mtMatch.mtInjuredA || mtMatch.mtInjuredX ||
    mtMatch.mtDisqualifiedA || mtMatch.mtDisqualifiedX ||
    mt.IsABye() || mt.IsXBye();

  if (!finished)
  {
    if (!infoSystem.Question(_("Match not finished. Store anyway?")))
      return;
  }      
  else
  {
    if (mtMatch.mtResX > mt.mtResA)
    {
      // A hat gewonnen, aber wer hat das w/o
      if (mtMatch.mtWalkOverX || mtMatch.mtInjuredX || mtMatch.mtDisqualifiedX)
      {
        infoSystem.Error(_("Result contradicts w/o, injured, or disqualified"));
        return;
      }
    }
    if (mtMatch.mtResA > mtMatch.mtResX)
    {
      // A hat gewonnen, aber wer hat das w/o
      if (mtMatch.mtWalkOverA || mtMatch.mtInjuredA || mtMatch.mtDisqualifiedA)
      {
        infoSystem.Error(_("Result contradicts w/o, injured, or disqualified"));
        return;
      }
    }
  }

  bool  rollback = false; // Reihenfolge der Veroderung beibehalten!

  TTDbse::instance()->GetDefaultConnection()->StartTransaction();

  rollback |= !mt.UpdateResult(
    mt.mtBestOf, mtSetList, 
    mtMatch.mtWalkOverA, mtMatch.mtWalkOverX, 
    mtMatch.mtInjuredA, mtMatch.mtInjuredX, 
    mtMatch.mtDisqualifiedA, mtMatch.mtDisqualifiedX
  );

  if (cp.cpType != 4)
    rollback |= !mt.UpdateScoreChecked(mt.mtID, finished);

  if (!rollback)
    TTDbse::instance()->GetDefaultConnection()->Commit();
  else
    TTDbse::instance()->GetDefaultConnection()->Rollback();

  CFormViewEx::OnOK();
}


// -----------------------------------------------------------------------
// CMtRes message handlers

void CMtRes::OnInitialUpdate() 
{
  cpItem = XRCCTRL(*this, "Event", CItemCtrl);
  grItem = XRCCTRL(*this, "Group", CItemCtrl);
  tmAItem = XRCCTRL(*this, "TeamA", CItemCtrl);
  tmXItem = XRCCTRL(*this, "TeamX", CItemCtrl);
  tmWinnerItem = XRCCTRL(*this, "Winner", CItemCtrl);
  
	CFormViewEx::OnInitialUpdate();
	
	if (idcList[0] == wxID_ANY)
	{
	  for (int i = 0; i < MAX_SET; i++)
	    idcList[i] = XRCID(wxString::Format("Game%i", i+1).c_str());
	}
	
	for (int i = 0; i < MAX_SET; i++)
	{
	  long id = XRCID(wxString::Format("Label_Game%i", i+1).c_str());
	  wxStaticText *ptr = (wxStaticText *) FindWindow(id);
	  ptr->SetLabel(wxString::Format(ptr->GetLabel(), i+1));
	}
	
	for (int i = 0; i < MAX_SET; i++)
	{
	  FindWindow(idcList[i])->Connect(wxEVT_KILL_FOCUS, wxFocusEventHandler(CMtRes::OnKillFocusSet), NULL, this);
	  FindWindow(idcList[i])->Connect(wxEVT_SET_FOCUS, wxFocusEventHandler(CMtRes::OnSetFocusSet), NULL, this);
      FindWindow(idcList[i])->Connect(wxEVT_CHAR_HOOK, wxCharEventHandler(CMtRes::OnCharHookSet), NULL, this);
	}
	  
	FindWindow(XRCID("BestOf"))->Connect(wxEVT_KILL_FOCUS, wxFocusEventHandler(CMtRes::OnKillFocusBestof), NULL, this);
	
  cpItem->SetItemHeight(1);
  grItem->SetItemHeight(1);
  tmAItem->SetItemHeight(2);
  tmXItem->SetItemHeight(2);
  tmWinnerItem->SetItemHeight(2);
  
  XRCCTRL(*this, "Result", CItemCtrl)->SetMinSize(wxSize(6 * cW, tmAItem->GetMinSize().GetHeight()));  
  XRCCTRL(*this, "Result", CItemCtrl)->SetListItem(new ResultListItem());
  
  FindWindow("MatchNo")->SetValidator(CLongValidator(&mt.mtNr));
  FindWindow("TeamMatch")->SetValidator(CShortValidator(&mtSetList[0].mtMS));
  
  FindWindow("BestOf")->SetValidator(CShortValidator(&mt.mtBestOf));

  Layout();
}


void CMtRes::OnKillFocusSet(wxFocusEvent &evt) 
{
  evt.Skip();
  
  wxTextCtrl *wndPtr = reinterpret_cast<wxTextCtrl *>(FindWindow(evt.GetId()));
  if (!wndPtr || !wndPtr->IsModified())
    return;

  long  idx;
  for (idx = 0; idx < MAX_SET; idx++)
  {
    if (idcList[idx] == evt.GetId())
      break;
  }

  if (idx >= mt.mtBestOf)
    return;

  short oldResA = mtSetList[idx].mtResA;
  short oldResX = mtSetList[idx].mtResX;

  short win = idx == mt.mtBestOf - 1 ? cp.cpPtsToWinLast : cp.cpPtsToWin;
  short ahd = idx == mt.mtBestOf - 1 ? cp.cpPtsAheadLast : cp.cpPtsAhead;
    
  TransferDataFromWindow();

  // Test, ob sich was am Satzergebnis geaendert hat.
  // Es passiert, dass man ein Ergebnis vom Liveticker aendern muss, aber das betrifft oft nur Punkte des Verlierers
  if (oldResA >= win && oldResA > oldResX + 1)
  {
    if (mtSetList[idx].mtResA >= win && mtSetList[idx].mtResA >= mtSetList[idx].mtResX + ahd)
    {
      TransferDataToWindow();
      return;
    }
  }

  if (oldResX >= win && oldResX > oldResA + 1)
  {
    if (mtSetList[idx].mtResX >= win && mtSetList[idx].mtResX >= mtSetList[idx].mtResA + ahd)
    {
      TransferDataToWindow();
      return;
    }
  }

  int lastIdx = idx;
  
  for (idx++; idx < mt.mtBestOf; idx++)
    mtSetList[idx].mtResA = mtSetList[idx].mtResX = 0;

  mtMatch.mtResA = mtMatch.mtResX = 0;
  
  // Fertige Saetze zaehlen
  int i;
  for (i = 0; i < mt.mtBestOf; i++)
  {
    short cw = i == mt.mtBestOf - 1 ? cp.cpPtsToWinLast : cp.cpPtsToWin;
    short ca = i == mt.mtBestOf - 1 ? cp.cpPtsAheadLast : cp.cpPtsAhead;
    
    if (mtSetList[i].mtResA >= cw && mtSetList[i].mtResA >= mtSetList[i].mtResX + ca)
      mtMatch.mtResA++;
    else if (mtSetList[i].mtResX >= cw && mtSetList[i].mtResX >= mtSetList[i].mtResA + ca)
      mtMatch.mtResX++;
    else
      break;
  }
  
  // Und den Rest auf 0 : 0 setzen
  for (; i < mt.mtBestOf; i++)
    mtSetList[i].mtResA = mtSetList[i].mtResX = 0;

  // Siegereintrag setzen
  if (2 * mtMatch.mtResA > mt.mtBestOf)
    tmWinnerItem->SetListItem(new TmItem(mt.mtReverse ? tmX : tmA));
  else if (2 * mtMatch.mtResX > mt.mtBestOf)
    tmWinnerItem->SetListItem(new TmItem(mt.mtReverse ? tmA : tmX));
  else
    tmWinnerItem->SetListItem(new TmItem());

  TransferDataToWindow();
  
  // Enable / Disable Window
  if (2 * mtMatch.mtResA > mt.mtBestOf ||
      2 * mtMatch.mtResX > mt.mtBestOf)
  {
    for (idx = lastIdx + 1; idx < mt.mtBestOf; idx++)
      FindWindow(idcList[idx])->Enable(false);

    FindWindow(wxID_OK)->SetFocus();
  }
  else
  {
    for (idx = lastIdx + 1; idx < mt.mtBestOf; idx++)
      FindWindow(idcList[idx])->Enable(true);
  }

  FindWindow("Winner")->Refresh();
  
  wndPtr->SetModified(false);
}


void CMtRes::OnSetFocusSet(wxFocusEvent &evt) 
{
  evt.Skip();
    
  if (!FindWindow(evt.GetId()) || !FindWindow(evt.GetId())->IsKindOf(CLASSINFO(wxTextCtrl)))
    return;
    
  wxTextCtrl *wndPtr = reinterpret_cast<wxTextCtrl *>(FindWindow(evt.GetId()));
  if (wndPtr)
  {
    if (!wndPtr->IsEnabled())
      FindWindow(wxID_OK)->SetFocus();
    else
      wndPtr->SetSelection(0, -1);    
  }
}


void CMtRes::OnKillFocusBestof(wxFocusEvent &evt)
{
  evt.Skip();
  
  short oldBestOf = mt.mtBestOf;
  
  TransferDataFromWindow();
  
  if (mt.mtBestOf < 1)
    mt.mtBestOf = 1;
  else if (mt.mtBestOf > MAX_SET)
    mt.mtBestOf = MAX_SET;

  for (long idx = mt.mtBestOf; idx < MAX_SET; idx++)
  {
    mtSetList[idx].mtResA = mtSetList[idx].mtResX = 0;
  }
  
  // Enable / Disable der Eingaben
  for (int i = 0; i < MAX_SET; i++)
    FindWindow(idcList[i])->Enable(mt.mtBestOf > i);
    
  TransferDataToWindow();
}


void CMtRes::OnCharHookSet(wxKeyEvent &evt)
{
  // Ein neues Event wird generiert
  evt.Skip();

  if (evt.GetKeyCode() == WXK_RETURN)
  {
     wxCommandEvent cmdEvt(wxEVT_COMMAND_BUTTON_CLICKED, wxID_OK);
     cmdEvt.SetEventObject(FindWindow(wxID_OK));
     wxPostEvent(FindWindow(wxID_OK)->GetEventHandler(), cmdEvt);
  }
}



void CMtRes::OnWalkOverA(wxCommandEvent &) 
{
  OnWalkOver();
}

void CMtRes::OnInjuredA(wxCommandEvent &)
{
  OnInjured();
}

void CMtRes::OnDisqualifiedA(wxCommandEvent &) 
{
  OnDisqualified();
}

void CMtRes::OnWalkOverX(wxCommandEvent &) 
{
  OnWalkOver();
}

void CMtRes::OnInjuredX(wxCommandEvent &)
{
  OnInjured();
}

void CMtRes::OnDisqualifiedX(wxCommandEvent &) 
{
  OnDisqualified();
}


void CMtRes::OnWalkOver()
{
  TransferDataFromWindow();

  int  winnerAX = 0;
  if (mtMatch.mtResA > mt.mtBestOf / 2)
    winnerAX = +1;
  else if (mtMatch.mtResX > mt.mtBestOf / 2)
    winnerAX = -1;

  int  flag   = mtMatch.mtWalkOverA || mtMatch.mtWalkOverX;

	if ( flag && 
       (mtMatch.mtWalkOverA && winnerAX == +1 ||
				mtMatch.mtWalkOverX && winnerAX == -1) )
  {
		for (int i = 0; i < mt.mtBestOf; i++)
      mtSetList[i].mtResA = mtSetList[i].mtResX = 0;
  }

  mtMatch.mtResA = mtMatch.mtResX = 0;

	// Ergebnis als 0:11, 0:11, ...
	int i;
	for (i = 0; i < mt.mtBestOf; i++)
	{
    short win = (i == mt.mtBestOf - 1 ? cp.cpPtsToWinLast : cp.cpPtsToWin);

    int  ballsA = (mtMatch.mtWalkOverX && !mtMatch.mtWalkOverA ? win : 0);
    int  ballsX = (mtMatch.mtWalkOverA && !mtMatch.mtWalkOverX ? win : 0);

		// Eindhoven: Auch abgebrochene Spiele zaehlen als WalkOver
		// Wenn der Sieger schliesslich feststeht, abbrechen.
		if (!mtSetList[i].mtResA && !mtSetList[i].mtResX)
		{
			mtSetList[i].mtResA = ballsA;
			mtSetList[i].mtResX = ballsX;
		}

		// Sind wir schon fertig ?
		// mt.CalcSets();
		if (mtSetList[i].mtResA > mtSetList[i].mtResX)
			mtMatch.mtResA++;
		else if (mtSetList[i].mtResX > mtSetList[i].mtResA)
			mtMatch.mtResX++;

		if ( (mtMatch.mtResA > mt.mtBestOf / 2) ||
				 (mtMatch.mtResX > mt.mtBestOf / 2) )
			break;
	}

	for (i++; i < mt.mtBestOf; i++)
	{
	  mtSetList[i].mtResA = 0;
	  mtSetList[i].mtResX = 0;
	 // fdSet[i]->FdSetNonSelectable(flag);
	}

  if (mtMatch.mtResA > mt.mtBestOf / 2)
    tmWinnerItem->SetListItem(new TmItem(mt.mtReverse ? tmX : tmA));
  else if (mtMatch.mtResX > mt.mtBestOf / 2)
    tmWinnerItem->SetListItem(new TmItem(mt.mtReverse ? tmA : tmX));
  else
    tmWinnerItem->SetListItem(new TmItem());

  FindWindow("Winner")->Refresh();

  TransferDataToWindow();
}


void CMtRes::OnInjured()
{
  TransferDataFromWindow();

  int  winnerAX = 0;
  if (mtMatch.mtResA > mt.mtBestOf / 2)
    winnerAX = +1;
  else if (mtMatch.mtResX > mt.mtBestOf / 2)
    winnerAX = -1;

  int  flag   = mtMatch.mtInjuredA || mtMatch.mtInjuredX;

	if ( flag && 
       (mtMatch.mtInjuredA && winnerAX == +1 ||
				mtMatch.mtInjuredX && winnerAX == -1) )
  {
		for (int i = 0; i < mt.mtBestOf; i++)
      mtSetList[i].mtResA = mtSetList[i].mtResX = 0;
  }

  mtMatch.mtResA = mtMatch.mtResX = 0;

	// Ergebnis als 0:11, 0:11, ...
	int i;
	for (i = 0; i < mt.mtBestOf; i++)
	{
    short win = (i == mt.mtBestOf - 1 ? cp.cpPtsToWinLast : cp.cpPtsToWin);

    int  ballsA = (mtMatch.mtInjuredX && !mtMatch.mtInjuredA ? win : 0);
    int  ballsX = (mtMatch.mtInjuredA && !mtMatch.mtInjuredX ? win : 0);

		// Eindhoven: Auch abgebrochene Spiele zaehlen als WalkOver
		// Wenn der Sieger schliesslich feststeht, abbrechen.
		if (!mtSetList[i].mtResA && !mtSetList[i].mtResX)
		{
			mtSetList[i].mtResA = ballsA;
			mtSetList[i].mtResX = ballsX;
		}

		// Sind wir schon fertig ?
		// mt.CalcSets();
		if (mtSetList[i].mtResA > mtSetList[i].mtResX)
			mtMatch.mtResA++;
		else if (mtSetList[i].mtResX > mtSetList[i].mtResA)
			mtMatch.mtResX++;

		if ( (2 * mtMatch.mtResA > mt.mtBestOf) ||
				 (2 * mtMatch.mtResX > mt.mtBestOf) )
			break;
	}

	for (i++; i < mt.mtBestOf; i++)
	{
	  mtSetList[i].mtResA = 0;
	  mtSetList[i].mtResX = 0;
	  // fdSet[i]->FdSetNonSelectable(flag);
	}

  if (2 * mtMatch.mtResA > mt.mtBestOf)
    tmWinnerItem->SetListItem(new TmItem(mt.mtReverse ? tmX : tmA));
  else if (2 * mtMatch.mtResX > mt.mtBestOf)
    tmWinnerItem->SetListItem(new TmItem(mt.mtReverse ? tmA : tmX));
  else
    tmWinnerItem->SetListItem(new TmItem());

  FindWindow("Winner")->Refresh();

  TransferDataToWindow();
}


void CMtRes::OnDisqualified()
{
  TransferDataFromWindow();

  if (mtMatch.mtDisqualifiedA && mtMatch.mtDisqualifiedX)
    tmWinnerItem->SetListItem(new TmItem());
  else if (mtMatch.mtDisqualifiedA)
    tmWinnerItem->SetListItem(new TmItem(tmX));
  else if (mtMatch.mtDisqualifiedX)
    tmWinnerItem->SetListItem(new TmItem(tmA));
  else if (2 * (mt.mtReverse ? mt.mtResX : mt.mtResA) > mt.mtBestOf)
    tmWinnerItem->SetListItem(new TmItem(tmA));
  else if (2 * (mt.mtReverse ? mt.mtResA : mt.mtResX) > mt.mtBestOf)
    tmWinnerItem->SetListItem(new TmItem(tmX));
  else
    tmWinnerItem->SetListItem(new TmItem());

#if 0
  for (int i = 0; i < sizeof(idcList) / sizeof(idcList[0]); i++)
    FindWindow(idcList[i])->Enable(!(mtDisqualifiedA || mtDisqualifiedX));
  
  FindWindow("Result")->Enable(!(mtDisqualifiedA || mtDisqualifiedX));
#else
  if (mtMatch.mtDisqualifiedA || mtMatch.mtDisqualifiedX)
  {
    for (int i = 0; i < MAX_SET; i++)
    {
      mtSetList[i].mtResA = 0;
      mtSetList[i].mtResX = 0;
    }

    mtMatch.mtResA = 0;
    mtMatch.mtResX = 0;
  }

  if (mtMatch.mtDisqualifiedA ^ mtMatch.mtDisqualifiedX)
  {
    for (int i = 0; i < (mt.mtBestOf + 1) / 2; i++)
    {
      short win = (i == mt.mtBestOf - 1 ? cp.cpPtsToWinLast : cp.cpPtsToWin);

      mtSetList[i].mtResA = mtMatch.mtDisqualifiedA ? 0 : win;
      mtSetList[i].mtResX = mtMatch.mtDisqualifiedX ? 0 : win;
    }

    mtMatch.mtResA = mtMatch.mtDisqualifiedA ? 0 : (mt.mtBestOf + 1) / 2;
    mtMatch.mtResX = mtMatch.mtDisqualifiedX ? 0 : (mt.mtBestOf + 1) / 2;
  }

#endif

  FindWindow("Winner")->Refresh();

  TransferDataToWindow();
}


// Swap result
void CMtRes::OnSwap(wxCommandEvent &)
{
  TransferDataFromWindow();

  std::swap(mtMatch.mtWalkOverA, mtMatch.mtWalkOverX);
  std::swap(mtMatch.mtDisqualifiedA, mtMatch.mtDisqualifiedX);
  std::swap(mtMatch.mtInjuredA, mtMatch.mtInjuredX);
  std::swap(mtMatch.mtResA, mtMatch.mtResX);

  for (int idx = 0; idx < MAX_SET; idx++)
    std::swap(mtSetList[idx].mtResA, mtSetList[idx].mtResX);

  TransferDataToWindow();
}

