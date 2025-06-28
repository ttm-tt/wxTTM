/* Copyright (C) 2020 Christoph Theis */

// GrListView.cpp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "GrListView.h"

#include "GrItem.h"
#include "CpItem.h"

#include "CpListStore.h"
#include "GrListStore.h"

#include "InfoSystem.h"
#include "Request.h"

#include "checked.xpm"
#include "unchecked.xpm"



IMPLEMENT_DYNAMIC_CLASS(CGrListView, CFormViewEx)

BEGIN_EVENT_TABLE(CGrListView, CFormViewEx)
  EVT_COMBOBOX(XRCID("Events"), CGrListView::OnSelChangeCp)
  EVT_RADIOBOX(XRCID("State"), CGrListView::OnChangeState)
  EVT_BUTTON(XRCID("Notes"), CGrListView::OnNotes)
  EVT_BUTTON(XRCID("Publish"), CGrListView::OnPublish)
  EVT_BUTTON(XRCID("Unpublish"), CGrListView::OnUnpublish)
  EVT_COLLAPSIBLEPANE_CHANGED(XRCID("Filter"), CGrListView::OnFilterCollapsibleChanged)
  END_EVENT_TABLE()


// -----------------------------------------------------------------------
// Use namespace to make class declaration local to this file
namespace
{
  class GrItemEx : public GrItem
  {
    public:
      GrItemEx(const GrListRec &gr);

    public:
      void DrawColumn(wxDC *, int col, wxRect &rect);

      int  Compare(const ListItem *, int col) const;

    private:
      wxString cpName;
      wxString syName;
      wxString mdName;
  };
}


GrItemEx::GrItemEx(const GrListRec& gr) : GrItem(gr) 
{ 
  SetLabel(""); // Sonst hat man einen Tooltip

  cpName = gr.cpName;
  syName = gr.syName;
  mdName = gr.mdName;

  switch (gr.grModus)
  {
    case MOD_RR :
      mdName = gr.mdName;
      break;

    case MOD_SKO :
      mdName = "KO";
      break;

    case MOD_DKO :
      mdName = "DK";
      break;

    case MOD_PLO :
      mdName = "PO";
      break;
  }
}


void GrItemEx::DrawColumn(wxDC *pDC, int col, wxRect &rect)
{
  static wxImage checkedImg(checked);
  static wxImage uncheckedImg(unchecked);

  switch (col)
  {
    case 0 :
      DrawImage(pDC, rect, gr.grPublished ? checkedImg : uncheckedImg);
      break;

    // 1 (Name) - 4 (Size) -> 0 - 3
    case 1 :
    case 2 :
    case 3 :
    case 4 :
    case 5 :
      GrItem::DrawColumn(pDC, col - 1, rect);
      break;

    case 6 :
      DrawString(pDC, rect, syName);
      break;

    case 7 :
      DrawString(pDC, rect, mdName);
      break;

    case 8 :
      if (gr.grPrinted.year)
        DrawString(pDC, rect, wxString::Format("%04d-%02d-%02d %02d:%02d", gr.grPrinted.year, gr.grPrinted.month, gr.grPrinted.day, gr.grPrinted.hour, gr.grPrinted.minute));
      break;

    // 6 Notes -> 5
    case 9 : 
      GrItem::DrawColumn(pDC, 5, rect);
      break;
  }
}


int GrItemEx::Compare(const ListItem *itemPtr, int col) const
{
  const GrItemEx *grItem = reinterpret_cast<const GrItemEx *>(itemPtr);
  if (!grItem)
    return -1;

  switch (col)
  {
    case 0 : // Pbl
      return 0;

    // 1 (Name) - 5 (Sort) -> 0 - 4
    case 1 :
    case 2 :
    case 3 :
    case 4 :
    case 5 :
      return GrItem::Compare(itemPtr, col - 1);

    case 6 :
    {
      int ret = wxStrcoll(syName, grItem->syName);

      // If equal compare by name
      if (ret == 0)
        ret = wxStrcoll(gr.grName, grItem->gr.grName);

      return ret;
    }

    case 7 :
    {
      int ret = wxStrcoll(mdName, grItem->mdName);

      // If equal compare by name
      if (ret == 0)
        ret = wxStrcoll(gr.grName, grItem->gr.grName);

      return ret;
    }

    case 8 :
      if (gr.grPrinted > ((GrItemEx *)itemPtr)->gr.grPrinted)
        return 1;
      else if (gr.grPrinted < ((GrItemEx *)itemPtr)->gr.grPrinted)
        return -1;
      else
        return 0;

    default :
      return 0;
  }
}

// -----------------------------------------------------------------------
// CGrListView
CGrListView::CGrListView() : CFormViewEx()
{
}


CGrListView::~CGrListView()
{
}


// -----------------------------------------------------------------------
void CGrListView::SaveSettings()
{
  m_listCtrl->SaveColumnInfo(GetClassInfo()->GetClassName());

  CFormViewEx::SaveSettings();
}


void CGrListView::RestoreSettings()
{
  CFormViewEx::RestoreSettings();

  m_listCtrl->RestoreColumnInfo(GetClassInfo()->GetClassName());
}


