/* Copyright (C) 2020 Christoph Theis */

#ifndef SQLEDITOR_H_
#define SQLEDITOR_H_

/////////////////////////////////////////////////////////////////////////////
// CSQLEditor form view

#include "FormViewEx.h"
#include "ListCtrlEx.h"

class CSQLEditor : public CFormViewEx
{
  public:
	  CSQLEditor(wxMDIParentFrame * = NULL);           
	  ~CSQLEditor();

    virtual bool Edit(va_list) {return true;}

    void FreezeTitle(bool b = true) {m_freezeTitle = b;}

  private:
    virtual void OnInitialUpdate();
    virtual void OnPrint();
	
  private:
	  void OnExecuteSQL(wxCommandEvent &);
    void OnSave(wxCommandEvent &);
    void OnSaveQuery(wxCommandEvent &);
    void OnOpenQuery(wxCommandEvent &);
    
  private:
	  CListCtrlEx * m_listCtrl;
	  wxString	    m_sqlString;
    bool          m_freezeTitle;

	DECLARE_DYNAMIC_CLASS(CSQLEditor)
	DECLARE_EVENT_TABLE()
};

/////////////////////////////////////////////////////////////////////////////

#endif 
