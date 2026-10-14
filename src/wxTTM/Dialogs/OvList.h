/* Copyright (C) 2020 Christoph Theis */

#ifndef _OVLIST_H_
#define _OVLIST_H_


// -----------------------------------------------------------------------
// COvList form view

#include  "FormViewEx.h"
#include  "ComboBoxEx.h"
#include  "SQLInclude.h"  // Definition of timestamp
#include  "ItemCtrl.h"


class Connection;
class wxDragImage;
class COvGridCtrl;
class OvItem;

struct MtListRec;

class COvList : public CFormViewEx
{
  friend COvGridCtrl;

  public:
	  COvList();           
	 ~COvList(); 
	 
	  virtual bool Edit(va_list);

    void SaveSettings();
    void RestoreSettings();

    void FreezeTitle(bool b = true) {m_freezeTitle = b;}

  protected:
    void SaveSize();
    void RestoreSize();

  protected:
	  virtual void OnInitialUpdate();

    virtual void OnUpdate(CRequest *);

    // Alt+Enter: edit the match at the grid cursor
    virtual void OnEdit();

  public:
    // public, weil sie von OvListBook aus aufgerufen werden
    virtual void OnRefresh();
    virtual void OnPrint();

  private:
    void OnUpdateMt(const MtListRec &, Connection * = nullptr);
    void OnUpdateTimer(wxTimerEvent &);
    void OnPopupTimer(wxTimerEvent &);
    void OnChar(wxKeyEvent &);
    void OnCharHook(wxKeyEvent &);

    // Open the edit view for the match in this cell, if there is one
    void EditCell(int row, int col);

    // Called when a dragged match was dropped on another cell in its row
    void OnDropMatch(long mtID, int row, int col);

    void SwapTable(short row, short oldTable, short newTable);
    void SetTable(OvItem *itemPtr, short table);

    // Move the match in fromCol to toCol, everything in between moves one table towards fromCol
    void InsertMatch(int row, int fromCol, int toCol);

  private:
    void OnShowUmpire(wxCommandEvent &);
    void OnCellLeftClick(wxGridEvent &);
    void OnCellDoubleClick(wxGridEvent &);
    void OnTimer(wxTimerEvent &);
    void OnContextMenuGrid(wxMouseEvent &);
    void OnContextMenuLabel(wxContextMenuEvent &);
    void OnMotion(wxMouseEvent &);
    void OnGridSizeChanged(wxCommandEvent &);
    void OnCommand(wxCommandEvent &);
    void OnCellBeginDrag(wxGridEvent&);
    void OnDragMotion(wxMouseEvent &);
    void OnDragLeftUp(wxMouseEvent &);
    void OnDragKey(wxKeyEvent &);
    void OnDragScrollTimer(wxTimerEvent &);
    void DragTo(const wxPoint &);
    int  GetDragScrollDir(int x) const;
    void OnDragCaptureChanged(wxMouseCaptureChangedEvent &);
    void EndDrag(bool drop);

  private:
    // Grid-Ctrl
    COvGridCtrl * m_gridCtrl = nullptr;
    CComboBoxEx * m_cbDate = nullptr;

    wxPopupTransientWindow *m_popup = nullptr;
    
    wxTimer      m_popupTimer;
    wxTimer      m_updateTimer;

    timestamp    m_lastUpdateTime;

    // Drag of a match: source cell, picture following the mouse and
    // mouse position in the grid window when the drag started
    wxGridCellCoords m_dragFrom = wxGridNoCellCoords;
    wxDragImage *    m_dragImage = nullptr;
    wxPoint          m_dragStart;
    wxPoint          m_dragPos;           // last mouse position while dragging
    wxTimer          m_dragScrollTimer;   // scrolls while the mouse is at the edge
    int              m_dragInsertGap = -1; // SHIFT: insert before this column, else -1

    // Einstellungen
    timestamp    m_fromTime;
    timestamp    m_toTime;
    short        m_fromTable;
    short        m_toTable;
    bool         m_showUmpire;
    bool         m_initialized;
    bool         m_freezeTitle;
    
  DECLARE_DYNAMIC_CLASS(COvList)
  DECLARE_EVENT_TABLE()
};


#endif 
