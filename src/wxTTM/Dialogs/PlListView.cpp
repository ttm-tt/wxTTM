/* Copyright (C) 2020 Christoph Theis */

// PlListView.cpp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "PlListView.h"

#include "PlListStore.h"
#include "PlItem.h"
#include "PlStore.h"

#include "GrListStore.h"
#include "NtListStore.h"
#include "TmListStore.h"

#include "LtStore.h"

#include "InfoSystem.h"
#include "Request.h"

#include <list>

// -----------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS(CPlListView, CFormViewEx)

BEGIN_EVENT_TABLE(CPlListView, CFormViewEx)
  EVT_BUTTON(IDC_EVENTS, CPlListView::OnPlEditEvents)
  EVT_BUTTON(XRCID("Undelete"), CPlListView::OnRestore)
  EVT_BUTTON(XRCID("Notes"), CPlListView::OnNotes)
  EVT_BUTTON(XRCID("Clone"), CPlListView::OnClone)
  EVT_BUTTON(XRCID("History"), CPlListView::OnHistory)
  EVT_BUTTON(XRCID("EventHistory"), CPlListView::OnEventHistory)
END_EVENT_TABLE()


const char * CPlListView::headers[] = 
{
  wxTRANSLATE("Pl.Nr."),
  wxTRANSLATE("Name"),
  wxTRANSLATE("Assoc."),
  wxTRANSLATE("Sex"),
  wxTRANSLATE("Born in"),
  wxTRANSLATE("Extern ID"),
  wxTRANSLATE("Rk.Pts."),
  wxTRANSLATE("Region"),
  wxTRANSLATE("Notes"),
  NULL
};

// -----------------------------------------------------------------------
CPlListView::CPlListView() : CFormViewEx(), m_listCtrl(0)
{
}

CPlListView::~CPlListView()
{
}



// -----------------------------------------------------------------------
void CPlListView::SaveSettings()
{
  m_listCtrl->SaveColumnInfo(GetClassInfo()->GetClassName());

  CFormViewEx::SaveSettings();
}


void CPlListView::RestoreSettings()
{
  CFormViewEx::RestoreSettings();

  m_listCtrl->RestoreColumnInfo(GetClassInfo()->GetClassName());
}


// -----------------------------------------------------------------------
bool CPlListView::Edit(va_list vaList)
{
  PlListStore  plList;
  plList.SelectAll();

  while (plList.Next())
  {
    m_listCtrl->AddListItem(new PlItemEx(plList));
  }
  
  m_listCtrl->ResizeColumn(1);
  

  m_listCtrl->SortItems();
  
  m_listCtrl->SetCurrentIndex(0);
  
  return true;
}


// -----------------------------------------------------------------------
void  CPlListView::OnAdd()
{
  if (!CTT32App::instance()->IsLicenseValid())
    return;

  CTT32App::instance()->OpenView(_("Add Player"), wxT("PlEdit"));
}


void  CPlListView::OnEdit()
{
  ListItem *itemPtr = m_listCtrl->GetCurrentItem();
  if (!itemPtr)
    return;

  CTT32App::instance()->OpenView(_("Edit Player"), wxT("PlEdit"), itemPtr->GetID());
}


void  CPlListView::OnDelete()
{
  if (!CTT32App::instance()->IsLicenseValid())
    return;

  if (m_listCtrl->GetSelectedItemCount() > 0)
  {
    if (infoSystem.Confirmation(_("Are you sure to delete selected players?")) == false)
      return;

    bool doIt = true;

    for (int idx = m_listCtrl->GetItemCount(); doIt && idx--; )
    {
      if (m_listCtrl->IsSelected(idx))
      {
        PlItem *itemPtr = (PlItem *) m_listCtrl->GetListItem(idx);

        if (!itemPtr)
          continue;
    
        std::list<TmRec> tmList;
        std::list<GrRec> grList;
        
        TmListStore tm;
        GrListStore gr;
        
        tm.SelectByPl(itemPtr->pl);
        while (tm.Next())
          tmList.push_back(tm);
          
        for (std::list<TmRec>::iterator it = tmList.begin();
            it != tmList.end(); it++)
        {
          gr.SelectByTm(*it);
          while (gr.Next())
            grList.push_back(gr);
        }
        
        for (std::list<GrRec>::iterator it = grList.begin();
            it != grList.end(); it = grList.begin())
        {
          gr.SelectById((*it).grID);
          gr.Next();
          gr.Close();
          
          if (gr.QryStarted())
            break;
            
          grList.pop_front();
        }
        
        if (grList.size() > 0)
        {
          infoSystem.Error(_("Player is member of a group that has already started"));
          // Weiter mit naechsten Spieler
          continue;
        }

        for (std::list<TmRec>::iterator it = tmList.begin(); it != tmList.end(); it++)
        {
          CpListStore cp;
          cp.SelectById((*it).cpID);
          cp.Next();

          if (cp.cpType != CP_TEAM)
            continue;

          if (NtListStore().Count(*it) == 1)
          {
            if (!infoSystem.Confirmation(_("Team %s will then be empty"), (*it).tmDesc))
            {
              doIt = false;
              break;
            }
          }
        }
          
        TTDbse::instance()->GetDefaultConnection()->StartTransaction();
        
        // TmStore muss getrennt geloescht werden, weil die Constraint ntLtRef haengt.
        for (std::list<TmRec>::iterator it = tmList.begin();
             it != tmList.end(); it = tmList.begin())
        {
          // Team-Wettbewerbe nicht loeschen.
          CpListStore cp;
          cp.SelectById((*it).cpID);
          cp.Next();
          cp.Close();
          
          if (cp.cpType == CP_TEAM)
          {
            LtStore lt;
            lt.SelectByCpPl((*it).cpID, itemPtr->pl.plID);
            lt.Next();
            lt.Close();
            
            TmStore().RemoveEntry(lt);
          }
          else
          {
            if (!TmStore().Remove((*it).tmID))
              break;
          }
            
          tmList.pop_front();
        }

        if (tmList.size() > 0)
          TTDbse::instance()->GetDefaultConnection()->Rollback();
        else if ( PlStore().Remove( ((PlItem *) itemPtr)->pl.plID ) )
          TTDbse::instance()->GetDefaultConnection()->Commit();
        else
          TTDbse::instance()->GetDefaultConnection()->Rollback();
      }
    }
  }
}


