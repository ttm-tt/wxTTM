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
  EVT_COMBOBOX(XRCID("Stages"), CGrListView::OnSelChangeStage)
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
  class StageItem : public ListItem
  {
    public:
      StageItem() : ListItem() {}
      StageItem(const wxString& stage) : ListItem(0, stage) {}

      virtual void DrawItem(wxDC* pDC, wxRect& rect)
      {
        DrawString(pDC, rect, m_label.IsEmpty() ? _("All Stages") : m_label);
      }
  };


  class GrItemEx : public GrItem
  {
    public:
      GrItemEx(const GrListRec &gr);

    public:
      void DrawColumn(wxDC *, int col, wxRect &rect);

      int  Compare(const ListItem *, int col) const;

    private:
      wxString  cpName;
      wxString  syName;
      wxString  mdName;
      timestamp stTimestamp = {0};
      timestamp mtTimestamp = {0};
  };
}


GrItemEx::GrItemEx(const GrListRec& gr) : GrItem(gr) 
{ 
  SetLabel(""); // Sonst hat man einen Tooltip

  cpName = gr.cpName;
  syName = gr.syName;
  stTimestamp = gr.stTimestamp;
  mtTimestamp = gr.mtTimestamp;

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

    // 1 (Event)
    case 1 :
      DrawString(pDC, rect, cpName);
      break;

    // 2 (Name) - 6 (Sort order) -> 0 - 4
    case 2 :
    case 3 :
    case 4 :
    case 5 :
    case 6 :
      GrItem::DrawColumn(pDC, col - 2, rect);
      break;

    case 7 :
      DrawString(pDC, rect, syName);
      break;

    case 8 :
      DrawString(pDC, rect, mdName);
      break;

    case 9 :
      if (gr.grPrinted.year)
      {
        DrawString(pDC, rect, wxString::Format("%04d-%02d-%02d %02d:%02d", 
          gr.grPrinted.year, gr.grPrinted.month, gr.grPrinted.day, gr.grPrinted.hour, gr.grPrinted.minute));
      }
      break;

    case 10:
      if (stTimestamp.year)
      {
        DrawString(pDC, rect, wxString::Format("%04d-%02d-%02d %02d:%02d", 
          stTimestamp.year, stTimestamp.month, stTimestamp.day, stTimestamp.hour, stTimestamp.minute));
      }
      break;

    case 11:
      if (mtTimestamp.year)
      {
        DrawString(pDC, rect, wxString::Format("%04d-%02d-%02d %02d:%02d", 
          mtTimestamp.year, mtTimestamp.month, mtTimestamp.day, mtTimestamp.hour, mtTimestamp.minute));
      }
      break;

    // 12 Notes -> 5
    case 12  : 
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

    case 1:
      return wxStrcoll(cpName, grItem->cpName);

    // 2 (Name) - 6 (Sort) -> 0 - 4
    case 2 :
    case 3 :
    case 4 :
    case 5 :
    case 6:
      return GrItem::Compare(itemPtr, col - 2);

    case 7 :
    {
      int ret = wxStrcoll(syName, grItem->syName);

      // If equal compare by name
      if (ret == 0)
        ret = wxStrcoll(gr.grName, grItem->gr.grName);

      return ret;
    }

    case 8 :
    {
      int ret = wxStrcoll(mdName, grItem->mdName);

      // If equal compare by name
      if (ret == 0)
        ret = wxStrcoll(gr.grName, grItem->gr.grName);

      return ret;
    }

    case 9 :
      if (gr.grPrinted > ((GrItemEx *)itemPtr)->gr.grPrinted)
        return 1;
      else if (gr.grPrinted < ((GrItemEx *)itemPtr)->gr.grPrinted)
        return -1;
      else
        return 0;

    case 10:
      if (stTimestamp > ((GrItemEx *) itemPtr)->stTimestamp)
        return 1;
      else if (stTimestamp < ((GrItemEx *) itemPtr)->stTimestamp)
        return -1;
      else
        return 0;

    case 11:
      if (mtTimestamp > ((GrItemEx *) itemPtr)->mtTimestamp)
        return 1;
      else if (mtTimestamp < ((GrItemEx *) itemPtr)->mtTimestamp)
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

  // "All Events" entry
  cp.cpID = 0;
  wxStrncpy(cp.cpDesc, _("All Events").c_str(), sizeof(cp.cpDesc) / sizeof(wxChar) - 1);
  m_cbCp->AddListItem(new CpItem(cp));

  cp.SelectAll();
  while (cp.Next())
    m_cbCp->AddListItem(new CpItem(cp));

  wxString cpName = CTT32App::instance()->GetDefaultCP();

  ListItem *cpItemPtr = m_cbCp->FindListItem(cpName);
  if (!cpItemPtr)
    cpItemPtr = m_cbCp->GetListItem(1);
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
  m_cbStage = XRCCTRL(*this, "Stages", CComboBoxEx);
  m_listCtrl = XRCCTRL(*this, "Groups", CListCtrlEx);
  
  int idx = 0;

  // Publish
  m_listCtrl->InsertColumn(idx, _("Pbl"), wxALIGN_CENTER, 3 * cW);
  idx++;

  // Event
  m_listCtrl->InsertColumn(idx, _("Event"), wxALIGN_LEFT, 6 * cW);
  idx++;

  // Name
  m_listCtrl->InsertColumn(idx++, _("Name"), wxALIGN_LEFT, 6 * cW);

  // Description
  m_listCtrl->InsertColumn(idx, _("Description"), wxALIGN_LEFT);
  m_listCtrl->SetResizeColumn(idx);
  idx++;

  // Stage
  m_listCtrl->InsertColumn(idx, _("Stage"), wxALIGN_LEFT, 10 * cW);
  m_listCtrl->SetSortColumn(idx);
  m_listCtrl->AllowHideColumn(idx);
  idx++;

  // Size
  m_listCtrl->InsertColumn(idx, _("Size"), wxALIGN_RIGHT, 6 * cW);
  m_listCtrl->HideColumn(idx);
  m_listCtrl->AllowHideColumn(idx);
  idx++;

  // Sort
  m_listCtrl->InsertColumn(idx, _("Sort"), wxALIGN_RIGHT, 1 * cW);
  m_listCtrl->HideColumn(idx);
  m_listCtrl->AllowHideColumn(idx);
  idx++;

  // System
  m_listCtrl->InsertColumn(idx, _("System"), wxALIGN_LEFT, 6 * cW);
  m_listCtrl->HideColumn(idx);
  m_listCtrl->AllowHideColumn(idx);
  idx++;

  // Modus
  m_listCtrl->InsertColumn(idx, _("Modus"), wxALIGN_LEFT, 4 * cW);
  m_listCtrl->HideColumn(idx);
  m_listCtrl->AllowHideColumn(idx);
  idx++;

  // Printed
  m_listCtrl->InsertColumn(idx, _("Printed"), wxALIGN_LEFT, 8 * cW);
  m_listCtrl->HideColumn(idx);
  m_listCtrl->AllowHideColumn(idx);
  idx++;

  // Draws updated
  m_listCtrl->InsertColumn(idx, _("Positions"), wxALIGN_LEFT, 8 * cW);
  m_listCtrl->HideColumn(idx);
  m_listCtrl->AllowHideColumn(idx);
  idx++;

  // Matches updated
  m_listCtrl->InsertColumn(idx, _("Matches"), wxALIGN_LEFT, 8 * cW);
  m_listCtrl->HideColumn(idx);
  m_listCtrl->AllowHideColumn(idx);
  idx++;

  // Notes
  m_listCtrl->InsertColumn(idx, _("Notes"), wxLIST_FORMAT_CENTER);
  m_listCtrl->HideColumn(idx);
  m_listCtrl->AllowHideColumn(idx);
  idx++;

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

  m_listCtrl->ShowColumn(1, itemPtr->GetID() == 0);

  FindWindow(IDC_ADD)->Enable(itemPtr->GetID() != 0);
  
  if (!itemPtr->GetID())
  {
    int col = m_listCtrl->GetSortColumn();

    cp.Init();

    OnChangeState(wxCommandEvent_);

    m_listCtrl->SortItems(col);
    m_listCtrl->SortItems(1);
    m_listCtrl->SetSortColumn(col);

    return;
  }

  cp.SelectById(itemPtr->GetID());
  cp.Next();

  CTT32App::instance()->SetDefaultCP(cp.cpName);

  std::list<wxString> stages = GrListStore().ListStages(cp);

  m_cbStage->Clear();

  // The All Stages item
  m_cbStage->AddListItem(new StageItem());

  for (wxString stage : stages)
    m_cbStage->AddListItem(new StageItem(stage));

  m_cbStage->SetCurrentItem(0);

  OnSelChangeStage(wxCommandEvent_);
}


