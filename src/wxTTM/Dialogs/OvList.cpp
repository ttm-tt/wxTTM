/* Copyright (C) 2020 Christoph Theis */

// OvList.cpp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"

#include "OvList.h"
#include "OvItem.h"

#include "CpListStore.h"
#include "GrListStore.h"
#include "MdListStore.h"
#include "MtListStore.h"

#include "MtListView.h"

#include "ReportManViewer.h"

#include "MtTime.h"
#include "MtRes.h"
#include "MtTeam.h"
#include "Score.h"
#include "TossSheet.h"

#include "MtEntryStore.h"

#include "Printer.h"
#include "Profile.h"
#include "Request.h"
#include "Res.h"

#include <set>
#include <map>


typedef  std::map<long, StEntry, std::less<long> >  StEntryMap;

static const int UPDATE_TIME = 5 * 1000;

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
    
    DateItem(const timestamp &ts) : ListItem(), m_ts(ts) 
    {
      if (ts.year == 0)
        SetLabel(_("No Time"));
      else
        SetLabel(wxString::Format("%d.%d.%d", ts.day, ts.month, ts.year));      
    };
  
  public:
    int Compare(const ListItem *itemPtr, int col) const
    { 
      return wxStrcoll(GetLabel(), itemPtr->GetLabel());
    }
    
    bool HasString(const wxString &str) const
    {
      return wxStrcoll(GetLabel(), str) == 0;
    }
    
    const timestamp & GetTimestamp() const 
    {
      return m_ts;
    }
    
    void DrawItem(wxDC *pDC, wxRect &rect)
    {
      if (m_ts.year == 0)
        DrawStringCentered(pDC, rect, _("No Time"));
      else
      {
        wxDateTime dt(m_ts.day, (wxDateTime::Month) (m_ts.month - 1), m_ts.year);
        DrawString(pDC, rect, dt.Format(" %d.%m.%Y, %a"));
      }
    }
    
    
  private:
    timestamp m_ts;
};

// =======================================================================
typedef std::vector<std::vector<OvItem *>> GridItemList;

class OvItemGridTable;

// Renderer, der ein OvItem zeichnet
class OvItemGridCellRenderer : public wxGridCellRenderer
{
  public:
    OvItemGridCellRenderer();
   ~OvItemGridCellRenderer();
   
  public:
    virtual wxGridCellRenderer * Clone() const {return new OvItemGridCellRenderer();}    
    virtual void Draw(wxGrid &grid, wxGridCellAttr &attr, wxDC &dc, const wxRect &rect, int row, int col, bool isSelected);
    virtual wxSize GetBestSize(wxGrid &grid, wxGridCellAttr &attr, wxDC &dc, int row, int col);
};


// Attribute provider, damit die Zellen r/o sind
class OvItemGridCellAttrProvider : public wxGridCellAttrProvider
{
  public:
    OvItemGridCellAttrProvider();
   ~OvItemGridCellAttrProvider();
   
  public:
    wxGridCellAttr * GetAttr(int row, int col, wxGridCellAttr::wxAttrKind kind) const;
    
  private:
    wxGridCellAttr *m_cellAttr;
};


// Model des Grids
class OvItemGridTable : public wxGridTableBase
{
  public:
    OvItemGridTable();
   ~OvItemGridTable();
   
  public:
    virtual int GetNumberRows() 
    {
      return m_rowLabels.GetCount();
    }
    
    virtual int GetNumberCols() 
    {
      return m_colLabels.GetCount();
    }
    
    virtual bool AppendRows(size_t numRows = 1);
    virtual bool AppendCols(size_t numCols = 1);
    
    virtual bool IsEmptyCell(int row, int col);
    virtual wxString GetValue(int row, int col) {return "";}
    virtual void SetValue(int row, int col, const wxString &) {}
    
    virtual void Clear();
    
    virtual void SetRowLabelValue(int row, const wxString &label) 
    {
      m_rowLabels[row] = label;
    }
    
    virtual void SetColLabelValue(int col, const wxString &label) 
    {
      m_colLabels[col] = label;
    }
    
    virtual wxString GetRowLabelValue(int row) 
    {
      return row < (int) m_rowLabels.GetCount() ? m_rowLabels[row].c_str() : wxEmptyString;
    }
    
    virtual wxString GetColLabelValue(int col) 
    {
      return col < (int) m_colLabels.GetCount() ? m_colLabels[col].c_str() : wxEmptyString;
    }

    void SetRowLabelType(int row, OvItem::OvType type)
    {
      m_rowTypes[row] = (int) type;
    }

    OvItem::OvType GetRowLabelType(int row)
    {
      if (m_rowTypes[row] == OvItem::OVLAST)
      {
        for (int col = 0; col < GetNumberCols(); col++)
        {
          OvItem *item = GetItem(row, col);
          if (item && item->GetOvType() < m_rowTypes[row])
            m_rowTypes[row] = item->GetOvType();
        }
      }

      return (OvItem::OvType) m_rowTypes[row];
    }

  public:
    void SetItem(int row, int col, OvItem *item);
    OvItem * GetItem(int row, int col);
    
  private:
    wxArrayString m_rowLabels;
    wxArrayString m_colLabels;
    wxArrayInt   m_rowTypes;
    GridItemList m_itemList; 
};


inline OvItemGridCellRenderer::OvItemGridCellRenderer()
{
}


inline OvItemGridCellRenderer::~OvItemGridCellRenderer()
{
}


inline wxSize OvItemGridCellRenderer::GetBestSize(wxGrid &grid, wxGridCellAttr &, wxDC &, int, int)
{
  return wxSize(grid.GetDefaultRowSize(), grid.GetDefaultColSize());
}


inline void OvItemGridCellRenderer::Draw(wxGrid &grid, wxGridCellAttr &attr, wxDC &dc, const wxRect &rect, int row, int col, bool isSelected)
{
  OvItem *itemPtr = ((OvItemGridTable *) grid.GetTable())->GetItem(row, col);
  
  if (itemPtr)
  {
    dc.SetBrush(wxBrush(itemPtr->GetBkColor()));
    dc.SetPen(*wxTRANSPARENT_PEN);
    
    dc.DrawRectangle(rect);
    
    dc.SetPen(wxPen(itemPtr->GetFgColor()));
    dc.SetFont(grid.GetDefaultCellFont());
    itemPtr->DrawItem(&dc, (wxRect &) rect);
  }
  else
  {
    dc.SetBrush(wxBrush(grid.GetLabelBackgroundColour()));
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(rect);
  }  
}


inline OvItemGridCellAttrProvider::OvItemGridCellAttrProvider()
{
  m_cellAttr = new wxGridCellAttr();
  m_cellAttr->SetReadOnly();
}


inline OvItemGridCellAttrProvider::~OvItemGridCellAttrProvider()
{
  m_cellAttr->DecRef();
}


inline wxGridCellAttr * OvItemGridCellAttrProvider::GetAttr(int row, int col, wxGridCellAttr::wxAttrKind kind) const
{
  wxGridCellAttr *attr = wxGridCellAttrProvider::GetAttr(row, col, kind);
  
  if (attr == NULL)
  {
    attr = m_cellAttr;
    m_cellAttr->IncRef();
  }
  else
  {
    attr->SetReadOnly();
  }  
  
  return attr;
}

inline OvItemGridTable::OvItemGridTable()
{
}


inline OvItemGridTable::~OvItemGridTable()

{
}


inline bool OvItemGridTable::IsEmptyCell(int row, int col)
{
  if (m_itemList.size() <= (size_t) row || m_itemList[row].size() <= (size_t) col)
    return true;
    
  return m_itemList[row][col] == NULL;
}


inline void OvItemGridTable::Clear()
{
  int oldRowCount = GetNumberRows();
  int oldColCount = GetNumberCols();
  
  if (GetView())  
  {
    // notify the grid
    if (oldRowCount > 0)
    {
      wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_DELETED, 0, oldRowCount);
      GetView()->ProcessTableMessage(msg);        
    }
    
    if (oldColCount > 0)
    {
      wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_COLS_DELETED, 0, oldColCount);
      GetView()->ProcessTableMessage(msg);        
    }
  }    

  m_itemList.clear();
  
  m_rowLabels.Clear();
  m_colLabels.Clear();
  m_rowTypes.Clear();
}


inline bool OvItemGridTable::AppendRows(size_t numRows)
{
  m_rowLabels.Add("", numRows);
  m_rowTypes.Add(OvItem::OVLAST);
  
  if (GetView())  
  {
    // notify the grid
    wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_APPENDED, numRows);
    GetView()->ProcessTableMessage(msg);        
  }    
  
  return true;
}


inline bool OvItemGridTable::AppendCols(size_t numCols)
{
  m_colLabels.Add("", numCols);
  
  if (GetView())  
  {
    // notify the grid
    wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_COLS_APPENDED, numCols);
    GetView()->ProcessTableMessage(msg);        
  }    
  
  return true;
}

inline void OvItemGridTable::SetItem(int row, int col, OvItem *item)
{
  if (m_itemList.size() <= (size_t) row)
    m_itemList.resize(row + 1);

  if (m_itemList[row].size() <= (size_t) col)
      m_itemList[row].resize(col + 1);
  
  m_itemList[row][col] = item;
}


