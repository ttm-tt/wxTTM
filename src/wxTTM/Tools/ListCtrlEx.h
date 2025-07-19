/* Copyright (C) 2020 Christoph Theis */

#ifndef LISTCTRLEX_H_
#define LISTCTRLEX_H_

#include <vector>

class  ListItem;

// -----------------------------------------------------------------------
// CListCtrlEx window

class CListCtrlEx : public wxListCtrl
{
  public:
    static int wxCALLBACK Compare(wxIntPtr item1, wxIntPtr item2, wxIntPtr param);

  // Construction
  public:
	  CListCtrlEx();
	 ~CListCtrlEx();

  // Implementation
  public:
    // virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
    
    // Add Item at end / at position idx / sorted
    void  AddListItem(const ListItem *itemPtr, int idx = -1);
    void  InsertListItem(const ListItem *itemPtr) {AddListItem(itemPtr);}
    void  RemoveListItem(long id);
    void  RemoveAllListItems();

    int   GetCount() const {return GetItemCount();}    
    // Lookup Item by ID / Label
    ListItem * FindListItem(long id);
    ListItem * FindListItem(const wxString &label);

    // Get item at pos idx
    ListItem * GetListItem(int idx);

    // Get Current (selected) Item
    ListItem * GetCurrentItem();

    // Get idx of current (selected) item
    long  GetCurrentIndex();

    long  GetListIndex(const ListItem *);

    // Cut Item with idx / ID / label and delete entry
    ListItem * CutListItem(int idx);
    ListItem * CutListItem(long id);

    // Cut Current (selected) Item and delete entry
    ListItem * CutCurrentItem();

    // Set current selection to item with label "str"
    void  SetCurrentItem(long id);
    
    // Set item idx to current
    void  SetCurrentIndex(long idx);
    
    bool  IsSelected(int idx) 
    {
      return (GetItemState(idx, wxLIST_STATE_SELECTED) & wxLIST_STATE_SELECTED ? true : false);
    }
    
    void SetSelected(int idx)
    {
      SetItemState(idx, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    }
    
    void SelectAll()
    {
      for (int idx = GetItemCount(); idx--; )
        SetSelected(idx);
    }

    void ClearSelected(int idx)
    {
      SetItemState(idx, 0, wxLIST_STATE_SELECTED);
    }
    
    void ClearSelection();
    
    int  GetSelectedCount();
    
    // Set new itemHeight
    void  SetItemHeight(double newHeight);

    // Sort items
    void  SortItems(int col = -1);
    void  SetSortColumn(int col) { m_sortIdx = col; }
    int   GetSortColumn() const  { return m_sortIdx; }

    // Insert column
    long InsertColumn(long col, const wxString &heading, int format = wxLIST_FORMAT_LEFT, int width = -1);
    bool DeleteColumn(long col);

    void HideColumn(long col);
    long ShowColumn(long col, bool show = true);
    void AllowHideColumn(long col, bool allow = true);

    // Resize Column, so that all columns take up the entire space
    void  ResizeColumn() { ResizeColumn(m_resizeColumn); }
    void  ResizeColumn(int col);
    
    void SetResizeColumn(int col);

    bool HasColumn(long col);

    long GetIdxForColumn(long col);
    long GetColumnForIdx(long idx);

    bool SaveColumnInfo(const wxString &prefix) const;
    bool RestoreColumnInfo(const wxString &prefix);

	  // Generated message map functions
  private:
    void OnKeyDown(wxKeyEvent &);
    void OnChar(wxKeyEvent &);
	  void OnSize(wxSizeEvent &);
	  void OnLButtonDblClk(wxMouseEvent &);
	  void OnColumnClick(wxListEvent &);
    void OnColumnRightClick(wxListEvent &);
	  void OnKillFocus(wxFocusEvent &);
	  void OnDestroy(wxWindowDestroyEvent &);
	  void OnSelectAll(wxCommandEvent &) { SelectAll(); }

    wxString MakeShortString(wxDC &dc, const wxString &str, const wxRect &rc) const;

    double   m_itemHeight;

    wxString m_editString;
    int      m_lastIdx;
    int      m_resizeColumn;
    int      m_sortIdx;
    
  protected:
    // Owner draw items
    virtual WXDWORD    MSWGetStyle(long style, WXDWORD *extstyle) const;
    
  private:
    virtual bool MSWOnMeasure(WXMEASUREITEMSTRUCT *);
    virtual bool MSWOnDraw(WXDRAWITEMSTRUCT *);

  private:
    class ColumnInfo : public wxListItem
    {
      public:
        int   column;
        bool  enableHide;
    };

    std::vector<ColumnInfo> columnInfo;
      
    
  DECLARE_DYNAMIC_CLASS(CListCtrlEx)   
  DECLARE_EVENT_TABLE() 
};


#endif 