bool CGrListView::Edit(va_list vaList)
{
  CpListStore  cp;
  cp.SelectAll();
  while (cp.Next())
    m_cbCp->AddListItem(new CpItem(cp));

  wxString cpName = CTT32App::instance()->GetDefaultCP();

  ListItem *cpItemPtr = m_cbCp->FindListItem(cpName);
  if (!cpItemPtr)
    cpItemPtr = m_cbCp->GetListItem(0);
  if (!cpItemPtr)
    return true;
    
  m_cbCp->SetCurrentItem(cpItemPtr->GetID());

  TransferDataToWindow();
  
  OnSelChangeCp(wxCommandEvent_);

  return true;
}



// -----------------------------------------------------------------------
// CGrListView message handlers
void CGrListView::OnInitialUpdate() 
{
  CFormViewEx::OnInitialUpdate();
  
  m_cbCp = XRCCTRL(*this, "Events", CComboBoxEx);
  m_listCtrl = XRCCTRL(*this, "Groups", CListCtrlEx);
  
  // Publish
  m_listCtrl->InsertColumn(0, _("Pbl"), wxALIGN_CENTER, 3 * cW);

  // Name
  m_listCtrl->InsertColumn(1, _("Name"), wxALIGN_LEFT, 6 * cW);

  // Description
  m_listCtrl->InsertColumn(2, _("Description"), wxALIGN_LEFT);
  
  // Stufe
  m_listCtrl->InsertColumn(3, _("Stage"), wxALIGN_LEFT, 10 * cW);

  // Size
  m_listCtrl->InsertColumn(4, _("Size"), wxALIGN_RIGHT, 4 * cW);
  m_listCtrl->HideColumn(4);
  m_listCtrl->AllowHideColumn(4);

  // Size
  m_listCtrl->InsertColumn(5, _("Sort"), wxALIGN_RIGHT, 4 * cW);
  m_listCtrl->HideColumn(5);
  m_listCtrl->AllowHideColumn(5);

  // System
  m_listCtrl->InsertColumn(6, _("System"), wxALIGN_LEFT, 6 * cW);
  m_listCtrl->HideColumn(6);
  m_listCtrl->AllowHideColumn(6);

  // Modus
  m_listCtrl->InsertColumn(7, _("Modus"), wxALIGN_LEFT, 4 * cW);
  m_listCtrl->HideColumn(7);
  m_listCtrl->AllowHideColumn(7);

  // Notes
  m_listCtrl->InsertColumn(8, _("Notes"), wxLIST_FORMAT_CENTER);
  m_listCtrl->HideColumn(8);
  m_listCtrl->AllowHideColumn(8);

  // Printed
  m_listCtrl->InsertColumn(9, _("Printed"), wxALIGN_LEFT, 10 * cW);
  m_listCtrl->HideColumn(9);
  m_listCtrl->AllowHideColumn(9);

  m_listCtrl->SetResizeColumn(2);

  m_listCtrl->SetSortColumn(3);

  m_listCtrl->Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(CGrListView::OnMouseLeftDown), NULL, this);
}


// -----------------------------------------------------------------------
void  CGrListView::OnAdd()
{
  ListItem *itemPtr = m_cbCp->GetCurrentItem();
  if (!itemPtr)
    return;

  long  grID = 0;
  long  cpID = itemPtr->GetID();

  CTT32App::instance()->OpenView(_("Edit Group"), wxT("GrEdit"), grID, cpID);
}


void  CGrListView::OnEdit()
{
  if (m_listCtrl->GetSelectedCount() != 1)
    return;

  ListItem *grPtr = m_listCtrl->GetCurrentItem();
  ListItem *cpPtr = m_cbCp->GetCurrentItem();

  if (!grPtr || !cpPtr)
    return;

  long grID = grPtr->GetID();
  long cpID = cpPtr->GetID();

  CTT32App::instance()->OpenView(_("Edit Group"), wxT("GrEdit"), grID, cpID);
}


void  CGrListView::OnDelete()
{
  if (m_listCtrl->GetSelectedCount() == 0)
    return;
    
  if (infoSystem.Confirmation(_("Are you sure to delete selected group(s)?")) == false)
    return;

  for (int idx = m_listCtrl->GetItemCount(); idx--; )
  {
    if (!m_listCtrl->IsSelected(idx))
      continue;
      
    ListItem *itemPtr = m_listCtrl->GetListItem(idx);
    if (!itemPtr)
      continue;
      
    TTDbse::instance()->GetDefaultConnection()->StartTransaction();

    if ( GrStore().Remove(itemPtr->GetID()) )
      TTDbse::instance()->GetDefaultConnection()->Commit();
    else
      TTDbse::instance()->GetDefaultConnection()->Rollback();
  }
}


void CGrListView::OnSelChangeCp(wxCommandEvent &) 
{
  ListItem *itemPtr = m_cbCp->GetCurrentItem();
  if (!itemPtr)
    return;

  cp.SelectById(itemPtr->GetID());
  cp.Next();

  CTT32App::instance()->SetDefaultCP(cp.cpName);

  OnChangeState(wxCommandEvent_);
}