void CPlListView::OnRestore(wxCommandEvent&)
{
  bool doIt = true;

  for (int idx = m_listCtrl->GetItemCount(); doIt && idx--; )
  {
    if (m_listCtrl->IsSelected(idx))
    {
      PlItem *itemPtr = (PlItem *) m_listCtrl->GetListItem(idx);

      if (!itemPtr)
        continue;

      PlStore pl;
      pl.SelectById(itemPtr->pl.plID);
      pl.Next();
      pl.Close();

      pl.Restore();
    }
  }
}


void CPlListView::OnPlEditEvents(wxCommandEvent &) 
{
  ListItem *itemPtr = m_listCtrl->GetCurrentItem();
  if (!itemPtr)
    return;

  if (itemPtr->IsDeleted())
    return;

  wxPanel *panel = CTT32App::instance()->OpenView(_T("Add to Events"), wxT("PlEvents"), itemPtr->GetID());

  if (panel)
    panel->GetParent()->Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(CPlListView::OnChildClose), NULL, this);
}


void CPlListView::OnEventHistory(wxCommandEvent &) 
{
  ListItem *itemPtr = m_listCtrl->GetCurrentItem();
  if (!itemPtr)
    return;

  CTT32App::instance()->OpenView(_T("Event History"), wxT("PlHistEvents"), itemPtr->GetID());
}


void CPlListView::OnNotes(wxCommandEvent &)
{
  ListItem *itemPtr = m_listCtrl->GetCurrentItem();
  if (!itemPtr)
    return;

  PlStore pl;
  pl.SelectById(itemPtr->GetID());
  if (!pl.Next())
    return;

  pl.Close();

  wxString note = pl.GetNote();

  wxDialog *dlg = wxXmlResource::Get()->LoadDialog(CTT32App::instance()->GetTopWindow(), "TextEntry");
  dlg->SetTitle(_("Add Note"));
  XRCCTRL(*dlg, "Text", wxTextCtrl)->SetValue(note);

  if (dlg->ShowModal() == wxID_OK)
  {
    wxString note = XRCCTRL(*dlg, "Text", wxTextCtrl)->GetValue();
    pl.InsertNote(note);
  }

  delete dlg;
}


void CPlListView::OnClone(wxCommandEvent &)
{
  if (!CTT32App::instance()->IsLicenseValid())
    return;

  ListItem *itemPtr = m_listCtrl->GetCurrentItem();
  if (!itemPtr)
    return;

  PlStore pl;
  pl.SelectById(itemPtr->GetID());
  if (!pl.Next())
    return;

  pl.Close();

  do
  {
    pl.plNr += 10000;
  } while (pl.NrToID(pl.plNr));

  pl.plID = 0;

  pl.Insert();
}


void CPlListView::OnHistory(wxCommandEvent &)
{
  ListItem *itemPtr = m_listCtrl->GetCurrentItem();
  if (!itemPtr)
    return;

  CTT32App::instance()->OpenView(_("Players History"), wxT("PlHistView"), itemPtr->GetID());
}


