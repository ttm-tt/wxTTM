/* Copyright (C) 2020 Christoph Theis */

// MtTeam.cpp : implementation file
// TODO: Update von Winner, wenn ein Ergebnis vorliegt

#include "stdafx.h"
#include "TT32App.h"
#include "MtTeam.h"

// #include "NmList.h"
#include "MtRes.h"

// #include "Score.h"

#include "CpItem.h"
#include "GrItem.h"
#include "TmItem.h"
#include "MtItem.h"

#include "StEntryStore.h"
#include "MtEntryStore.h"

#include "InfoSystem.h"
#include "Request.h"


IMPLEMENT_DYNAMIC_CLASS(CMtTeam, CFormViewEx)


BEGIN_EVENT_TABLE(CMtTeam, CFormViewEx)
  EVT_CHECKBOX(XRCID("WalkOverA"), CMtTeam::OnWalkOverA)
  EVT_CHECKBOX(XRCID("WalkOverX"), CMtTeam::OnWalkOverX)
  EVT_BUTTON(XRCID("Reverse"), CMtTeam::OnMtReverse)
  EVT_BUTTON(XRCID("ScoreSheet"), CMtTeam::OnScore)
END_EVENT_TABLE()



class ResultListItem : public ListItem
{
  public:
    ResultListItem(const wxString &str) : ListItem(0, str, wxID_ANY) {}
        
  public:
    void DrawItem(wxDC *pDC, wxRect &rect) {DrawStringCentered(pDC, rect, m_label);}
};


// =======================================================================
// CMtTeam

CMtTeam::CMtTeam() 
       : CFormViewEx(), cpItem(NULL), grItem(NULL), tmAItem(NULL), tmXItem(NULL), tmWinnerItem(NULL), mtList(NULL)
{
	m_woa = 0;
	m_wox = 0;
}


CMtTeam::~CMtTeam()
{
}


bool  CMtTeam::Edit(va_list vaList)
{
  long id = va_arg(vaList, long);

  mt.SelectById(id);
  mt.Next();
  mt.Close();  // XXX Hack

  m_woa = mt.mtWalkOverA;
  m_wox = mt.mtWalkOverX;

  gr.SelectById(mt.mtEvent.grID);
  gr.Next();

  sy.SelectById(gr.syID);
  sy.Next();

  cp.SelectById(gr.cpID);
  cp.Next();

  StEntryStore  st;
  st.SelectById(mt.stA, cp.cpType);
  st.Next();
  tmA = st;

  st.SelectById(mt.stX, cp.cpType);
  st.Next();
  tmX = st;

  mtList->SetItemHeight(2);

  cpItem->SetListItem(new CpItem(cp));
  grItem->SetListItem(new GrItem(gr));
  
  // Die Mannschaften muessen bei mtReverse vertauscht ausgegeben werden
  // Die Sieger (s.u.) jedoch nicht, weil das Ergebnis "richtig" ist:
  // mtResA gehoert zu tmA, mtResX gehoert zu tmX. Nur die Einzelspiele 
  // werden vertauscht.
  tmAItem->SetListItem(new TmItem(mt.mtReverse ? tmX : tmA));
  tmXItem->SetListItem(new TmItem(mt.mtReverse ? tmA : tmX));
  
  switch (mt.QryWinnerAX())
  {
    case +1 :
      tmWinnerItem->SetListItem(new TmItem(tmA));
      break;

    case -1 :
      tmWinnerItem->SetListItem(new TmItem(tmX));
      break;

    default :
      if (mt.IsFinished())
      {
        // Ein Team mit "TIE" faelschen
        wxString tmp = _("Tie");

        TmItem *itemPtr = new TmItem();
        itemPtr->entry.team.cpType = CP_TEAM;
        wxStrcpy(itemPtr->entry.team.tm.tmDesc, tmp);
        
        tmWinnerItem->SetListItem(itemPtr);
      }
      else
      {
        tmWinnerItem->SetListItem(new TmItem());
      }
      break;
  }

  // Liste ausgrauen, wenn WalkOver gesetzt ist
  FindWindow("MtList")->Enable(!mt.mtWalkOverA && !mt.mtWalkOverX);
  
  // Merker, ob eine Aufstellung existiert
  bool hasNomination = false;

  // Spieleliste
  MtEntryStore  mtEntry;
  mtEntry.SelectByMS(mt);
  while (mtEntry.Next())
  {
    hasNomination |= mtEntry.tmA.team.pl.plNr != 0;
    hasNomination |= mtEntry.tmX.team.pl.plNr != 0;
    
    mtList->AddListItem(new MtItem(mtEntry, false));
  } 
   
  // Kein Vertauschen, wenn Ergebnisse und Aufstellung existieren  
  hasNomination &= (mt.mtResA  + mt.mtResX) != 0;
  
  // GetDlgItem(IDC_MTTEAM_REVERSE)->EnableWindow(hasNomination ? FALSE : TRUE);
  
  mtList->SetCurrentIndex(0);

  mtList->ResizeColumn();

  // WalkOver setzen
  FindWindow("WalkOverA")->SetValidator(CEnumValidator(mt.mtReverse ? &m_wox : &m_woa, 1));
  FindWindow("WalkOverX")->SetValidator(CEnumValidator(mt.mtReverse ? &m_woa : &m_wox, 1));

  TransferDataToWindow();
  
  Refresh();
  
  return true;
}


