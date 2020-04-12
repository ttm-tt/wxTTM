/* Copyright (C) 2020 Christoph Theis */

#include "FormViewEx.h"


class CSQLEditorBook : public CFormViewEx
{

  public:
	  CSQLEditorBook();           
	 ~CSQLEditorBook(); 
	 
	  virtual bool Edit(va_list);

  protected:
	  virtual void OnInitialUpdate();
    virtual void OnRefresh();
    virtual void OnPrint();

  private:
    void OnPageChanging(wxAuiNotebookEvent &);
    void OnPageClose(wxAuiNotebookEvent &);
    void OnDragEnd(wxAuiNotebookEvent &);
    void OnPageDragged(wxCommandEvent &);
    void OnSetTitle(wxCommandEvent &);
    void OnCharHook(wxKeyEvent &);
    void OnTabRightUp(wxAuiNotebookEvent &);

  private:
    void AddNewPage();

    bool m_inDrag;

    wxAuiNotebook *m_notebook;

  DECLARE_DYNAMIC_CLASS(CSQLEditorBook)
  DECLARE_EVENT_TABLE()
};