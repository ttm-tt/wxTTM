/* Copyright (C) 2025 Christoph Theis */

#pragma once
#include <stdafx.h>
#include  "ListItem.h"

// =======================================================================
class DateItem : public ListItem
{
public:
  DateItem() : ListItem()
  {
    SetLabel(_("No Time"));

    m_ts.year = m_ts.month = m_ts.day = 0;
    m_ts.hour = m_ts.minute = m_ts.second = 0;
  }

  DateItem(const timestamp& ts) : ListItem(), m_ts(ts)
  {
    if (ts.year == 0)
      SetLabel(_("No Time"));
    else if (ts.year < 0)
      SetLabel(_("All Dates"));
    else
      SetLabel(wxString::Format("%d.%d.%d", ts.day, ts.month, ts.year));
  };

public:
  int Compare(const ListItem* itemPtr, int col) const
  {
    return wxStrcoll(GetLabel(), itemPtr->GetLabel());
  }

  bool HasString(const wxString& str) const
  {
    return wxStrcoll(GetLabel(), str) == 0;
  }

  const timestamp& GetTimestamp() const
  {
    return m_ts;
  }

  void DrawItem(wxDC* pDC, wxRect& rect)
  {
    if (m_ts.year == 0)
      DrawStringCentered(pDC, rect, _("No Time"));
    else if (m_ts.year < 0)
      DrawStringCentered(pDC, rect, _("All Dates"));
    else
    {
      wxDateTime dt(m_ts.day, (wxDateTime::Month)(m_ts.month - 1), m_ts.year);
      DrawString(pDC, rect, dt.Format(" %d.%m.%Y, %a"));
    }
  }

public:
  timestamp m_ts;
};