inline OvItem * OvItemGridTable::GetItem(int row, int col)
{
  if (m_itemList.size() <= (size_t) row || m_itemList[row].size() <= (size_t) col)
    return NULL;
    
  return m_itemList[row][col];
}


class COvGridCtrl : public wxGrid
{
  public:
    COvGridCtrl();
   ~COvGridCtrl();
   
  public:
    OvItem * GetItem(int row, int col);
    void SetItem(int row, int col, OvItem *item); 
    
    void Clear() {GetTable()->Clear();}
    
    virtual void DoSetSize(int x, int y,
                           int width, int height,
                           int sizeFlags = wxSIZE_AUTO)
    {
      wxGrid::DoSetSize(x, y, width, height, sizeFlags);
    }
    
    virtual void DoSetClientSize(int width, int height)
    {
      wxGrid::DoSetClientSize(width, height);
    }

    virtual void DrawRowLabel(wxDC &dc, int row)
    {
      if ( GetRowHeight(row) <= 0 || m_rowLabelWidth <= 0 )
          return;

      wxGridCellAttrProvider * const
          attrProvider = m_table ? m_table->GetAttrProvider() : NULL;

      // notice that an explicit static_cast is needed to avoid a compilation
      // error with VC7.1 which, for some reason, tries to instantiate (abstract)
      // wxGridRowHeaderRenderer class without it
      const wxGridRowHeaderRenderer &rend = attrProvider ?
           attrProvider->GetRowHeaderRenderer(row) : wxGridCellAttrProvider().GetRowHeaderRenderer(row);

      wxRect rect(0, GetRowTop(row), m_rowLabelWidth, GetRowHeight(row));
      rend.DrawBorder(*this, dc, rect);

      OvItem::OvType type = ((OvItemGridTable *) GetTable())->GetRowLabelType(row);
      wxColor fgColor = OvItem::GetFgColor(type);
      wxColor bkColor = OvItem::GetBkColor(type);

      dc.SetBackground(bkColor);

      dc.SetBrush(wxBrush(bkColor));
      dc.SetPen(*wxTRANSPARENT_PEN);
      dc.SetFont(GetLabelFont());
      dc.DrawRectangle(rect);

      int hAlign, vAlign;
      GetRowLabelAlignment(&hAlign, &vAlign);

      dc.SetTextBackground(bkColor);
      dc.SetTextForeground(fgColor);

      dc.SetBrush(*wxTRANSPARENT_BRUSH);
      dc.SetPen(wxPen(fgColor));

      DrawTextRectangle(dc, GetRowLabelValue(row), rect, hAlign, vAlign, wxHORIZONTAL);

      // rend.DrawLabel(*this, dc, GetRowLabelValue(row),
      //                rect, hAlign, vAlign, wxHORIZONTAL);
    }
    
  DECLARE_DYNAMIC_CLASS(COvGridCtrl)
  DECLARE_EVENT_TABLE()
};


IMPLEMENT_DYNAMIC_CLASS(COvGridCtrl, wxGrid)

BEGIN_EVENT_TABLE(COvGridCtrl, wxGrid)

END_EVENT_TABLE()


inline COvGridCtrl::COvGridCtrl() : wxGrid()
{
}


inline COvGridCtrl::~COvGridCtrl()
{
}


inline OvItem * COvGridCtrl::GetItem(int row, int col)
{
  return ((OvItemGridTable *) GetTable())->GetItem(row, col);
}


inline void COvGridCtrl::SetItem(int row, int col, OvItem *item)
{
  ((OvItemGridTable *) GetTable())->SetItem(row, col, item);
}
// -----------------------------------------------------------------------
// COvList

IMPLEMENT_DYNAMIC_CLASS(COvList, CFormViewEx)


BEGIN_EVENT_TABLE(COvList, CFormViewEx)
  EVT_BUTTON(IDC_REFRESH, CFormViewEx::OnCommand)
  EVT_BUTTON(XRCID("Umpire"), COvList::OnShowUmpire)

  EVT_GRID_CELL_LEFT_CLICK(COvList::OnCellLeftClick)
  EVT_GRID_CELL_LEFT_DCLICK(COvList::OnCellDoubleClick)

  EVT_GRID_LABEL_LEFT_CLICK(COvList::OnCellLeftClick)
  
  EVT_TIMER(wxID_ANY, COvList::OnTimer)

  EVT_COMBOBOX(XRCID("CellSize"), COvList::OnGridSizeChanged)
END_EVENT_TABLE()



COvList::COvList() : CFormViewEx(), m_popupTimer(this), m_updateTimer(this)
{
  m_showUmpire = false;
  m_initialized = false;
  m_freezeTitle = false;

  memset(&m_fromTime, 0, sizeof(timestamp));
  memset(&m_toTime, 0, sizeof(timestamp));

  time_t t = time(0);
  struct tm *tm = localtime(&t);

  m_fromTime.year  = tm->tm_year + 1900;
  m_fromTime.month = tm->tm_mon + 1;
  m_fromTime.day   = tm->tm_mday;

  m_toTime.year = m_fromTime.year;
  m_toTime.month = m_fromTime.month;
  m_toTime.day = m_fromTime.day;

  m_toTime.hour = 23;
  m_toTime.minute = 59;

  m_fromTable = 0;
  m_toTable = 999;
  
}


COvList::~COvList()
{
}


// -----------------------------------------------------------------------
void COvList::SaveSettings()
{
  wxString str = wxString::Format(
    "%d;%d;%d;%d;%d;%d",
    m_fromTable, m_fromTime.hour, m_fromTime.minute, m_toTable, m_toTime.hour, m_toTime.minute);

  ttProfile.AddString(PRF_LOCAL_SETTINGS, PRF_OVLIST_SETTINGS, str);

  CFormViewEx::SaveSettings();
}


void COvList::RestoreSettings()
{
  CFormViewEx::RestoreSettings();

  wxString str = ttProfile.GetString(PRF_LOCAL_SETTINGS, PRF_OVLIST_SETTINGS, "");

  int ft = m_fromTable, fh = m_fromTime.hour, fm = m_fromTime.minute;
  int tt = m_toTable, th = m_toTime.hour, tm = m_toTime.minute;

  if (wxSscanf(str.t_str(), "%d;%d;%d;%d;%d;%d", &ft, &fh, &fm, &tt, &th, &tm) != 6)
    return;

  m_fromTable = ft;
  m_toTable = tt;

#if 0
  // Erstmal noch nicht die Zeit setzen
  m_fromTime.hour = fh;
  m_fromTime.minute = fm;
  m_toTime.hour = th;
  m_toTime.minute = tm;
# endif
}


// Wir speichern nicht unsere Groesse
void COvList::SaveSize()
{
}


void COvList::RestoreSize()
{
}


bool COvList::Edit(va_list vaList)
{
  std::list<timestamp> tsList;
  tsList = MtListStore().ListVenueDays(m_fromTable, m_toTable);

  ListItem *currentDateItem = NULL;

  for (int i = 0; i < OvItem::OVLAST; i++)
  {
    OvItem::OvType type = OvItem::OvType(i);
    wxColor fg = CTT32App::instance()->GetOvFgColor(OvItem::OvTypeToName(type));
    wxColor bk = CTT32App::instance()->GetOvBkColor(OvItem::OvTypeToName(type));

    if (fg != wxNullColour)
    {
      OvItem::SetFgColor(type, fg);
      FindWindow(OvItem::OvTypeToName(type))->SetForegroundColour(fg);
      FindWindow(OvItem::OvTypeToName(type))->Refresh();
    }

    if (bk != wxNullColour)
    {
      OvItem::SetBkColor(type, bk);
      FindWindow(OvItem::OvTypeToName(type))->SetBackgroundColour(bk);
      FindWindow(OvItem::OvTypeToName(type))->Refresh();
    }
  }

  timestamp fromTime = m_fromTime;;
  fromTime.hour = fromTime.minute = fromTime.second = fromTime.fraction = 0;
  
  while (tsList.size() > 0)
  {
    ListItem *itemPtr = NULL;
    
    timestamp ts = *tsList.begin();
    tsList.pop_front();
    
    m_cbDate->AddListItem(itemPtr = new DateItem(ts));
    
    if (currentDateItem == NULL && ts >= fromTime)
      currentDateItem = itemPtr;
  }
  
  if (m_cbDate->GetCount() == 0)
    m_cbDate->AddListItem(new DateItem());
  
  if (currentDateItem == NULL)
    m_cbDate->SetSelection(0);
  else
    m_cbDate->SetCurrentItem(currentDateItem);

  TransferDataToWindow();

  // OnRefresh();
  return true;
}