void CGrListView::OnChangeState(wxCommandEvent&)
{
  m_listCtrl->RemoveAllListItems();

  // wxCollapsiblePane *filter = XRCCTRL(*this, "Filter", wxCollapsiblePane);

  bool published = XRCCTRL(*this, "State", wxRadioBox)->GetSelection() != 1;
  bool unpublished = XRCCTRL(*this, "State", wxRadioBox)->GetSelection() != 2;

  // bool published = false;
  // bool unpublished = true;

  GrListStore  grList;
  grList.SelectAll(cp);

  while (grList.Next())
  {
    if (grList.grPublished && !published)
      continue;
    if (!grList.grPublished && !unpublished)
      continue;

    m_listCtrl->AddListItem(new GrItemEx(grList));
  }

  m_listCtrl->SortItems();
}


void CGrListView::OnNotes(wxCommandEvent &)
{
  wxString note;

  // Nur wenn lediglich eine Gruppe ausgewaehlt wurde die aktuelle Anmerkung auslesen
  // Bei mehreren macht es keinen Sinn, wenn die Anmerkungen sich unterscheiden
  if (m_listCtrl->GetSelectedCount() == 1)
  {
    GrItem *itemPtr = (GrItem *)m_listCtrl->GetCurrentItem();
    GrStore gr = itemPtr->gr;
    note = gr.GetNote();
  }

  wxDialog *dlg = wxXmlResource::Get()->LoadDialog(CTT32App::instance()->GetTopWindow(), "TextEntry");
  dlg->SetTitle(_("Add Note"));
  XRCCTRL(*dlg, "Text", wxTextCtrl)->SetValue(note);

  if (dlg->ShowModal() == wxID_OK)
  {
    note = XRCCTRL(*dlg, "Text", wxTextCtrl)->GetValue();

    for (int idx = m_listCtrl->GetItemCount(); idx--; )
    {
      if (!m_listCtrl->IsSelected(idx))
        continue;

      GrItem *itemPtr = (GrItem *)m_listCtrl->GetListItem(idx);
      GrStore gr = itemPtr->gr;

      gr.InsertNote(note);
    }
  }

  delete dlg;
}


void CGrListView::OnPublish(wxCommandEvent &)
{
  for (int idx = m_listCtrl->GetItemCount(); idx--; )
  {
    if (!m_listCtrl->IsSelected(idx))
      continue;

    GrItem *itemPtr = (GrItem *)m_listCtrl->GetListItem(idx);
    GrStore gr = itemPtr->gr;
    gr.SetPublish(true);
  }
}


void CGrListView::OnUnpublish(wxCommandEvent &)
{
  for (int idx = m_listCtrl->GetItemCount(); idx--; )
  {
    if (!m_listCtrl->IsSelected(idx))
      continue;

    GrItem *itemPtr = (GrItem *)m_listCtrl->GetListItem(idx);
    GrStore gr = itemPtr->gr;
    gr.SetPublish(false);
  }
}


void CGrListView::OnMouseLeftDown(wxMouseEvent &evt)
{
  if (evt.LeftDown())
  {
    int flags;
    long col;
    long item = m_listCtrl->HitTest(evt.GetPosition(), flags, &col);
    if (item >= 0 && col == 0)
    {
      GrItem *itemPtr = (GrItem *) m_listCtrl->GetListItem( (int) item );

      GrStore gr;
      gr.grID = itemPtr->gr.grID;

      switch (col)
      {
        case 0 :
          gr.SetPublish(!itemPtr->gr.grPublished);
          break;
      }
    }
    else
      evt.Skip();
  }
  else
    evt.Skip();
}


// -----------------------------------------------------------------------
void CGrListView::OnUpdate(CRequest *reqPtr) 
{
  if (!reqPtr)
  {
    CFormViewEx::OnUpdate(reqPtr);
    return;
  }

  if (reqPtr->rec != CRequest::GRREC)
    return;

  long id = m_listCtrl->GetCurrentItem() ? 
            m_listCtrl->GetCurrentItem()->GetID() : 0;

  switch (reqPtr->type)
  {
    case CRequest::INSERT :
    case CRequest::UPDATE :
    {
      GrListStore  grList;
      if (!grList.SelectById(reqPtr->id))
        return;
      if (!grList.Next())
        return;
      if (grList.cpID != cp.cpID)
        return;

      // Add / Set Item Data
      m_listCtrl->RemoveListItem(reqPtr->id);
      m_listCtrl->AddListItem(new GrItemEx(grList));
      break;
    }

    case CRequest::REMOVE :
    {
      m_listCtrl->RemoveListItem(reqPtr->id);
      break;
    }

    default :
      break;
  }

  m_listCtrl->SortItems();

  if (id)
    m_listCtrl->SetCurrentItem(id);
}

void CGrListView::OnFilterCollapsibleChanged(wxCollapsiblePaneEvent& evt)
{
  XRCCTRL(*this, "Filter", wxCollapsiblePane)->GetParent()->Layout();
}