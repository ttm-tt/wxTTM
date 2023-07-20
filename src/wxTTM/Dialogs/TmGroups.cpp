/* Copyright (C) 2023 Christoph Theis */

#include "stdafx.h"

#include "TT32App.h"
#include "TmGroups.h"

#include "GrItem.h"

#include "GrListStore.h"
#include "MtListStore.h"


// Put specific classes into a namespace
namespace
{
  class ScheduleItem : public ListItem
  {
  public:
    ScheduleItem(const MtListRec& mt, bool reverse);

    virtual void DrawItem(wxDC* pDC, wxRect& rect);

  private:
    wxString date;
    wxString time;
    wxString table;
    wxString res;
  };



  ScheduleItem::ScheduleItem(const MtListRec& mt, bool reverse) : ListItem(mt.mtID)
  {
    struct tm  tmp;
    memset(&tmp, 0, sizeof(struct tm));

    tmp.tm_year = mt.mtPlace.mtDateTime.year - 1900;
    tmp.tm_mon = mt.mtPlace.mtDateTime.month - 1;
    tmp.tm_mday = mt.mtPlace.mtDateTime.day;
    tmp.tm_hour = mt.mtPlace.mtDateTime.hour;
    tmp.tm_min = mt.mtPlace.mtDateTime.minute;
    tmp.tm_sec = mt.mtPlace.mtDateTime.second;
    tmp.tm_isdst = -1;

    date = wxDateTime(tmp).Format(CTT32App::instance()->GetDateFormat());
    if (mt.mtPlace.mtDateTime.hour)
      time = wxDateTime(tmp).Format(CTT32App::instance()->GetTimeFormatW());
    if (mt.mtPlace.mtTable)
      table = wxString::Format("T. %d", mt.mtPlace.mtTable);

    if (reverse && mt.mtWalkOverX || !reverse && mt.mtWalkOverA)
      res = _("w/o");
    else if (reverse && mt.mtDisqualifiedX || !reverse && mt.mtDisqualifiedA)
      res = _("Disqu.");
    else if (reverse && mt.mtInjuredX || !reverse && mt.mtInjuredA)
      res = _("Inj.");
    else if (mt.mtResA || mt.mtResX)
      res = wxString::Format("%d : %d", reverse ? mt.mtResX : mt.mtResA, reverse ? mt.mtResA : mt.mtResX);
  }


  void ScheduleItem::DrawItem(wxDC* pDC, wxRect& rect)
  {
    wxRect rectDate = rect, rectTime = rect, rectTable = rect, rectRes = rect;
    rectDate.SetRight(rectDate.GetLeft() + rect.GetWidth() / 3);
    rectTime.SetLeft(rectDate.GetRight());
    rectTime.SetRight(rectTime.GetLeft() + rect.GetWidth() / 3);
    rectTable.SetLeft(rectTime.GetRight());
    rectTable.SetRight(rectTable.GetLeft() + rect.GetWidth() / 6);
    rectRes.SetLeft(rectTable.GetRight());

    DrawString(pDC, rectDate, date);
    DrawString(pDC, rectTime, time);
    DrawString(pDC, rectTable, table);
    DrawString(pDC, rectRes, res);
  }
}



IMPLEMENT_DYNAMIC_CLASS(CTmGroups, CFormViewEx)

BEGIN_EVENT_TABLE(CTmGroups, CFormViewEx)
  EVT_LIST_ITEM_SELECTED(XRCID("Groups"), CTmGroups::OnSelChangedEnteredGroups)
  END_EVENT_TABLE()


CTmGroups::CTmGroups() : CFormViewEx()
{
}


CTmGroups::~CTmGroups()
{
}

// -----------------------------------------------------------------------
bool CTmGroups::Edit(va_list vaList)
{
  long id = va_arg(vaList, long);

  tm.SelectTeamById(id, CP_TEAM);
  tm.Next();
  tm.Close();

  m_tmItem->SetListItem(new TmItem(tm));

  GrListStore gr;
  gr.SelectByTm(tm);
  while (gr.Next())
    m_tmGroups->AddListItem(new GrItem(gr));
;


  return true;
}


// -----------------------------------------------------------------------
void CTmGroups::OnInitialUpdate()
{
  m_tmItem = XRCCTRL(*this, "Team", CItemCtrl);
  m_tmGroups = XRCCTRL(*this, "Groups", CListCtrlEx);
  m_tmSchedules = XRCCTRL(*this, "Schedules", CListCtrlEx);
}



// -----------------------------------------------------------------------
void CTmGroups::OnSelChangedEnteredGroups(wxListEvent&)
{
  ListItem* itemPtr = m_tmGroups->GetCurrentItem();
  if (!itemPtr)
    return;

  m_tmSchedules->RemoveAllListItems();

  GrListStore gr;
  gr.SelectById(itemPtr->GetID());
  gr.Next();
  gr.Close();

  MtListStore mt;
  mt.SelectByGrTm(gr, tm);
  while (mt.Next())
  {
    if (!mt.mtPlace.mtDateTime.day)
      continue;

    ListItem* itemPtr = new ScheduleItem(mt, tm.tmID == mt.tmX);
    m_tmSchedules->InsertListItem(itemPtr);
  }
}