// -----------------------------------------------------------------------
// COvList message handlers
void COvList::OnInitialUpdate() 
{
  const int cellSize = ttProfile.GetInt(PRF_GLOBAL_SETTINGS, PRF_OVLIST_CELLSIZE, 60);
  int fontSize = ttProfile.GetInt(PRF_GLOBAL_SETTINGS, PRF_OVLIST_FONTSIZE, 8);
  if (fontSize <= 3)
    XRCCTRL(*this, "CellSize", wxComboBox)->Select(0);
  else if (fontSize <= 5)
    XRCCTRL(*this, "CellSize", wxComboBox)->Select(1);
  else if (fontSize <= 7)
    XRCCTRL(*this, "CellSize", wxComboBox)->Select(2);
  else if (fontSize <= 8)
    XRCCTRL(*this, "CellSize", wxComboBox)->Select(3);
  else if (fontSize >= 9)
    XRCCTRL(*this, "CellSize", wxComboBox)->Select(4);

	CFormViewEx::OnInitialUpdate();	

  FindWindow("Refresh")->SetId(IDC_REFRESH);

  m_gridCtrl = XRCCTRL(*this, "Grid", COvGridCtrl);
  m_gridCtrl->SetTable(new OvItemGridTable(), true);
  m_gridCtrl->SetDefaultRenderer(new OvItemGridCellRenderer());
  m_gridCtrl->GetTable()->SetAttrProvider(new OvItemGridCellAttrProvider());
  m_gridCtrl->SetColLabelSize(cellSize);
  m_gridCtrl->SetDefaultColSize(cellSize);
  m_gridCtrl->SetRowLabelSize(cellSize);
  m_gridCtrl->SetDefaultRowSize(cellSize);
  m_gridCtrl->SetScrollRate(cellSize, cellSize);
  m_gridCtrl->SetDefaultCellBackgroundColour(GetParent()->GetBackgroundColour());
  m_gridCtrl->SetGridLineColour(*wxBLACK);
  
  // Kann ein wxGrid kein wxEVT_CONTEXT_MENU?
  m_gridCtrl->GetGridCornerLabelWindow()->Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(COvList::OnContextMenuGrid), NULL, this);
  m_gridCtrl->GetGridRowLabelWindow()->Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(COvList::OnContextMenuGrid), NULL, this);
  m_gridCtrl->GetGridWindow()->Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(COvList::OnContextMenuGrid), NULL, this);

  wxFont labelFont = m_gridCtrl->GetLabelFont();
  labelFont.SetPointSize(fontSize);
  m_gridCtrl->SetLabelFont(labelFont);

  wxFont cellFont = m_gridCtrl->GetDefaultCellFont();
  cellFont.SetPointSize(fontSize);
  m_gridCtrl->SetDefaultCellFont(cellFont);
  
  // m_gridCtrl->GetGridWindow()->Connect(wxEVT_MOTION, wxMouseEventHandler(COvList::OnMotion), NULL, this);
  // m_gridCtrl->GetGridRowLabelWindow()->Connect(wxEVT_MOTION, wxMouseEventHandler(COvList::OnMotion), NULL, this);
  
  m_cbDate = XRCCTRL(*this, "Date", CComboBoxEx);
  
  FindWindow("FromTime")->SetValidator(CTimeValidator(&m_fromTime, false));
  FindWindow("ToTime")->SetValidator(CTimeValidator(&m_toTime, false));
  FindWindow("FromTable")->SetValidator(CShortValidator(&m_fromTable));
  FindWindow("ToTable")->SetValidator(CShortValidator(&m_toTable));

  FindWindow("Error")->Connect(wxEVT_CONTEXT_MENU, wxContextMenuEventHandler(COvList::OnContextMenuLabel), NULL, this);
  FindWindow("Incomplete")->Connect(wxEVT_CONTEXT_MENU, wxContextMenuEventHandler(COvList::OnContextMenuLabel), NULL, this);
  FindWindow("NotPrinted")->Connect(wxEVT_CONTEXT_MENU, wxContextMenuEventHandler(COvList::OnContextMenuLabel), NULL, this);
  FindWindow("Missing")->Connect(wxEVT_CONTEXT_MENU, wxContextMenuEventHandler(COvList::OnContextMenuLabel), NULL, this);
  FindWindow("Printed")->Connect(wxEVT_CONTEXT_MENU, wxContextMenuEventHandler(COvList::OnContextMenuLabel), NULL, this);
  FindWindow("Finished")->Connect(wxEVT_CONTEXT_MENU, wxContextMenuEventHandler(COvList::OnContextMenuLabel), NULL, this);
  FindWindow("Checked")->Connect(wxEVT_CONTEXT_MENU, wxContextMenuEventHandler(COvList::OnContextMenuLabel), NULL, this);

  FindWindow("Close")->Show(GetParent()->IsTopLevel());

  Connect(IDC_REFRESH, wxCommandEventHandler(COvList::OnCommand));
  Connect(wxID_PRINT, wxCommandEventHandler(COvList::OnCommand));
}


