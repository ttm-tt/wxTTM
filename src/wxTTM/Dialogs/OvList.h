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
class COvGridCtrl;

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

  public:
    // public, weil sie von OvListBook aus aufgerufen werden
    virtual void OnRefresh();
    virtual void OnPrint();

  private:
    void OnUpdateMt(const MtListRec &, Connection * = nullptr);
    void OnUpdateTimer(wxTimerEvent &);
    void OnPopupTimer(wxTimerEvent &);

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

  private:
    // Grid-Ctrl
    COvGridCtrl * m_gridCtrl = nullptr;
    CComboBoxEx * m_cbDate = nullptr;
    
    wxTimer      m_popupTimer;
    wxTimer      m_updateTimer;

    timestamp    m_lastUpdateTime;

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
