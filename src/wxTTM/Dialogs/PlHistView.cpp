/* Copyright (C) 2020 Christoph Theis */

#include "stdafx.h"

#include "PlHistView.h"

#include "TT32App.h"

#include "PlItem.h"


// -----------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS(CPlHistView, CFormViewEx)

BEGIN_EVENT_TABLE(CPlHistView, CFormViewEx)
  // TODO: IDC_EVENTS doesn't work here
  // EVT_BUTTON(IDC_EVENTS, CPlHistView::OnPlHistEvents)
  EVT_BUTTON(XRCID("Events"), CPlHistView::OnPlHistEvents)
END_EVENT_TABLE()


const char * CPlHistView::headers[] = 
{
  wxTRANSLATE("Pl.Nr."),
  wxTRANSLATE("Name"),
  wxTRANSLATE("Assoc."),
  wxTRANSLATE("Sex"),
  wxTRANSLATE("Born in"),
  wxTRANSLATE("Extern ID"),
  wxTRANSLATE("Updated"),
  NULL
};

// -----------------------------------------------------------------------
namespace
{
  class PlItemHist : public PlItem
  {
    public:
      PlItemHist(const PlRec &rec) : PlItem(rec) {}

    public:
      virtual int  Compare(const ListItem *itemPtr, int col) const;
      virtual void DrawColumn(wxDC *pDC, int col, wxRect &rect);
      
  };

  int PlItemHist::Compare(const ListItem *itemPtr, int col) const
  {
    switch (col)
    {
      case 6 :
        if ( ((const PlItem *) itemPtr)->pl.psTimestamp < pl.psTimestamp )
          return -1;
        else if ( ((const PlItem *) itemPtr)->pl.psTimestamp > pl.psTimestamp )
          return +1;
        else
          return 0;

        break;

      default :
        return PlItem::Compare(itemPtr, col);
    }
  }

  void PlItemHist::DrawColumn(wxDC *pDC, int col, wxRect &rect)
  {
    switch (col)
    {
      case 6 :
      {
        timestamp ts = pl.psTimestamp;
        wxString when;
        if (ts.year == 9999)
          when = _("Current");
        else
          when = wxString::Format(
            "%04d-%02d-%02d %02d:%02d:%02d.%03d",
            ts.year, ts.month, ts.day, ts.hour, ts.minute, ts.second, ts.fraction / 1000000
          );
        DrawString(pDC, rect, when);

        break;
      }

      default :
        PlItem::DrawColumn(pDC, col, rect);
        break;
    }
  }
}

CPlHistView::CPlHistView() : CFormViewEx(), m_listCtrl(NULL)
{

}


CPlHistView::~CPlHistView()
{

}


// -----------------------------------------------------------------------
void CPlHistView::SaveSettings()
{
  m_listCtrl->SaveColumnInfo(GetClassInfo()->GetClassName());

  CFormViewEx::SaveSettings();
}


void CPlHistView::RestoreSettings()
{
  CFormViewEx::RestoreSettings();

  m_listCtrl->RestoreColumnInfo(GetClassInfo()->GetClassName());
}


// -----------------------------------------------------------------------
bool CPlHistView::Edit(va_list vaList)
{
  plID = va_arg(vaList, long);

  PlListStore pl;
  pl.SelectById(plID);
  pl.Next();
  pl.Close();

  // Marker for actual record
  pl.psTimestamp.year = 9999;
  m_listCtrl->AddListItem(new PlItemHist(pl));

  timestamp ts = {};
  pl.SelectById(plID, &ts);

  while (pl.Next())
    m_listCtrl->AddListItem(new PlItemHist(pl));

  pl.Close();

  m_listCtrl->SortItems(6);

  return true;
}


// -----------------------------------------------------------------------
void CPlHistView::OnInitialUpdate()
{
  m_listCtrl = XRCCTRL(*this, "History", CListCtrlEx);

  for (size_t i = 0; headers[i]; i++)
  {
    m_listCtrl->InsertColumn(i, wxGetTranslation(headers[i]), wxLIST_FORMAT_LEFT, i == 1 ? GetClientSize().GetWidth() : wxLIST_AUTOSIZE_USEHEADER);
  }

  for (size_t i = 0; headers[i]; i++)
  {
    if (i == 6)
      m_listCtrl->SetColumnWidth(i, 15 * cW);
    else if (i != 1)
      m_listCtrl->SetColumnWidth(i, wxLIST_AUTOSIZE_USEHEADER);
  }

  m_listCtrl->ShowColumn(4, CTT32App::instance()->GetType() == TT_YOUTH || CTT32App::instance()->GetType() == TT_SCI);
  m_listCtrl->HideColumn(5);

  m_listCtrl->AllowHideColumn(0);
  m_listCtrl->AllowHideColumn(2);
  m_listCtrl->AllowHideColumn(3);
  m_listCtrl->AllowHideColumn(4);
  m_listCtrl->AllowHideColumn(5);

  m_listCtrl->SetResizeColumn(1);

  FindWindow("Events")->SetId(IDC_EVENTS);
}


// -----------------------------------------------------------------------
void CPlHistView::OnPlHistEvents(wxCommandEvent &)
{
  CTT32App::instance()->OpenView(_T("Event History"), wxT("PlHistEvents"), plID);
}

