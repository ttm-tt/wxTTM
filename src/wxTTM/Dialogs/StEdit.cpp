/* Copyright (C) 2020 Christoph Theis */

// StEdit.cpp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "StEdit.h"

#include "MtStore.h"
#include "GrStore.h"
#include "CpStore.h"

#include "TmItem.h"

// -----------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS(CStEdit, CFormViewEx)

BEGIN_EVENT_TABLE(CStEdit, CFormViewEx)

END_EVENT_TABLE()



// -----------------------------------------------------------------------
// CStEdit

CStEdit::CStEdit() : CFormViewEx(), m_cbTeam(0)
{
}

CStEdit::~CStEdit()
{
}


// -----------------------------------------------------------------------
bool  CStEdit::Edit(va_list vaList)
{
  long id = va_arg(vaList, long);

  st.SelectById(id);
  st.Next();

  gr.SelectById(st.grID);
  gr.Next();

  cp.SelectById(gr.cpID);
  cp.Next();

  if (cp.cpType == CP_SINGLE || cp.cpType == CP_TEAM)
    m_cbTeam->SetItemHeight(1);
  
  if (st.tmID)
  {
    tm.SelectTeamById(st.tmID, cp.cpType);
    tm.Next();
  }
  else
  {
    XxStore  xx;
    xx.Select(st);
    xx.Next();

    GrStore  grQual;
    
    if (xx.grID)
    {
      grQual.SelectById(xx.grID);
      grQual.Next();
    }

    tm.team.cpType = CP_GROUP;
    tm.tmID = grQual.grID;
    tm.SetGroup(grQual, xx.grPos);
  }
  
  if (gr.grModus != MOD_RR)
  {
    FindWindow("FinalPositionLabel")->Show(false);
    FindWindow("FinalPosition")->Show(false);
  }

  // Nur wenn noch moeglich
  if (!gr.QryStarted())
    ;
  else if (gr.grModus == MOD_RR)
  {
    if (gr.QryFinished())
      m_cbTeam->Enable(false);
  }
  else
  {
    MtRec mt;
    mt.mtEvent.grID = gr.grID;
    mt.mtEvent.mtRound = 1;
    mt.mtEvent.mtMatch = (st.stNr + 1) / 2;
    mt.mtEvent.mtMS = 0;
    
    MtStore mtWinner, mtLoser;
    short ax;

    if (gr.QryToWinner(mt, &mtWinner, ax))
    {
      mtWinner.SelectByEvent(mtWinner.mtEvent);
      mtWinner.Next();
      if (mtWinner.mtResA || mtWinner.mtResX)
        m_cbTeam->Enable(false);
    }
    
    if (gr.QryToLoser(mt, &mtLoser, ax))
    {
      mtLoser.SelectByEvent(mtLoser.mtEvent);
      mtLoser.Next();
      if (mtLoser.mtResA || mtLoser.mtResX)
        m_cbTeam->Enable(false);
    }    
  }

  // Erster Eintrag ist leer
  m_cbTeam->AddListItem(new TmItem());

  // TODO: Nur ausgewaehlte Paare + aktuelles Paar
  TmEntryStore  tmEntry;
  tmEntry.SelectTeamForSeeding(gr, cp.cpType);
  while (tmEntry.Next())
    m_cbTeam->AddListItem(new TmItem(tmEntry));
    
  int startIdx = m_cbTeam->GetCount();

  // Und jetzt noch die Gruppen
  GrStore  grQual;
  grQual.SelectAll(cp);
  while (grQual.Next())
  {
    if (!wxStrcoll(grQual.grStage, gr.grStage))
      continue;

    tmEntry.Init();
    tmEntry.team.cpType = CP_GROUP;
    tmEntry.tmID = grQual.grID;
    
    if (grQual.grModus == MOD_RR)
    {
      if (!wxStrcmp(grQual.grStage, "Qualification"))
      {
        if (!wxStrcmp(gr.grStage, "Championship"))
        {
          for (int i = 1; i <= 2 && i <= grQual.grSize; i++)
          {
            tmEntry.SetGroup(grQual, i);
            m_cbTeam->AddListItem(new TmItem(tmEntry));
          }
        }
        else if (!wxStrcmp(gr.grStage, "Consolation"))
        {
          for (int i = 3; i <= grQual.grSize; i++)
          {
            tmEntry.SetGroup(grQual, i);
            m_cbTeam->AddListItem(new TmItem(tmEntry));
          }
        }
        else
        {
          tmEntry.SetGroup(grQual, 1);
          m_cbTeam->AddListItem(new TmItem(tmEntry));
        }
      }
      else
      {
        for (int i = 1; i <= grQual.grSize; i++)
        {
          tmEntry.SetGroup(grQual, i);
          m_cbTeam->AddListItem(new TmItem(tmEntry));
        }
      }
    }
    else if (grQual.grModus == MOD_PLO ||
             grQual.grModus == MOD_SKO && grQual.grSize == 2)
    {
      for (int i = 1; i <= grQual.grSize; i++)
      {  
        tmEntry.SetGroup(grQual, i);
        m_cbTeam->AddListItem(new TmItem(tmEntry));
      }
    }
    else
    {
      tmEntry.SetGroup(grQual, 1);
      m_cbTeam->AddListItem(new TmItem(tmEntry));
    }    
  }
  
  // "To be defined"
  tmEntry.SetGroup(GrRec(), 0);
  m_cbTeam->AddListItem(new TmItem(tmEntry));
  
  if (!m_cbTeam->FindListItem(this->tm.tmID))
	  m_cbTeam->AddListItem(new TmItem(this->tm));

  m_cbTeam->SetCurrentItem(this->tm.tmID);
  
  XxStore xx(st.GetConnectionPtr());
  xx.SelectAll();
  while (xx.Next())
  {
    for (int idx = startIdx; idx < (int) m_cbTeam->GetCount(); idx++)
    {
      TmItem *itemPtr = (TmItem *) m_cbTeam->GetListItem(idx);
      if (!itemPtr)
        continue;
      
      if (itemPtr->entry.team.cpType == CP_GROUP &&
          itemPtr->entry.team.gr.grID == xx.grID &&
          itemPtr->entry.team.gr.grPos == xx.grPos)      
      {
        if (xx.stID == st.stID)
          m_cbTeam->SetCurrentItem(itemPtr);
        else if (xx.grID != 0)
          m_cbTeam->RemoveListItem(idx);
          
        break;
      }
    } 
  }
  
  xx.Close();  
  
  TransferDataToWindow();
  
  return true;
}