// -----------------------------------------------------------------------
void CPlListView::OnInitialUpdate() 
{
  m_listCtrl = XRCCTRL(*this, "Players", CListCtrlEx);

  for (size_t i = 0; headers[i]; i++)
  {
    m_listCtrl->InsertColumn(i, wxGetTranslation(headers[i]), wxLIST_FORMAT_LEFT, i == 1 ? GetClientSize().GetWidth() : wxLIST_AUTOSIZE_USEHEADER);
  }

  for (size_t i = 0; headers[i]; i++)
  {
    if (i != 1)
      m_listCtrl->SetColumnWidth(i, wxLIST_AUTOSIZE_USEHEADER);
  }

  m_listCtrl->ShowColumn(4, CTT32App::instance()->GetType() == TT_YOUTH || CTT32App::instance()->GetType() == TT_SCI);
  m_listCtrl->HideColumn(5);
  m_listCtrl->HideColumn(6);
  m_listCtrl->HideColumn(7);
  m_listCtrl->HideColumn(8);

  m_listCtrl->AllowHideColumn(0);
  m_listCtrl->AllowHideColumn(2);
  m_listCtrl->AllowHideColumn(3);
  m_listCtrl->AllowHideColumn(4);
  m_listCtrl->AllowHideColumn(5);
  m_listCtrl->AllowHideColumn(6);
  m_listCtrl->AllowHideColumn(7);
  m_listCtrl->AllowHideColumn(8);

  m_listCtrl->SetResizeColumn(1);

  FindWindow("Add")->Enable(CTT32App::instance()->IsLicenseValid());
  FindWindow("Clone")->Enable(CTT32App::instance()->IsLicenseValid());
  FindWindow("Delete")->Enable(CTT32App::instance()->IsLicenseValid());
  // FindWindow("Events")->Enable(CTT32App::instance()->IsLicenseValid());

  FindWindow("Events")->SetId(IDC_EVENTS);
}


void CPlListView::OnUpdate(CRequest *reqPtr) 
{
	if (!reqPtr)
  {
    CFormViewEx::OnUpdate(reqPtr);
    return;
  }

  switch (reqPtr->type)
  {
    case CRequest::INSERT :
    case CRequest::UPDATE :
    {
      if (reqPtr->rec != CRequest::PLREC)
        return;
        
      PlListStore  plList;
      if (!plList.SelectById(reqPtr->id))
        return;
      if (!plList.Next())
        return;
        
      plList.Close();
        
      long id = m_listCtrl->GetCurrentItem() ? 
                m_listCtrl->GetCurrentItem()->GetID() : 0;

      // Add / Set Item Data
      if (reqPtr->type == CRequest::INSERT)
      {
        m_listCtrl->AddListItem(new PlItemEx(plList));
        id = reqPtr->id;
      }
      else
      {
        PlItem *itemPtr = (PlItem *) m_listCtrl->FindListItem(reqPtr->id);
        if (itemPtr)
          itemPtr->pl = plList;
      }
      
      m_listCtrl->SortItems();
      
      if (id)
        m_listCtrl->SetCurrentItem(id);
      else
        m_listCtrl->SetCurrentItem(reqPtr->id);
        
      break;
    }

    case CRequest::REMOVE :
    {
      int idx = m_listCtrl->GetCurrentIndex();

      switch (reqPtr->rec)
      {
        case CRequest::PSREC :
        {
          // Es koennte das aktuelle sein
          if ( m_listCtrl->GetCurrentItem() && 
               ((PlItem *) m_listCtrl->GetCurrentItem())->pl.psID == reqPtr->id)
          {
            m_listCtrl->DeleteItem(m_listCtrl->GetCurrentIndex());
            break;
          }
        
          // Ansonsten suchen
          for (int i = m_listCtrl->GetItemCount(); i--; )
          {
            if ( ((PlItem *) m_listCtrl->GetListItem(i))->pl.psID == reqPtr->id)
            {
              m_listCtrl->DeleteItem(i);
              break;
            }
          }
        }

        break;

        case CRequest::PLREC :
        {      
          // Hier sucht m_listCtrl selbst
          m_listCtrl->RemoveListItem(reqPtr->id);
          break;
        }

        default :
          break;
      }

      if (m_listCtrl->GetSelectedCount() > 1)
        ;
      else if (idx < m_listCtrl->GetCount())
        m_listCtrl->SetCurrentIndex(idx);
      else if (idx > 0)
        m_listCtrl->SetCurrentIndex(idx - 1);
      else if (m_listCtrl->GetCount())
        m_listCtrl->SetCurrentIndex(0);

      break;
    }

    default :
      break;
  }
}


// -----------------------------------------------------------------------
void CPlListView::OnChildClose(wxCloseEvent& evt)
{
  evt.Skip();

  long idx = m_listCtrl->GetCurrentIndex();
  while (idx < m_listCtrl->GetItemCount() - 1)
  {
    if (m_listCtrl->GetListItem(++idx) && !m_listCtrl->GetListItem(idx)->IsDeleted())
      break;
  }

  if (idx < m_listCtrl->GetItemCount())
    m_listCtrl->SetCurrentIndex(idx);
}

