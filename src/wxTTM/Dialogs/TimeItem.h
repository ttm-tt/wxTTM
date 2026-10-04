/* Copyright (C) 2026 Christoph Theis */

#pragma once
#include <stdafx.h>
#include "ListItem.h"

// =======================================================================
class TimeItem : public ListItem
{
public:
  TimeItem() : ListItem()
  {
    SetLabel(_("No Time"));

    m_ts.year = m_ts.month = m_ts.day = 0;
    m_ts.hour = m_ts.minute = m_ts.second = 0;
  }

  TimeItem(const timestamp& ts) : ListItem(), m_ts(ts)
  {
    if (ts.year == 0)
      SetLabel(_("No Time"));
    else if (ts.hour < 0)
      SetLabel(_("All Times"));
    else
      SetLabel(wxString::Format(" %02d:%02d", ts.hour, ts.minute));
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
    if (m_ts.year < 0)
      DrawStringCentered(pDC, rect, _("All Times"));
    else if (m_ts.hour == 0 && m_ts.minute == 0)
      DrawStringCentered(pDC, rect, _("No Time"));
    else
    {
      DrawString(pDC, rect, wxString::Format(" %02d:%02d", m_ts.hour, m_ts.minute));
    }
  }

public:
  timestamp m_ts;
};