void  CStEdit::OnOK()
{
  TransferDataFromWindow();

  TmItem * itemPtr = (TmItem *) m_cbTeam->GetCurrentItem();
  tm.TmEntry::operator=(itemPtr->entry);

  TTDbse::instance()->GetDefaultConnection()->StartTransaction();

  if (st.Update() && gr.SetTeam(st.stNr, tm, st.stSeeded ? StStore::ST_SEEDED : 0))
    TTDbse::instance()->GetDefaultConnection()->Commit();
  else
    TTDbse::instance()->GetDefaultConnection()->Rollback();

  CFormViewEx::OnOK();
}


// -----------------------------------------------------------------------
void CStEdit::OnInitialUpdate() 
{
	CFormViewEx::OnInitialUpdate();	

  m_cbTeam = XRCCTRL(*this, "Entry", CComboBoxEx);	
  
  m_cbTeam->SetItemHeight(2);
  
  FindWindow("Seeded")->SetValidator(CEnumValidator(&st.stSeeded, 1));
  FindWindow("Disqualified")->SetValidator(CEnumValidator(&st.stDisqu, 1));
  FindWindow("NoConsolation")->SetValidator(CEnumValidator(&st.stNocons, 1));
  FindWindow("GaveUp")->SetValidator(CEnumValidator(&st.stGaveup, 1));
  FindWindow("FinalPosition")->SetValidator(CShortValidator(&st.stPos));
}

