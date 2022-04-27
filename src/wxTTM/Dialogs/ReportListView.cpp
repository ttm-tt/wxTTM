/* Copyright (C) 2020 Christoph Theis */

// ReportList.cpp : implementation file
//

#include "stdafx.h"

#include "TT32App.h"
#include "ReportListView.h"
#include "ListItem.h"

#include "TTDbse.h"

#include "InfoSystem.h"
#include "Profile.h"
#include "Res.h"
#include "ReportManViewer.h"

#include <map>
#include <string>
#include <io.h>



// -----------------------------------------------------------------------
// Items fuer die Liste
class  ReportItem : public ListItem
{
  public:
    ReportItem(const wxString &title, const wxString &fname);

    const wxString & GetTitle() const {return m_title;}
    const wxString & GetFName() const {return m_fname;}

    virtual void DrawItem(wxDC *pDC, wxRect &rect);

  private:
    wxString m_title;
    wxString m_fname;
};


ReportItem::ReportItem(const wxString &title, const wxString &fname)
          : ListItem(0)
{
  m_title = title;
  m_fname = fname;
}


void ReportItem::DrawItem(wxDC *pDC, wxRect &rect)
{
  DrawString(pDC, rect, m_title.data());
}

// -----------------------------------------------------------------------
// CReportListView

IMPLEMENT_DYNAMIC_CLASS(CReportListView, CFormViewEx)

BEGIN_EVENT_TABLE(CReportListView, CFormViewEx)
END_EVENT_TABLE()


CReportListView::CReportListView() : CFormViewEx()
{
}


CReportListView::~CReportListView()
{
}


bool CReportListView::Edit(va_list vaList)
{
  wxString key;
  
  std::map<wxString, wxString> reportList;

  for (key = ttProfile.GetFirstKey(PRF_REPORTS); !key.IsEmpty(); key = ttProfile.GetNextKey())
  {
    wxString desc = ttProfile.GetString(PRF_REPORTS, key);
    reportList.insert(std::map<wxString, wxString>::value_type(
      wxString(key), wxString(desc) ));
      
    // m_reportList.AddListItem(new ReportItem(desc, key));
  }
  
  // And now the sepcial ones
  wxString pattern = CTT32App::instance()->GetPath();
  pattern += "\\*.rep";
  
  _finddata_t findinfo; 
  intptr_t h = _findfirst(pattern.data(), &findinfo);
  if (h != -1L)
  {
    do
    {
      wxString fname = findinfo.name;
      fname = fname.substr(0, fname.length() - 4);
      reportList.insert(std::map<wxString, wxString>::value_type(
        fname, fname));
    } while (_findnext(h, &findinfo) == 0);
    
    _findclose(h);
  }
    
  
  for (std::map<wxString, wxString>::iterator it = reportList.begin();
       it != reportList.end(); it++)
  {
    m_reportList->AddListItem(new ReportItem((*it).second.data(), (*it).first.data()));
  }

  return true;
}  


// -----------------------------------------------------------------------
// CReportListView message handlers

void  CReportListView::OnPrint()
{
  OnOK();
}


void  CReportListView::OnOK()
{
  ReportItem *itemPtr = (ReportItem *) m_reportList->GetCurrentItem();

  if (!itemPtr)
    return;

  CReportManViewer::DoReport(itemPtr->GetFName(), std::map<wxString, wxString>(), this);
}


void CReportListView::OnEdit()
{
  // Doppelklick (Edit) auf OK (Drucken) mappen
  OnOK();
}


// -----------------------------------------------------------------------
void CReportListView::OnInitialUpdate() 
{
	CFormViewEx::OnInitialUpdate();

  m_reportList = XRCCTRL(*this, "Reports", CListCtrlEx);
}

	
