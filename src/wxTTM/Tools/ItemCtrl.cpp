/* Copyright (C) 2020 Christoph Theis */

#include "stdafx.h"

#include "ItemCtrl.h"

#include "ListItem.h"


IMPLEMENT_DYNAMIC_CLASS(CItemCtrl, wxControl)

BEGIN_EVENT_TABLE(CItemCtrl, wxControl)
 EVT_PAINT(CItemCtrl::OnPaint)
 EVT_SIZE(CItemCtrl::OnSize)
END_EVENT_TABLE()


CItemCtrl::CItemCtrl() : wxControl(), m_itemPtr(0)
{
  // Enable(false);
  SetItemHeight(1);
}


CItemCtrl::~CItemCtrl()
{
  delete m_itemPtr;
}


// -----------------------------------------------------------------------
void CItemCtrl::SetItemHeight(int height)
{
  wxSize sz =  wxScreenDC().GetTextExtent("M"); // wxClientDC(this).GetTextExtent("M");
  int w = sz.GetWidth();
  int h = sz.GetHeight();

  w *= 5;
  h *= (1.5 * height);
  
  SetMinSize(wxSize(w, h));  
}


// -----------------------------------------------------------------------
void CItemCtrl::OnPaint(wxPaintEvent &evt)
{
  wxRect rect = GetClientRect();
  wxPaintDC dc(this);
  
  dc.SetBrush(wxBrush(wxSystemSettings::GetColour(IsEnabled() ? wxSYS_COLOUR_WINDOW : wxSYS_COLOUR_BTNFACE)));
  dc.SetTextBackground(wxSystemSettings::GetColour(IsEnabled() ? wxSYS_COLOUR_WINDOW : wxSYS_COLOUR_BTNFACE));
  dc.SetTextForeground(wxSystemSettings::GetColour(IsEnabled() ? wxSYS_COLOUR_WINDOWTEXT : wxSYS_COLOUR_GRAYTEXT));
  
  dc.SetPen(*wxTRANSPARENT_PEN);
  
  dc.DrawRectangle(rect);
  
  if (m_itemPtr)
    m_itemPtr->DrawItem(&dc, rect);
} 



void CItemCtrl::OnSize(wxSizeEvent &size)
{
  size.Skip();
  Refresh();
}

// =======================================================================
IMPLEMENT_DYNAMIC_CLASS(CItemCtrlXmlResourceHandler, wxXmlResourceHandler)

wxObject * CItemCtrlXmlResourceHandler::DoCreateResource()
{
   XRC_MAKE_INSTANCE(item, CItemCtrl)

   item->Create(m_parentAsWindow,
                GetID(),
                GetPosition(), GetSize(),
                GetStyle(),
                wxDefaultValidator,
                GetName());

    SetupWindow(item);

    return item;
}


bool CItemCtrlXmlResourceHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, wxT("CItemCtrl"));
}