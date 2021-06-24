/* Copyright (C) 2020 Christoph Theis */

#include "stdafx.h"
#include "MtUnscheduled.h"

#include "TT32App.h"

#include "MtEntryStore.h"

#include "CpItem.h"
#include "GrItem.h"
#include "MtItem.h"

#include "TTDbse.h"

#include "Request.h"

IMPLEMENT_DYNAMIC_CLASS(CMtUnscheduled, CFormViewEx)

BEGIN_EVENT_TABLE(CMtUnscheduled, CFormViewEx)
  EVT_COMBOBOX(XRCID("Event"), CMtUnscheduled::OnSelChangeCp)
  EVT_COMBOBOX(XRCID("Group"), CMtUnscheduled::OnSelChangeGr)
END_EVENT_TABLE()


class MtUnscheduledItem : public MtItem 
{
  public:
    MtUnscheduledItem(MtEntry &mt) : MtItem(mt, true) {}

    virtual int  Compare(const ListItem *itemPtr, int col) const;
    virtual void DrawColumn(wxDC *pDC, int col, wxRect &rect);
};


int MtUnscheduledItem::Compare(const ListItem *itemPtr, int col) const
{
  const MtUnscheduledItem *other = (const MtUnscheduledItem *) itemPtr;
  int i;

  // Fall-trhough beabsichtigt
  switch (col)
  {
    // Erst nach Datum und Zeit sortieren, dann nach WB, GR, ...
    case 6 :
    case 7 :
      if ( (mt.mt.mtPlace.mtDateTime < other->mt.mt.mtPlace.mtDateTime) )
        return -1;

      if ( (mt.mt.mtPlace.mtDateTime > other->mt.mt.mtPlace.mtDateTime) )
        return +1;

    case 1 :
      if ( (i =  wxStrcoll(mt.mt.cpName, other->mt.mt.cpName)) )
        return i;

    case 2 :
      if ( (i = wxStrcoll(mt.mt.grName, other->mt.mt.grName)) )
        return i;

    case 3 :
      if ( (i = mt.mt.mtEvent.mtRound - other->mt.mt.mtEvent.mtRound) )
        return i;

    case 4 :
      if ( (i = mt.mt.mtEvent.mtMatch - other->mt.mt.mtEvent.mtMatch) )
        return i;

    case 5 :
      return 0;

    default :
      return 0;
  }
}

void MtUnscheduledItem::DrawColumn(wxDC *pDC, int col, wxRect &rect)
{
  wxRect  rc = rect;

  rc.Deflate(offset);

  switch (col)
  {
    case 0 :
      DrawLong(pDC, rc, mt.mt.mtNr);
      break;

    case 1 :
      DrawString(pDC, rc, mt.mt.cpName);
      break;

    case 2 :
      DrawString(pDC, rc, mt.mt.grName);
      break;

    case 3 :
    {
      wxString str;

      if (mt.mt.mtEvent.mtRound > mt.mt.grQualRounds)
        str = wxString::Format("%d", mt.mt.mtEvent.mtRound - mt.mt.grQualRounds);
      else if (mt.mt.grQualRounds == 1)
        str = wxString::Format(_("Qu."));
      else
        str = wxString::Format(_("Qu.") + " %d", mt.mt.mtEvent.mtRound);

      DrawString(pDC, rect, str);

      break;
    }

    case 4 :
      DrawLong(pDC, rect, mt.mt.mtEvent.mtMatch);
      break;

    case 5 :
      DrawPair(pDC, rc, mt);
      break;

    case 6 :
      DrawString(pDC, rc, date);
      break;

    case 7 :
      DrawString(pDC, rc, time);
      break;
  }
}


CMtUnscheduled::CMtUnscheduled(wxMDIParentFrame *parent) : CFormViewEx(parent), m_listCtrl(0)
{
}

CMtUnscheduled::~CMtUnscheduled()
{
}


// -----------------------------------------------------------------------
void CMtUnscheduled::OnInitialUpdate()
{
  m_cbCp = XRCCTRL(*this, "Event", CComboBoxEx);
  m_cbGr = XRCCTRL(*this, "Group", CComboBoxEx);
  m_listCtrl = XRCCTRL(*this, "Matches", CListCtrlEx);

  m_listCtrl->SetItemHeight(1.5);

  m_listCtrl->InsertColumn(0, _("Mt.Nr"), wxALIGN_LEFT);
  m_listCtrl->InsertColumn(1, _("Event"), wxALIGN_LEFT);
  m_listCtrl->InsertColumn(2, _("Group"), wxALIGN_LEFT);
  m_listCtrl->InsertColumn(3, _("Round"), wxALIGN_LEFT);
  m_listCtrl->InsertColumn(4, _("Match"), wxALIGN_LEFT);
  m_listCtrl->InsertColumn(5, _("Players / Teams"), wxALIGN_LEFT);
  m_listCtrl->InsertColumn(6, _("Date"), wxALIGN_LEFT);
  m_listCtrl->InsertColumn(7, _("Time"), wxALIGN_LEFT);

  m_listCtrl->ResizeColumn(5);

  // CP fuellen
  CpListStore  cpList;

  // "All Events" entry
  cpList.cpID = 0;
  wxStrncpy(cpList.cpDesc, _("All Events").c_str(), sizeof(cpList.cpDesc) / sizeof(wxChar) - 1);
  m_cbCp->AddListItem(new CpItem(cpList));

  cpList.SelectAll();

  while (cpList.Next())
    m_cbCp->AddListItem(new CpItem(cpList));

  m_cbCp->SetCurrentItem(m_cbCp->GetListItem(0));
  if (!CTT32App::instance()->GetDefaultCP().IsEmpty())
    m_cbCp->SetCurrentItem(CTT32App::instance()->GetDefaultCP());

  OnSelChangeCp(wxCommandEvent());
}