void COvList::OnRefresh()
{
  // Letzte Updatezeit in DB merken
  m_lastUpdateTime = MtListStore().GetLastUpdateTime();

  if (m_updateTimer.IsRunning())
    m_updateTimer.Stop();

  TransferDataFromWindow();
  
  std::list<timestamp> tsList;
  tsList = MtListStore().ListVenueDays(m_fromTable, m_toTable);

  DateItem *currentDateItem = (DateItem *) m_cbDate->GetCurrentItem();

  timestamp oldDate = currentDateItem->GetTimestamp();
  currentDateItem = NULL;

  int sbX, sbY;
  m_gridCtrl->GetViewStart(&sbX, &sbY);
  
  int nofX = m_gridCtrl->GetNumberCols();
  int nofY = m_gridCtrl->GetNumberRows();
  
  m_cbDate->RemoveAllListItems();
  
  while (tsList.size() > 0)
  {
    DateItem *itemPtr = NULL;
    
    timestamp ts = *tsList.begin();
    tsList.pop_front();
    
    m_cbDate->AddListItem(itemPtr = new DateItem(ts));
    
    if ( currentDateItem == NULL && ts == oldDate )
      currentDateItem = itemPtr;
  }
  
  if (m_cbDate->GetCount() == 0)
    m_cbDate->AddListItem(new DateItem());
  
  if (currentDateItem != NULL)
    m_cbDate->SetCurrentItem(currentDateItem);
  else
    m_cbDate->SetSelection(0);
  
  DateItem *itemPtr = (DateItem *) m_cbDate->GetCurrentItem();
  
  timestamp from;
  if (itemPtr)
    from = itemPtr->GetTimestamp();

  if (m_freezeTitle)
    ; // Nothing, title frozen in time
  else if (from.year)
  {
    // Title entsprechend aendern
    wxDateTime dt(from.day, (wxDateTime::Month) (from.month - 1), from.year);
    wxString title = dt.Format("%a, %d.%m.%Y");
    if (m_fromTime.hour > 0 || m_toTime.hour < 23 || m_toTime.hour == 2 && m_toTime.minute < 59)
      title += wxString::Format(
        " %02d:%02d - %02d:%02d", m_fromTime.hour, m_fromTime.minute, m_toTime.hour, m_toTime.minute);

    if (m_fromTable > 0 || m_toTable < 999)
      title += wxString::Format(" T. %d - %d", m_fromTable, m_toTable);

    wxCommandEvent evt(IDC_SET_TITLE);
    evt.SetEventObject(this);
    evt.SetString(title);
    ProcessEvent(evt);
  }
  else
  {
    wxCommandEvent evt(IDC_SET_TITLE);
    ProcessEvent(evt);
  }
  
  m_fromTime.year = m_toTime.year = from.year;
  m_fromTime.month = m_toTime.month = from.month;
  m_fromTime.day = m_toTime.day = from.day;
  
  m_initialized = false;
  
  m_gridCtrl->Freeze();

  m_gridCtrl->Clear();

  MtListStore  mt;

  short lastTable = 0;
  timestamp  lastTime;
  memset(&lastTime, 0, sizeof(timestamp));

  mt.SelectByTime(m_fromTime, m_fromTable, m_toTime, m_toTable, true);

  if (!mt.Next())
  {
    m_gridCtrl->Thaw();
    return;
  }
    
  // Gruppensetzungen merken
  StEntryMap stMap;

  std::list<MtListRec> mtList;
    
  long lastGrID = 0;
  GrListStore  gr;
  MdListStore  md;

  long lastType = OvItem::OVLAST;
    
  do
  {
    if (mt.mtPlace.mtTable)
    {
      if (m_fromTable && mt.mtPlace.mtTable < m_fromTable ||
          m_toTable && mt.mtPlace.mtTable > m_toTable)
        continue;
    }
    else if (mt.mtPlace.mtDateTime.year == 0)
    {
      // Keine Spiele mit Freilosen, die keine Zeit haben
      if ( mt.stA != 0 && mt.tmA == 0 && mt.xxAstID == 0 ||
           mt.stX != 0 && mt.tmX == 0 && mt.xxXstID == 0 )
        continue;
    }
      
    mtList.push_back(mt);
  } while (mt.Next());
  
  // Gruppen und Modi als Bulk abfragen.
  // Bei hohen Latenzen des Netwerks dauert eine Abfrage one-per-one sonst Minute(n)
  std::set<long> grIdList;
  std::map<long, GrRec> grMap;

  for (std::list<MtListRec>::iterator itMT = mtList.begin();
    itMT != mtList.end(); itMT++)
  {
    MtListRec mt = (*itMT);

    grIdList.insert(mt.mtEvent.grID);
  }

  gr.SelectById(grIdList);

  while (gr.Next())
  {
    grMap[gr.grID] = gr;
  }

  std::set<long> mdIdList;
  std::map<long, MdRec> mdMap;
  for (auto it = grMap.begin(); it != grMap.end(); it++)
  {
    if (gr.mdID)
      mdIdList.insert(gr.mdID);
  }

  md.SelectById(mdIdList);

  while (md.Next())
  {
    mdMap[gr.grID] = md;
  }

  int noTableCount = 0;

  for (std::list<MtListRec>::iterator itMT = mtList.begin();
    itMT != mtList.end(); itMT++)
  {
    MtListRec mt = (*itMT);

    // Neue Zeile, wenn es eine andere Zeit ist, 
    // oder der gleiche Tisch 
    // oder die erste Zeile (0-basiert)
    if (mt.mtPlace.mtDateTime != lastTime ||
        (mt.mtPlace.mtTable > 0) && (mt.mtPlace.mtTable == lastTable) || 
        m_gridCtrl->GetNumberRows() == 0)
    {
      int oldCount = m_gridCtrl->GetNumberRows();
      m_gridCtrl->AppendRows(1);

      if (mt.mtPlace.mtDateTime.hour || mt.mtPlace.mtDateTime.minute)
      {
        wxString tmp;
        if (mt.mtPlace.mtDateTime.second == 0)
          tmp = wxString::Format("%02d : %02d", mt.mtPlace.mtDateTime.hour, mt.mtPlace.mtDateTime.minute);
        else if (mt.mtPlace.mtDateTime.second == 1)
          tmp = wxString::Format("%02d : %02d", mt.mtPlace.mtDateTime.hour, mt.mtPlace.mtDateTime.minute);
        else
          tmp = _("Next");

        m_gridCtrl->SetRowLabelValue(oldCount, tmp);
      }
      else
        m_gridCtrl->SetRowLabelValue(oldCount, _("No Time"));

      lastType = OvItem::OVLAST;

      noTableCount = 0;
    }

    if (mt.mtPlace.mtTable == 0 && m_fromTable > 0)
    {
      lastTime = mt.mtPlace.mtDateTime;
      lastTable = mt.mtPlace.mtTable;

      ((OvItemGridTable *) m_gridCtrl->GetTable())->SetRowLabelType(m_gridCtrl->GetNumberRows() - 1, OvItem::OVERROR);

      continue;
    }

    if (lastGrID != mt.mtEvent.grID)
    {
      gr.GrRec::operator=(grMap[mt.mtEvent.grID]);

      lastGrID = gr.grID;
      
      if (gr.grModus == MOD_RR && gr.mdID != md.mdID)
        md.MdRec::operator=(mdMap[gr.mdID]);
    }
      
    OvItem::OvType type = OvItem(mt).GetOvType();

    // Alle Spiele einer Runde (oder einer Gruppe) duerfen auf einem Tisch sein
    // TODO: Eine Runde an mehreren Tischen spielen?
    if ( gr.grModus == MOD_RR && mt.mtEvent.mtMatch == 1 )
    {
      std::list<MtListRec>::iterator itTmp = itMT;   // Iterator
      std::list<MtListRec>::iterator itTmp2 = itMT;  // Letztes Spiel einer Gruppe auf einem Tisch
      std::list<MtListRec>::iterator itTmp3 = itMT;  // Erstes Spiel, das kein Freilos ist
      while ( itTmp != mtList.end() &&
              (*itTmp).mtPlace.mtDateTime == mt.mtPlace.mtDateTime && 
              (*itTmp).mtPlace.mtTable == mt.mtPlace.mtTable &&
              (*itTmp).mtEvent.grID == mt.mtEvent.grID
            )
      {
        // Merken, welches Spiel kein Freilos war
        if ( ((*itTmp3).IsABye() || (*itTmp3).IsXBye()) && !(*itTmp).IsABye() && !(*itTmp).IsXBye() )
          itTmp3 = itTmp;
          
        itTmp2 = itTmp;
        itTmp++;    

        if (OvItem(*itTmp2).GetOvType() < type)
          type = OvItem(*itTmp2).GetOvType();
      }
      
      if ( itTmp == mtList.end() ||
           (*itTmp).mtEvent.grID != mt.mtEvent.grID ||
           (*itTmp).mtEvent.mtMatch == 1
         )
      {
        // Weiter geht es mit dem letzten Spiel einer Serie, 
        // eingetragen wird aber das erste Spiel, das kein Freilos ist.
        itMT = itTmp2;
        mt = (*itTmp3);

        // Wenn auf einem Tisch alle (mehr als 1) Spiele ab der ersten Runde, stattfindet,
        // dann die Runde auf 0 setzen (als Marker). OvItem zeigt dann keine Runde an.
        if ( mt.mtEvent.mtRound == 1 && mt.mtID != (*itTmp2).mtID && 
             (itTmp == mtList.end() || (*itTmp).mtEvent.grID != mt.mtEvent.grID) )
          mt.mtEvent.mtRound = 0;
      }
    }
        
    if ( (mt.mtPlace.mtTable - m_fromTable) > m_gridCtrl->GetNumberCols() - 1)
    {
      int oldCount = m_gridCtrl->GetNumberCols();
      m_gridCtrl->AppendCols(mt.mtPlace.mtTable - m_fromTable - oldCount + 1);
      
      for (; oldCount < m_gridCtrl->GetNumberCols(); oldCount++)
      {
        if (oldCount + m_fromTable)
          m_gridCtrl->SetColLabelValue(oldCount, wxString::Format(_("T %d"), oldCount + m_fromTable));
        else
          m_gridCtrl->SetColLabelValue(oldCount, _("No Table"));
      }
    }

		int col  = mt.mtPlace.mtTable - m_fromTable;
		int row  = m_gridCtrl->GetNumberRows() - 1;
		
		if (row < 0)
		  row = 0;
		
    // Ein Spiel am gleichen Tisch zur gleichen Zeit wie zuletzt?
    if ( row > 0 && mt.mtPlace.mtTable > 0 && 
         mt.mtPlace.mtTable == lastTable && 
         mt.mtPlace.mtDateTime == lastTime )
    {
      OvItem *prevItem = m_gridCtrl->GetItem(row - 1, col);
      
      if (prevItem != NULL)
      {
        prevItem->SetOvType(OvItem::OVERROR);
        
        m_gridCtrl->SetItem(row, col, new OvItem(mt, OvItem::OVERROR)); 
      }
    }
    else if (mt.mtPlace.mtTable == 0)
    {
      ++noTableCount;

      wxString ttText;
      if (noTableCount == 1)
        m_gridCtrl->SetItem(row, col, new OvItem(mt, OvItem::OVERROR));
      else
      {
        ttText = m_gridCtrl->GetItem(row, col)->GetToolTipText();
        if (ttText.IsEmpty())
        {
          MtListRec oldMt = m_gridCtrl->GetItem(row, col)->mt;
          if (oldMt.mtID)
            ttText = wxString::Format("%s %s Rd. %d Mt. %d", oldMt.cpName, oldMt.grName, oldMt.mtEvent.mtRound, oldMt.mtEvent.mtMatch);
        }

        ttText += wxString::Format("\n%s %s Rd. %d Mt. %d", mt.cpName, mt.grName, mt.mtEvent.mtRound, mt.mtEvent.mtMatch);

        m_gridCtrl->SetItem(row, col, new OvItem(wxString::Format(_(" \n%d\nMts.\n "), noTableCount), OvItem::OVERROR));
        m_gridCtrl->GetItem(row, col)->SetToolTipText(ttText);
      }
    }
    else
    {
      m_gridCtrl->SetItem(row, col, new OvItem(mt, type));
    }
    
    // Jetzt erst umsetzen
    lastTable = mt.mtPlace.mtTable;
    lastTime = mt.mtPlace.mtDateTime;

		OvItem *itemPtr = m_gridCtrl->GetItem(row, col);
    
    m_gridCtrl->SetCellBackgroundColour(row, col, itemPtr->GetBkColor());
    m_gridCtrl->SetCellTextColour(row, col, itemPtr->GetFgColor());
  
    itemPtr->SetShowUmpire(&m_showUmpire);
  }

  if (nofX == m_gridCtrl->GetNumberCols() && nofY == m_gridCtrl->GetNumberRows())
    m_gridCtrl->Scroll(sbX, sbY);
  
  // Die SchiRi-Uebersicht hintenrum reinfummeln, wenn sie angewaehlt war
  if (m_showUmpire)
  {
    m_showUmpire = !m_showUmpire;
    OnShowUmpire(wxCommandEvent_);
  }  

  m_gridCtrl->Thaw();
  
  // m_gridCtrl->Fit();

  m_updateTimer.Start(UPDATE_TIME, false);
}


