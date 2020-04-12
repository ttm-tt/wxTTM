/* Copyright (C) 2020 Christoph Theis */

#ifndef _COMBOBOXEX_H_
#define _COMBOBOXEX_H_

class  ListItem;

// -----------------------------------------------------------------------
// CComboBoxExEx window

class CComboBoxEx : public wxOwnerDrawnComboBox
{
  // Construction
  public:
	  CComboBoxEx();
	 ~CComboBoxEx();

  // Implementation
  public:
    // Add Item at end / at position idx
    void  AddListItem(const ListItem *itemPtr, int idx = -1);
    
    void  RemoveListItem(long id);
    void  RemoveListItem(int idx);
    void  RemoveListItem(const wxString &);
    void  RemoveAllListItems();

    // Lookup Item by ID / Label
    ListItem * FindListItem(long id);
    ListItem * FindListItem(const wxString &);

    // Get item at position idx
    ListItem * GetListItem(int idx);

    // Get Current (selected) Item
    ListItem * GetCurrentItem();

    // Cut Item with ID / label and delete entry
    ListItem * CutListItem(long id);
    ListItem * CutListItem(const wxString &);

    // Cut Current (selected) Item and delete entry
    ListItem * CutCurrentItem();

    // Set current selection to item with label "str" / id
    void  SetCurrentItem(const wxString &str);
    void  SetCurrentItem(long id);
    void  SetCurrentItem(const ListItem *);

    void  SetItemHeight(unsigned height);

	  // Generated message map functions
  protected:
        // this method can be overridden instead of DoGetBestSize() if it computes
    // the best size of the client area of the window only, excluding borders
    // (GetBorderSize() will be used to add them)
    virtual wxSize DoGetBestClientSize() const;

    virtual void OnDrawItem(wxDC &, const wxRect &, int item, int flags) const;
    virtual wxCoord OnMeasureItem(size_t item) const;

    void OnKeyDown(wxKeyEvent &);
    void OnChar(wxKeyEvent &);
    void OnKillFocus(wxFocusEvent &);

    wxString MakeShortString(wxDC &dc, const wxString &str, const wxRect &rc) const;
  	
    int      m_cxClient;
    double   m_itemHeight;

    wxString m_editString;
    int      m_lastIdx;

    DECLARE_DYNAMIC_CLASS(CComboBoxEx)
    DECLARE_EVENT_TABLE()
};

/////////////////////////////////////////////////////////////////////////////



#endif 
