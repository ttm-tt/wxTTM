/* Copyright (C) 2023 Christoph Theis */

#include "stdafx.h"

#include "TT32App.h"
#include "MdGroups.h"

#include "GrItem.h"
#include "MdItem.h"

#include "GrListStore.h"
#include "CpListStore.h"

#include <map>


IMPLEMENT_DYNAMIC_CLASS(CMdGroups, CFormViewEx)

BEGIN_EVENT_TABLE(CMdGroups, CFormViewEx)
  EVT_COMBOBOX(XRCID("GroupSystem"), CMdGroups::OnSelChangedGroupSystem)
  EVT_BUTTON(XRCID("Positions"), CMdGroups::OnPositions)
END_EVENT_TABLE()

namespace
{
  class GrItemEx : public ListItem
  {
    public :
      GrItemEx(const CpRec& cp_, const GrRec& gr_) : ListItem(), cp(cp_), gr(gr_) {}
      ~GrItemEx() {}

    public: 
      virtual int  Compare(const ListItem* itemPtr, int col) const;
      virtual void DrawColumn(wxDC* pDC, int col, wxRect& rect);

    public :
      CpRec cp;
      GrRec gr;

  };

  int  GrItemEx::Compare(const ListItem* itemPtr, int col) const
  {
    const GrItemEx * grItem = reinterpret_cast<const GrItemEx *> (itemPtr);
    if (!grItem)
      return -1;

    switch (col)
    {
      case 0 :
        return wxStrcoll(cp.cpName, grItem->cp.cpName);
        break;

      case 1 :
        return wxStrcoll(gr.grName, grItem->gr.grName);
        break;

      case 2 :
        return wxStrcoll(gr.grDesc, grItem->gr.grDesc);
        break;

      case 3:
        return wxStrcoll(gr.grStage, grItem->gr.grStage);
        break;

      default :
        return ListItem::Compare(itemPtr, col);
    }
  }


  void GrItemEx::DrawColumn(wxDC* pDC, int col, wxRect& rect)
  {
    wxRect    rcColumn = rect;

    switch (col)
    {
      case 0 :
        DrawString(pDC, rcColumn, cp.cpName);
        break;

      case 1 :
        DrawString(pDC, rcColumn, gr.grName);
        break;

      case 2 :
        DrawString(pDC, rcColumn, gr.grDesc);
        break;

      case 3:
        DrawString(pDC, rcColumn, gr.grStage);
        break;
    }
  }
}


CMdGroups::CMdGroups() : CFormViewEx()
{
}


CMdGroups::~CMdGroups()
{
}

// -----------------------------------------------------------------------
bool CMdGroups::Edit(va_list vaList)
{
  long mdID = va_arg(vaList, long);

  MdListStore md;
  md.SelectAll();
  while (md.Next())
    m_cbModus->AddListItem(new MdItem(md));

  m_cbModus->SetCurrentItem(mdID);

  OnSelChangedGroupSystem(wxCommandEvent_);

  return true;
}


// -----------------------------------------------------------------------
void CMdGroups::OnInitialUpdate()
{
  CFormViewEx::OnInitialUpdate();

  m_cbModus = XRCCTRL(*this, "GroupSystem", CComboBoxEx);
  m_grList = XRCCTRL(*this, "Groups", CListCtrlEx);

  m_grList->InsertColumn(0, _("Event"), wxALIGN_LEFT, 6 * cW);
  m_grList->InsertColumn(1, _("Name"), wxALIGN_LEFT, 6 * cW);
  m_grList->InsertColumn(2, _("Description"), wxALIGN_LEFT);
  m_grList->InsertColumn(3, _("Stage"), wxALIGN_LEFT, 16 * cW);

  m_grList->SetResizeColumn(2);
  m_grList->SetResizeColumn(3);
}


// -----------------------------------------------------------------------
void CMdGroups::OnSelChangedGroupSystem(wxCommandEvent &)
{
  ListItem* itemPtr = m_cbModus->GetCurrentItem();
  if (!itemPtr)
    return;

  m_grList->RemoveAllListItems();

  long mdID = itemPtr->GetID();

  MdListStore md;
  md.SelectById(mdID);
  md.Next();

  std::map<long, CpRec> cpList;
  CpListStore cp;
  cp.SelectAll();
  while (cp.Next())
    cpList[cp.cpID] = cp;
  cp.Close();

  md.Close();

  GrListStore gr;
  gr.SelectAll(md);
  while (gr.Next())
  {
    m_grList->AddListItem(new GrItemEx(cpList[gr.cpID], gr));
  }

  m_grList->SortItems(1);
  m_grList->SortItems(0);

  gr.Close();
}


// -----------------------------------------------------------------------
void CMdGroups::OnPositions(wxCommandEvent&)
{
  if (m_grList->GetSelectedCount() != 1)
    return;

  GrItemEx * itemPtr = (GrItemEx *) m_grList->GetCurrentItem();

  if (!itemPtr)
    return;

  long grID = itemPtr->gr.grID;

  CTT32App::instance()->OpenView(_("Group Positioning"), wxT("StListView"), grID);
}


// -----------------------------------------------------------------------
void CMdGroups::OnEdit()
{
  if (m_grList->GetSelectedCount() != 1)
    return;

  GrItemEx * itemPtr = (GrItemEx *) m_grList->GetCurrentItem();

  if (!itemPtr)
    return;

  long grID = itemPtr->gr.grID;
  long cpID = itemPtr->gr.cpID;

  CTT32App::instance()->OpenView(_("Edit Group"), wxT("GrEdit"), grID, cpID);
}