void  COvList::OnShowUmpire(wxCommandEvent &)
{
  if (!m_initialized)
  {
    // Die Verbaende werden nur bei Bedarf gelesen und in OvItem gesetzt
    // Sonst dauert das normale Aufschalten zu lange  
    long lastGrID = 0;
    
    StEntryMap  stMap;

    {
      std::map<short, std::set<long>> stSets;
      for (int row = 0; row < m_gridCtrl->GetNumberRows(); row++)
      {
        for (int col = 0; col < m_gridCtrl->GetNumberCols(); col++)
        {
          OvItem *itemPtr = m_gridCtrl->GetItem(row, col);

          if (!itemPtr)
            continue;

          // Beide Spieler bekannt oder existieren nicht
          if ((!itemPtr->mt.stA || itemPtr->tmA.tmID) &&
              (!itemPtr->mt.stX || itemPtr->tmX.tmID))
            continue;

          if (itemPtr->mt.stA)
            stSets[itemPtr->mt.cpType].insert(itemPtr->mt.stA);
          if (itemPtr->mt.stX)
            stSets[itemPtr->mt.cpType].insert(itemPtr->mt.stX);
        }
      }

      StEntryStore st;

      for (const auto &it : stSets)
      {
        st.SelectById(it.second, it.first);
        while (st.Next())
          stMap[st.st.stID] = st;

        st.Close();
      }
    }

    for (int row = 0; row < m_gridCtrl->GetNumberRows(); row++)
    {
      for (int col = 0; col < m_gridCtrl->GetNumberCols(); col++)
      {
        OvItem *itemPtr = m_gridCtrl->GetItem(row, col);
        
        if (!itemPtr)
          continue;

        // Beide Spieler bekannt oder existieren nicht
        if ( (!itemPtr->mt.stA || itemPtr->tmA.tmID) &&
            (!itemPtr->mt.stX || itemPtr->tmX.tmID) )
          continue;
                    
        // Und hier die Spielerdaten aus der Map rausholen
        if (itemPtr->mt.stA && stMap.find(itemPtr->mt.stA) != stMap.end())
          itemPtr->tmA = (*stMap.find(itemPtr->mt.stA)).second;
        if (itemPtr->mt.stX && stMap.find(itemPtr->mt.stX) != stMap.end())
          itemPtr->tmX = (*stMap.find(itemPtr->mt.stX)).second;    
      }
    }
    
    m_initialized = true;
  }  
  
  m_showUmpire = !m_showUmpire;
  
  XRCCTRL(*this, "Umpire", wxButton)->SetLabel(m_showUmpire ? _("Match") : _("Umpire"));

  m_gridCtrl->Refresh();
}


// -----------------------------------------------------------------------
void  COvList::OnPrint()
{
  // colCount / rowCount zaehlen den Inhalt der Tabelle, also ohne Header
  int rowCount = m_gridCtrl->GetNumberRows();
  int colCount = m_gridCtrl->GetNumberCols();
  
  // colCount und rowCount sind 0, wenn es nichts zu drucken gibt
  if (colCount < 0 || rowCount < 0)
    return;

  bool showDlg = !wxGetKeyState(WXK_SHIFT);

  Printer *printer;
  if (CTT32App::instance()->GetPrintPreview())
    printer = new PrinterPreview(_("Overview"), showDlg);
  else if (CTT32App::instance()->GetPrintPdf())
  {
    wxFileDialog fileDlg(
      this, wxFileSelectorPromptStr, CTT32App::instance()->GetPath(), "Overview", 
      wxT("PDF Files (*.pdf)|*.pdf|All Files (*.*)|*.*||"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (fileDlg.ShowModal() != wxID_OK)
      return;

    printer = new PrinterPdf(fileDlg.GetPath(), showDlg);
  }
  else
    printer = new Printer(showDlg);

  if (printer->PrinterAborted())
  {
    delete printer;
    return;
  }

  if (!printer->StartDoc(_("Overview")))
    return;
    
  printer->StartPage();
  
  short  boldFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_SMALLB);
	short  tinyFont = printer->LoadFont(TT_PROFILE, PRF_RASTER, PRF_RASTER_TINY);
  printer->SelectFont(tinyFont);

  // colCount solange verkleinern, bis ein Teil der Tabelle auf die Seite passt
  while (printer->width / (colCount + 1) < 10 * printer->cW)  // 20 CW / 2
    colCount = (colCount + 1) / 2;
    
  // Die Zellengroesse beruecksichtigt natuerlich den Header
  long cellSize = std::min(printer->width / (colCount + 1), 
                           printer->height / (rowCount + 1));

  if (m_showUmpire)
    cellSize = std::max(cellSize, 14 * printer->cW);
  else
    cellSize = std::max(cellSize, 10 * printer->cW);

  // Aber mindestens so breit wie "No Table"
  cellSize = std::max(cellSize, printer->TextWidth(_("No Table")));
    
  cellSize = std::min(cellSize, 20 * printer->cW);

  // rowCount und colCount endgueltig berechnen
  rowCount = std::min(m_gridCtrl->GetNumberRows() + 1, (int) (printer->height / cellSize)) - 1;
  colCount = std::min(m_gridCtrl->GetNumberCols() + 1, (int) (printer->width / cellSize)) - 1;
  
  int startCol = 0;
  int startRow = 0;
  int offsetY  = 0;
  
  while (startCol < m_gridCtrl->GetNumberCols())
  {
    int neededCols = std::min(colCount, m_gridCtrl->GetNumberCols() - startCol);
    
    // neededCols ist einschliesslich Header (einfacher zu rechnen)
    neededCols += 1;
    
    while (startRow < m_gridCtrl->GetNumberRows())
    {
      int neededRows = std::min(rowCount, m_gridCtrl->GetNumberRows() - startRow);
      
      // neededRows ist einschliesslich Header (einfacher zu rechnen)
      neededRows += 1;
      
      if ( (startRow > 0) || (offsetY + neededRows * cellSize > printer->height) )
      {
        printer->EndPage();
        printer->StartPage();
 
        printer->SelectFont(tinyFont);
        
        offsetY = 0;
      }
      
      // Raster
      for (int col = 1; col <= neededCols; col++)
      {
        printer->Line(col * cellSize, offsetY + 0, 
                      col * cellSize, offsetY + neededRows * cellSize);
      }

      for (int row = 1; row <= neededRows; row++)
      {
        printer->Line(0, offsetY + row * cellSize, 
                      neededCols * cellSize, offsetY + row * cellSize);
      }

      for (int col = 0; col < neededCols; col++)
      {
        for (int row = 0; row < neededRows; row++)
        {
          // Kopfzeile und -Spalte fett drucken.
          if (col == 0 || row == 0)
            printer->SelectFont(boldFont);
          else
            printer->SelectFont(tinyFont);

          wxRect rect(col * cellSize, offsetY + row * cellSize, cellSize, cellSize);
           
          // In die Ecke kommt das Datum          
          if (col == 0 && row == 0)
          {
            tm  tmp;
            memset(&tmp, 0, sizeof(tmp));
            tmp.tm_year = m_fromTime.year - 1900;
            tmp.tm_mon  = m_fromTime.month - 1;
            tmp.tm_mday = m_fromTime.day;
            tmp.tm_hour = m_fromTime.hour;
            tmp.tm_min  = m_fromTime.minute;
            tmp.tm_sec  = m_fromTime.second;
            tmp.tm_isdst = -1;

            wxChar  strTime[64];
            wxStrftime(strTime, 63, "%d %b", &tmp);
            
            printer->Text(rect, strTime, wxALIGN_CENTER);
            
            continue;
          }   
          else if (col == 0)
          {
            printer->Text(rect, m_gridCtrl->GetRowLabelValue(row + startRow - 1), wxALIGN_CENTER);

            continue;
          }
          else if (row == 0)
          {
            printer->Text(rect, m_gridCtrl->GetColLabelValue(col + startCol - 1), wxALIGN_CENTER);

            continue;
          }
          
          OvItem *itemPtr = m_gridCtrl->GetItem(row + startRow - 1, col + startCol - 1);              
  
          if (!itemPtr)
            continue;        

          itemPtr->PrintItem(printer, rect);
        }
      }
      
      // Soweit sind wir weiter
      offsetY += neededRows * cellSize;
      
      // Plus Abstand zwischen den Tabellen
      offsetY += cellSize;

      startRow += rowCount;            
    }
    
    startRow = 0;
    startCol += colCount;
  }

  printer->EndPage();
  printer->EndDoc();
  
  printer->DeleteFont(boldFont);
  printer->DeleteFont(tinyFont);
}


// -----------------------------------------------------------------------

class ToolTipItem : public wxStaticText
{
  public:
    ToolTipItem(wxWindow *parent,OvItem *itemPtr) 
        : wxStaticText(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE), 
          m_itemPtr(itemPtr) 
    {
      Connect(wxEVT_PAINT, wxPaintEventHandler(ToolTipItem::OnPaint));
    }

    ToolTipItem(wxWindow *parent, const wxString &text) 
        : wxStaticText(parent, wxID_ANY, text, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE), 
          m_itemPtr(NULL) 
    {
    }

  public:
    virtual bool AcceptFocus() const { return false; }
    
  private:
    void OnPaint(wxPaintEvent &evt);
  
    
  private:
    OvItem *m_itemPtr;
};

void ToolTipItem::OnPaint(wxPaintEvent &evt)
{
  wxPaintDC dc(this);
  dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK)));
  dc.SetPen(*wxTRANSPARENT_PEN);
  dc.DrawRectangle(GetPosition(), GetSize());
  
  dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOTEXT)));
  m_itemPtr->DrawToolTip(&dc, GetClientRect());
}


void COvList::OnCellLeftClick(wxGridEvent &evt)
{
  if (m_popupTimer.IsRunning())
  {
    m_popupTimer.Stop();
    OnCellDoubleClick(evt);

    return;
  }

  // Tooltip nur wenn RowLabel oder Zelle mit Inhalt
  if (evt.GetCol() < 0 || m_gridCtrl->GetItem(evt.GetRow(), evt.GetCol()))
    m_popupTimer.Start(wxSystemSettings::GetMetric(wxSYS_DCLICK_MSEC, this), true);

   evt.Skip();
}