void CGrListView::OnSelChangeStage(wxCommandEvent&)
{
  OnChangeState(wxCommandEvent_);
}


void CGrListView::OnChangeState(wxCommandEvent&)
{
  m_listCtrl->RemoveAllListItems();

  bool published = XRCCTRL(*this, "State", wxRadioBox)->GetSelection() != 1;
  bool unpublished = XRCCTRL(*this, "State", wxRadioBox)->GetSelection() != 2;

  StageItem* stageItemPtr = (StageItem*)m_cbStage->GetCurrentItem();
  if (!stageItemPtr)
    return;

  wxString stage = stageItemPtr->GetLabel();

  GrListStore  grList;
  grList.SelectByStage(cp, stage);

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
  bool published = XRCCTRL(*this, "State", wxRadioBox)->GetSelection() != 1;
  bool unpublished = XRCCTRL(*this, "State", wxRadioBox)->GetSelection() != 2;

  for (int idx = m_listCtrl->GetItemCount(); idx--; )
  {
    if (!m_listCtrl->IsSelected(idx))
      continue;

    GrItem *itemPtr = (GrItem *)m_listCtrl->GetListItem(idx);
    GrStore gr = itemPtr->gr;
    gr.SetPublish(true);

    if (unpublished &&!published)
      m_listCtrl->RemoveListItem(idx);
  }
}


void CGrListView::OnUnpublish(wxCommandEvent &)
{
  bool published = XRCCTRL(*this, "State", wxRadioBox)->GetSelection() != 1;
  bool unpublished = XRCCTRL(*this, "State", wxRadioBox)->GetSelection() != 2;

  for (int idx = m_listCtrl->GetItemCount(); idx--; )
  {
    if (!m_listCtrl->IsSelected(idx))
      continue;

    GrItem *itemPtr = (GrItem *)m_listCtrl->GetListItem(idx);
    GrStore gr = itemPtr->gr;
    gr.SetPublish(false);

    if (published && !unpublished)
      m_listCtrl->RemoveListItem(idx);

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