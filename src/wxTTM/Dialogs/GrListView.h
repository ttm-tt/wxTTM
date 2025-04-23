/* Copyright (C) 2020 Christoph Theis */

#ifndef _GRLISTVIEW_H_
#define _GRLISTVIEW_H_


// -----------------------------------------------------------------------
// CGrListView form view

#include  "FormViewEx.h"
#include  "ListCtrlEx.h"
#include  "ComboBoxEx.h"
#include  "CpListStore.h"


class CGrListView : public CFormViewEx
{
  public:
	  CGrListView();           
	 ~CGrListView(); 
	 
	  virtual bool Edit(va_list);

  public:
    virtual void SaveSettings();
    virtual void RestoreSettings();

  protected:
    virtual void OnInitialUpdate();
    virtual void OnAdd();
    virtual void OnEdit();
    virtual void OnDelete();

	virtual void OnUpdate(CRequest *);

  private:
    void OnSelChangeCp(wxCommandEvent &);
    void OnNotes(wxCommandEvent &);
    void OnPublish(wxCommandEvent &);
    void OnUnpublish(wxCommandEvent &);

    void OnMouseLeftDown(wxMouseEvent &);

  private:
    CpListStore  cp;
    CComboBoxEx * m_cbCp = nullptr;
    CListCtrlEx * m_listCtrl = nullptr;
	  
	DECLARE_DYNAMIC_CLASS(CGrListView)
	DECLARE_EVENT_TABLE()
};


#endif 