void COvList::OnMotion(wxMouseEvent &evt)
{
  if (m_popupTimer.IsRunning())
    m_popupTimer.Stop();
    
  m_popupTimer.Start(wxSystemSettings::GetMetric(wxSYS_DCLICK_MSEC, this), true);
}


void COvList::OnTimer(wxTimerEvent &evt)   
{   
  if (evt.GetTimer().GetId() == m_updateTimer.GetId())
    OnUpdateTimer(evt);
  else if (evt.GetTimer().GetId() == m_popupTimer.GetId())
    OnPopupTimer(evt);
}


void COvList::OnPopupTimer(wxTimerEvent &evt)
{
  // Umrechnen in Grid-Koordinaten. Da das Contextmenu fuer Grid und RowLabel 
  // registriert wird, bekomme ich im MouseEvent unterschiedliche Koordinaten.
  // Daher von absoluten Koordinaten ausgehen und auf das Grid umrechnen..
  wxPoint pt = m_gridCtrl->ScreenToClient(wxGetMousePosition());

  // Scrolling beruecksichtigen
  wxPoint pos = m_gridCtrl->CalcUnscrolledPosition(pt);

  // Die Mauskoordinatenbeziehen sich auf das gesamte Fenster, die Umrechnung auf das Grid nimmt 
  // aber Koordinaaten im GridWindow an. Darum die entsprechenden Groessen abziehen.
  // Ebenfalls getrennt umrechnen, denn wenn col oder row < 0 sind, setzt XYToCell beide auf -1.
  int row = pt.y < m_gridCtrl->GetColLabelSize() ? -1 : m_gridCtrl->YToRow(pos.y - m_gridCtrl->GetColLabelSize());
  int col = pt.x < m_gridCtrl->GetRowLabelSize() ? -1 : m_gridCtrl->XToCol(pos.x - m_gridCtrl->GetRowLabelSize());

  // coordinates of the cell under mouse
  wxGridCellCoords coords(row, col); 

  OvItem *itemPtr = m_gridCtrl->GetItem(coords.GetRow(), coords.GetCol());

  if (itemPtr == NULL && coords.GetCol() > 0)
    return;

  if (coords.GetRow() < 0)
    return;

  if (!m_gridCtrl->HasFocus() && coords.GetCol() < 0)
    m_gridCtrl->SetFocus();
  
  wxPopupTransientWindow *popup = new wxPopupTransientWindow(this);
  
  if (itemPtr)
  {
    ToolTipItem *toolTipItem = new ToolTipItem(popup, itemPtr);
    wxClientDC dc(this);
    popup->SetSize(itemPtr->GetToolTipSize(&dc));
    toolTipItem->SetSize(popup->GetSize());
  }
  else
  {
    static wxString texts[] = {
      wxTRANSLATE("Error"),
      wxTRANSLATE("Incomplete"),
      wxTRANSLATE("Not Printed"),
      wxTRANSLATE("Missing"),
      wxTRANSLATE("Printed"),
      wxTRANSLATE("Finished"),
      wxTRANSLATE("Checked")
    };
    
    int count[OvItem::OVLAST] = {0};
    
    for (int col = 0; col < m_gridCtrl->GetNumberCols(); col++)
    {
      if (m_gridCtrl->GetItem(coords.GetRow(), col))
        count[(int) m_gridCtrl->GetItem(coords.GetRow(), col)->GetOvType()]++;
    }
        
    wxString text;
    int maxWidth = 0;
    int nofLines = 0;
    
    for (int i = 0; i < OvItem::OVLAST; i++)
    {
      if (count[i])
      {
        nofLines++;
        wxString tmp = wxString::Format("%s: %d\n", wxGetTranslation(texts[i]).wx_str(), count[i]);        
        text += tmp;
      }
    }
    
    // Letztes \n entfernen
    text.RemoveLast();
    
    ToolTipItem *toolTipItem = new ToolTipItem(popup, text);
            
    popup->SetSize(toolTipItem->GetSize());
    
    toolTipItem->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOBK));
    toolTipItem->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_INFOTEXT));
  }
  
  pt.y -= popup->GetSize().GetHeight();
  popup->Position(m_gridCtrl->ClientToScreen(pt), wxSize());
  
  popup->Popup();
}


void  COvList::OnCellDoubleClick(wxGridEvent &evt)
{
  if (m_popupTimer.IsRunning())
    m_popupTimer.Stop();
    
  OvItem *itemPtr = m_gridCtrl->GetItem(evt.GetRow(), evt.GetCol());
  
  if (!itemPtr || !itemPtr->mt.mtNr)
    return;

  bool combined = itemPtr->mt.mtEvent.mtRound == 0;
    
  if (m_showUmpire)
    CTT32App::instance()->OpenView(_("Edit Schedule"), wxT("MtTime"), itemPtr->mt.mtID, combined ? 5 : 1);
  else if (combined)
    CTT32App::instance()->OpenView(_("Edit Group Results"), wxT("MtListView"), itemPtr->mt.mtID, false);
  else if (itemPtr->mt.cpType == CP_TEAM)
    CTT32App::instance()->OpenView(_("Edit Team Result"), wxT("MtTeam"), itemPtr->mt.mtID);
  else
    CTT32App::instance()->OpenView(_("Edit Result"), wxT("MtRes"), itemPtr->mt.mtID, 0);  
}