void  CMtTeam::OnOK()
{
  TransferDataFromWindow();
  
  bool commit = true;
  
  mt.GetConnectionPtr()->StartTransaction();

  if (m_woa != mt.mtWalkOverA || m_wox != mt.mtWalkOverX)
  {
    mt.mtWalkOverA = m_woa;
    mt.mtWalkOverX = m_wox;
    
    // Verbliebene Einzelspiele auffuellen
    MtSet *mtSetList = new MtSet[mt.mtBestOf];
    int i;
    for (i = 0; i <= mt.mtBestOf / 2; i++)
    {
      mtSetList[i].mtID = mt.mtID;
      mtSetList[i].mtSet = i+1;
      mtSetList[i].mtResA = (mt.mtWalkOverX ? MIN_POINTS : 0);
      mtSetList[i].mtResX = (mt.mtWalkOverA ? MIN_POINTS : 0);
    }
    
    for (; i < mt.mtBestOf; i++)
    {
      mtSetList[i].mtID = mt.mtID;
      mtSetList[i].mtSet = i+1;
    }
    
    int resA = 0, resX = 0;
    for (int ms = 1; ms <= sy.syMatches; ms++)
    {
      MtMatchStore mtMatch(mt, mt.GetConnectionPtr());
      mtMatch.SelectAll(ms);
      mtMatch.Next();
      mtMatch.Close();
      
      if (resA < mt.mtResA && mtMatch.mtResA > mt.mtBestOf / 2)
        resA++;
      else if (resX < mt.mtResX && mtMatch.mtResX > mt.mtBestOf / 2)
        resX++;
      else
      {
        for (int i = 0; i < mt.mtBestOf; i++)
          mtSetList[i].mtMS = ms;
          
        if ( !(commit = mt.UpdateResult(mt.mtBestOf, mtSetList, mt.mtWalkOverA, mt.mtWalkOverX)) )
          break;
        
        mt.mtWalkOverA ? resX++ : resA++;
      }
      
      // Abruch, wenn der Sieger feststeht
      if (resA > sy.syMatches / 2 || resX > sy.syMatches / 2)
        break;
    }
    
    delete[] mtSetList;
  }

  if (mt.mtWalkOverA && 2 * mt.mtResA > mt.mtMatches)
  {
    infoSystem.Error(_("Result contradicts w/o"));
    return;
  }

  if (mt.mtWalkOverX && 2 * mt.mtResX > mt.mtMatches)
  {
    infoSystem.Error(_("Result contradicts w/o"));
    return;
  }

  commit &= mt.UpdateWalkOver();
  
  // if (commit)
  //   commit = mt.UpdateReverseFlag();

  commit &= mt.UpdateScoreChecked(mt.mtID, mt.IsFinished());
    
  if (commit)
    mt.GetConnectionPtr()->Commit();
  else
    mt.GetConnectionPtr()->Rollback();
    
  CTT32App::CommitChanges();

  CFormViewEx::OnOK();
}


// -----------------------------------------------------------------------

void CMtTeam::OnInitialUpdate() 
{
	CFormViewEx::OnInitialUpdate();
	
	cpItem = XRCCTRL(*this, "Event", CItemCtrl);
	grItem = XRCCTRL(*this, "Group", CItemCtrl);
	tmAItem = XRCCTRL(*this, "TeamA", CItemCtrl);
	tmXItem = XRCCTRL(*this, "TeamX", CItemCtrl);
	tmWinnerItem = XRCCTRL(*this, "Winner", CItemCtrl);
	mtList = XRCCTRL(*this, "MtList", CListCtrlEx);
	
  // Match No
  mtList->InsertColumn(0, _("Mt.Nr"));

  // Spieler / Teams
  mtList->InsertColumn(1, _("Players"), wxALIGN_CENTER);

  // Ergebnis
  mtList->InsertColumn(2, _("Result"));

  mtList->SetColumnWidth(0, 6*cW);
  mtList->SetColumnWidth(2, 7*cW);
  mtList->SetResizeColumn(1);
  
  FindWindow("TeamA")->Connect(wxEVT_LEFT_DCLICK, wxMouseEventHandler(CMtTeam::OnNominationA), NULL, this); 
  FindWindow("TeamX")->Connect(wxEVT_LEFT_DCLICK, wxMouseEventHandler(CMtTeam::OnNominationX), NULL, this); 
  
  FindWindow("MatchNo")->SetValidator(CLongValidator(&mt.mtNr));
  
  // ((wxButton *) FindWindow(wxID_OK))->SetDefault();
}