void CMtUnscheduled::OnSelChangeCp(wxCommandEvent &)
{
  CpItem *cpItemPtr = (CpItem *)m_cbCp->GetCurrentItem();

  if (!cpItemPtr)
    return;

  CpRec cp = cpItemPtr->cp;

  m_cbGr->Clear();
  GrListStore grList;

  wxStrncpy(grList.grDesc, _("All Groups").c_str(), sizeof(grList.grDesc) / sizeof(wxChar) - 1);
  m_cbGr->AddListItem(new GrItem(grList));

  grList.SelectAll(cp);
  while (grList.Next())
    m_cbGr->AddListItem(new GrItem(grList));

  m_cbGr->SetCurrentItem(m_cbGr->GetListItem(0));
  if (cp.cpID && !CTT32App::instance()->GetDefaultGR().IsEmpty())
    m_cbGr->SetCurrentItem(CTT32App::instance()->GetDefaultGR());

  OnSelChangeGr(wxCommandEvent());
}


void CMtUnscheduled::OnSelChangeGr(wxCommandEvent &)
{
  m_listCtrl->RemoveAllListItems();

  MtEntryStore mt;

  CpItem *cpItemPtr = (CpItem *)m_cbCp->GetCurrentItem();

  if (!cpItemPtr)
    return;

  CpRec cp = cpItemPtr->cp;

  GrItem *grItemPtr = (GrItem *)m_cbGr->GetCurrentItem();

  if (!grItemPtr)
    return;

  GrRec gr = grItemPtr->gr;

  if (cp.cpID == 0 || cp.cpType == CP_SINGLE)
  {
    mt.SelectUnscheduled(CP_SINGLE, cp.cpID, gr.grID);

    while (mt.Next())
      m_listCtrl->AddListItem(new MtUnscheduledItem(mt));
    mt.Close();
  }

  if (cp.cpID == 0 || cp.cpType == CP_DOUBLE)
  {
    mt.SelectUnscheduled(CP_DOUBLE, cp.cpID, gr.grID);
    while (mt.Next())
      m_listCtrl->AddListItem(new MtUnscheduledItem(mt));
    mt.Close();
  }

  if (cp.cpID == 0 || cp.cpType == CP_MIXED)
  {
    mt.SelectUnscheduled(CP_MIXED, cp.cpID, gr.grID);
    while (mt.Next())
      m_listCtrl->AddListItem(new MtUnscheduledItem(mt));
    mt.Close();
  }

  if (cp.cpID == 0 || cp.cpType == CP_TEAM)
  {
    mt.SelectUnscheduled(CP_TEAM, cp.cpID, gr.grID);
    while (mt.Next())
      m_listCtrl->AddListItem(new MtUnscheduledItem(mt));
    mt.Close();
  }

  m_listCtrl->SortItems(6);
}

void  CMtUnscheduled::OnEdit()
{
  ListItem *itemPtr = m_listCtrl->GetCurrentItem();
  if (!itemPtr)
    return;

  CTT32App::instance()->OpenView(_("Edit Schedule"), wxT("MtTime"), itemPtr->GetID(), 1);
}


void CMtUnscheduled::OnUpdate(CRequest *reqPtr)
{
  // return;
  // Paranoia
  if (!reqPtr)
    return;

  // Nur Spiele beruecksichtigen
  if (reqPtr->rec != CRequest::MTREC)
    return;

  // Nur wenn eine ID bekannt ist
  if (!reqPtr->id)
    return;

  if (reqPtr->type != CRequest::UPDATE_SCHEDULE)
    return;

  MtListStore mtList;
  mtList.SelectById(reqPtr->id);
  if (!mtList.Next())
    return;
  mtList.Close();

  MtEntryStore mt;
  mt.SelectById(reqPtr->id, mtList.cpType);
  if (!mt.Next())
    return;
  mt.Close();

  MtUnscheduledItem *itemPtr = (MtUnscheduledItem *) m_listCtrl->FindListItem(reqPtr->id);
  if (itemPtr)
  {
    if (mt.mt.mtPlace.mtTable)
    {
      long idx = m_listCtrl->GetCurrentIndex();
      if (idx >= 0)
        m_listCtrl->SetCurrentIndex(idx + 1);
      m_listCtrl->RemoveListItem(reqPtr->id);
    }
    else
      itemPtr->mt = mt;
  }
  else if (!mt.mt.mtPlace.mtTable)
    m_listCtrl->AddListItem(new MtUnscheduledItem(mt));

  m_listCtrl->SortItems();
  m_listCtrl->Refresh();
}