void COvList::OnContextMenuGrid(wxMouseEvent &evt)
{
  static wxString emptyCellMenu[] = 
  {
    wxTRANSLATE("Print Tossheet"),
    wxTRANSLATE("Print Scoresheet"),
    wxTRANSLATE("Print Match List")
  };
  
  static wxString matchCellMenu[] = 
  {
    wxTRANSLATE("Print Tosssheet"),
    wxTRANSLATE("Print Scoresheet"),
    wxTRANSLATE("Find Match"),
    wxTRANSLATE("View Details"),
    wxTRANSLATE("Edit Result"),
    wxTRANSLATE("Edit Schedule"),
    wxTRANSLATE("Swap Table"),
  };
  
  if (m_popupTimer.IsRunning())
    m_popupTimer.Stop();
    
  // Umrechnen in Grid-Koordinaten. Da das Contextmenu fuer Grid und RowLabel 
  // registriert wird, bekomme ich im MouseEvent unterschiedliche Koordinaten.
  // Daher von absoluten Koordinaten ausgehen und auf das Grid umrechnen..
  wxPoint pt = m_gridCtrl->ScreenToClient(wxGetMousePosition());

  // Scrolling beruecksichtigen
  wxPoint pos = m_gridCtrl->CalcUnscrolledPosition(pt);

  // Die Mauskoordinatenbeziehen sich auf das gesamte Fenster, die Umrechnung auf das Grid nimmt 
  // aber Koordinaaten im GridWindow an. Darum die entsprechenden Groessen abziehen.
  // Ebenfalls getrennt umrechnen, denn wenn col oder row < 0 sind, setzt XYToCell beide auf -1.
  int row = pt.y < m_gridCtrl->GetColLabelSize() ? -1 : m_gridCtrl->YToRow(pos.y - m_gridCtrl->GetColLabelSize());
  int col = pt.x < m_gridCtrl->GetRowLabelSize() ? -1 : m_gridCtrl->XToCol(pos.x - m_gridCtrl->GetRowLabelSize());

  // coordinates of the cell under mouse
  wxGridCellCoords coords(row, col); 

  OvItem *itemPtr = (coords.GetRow() < 0 || coords.GetCol() < 0) ? NULL : m_gridCtrl->GetItem(coords.GetRow(), coords.GetCol());

  wxMenu popupMenu("");  
  
  if (coords.GetRow() < 0)
  {
    for (int i = 0; i < WXSIZEOF(emptyCellMenu); i++)
      popupMenu.Append(2000 + i, wxGetTranslation(emptyCellMenu[i]));
  }
  else if (itemPtr == NULL || itemPtr->mt.mtID == 0)
  {
    for (int i = 0; i < WXSIZEOF(emptyCellMenu); i++)
      popupMenu.Append(2000 + i, wxGetTranslation(emptyCellMenu[i]));
  }
  else
  {
    for (int i = 0; i < WXSIZEOF(matchCellMenu); i++)
      popupMenu.Append(2000 + i, wxGetTranslation(matchCellMenu[i]));
  }
  
  wxPoint popupPos = ScreenToClient(wxGetMousePosition());
  
  int rc = GetPopupMenuSelectionFromUser(popupMenu, popupPos);
  if (rc == wxID_NONE)
    return;

  bool emptyCell = itemPtr == NULL || itemPtr->mt.mtID == 0;
    
  if (itemPtr == NULL || itemPtr->mt.mtID == 0)
  {
    for (int col = 0; col < m_gridCtrl->GetNumberCols(); col++)
    {
      if ( (itemPtr = m_gridCtrl->GetItem(coords.GetRow(), col)) && itemPtr->mt.mtID )
        break;                     
    }
  }
  
  // Combined Group haben Runde auf 0 gesetzt (da alle Spiele zur gleichen Zeit stattfinden)
  bool combined = itemPtr && itemPtr->mt.mtEvent.mtRound == 0;

  if (emptyCell)
  {
    switch (rc - 2000)
    {
      case 0 : // Toss sheet
      {
        short from = m_fromTable ? m_fromTable : 1;
        short to = m_toTable ? m_toTable : m_fromTable + m_gridCtrl->GetNumberCols() - 1;

        timestamp fromTime = m_fromTime;
        timestamp toTime = m_toTime;

        CTT32App::instance()->OpenDialog(true, _("Print Toss Sheet"), wxT("TossSheet"), itemPtr ? itemPtr->mt.mtID : 0, 2, from, to, fromTime, toTime);

        break;
      }

      case 1 : // Score sheet
      {
        short from = m_fromTable ? m_fromTable : 1;
        short to   = m_toTable ? m_toTable : m_fromTable + m_gridCtrl->GetNumberCols() - 1;

        timestamp fromTime = m_fromTime;
        timestamp toTime = m_toTime;
        
        CTT32App::instance()->OpenDialog(true, _("Print Score Sheet"), wxT("Score"), itemPtr ? itemPtr->mt.mtID : 0, 5, from, to, fromTime, toTime);
        
        break;
      }

      case 2 : // Match list
      {
        std::map<wxString, wxString> params;
        timestamp fromDateTime = m_fromTime, toDateTime = m_toTime;
        
        if (itemPtr)
         fromDateTime = toDateTime = itemPtr->mt.mtPlace.mtDateTime;

        params["PSTARTTIME"] = wxString::Format("%04d-%02d-%02d %02d:%02d:00", 
          fromDateTime.year, fromDateTime.month, fromDateTime.day, fromDateTime.hour, fromDateTime.minute);
        params["PENDTIME"] = wxString::Format("%04d-%02d-%02d %02d:%02d:00", 
          toDateTime.year, toDateTime.month, toDateTime.day, toDateTime.hour, toDateTime.minute);
        params["PSTARTTABLE"] = wxString::Format("%d", m_fromTable);
        params["PENDTABLE"] = wxString::Format("%d", m_toTable);

        CReportManViewer::DoReport("MatchList", params, this);
        break;
      }
    }
  }
  else
  {
    switch (rc - 2000)
    {
      case 0 : // Tosssheet
        CTT32App::instance()->OpenDialog(true, _("Print Tossheet"), wxT("TossSheet"), itemPtr->mt.mtID, 0);

        break;

      case 1 : // Score sheet
        CTT32App::instance()->OpenDialog(true, _("Print Scoresheet"), wxT("Score"), itemPtr->mt.mtID, combined ? 4 : 0);
        
        break;

      case 2 : // Find match
        CTT32App::instance()->OpenView(_T("MatchResult"), wxT("MtListView"), itemPtr->mt.mtID, m_showUmpire);
        break;

      case 3: // Match Details
        CTT32App::instance()->OpenView(_T("Match Details"), wxT("MtDetails"), itemPtr->mt.mtID);
        break;

      case 4 : // Edit Result
        if (combined)
          CTT32App::instance()->OpenView(_("Edit Group Results"), wxT("MtListView"), itemPtr->mt.mtID, false);
        else if (itemPtr->mt.cpType == CP_TEAM)
          CTT32App::instance()->OpenView(_("Edit Team Result"), wxT("MtTeam"), itemPtr->mt.mtID);
        else
          CTT32App::instance()->OpenView(_("Edit Result"), wxT("MtRes"), itemPtr->mt.mtID, 0);      

        break;
      
      case 5 : // Edit Schedule
        if (combined)
          CTT32App::instance()->OpenView(_("Edit Group Schedule"), wxT("MtListView"), itemPtr->mt.mtID, true);
        else
          CTT32App::instance()->OpenView(_("Edit Schedule"), wxT("MtTime"), itemPtr->mt.mtID, 1);
      
        break;

      case 6 : // Swap Taable
        {
          // Alter Tisch ergibt sich aus aktuellem Item
          // Neuen Tisch vom Benutzer erfragen, aber nur innerhalb des angezeigten Bereiches.
          short oldTable = itemPtr->mt.mtPlace.mtTable;
          short newTable = ::wxGetNumberFromUser(
                _("Swap Table"), _("Enter new table no."), _("Swap Table"), 
                oldTable, m_fromTable, m_toTable, this, popupPos);

          if (newTable >= 0 && newTable != oldTable)
          {
            // Zu tauschendes Spiel ermitteln
            OvItem *newItemPtr = m_gridCtrl->GetItem(row, newTable - m_fromTable);

            // Neuen Tisch setzen und printed Flag zuruecksetzen
            itemPtr->mt.mtPlace.mtTable = newTable;
            itemPtr->mt.mtScorePrinted = 0;

            // Combined haben Runde auf 0 gesetzt (s.o.)
            if (itemPtr->mt.mtEvent.mtRound == 0)
              MtStore().UpdateScheduleGroup(itemPtr->mt.mtEvent, itemPtr->mt.mtPlace, itemPtr->mt.mtUmpire, itemPtr->mt.mtUmpire2);
            else
              MtStore().UpdateScheduleMatch(itemPtr->mt.mtEvent, itemPtr->mt.mtPlace, itemPtr->mt.mtUmpire, itemPtr->mt.mtUmpire2);

            if (newItemPtr)
            {
              // Anderes Spiel existiert, setzen. Und printed flag loeschen
              newItemPtr->mt.mtPlace.mtTable = oldTable;
              newItemPtr->mt.mtScorePrinted = 0;

              // Combined haben Runde auf 0 gesetzt (s.o.)
              if (newItemPtr->mt.mtEvent.mtRound == 0)
                MtStore().UpdateScheduleGroup(newItemPtr->mt.mtEvent, newItemPtr->mt.mtPlace, newItemPtr->mt.mtUmpire, newItemPtr->mt.mtUmpire2);
              else
                MtStore().UpdateScheduleMatch(newItemPtr->mt.mtEvent, newItemPtr->mt.mtPlace, newItemPtr->mt.mtUmpire, newItemPtr->mt.mtUmpire2);

              // Spiel tauschen
              MtListRec oldMt = itemPtr->mt;
              MtListRec newMt = newItemPtr->mt;

              itemPtr->SetValue(newMt);
              newItemPtr->SetValue(oldMt);

              // Spieler / Team tauschen
              TmEntry oldTm, newTm;
            
              oldTm = itemPtr->tmA;
              newTm = newItemPtr->tmA;
              itemPtr->tmA = newTm;
              newItemPtr->tmA = oldTm;

              oldTm = itemPtr->tmX;
              newTm = newItemPtr->tmX;
              itemPtr->tmX = newTm;
              newItemPtr->tmX = oldTm;
            }
            else
            {
              // Anderes Spiel existiert nicht, das heisst, es ist nur eine Spielverlegung
              OvItem *newItemPtr = new OvItem(itemPtr->mt, itemPtr->GetOvType() == OvItem::OVPRINTED ? OvItem::OVNOTPRINTED : itemPtr->GetOvType());              
              newItemPtr->SetShowUmpire(&m_showUmpire);

              newItemPtr->tmA = itemPtr->tmA;
              newItemPtr->tmX = itemPtr->tmX;

              m_gridCtrl->SetItem(row, newTable - m_fromTable, newItemPtr);
              m_gridCtrl->SetItem(row, col, NULL);
            }

            // Wenn das neue Spiel innerhalb des angezeigten Rasters liegt, reicht ein redraw
            // Ansonsten muss ich das Grid neu aufbauen
            if (newTable < m_fromTable + m_gridCtrl->GetNumberCols())
              m_gridCtrl->Refresh();
            else
              OnRefresh();
          }

          break;
        }
    }
  }    
}


void COvList::OnContextMenuLabel(wxContextMenuEvent &evt) 
{
  static wxString labelMenu[] = 
  {
    wxTRANSLATE("Select Foreground"),
    wxTRANSLATE("Select Background")
  };
  
  wxMenu popupMenu("");  
  
  for (int i = 0; i < WXSIZEOF(labelMenu); i++)
    popupMenu.Append(2000 + i, wxGetTranslation(labelMenu[i]));

  wxPoint popupPos = ScreenToClient(wxGetMousePosition());
  
  int rc = GetPopupMenuSelectionFromUser(popupMenu, popupPos);

  wxWindow *label = (wxWindow *) evt.GetEventObject();

  bool fgbk = false;

  switch (rc) 
  {
    case wxID_NONE :
      return;

    case 2000 :
      fgbk = true;
      break;

    case 2001 :
      fgbk = false;
      break;

    default :
      return;
  }

  wxColor color = (fgbk ? label->GetForegroundColour() : label->GetBackgroundColour());

  OvItem::OvType type = OvItem::NameToOvType(label->GetName());
  wxColourData data;
  for (int i = 0; i < OvItem::OVLAST; i++)
  {
    data.SetCustomColour(i, fgbk ? OvItem::GetFgColor(OvItem::OvType(i)) : OvItem::GetBkColor(OvItem::OvType(i)));
    data.SetCustomColour(i + 8, fgbk ? OvItem::GetDefaultFgColor(OvItem::OvType(i)) : OvItem::GetDefaultBkColor(OvItem::OvType(i)));
  }

  wxColor newColor = wxGetColourFromUser(this, color, wxEmptyString, &data);

  if (!newColor.IsOk())
    return;

  if (newColor != color)
  {
    if (fgbk)
      CTT32App::instance()->SetOvFgColor(label->GetName(), newColor == OvItem::GetDefaultFgColor(type) ? wxNullColour : newColor);
    else
      CTT32App::instance()->SetOvBkColor(label->GetName(), newColor == OvItem::GetDefaultBkColor(type) ? wxNullColour : newColor);

    if (fgbk)
      OvItem::SetFgColor(type, newColor);
    else
      OvItem::SetBkColor(type, newColor);

    if (fgbk)
      label->SetForegroundColour(newColor);
    else
      label->SetBackgroundColour(newColor);

    label->Refresh();
    OnRefresh();
  }
}