void CMtTeam::OnWalkOverA(wxCommandEvent &)
{
  OnWalkOver(mt.mtReverse ? -1 : +1);
}


void CMtTeam::OnWalkOverX(wxCommandEvent &)
{
  OnWalkOver(mt.mtReverse ? +1 : -1);
}


void CMtTeam::OnWalkOver(int ax)
{
  TransferDataFromWindow();
  
  mtList->Enable(m_woa == 0 && m_wox == 0);
  
  if (m_woa)
    mt.mtResX = (sy.syMatches + 1) / 2;
  else if (m_wox)
    mt.mtResA = (sy.syMatches + 1) / 2;
  else
    mt.mtResA = mt.mtResX = 0;
    
  if (mt.QryWinnerAX() == 1)
    tmWinnerItem->SetListItem(new TmItem(tmA));
  else if (mt.QryWinnerAX() == -1)
    tmWinnerItem->SetListItem(new TmItem(tmX));
  else if (mt.IsFinished())
  {
    // Ein Team mit "TIE" faelschen
    wxString tmp = _("Tie");

    TmItem *itemPtr = new TmItem();
    itemPtr->entry.team.cpType = CP_TEAM;
    wxStrcpy(itemPtr->entry.team.tm.tmDesc, tmp);
    
    tmWinnerItem->SetListItem(itemPtr);
  }
  else
    tmWinnerItem->SetListItem(new TmItem());
  
  TransferDataToWindow();
  
  Refresh();
}


void CMtTeam::OnNominationA(wxMouseEvent &) 
{
  long tmID = tmAItem->GetListItem()->GetID();
  if (tmID)
    OnNomination(tmID, -1);
}

void CMtTeam::OnNominationX(wxMouseEvent &) 
{
  long tmID = tmXItem->GetListItem()->GetID();
  if (tmID)
    OnNomination(tmID, +1);
}


void CMtTeam::OnNomination(long tmID, int ax)
{
  if (wxStrcmp(sy.syName, wxT("OTS")) == 0)
    CTT32App::instance()->OpenView(_("Edit Nomination"), wxT("NmEditOTS"), mt.mtID, tmID, ax);
  else if (wxStrcmp(sy.syName, wxT("ETS")) == 0)
    CTT32App::instance()->OpenView(_("Edit Nomination"), wxT("NmEditETS"), mt.mtID, tmID, ax);
  else
    CTT32App::instance()->OpenView(_("Edit Nomination"), wxT("NmEdit"), mt.mtID, tmID, ax);
}


void CMtTeam::OnMtReverse(wxCommandEvent &) 
{
  mt.mtReverse = (mt.mtReverse ? 0 : 1);	

  // In DB speichern? Andernfalls kann man nicht gleich Ergebnisse eingeben
  TTDbse::instance()->GetDefaultConnection()->StartTransaction();

  if ( mt.UpdateReverseFlag() )
    TTDbse::instance()->GetDefaultConnection()->Commit();
  else
    TTDbse::instance()->GetDefaultConnection()->Rollback();

  // WalkOver setzen
  FindWindow("WalkOverA")->SetValidator(CEnumValidator(mt.mtReverse ? &m_wox : &m_woa, 1));
  FindWindow("WalkOverX")->SetValidator(CEnumValidator(mt.mtReverse ? &m_woa : &m_wox, 1));

  tmAItem->SetListItem(new TmItem(mt.mtReverse ? tmX : tmA));
  tmXItem->SetListItem(new TmItem(mt.mtReverse ? tmA : tmX));

  tmAItem->Refresh();
  tmXItem->Refresh();

  CTT32App::CommitChanges();
}


void  CMtTeam::OnEdit()
{
  MtItem *itemPtr = (MtItem *) mtList->GetCurrentItem();
  if (!itemPtr)
    return;
    
  long  idx = mtList->GetCurrentIndex();
  
  if ( sy.syComplete == 0 && mt.QryWinnerAX() && 
       (mt.mtResA + mt.mtResX) <= idx )
  {
    infoSystem.Information(_("Team match is already finished."));
    return;
  }      
           
  MtItem *prevItem = (idx > 0 ? (MtItem *) mtList->GetListItem(idx - 1) : NULL);

  if (prevItem && prevItem->mt.mt.mtResA == 0 && prevItem->mt.mt.mtResX == 0)
  {
    // Voriges Spiel hat noch kein Ergebnis)
    infoSystem.Information(_("Previous team match is not finished"));
  }
  
  idx += 1;
  
  if (idx < mtList->GetItemCount())
  {
    mtList->SetCurrentIndex(idx);    
    mtList->EnsureVisible(idx);
  }

  CTT32App::instance()->OpenView(_("Edit Result"), wxT("MtRes"), itemPtr->GetID(), itemPtr->mt.mt.mtEvent.mtMS);  
}


// -----------------------------------------------------------------------
void CMtTeam::OnUpdate(CRequest *reqPtr) 
{
  if (!reqPtr)
    return;

  if (reqPtr->rec != CRequest::MTREC)
    return;

  switch (reqPtr->type)
  {
    case CRequest::UPDATE :
    case CRequest::UPDATE_REVERSE :
    case CRequest::UPDATE_NOMINATION :
      if (reqPtr->id == mt.mtID)
      {
        int idx = mtList->GetCurrentIndex();
        mt.SelectById(mt.mtID);
        mt.Next();
        mt.Close();  // XXX Hack

        mtList->RemoveAllListItems();

        MtEntryStore  mtEntry;
        mtEntry.SelectByMS(mt);
        while (mtEntry.Next())
          mtList->AddListItem(new MtItem(mtEntry, false));

        TransferDataToWindow();
        Refresh();

        mtList->SetCurrentIndex(idx);
      }

      break;

    case CRequest::UPDATE_RESULT :
    {
      if (reqPtr->id == mt.mtID)
      {
        // Gesamtergebnis nochmals lesen
        mt.SelectById(mt.mtID);
        mt.Next();
        mt.Close(); // XXX Hack

        // Ergebnisse einlesen
        MtMatchStore  mtMatch(mt, mt.GetConnectionPtr());
        mtMatch.SelectAll(0);
        short resA = 0, resX = 0;
        bool commit = false;
        bool rollback = false;

        while (mtMatch.Next())
        {
          if (mtMatch.mtMS > 0)
          {
            MtItem *itemPtr = (MtItem *) mtList->GetListItem(mtMatch.mtMS - 1);
            itemPtr->SetResult(mtMatch.mtResA, mtMatch.mtResX);

            if ( !sy.syComplete && (2 * resA > mt.mtMatches || 2 * resX > mt.mtMatches) )
              itemPtr->SetForeground(*wxRED);
            else
              itemPtr->SetForeground(wxNullColour);

            if ( 2 * mtMatch.mtResA > mt.mtBestOf )
              resA++;
            if ( 2 * mtMatch.mtResX > mt.mtBestOf )
              resX++;
          }
        }

        // Endergebnis. mtResA gehoert zu tmA, mtResX zu tmX.
        if (mt.QryWinnerAX() == 1)
          tmWinnerItem->SetListItem(new TmItem(tmA));
        else if (mt.QryWinnerAX() == -1)
          tmWinnerItem->SetListItem(new TmItem(tmX));
        else if (mt.IsFinished())
        {
          // Ein Team mit "TIE" faelschen
          wxString tmp = _("Tie");

          TmItem *itemPtr = new TmItem();
          itemPtr->entry.team.cpType = CP_TEAM;
          wxStrcpy(itemPtr->entry.team.tm.tmDesc, tmp);
          
          tmWinnerItem->SetListItem(itemPtr);
        }
        else
          tmWinnerItem->SetListItem(new TmItem());
          
        m_woa = mt.mtWalkOverA;
        m_wox = mt.mtWalkOverX;
        
        TransferDataToWindow();

        Refresh();
      }

      break;
    }

    default :
      break;
  }
}

void CMtTeam::OnScore(wxCommandEvent &)
{
  CTT32App::instance()->OpenView(_("Print Scoresheets"), wxT("Score"), mt.mtID, 0);
}


bool CMtTeam::TransferDataToWindow()
{
  if (!CFormViewEx::TransferDataToWindow())
    return false;
  
  wxString str;
  if (mt.mtReverse)
    str = wxString::Format("%2hd : %2hd", mt.mtResX, mt.mtResA);
  else
    str = wxString::Format("%2hd : %2hd", mt.mtResA, mt.mtResX);
    
  XRCCTRL(*this, "Result", CItemCtrl)->SetListItem(new ResultListItem(str));   
  
  return true;
}