// -----------------------------------------------------------------------
void  COvList::OnGridSizeChanged(wxCommandEvent &)
{
  int pos = XRCCTRL(*this, "CellSize", wxComboBox)->GetSelection();

  switch (pos)
  {
    case 0 :
      ttProfile.AddInt(PRF_GLOBAL_SETTINGS, PRF_OVLIST_CELLSIZE, 10);
      ttProfile.AddInt(PRF_GLOBAL_SETTINGS, PRF_OVLIST_FONTSIZE, 3);
      break;

    case 1:
      ttProfile.AddInt(PRF_GLOBAL_SETTINGS, PRF_OVLIST_CELLSIZE, 30);
      ttProfile.AddInt(PRF_GLOBAL_SETTINGS, PRF_OVLIST_FONTSIZE, 5);
      break;

    case 2 :
      ttProfile.AddInt(PRF_GLOBAL_SETTINGS, PRF_OVLIST_CELLSIZE, 50);
      ttProfile.AddInt(PRF_GLOBAL_SETTINGS, PRF_OVLIST_FONTSIZE, 7);
      break;

    case 3 :
      ttProfile.AddInt(PRF_GLOBAL_SETTINGS, PRF_OVLIST_CELLSIZE, 60);
      ttProfile.AddInt(PRF_GLOBAL_SETTINGS, PRF_OVLIST_FONTSIZE, 8);
      break;

    case 4 :
      ttProfile.AddInt(PRF_GLOBAL_SETTINGS, PRF_OVLIST_CELLSIZE, 80);
      ttProfile.AddInt(PRF_GLOBAL_SETTINGS, PRF_OVLIST_FONTSIZE, 9);
      break;
  }

  const int cellSize = ttProfile.GetInt(PRF_GLOBAL_SETTINGS, PRF_OVLIST_CELLSIZE, 60);
  const int fontSize = ttProfile.GetInt(PRF_GLOBAL_SETTINGS, PRF_OVLIST_FONTSIZE, 8);

  m_gridCtrl->SetColLabelSize(cellSize);
  m_gridCtrl->SetDefaultColSize(cellSize);
  m_gridCtrl->SetRowLabelSize(cellSize);
  m_gridCtrl->SetDefaultRowSize(cellSize);
  m_gridCtrl->SetScrollRate(cellSize, cellSize);

  wxFont labelFont = m_gridCtrl->GetLabelFont();
  labelFont.SetPointSize(fontSize);
  m_gridCtrl->SetLabelFont(labelFont);

  wxFont cellFont = m_gridCtrl->GetDefaultCellFont();
  cellFont.SetPointSize(fontSize);
  m_gridCtrl->SetDefaultCellFont(cellFont);
  
  if (m_gridCtrl->GetNumberRows())
    OnRefresh();
}


// -----------------------------------------------------------------------
void  COvList::OnUpdate(CRequest *reqPtr)
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

  switch (reqPtr->type)
  {
    case CRequest::UPDATE :
    case CRequest::UPDATE_RESULT :
    {
      Connection* connPtr = TTDbse::instance()->GetNewConnection();

      MtListStore mt(connPtr);
      mt.SelectById(reqPtr->id);
      if (mt.Next())
        OnUpdateMt(mt, connPtr);

      mt.Close();
      delete connPtr;

      return;
    }


    default :
      return;
  }
}


void COvList::OnUpdateMt(const MtListRec &mt_, Connection *connPtr)
{
  MtListRec mt = mt_;

  // Test, ob Spiel im gewaehlten Bereich liegt
  if (mt.mtPlace.mtTable < m_fromTable || mt.mtPlace.mtTable > m_toTable)
    return;

  if (mt.mtPlace.mtDateTime < m_fromTime || mt.mtPlace.mtDateTime > m_toTime)
    return;

  // Item suchen. 
  // XXX: Kann man per Zeit und Tisch direkt suchen?
  OvItem *ovItem = NULL;

  for (int row = 0; row < m_gridCtrl->GetNumberRows(); row++)
  {
    for (int col = 0; col < m_gridCtrl->GetNumberCols(); col++)
    {
      OvItem *tmpItem = m_gridCtrl->GetItem(row, col);

      if (!tmpItem)
        continue;

      // Wenn die Zeit zu klein ist, weiter mit naechster Zeile.
      // Wenn die Zeit zu gross ist, fertig.
      // Wenn die Zeit stimmt und der Tisch zu gross ist, fertig.
      if (tmpItem->mt.mtPlace.mtDateTime < mt.mtPlace.mtDateTime)
        break;
      else if (tmpItem->mt.mtPlace.mtDateTime > mt.mtPlace.mtDateTime)
        break;
      else if (tmpItem->mt.mtPlace.mtTable > mt.mtPlace.mtTable)
        break;

      // Wenn es nicht unser Spiel ist, weiter mit naechster Spalte
      if ( tmpItem->GetID() != mt.mtID && 
          (tmpItem->mt.mtEvent.mtRound != 0 || tmpItem->mt.mtEvent.grID != mt.mtEvent.grID) )
        continue;

      ovItem = tmpItem;
      ((OvItemGridTable *) m_gridCtrl->GetTable())->SetRowLabelType(row, OvItem::OVLAST);

      break;
    }
  }

  if (!ovItem)
  {
    // Wenn nicht gefunden und Spiel hat keinen Ansetzung, dann ignorieren
    if (mt.mtPlace.mtDateTime.hour > 0 && mt.mtPlace.mtTable > 0)
      return;

    wxString time = wxString::Format("%2d : %2d", mt.mtPlace.mtDateTime.hour, mt.mtPlace.mtDateTime.minute);

    for (int row = 0; row < m_gridCtrl->GetNumberRows(); row++)
    {
      if (m_gridCtrl->GetRowLabelValue(row) == time)
      {
        // Wenn es ein Item gibt, wir es aber vorher nicht gefunden haben, auch hier ignorieren
        if (m_gridCtrl->GetItem(row, mt.mtPlace.mtTable - m_fromTable))
          continue;

        ovItem = new OvItem(mt);
        m_gridCtrl->SetItem(row, mt.mtPlace.mtTable - m_fromTable, ovItem);

        ((OvItemGridTable *) m_gridCtrl->GetTable())->SetRowLabelType(row, OvItem::OVLAST);

        break;
      }
    }
  }

  // Wenn kein Item gefunden, abbrechen
  if (!ovItem)
    return;

  // Merker uebernehmen
  if (ovItem->mt.mtID && ovItem->mt.mtEvent.mtRound == 0)
    mt.mtEvent.mtRound = 0;

  OvItem::OvType type = ovItem->GetOvType();

  if ( ovItem->GetID() == mt.mtID)
    ovItem->SetValue(mt);

  // Im Fall von combined wenn ein fertiges Spiel daher kommt und der Typ nicht Fehler ist,
  // pruefen, ob alle Spiele beendet sind. Falls ja, Typ entsprechend setzen, falls nein,
  // alten Typ lassen.
  if (mt.mtEvent.mtRound == 0 && type != OvItem::OVERROR)
  {
    GrListStore gr(connPtr);
    gr.grID = mt.mtEvent.grID;
    if (gr.QryUnchecked())
      ovItem->SetOvType(OvItem::OVCHECKED);
    else if (gr.QryFinished())
      ovItem->SetOvType(OvItem::OVFINISHED);
    else if (type < OvItem(mt).GetOvType())
      ovItem->SetOvType(type);
		else if (ovItem->GetID() != mt.mtID)
      ovItem->SetOvType(OvItem(mt).GetOvType());
  }

  // m_gridCtrl->SetItem(row, mt.mtPlace.mtTable - m_fromTable, ovItem);

  OvItem::OvType lastType = OvItem::OVLAST;

  m_gridCtrl->Refresh();
}


// -----------------------------------------------------------------------
void COvList::OnUpdateTimer(wxTimerEvent &)
{
  // OnUpdateTimer may be called in an own thread, so use a new connection
  Connection *connPtr = TTDbse::instance()->GetNewConnection();

  // Make local variable live no longer than livetime of connPtr
  {
  timestamp ts = MtListStore(connPtr).GetLastUpdateTime();

  std::list<MtListRec> list;

  if (ts > m_lastUpdateTime)
  {
    MtListStore mt(connPtr);
    mt.SelectByTimestamp(m_lastUpdateTime);
    while (mt.Next())
      list.push_back(mt);

    mt.Close();

    for (std::list<MtListRec>::iterator it = list.begin(); it != list.end(); it++)
        OnUpdateMt((*it), connPtr);
  }

  m_lastUpdateTime = ts;
  }

  delete connPtr;
}


// -----------------------------------------------------------------------
void COvList::OnCommand(wxCommandEvent &evt)
{
  if (evt.GetEventType() == IDC_REFRESH)
    OnRefresh();
  else if (evt.GetEventType() == wxID_PRINT)
    OnPrint();
  else
    evt.Skip();
}
