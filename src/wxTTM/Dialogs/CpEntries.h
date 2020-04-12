/* Copyright (C) 2020 Christoph Theis */

#ifndef _CPENTRIES_H_
#define _CPENTRIES_H_


#include  "FormViewEx.h"
#include  "ComboBoxEx.h"
#include  "ListCtrlEx.h"

#include "CpListStore.h"


// -----------------------------------------------------------------------
// CCpEntries dialog
class CCpEntries : public CFormViewEx
{
  // Construction
  public:
	  CCpEntries();   
	 ~CCpEntries(); 
	 
    virtual bool Edit(va_list);
    
	protected:
	  virtual void OnInitialUpdate();

	  virtual void OnUpdate(CRequest *);

    virtual void OnAdd();
    virtual void OnEdit();
    virtual void OnDelete();

  protected:
	  void OnSelChangeCp(wxCommandEvent &);
	  void OnAddComplete(wxCommandEvent &);
    void OnEditComplete(wxCommandEvent &);
    void OnDeleteComplete(wxCommandEvent &);
	  void OnAddOpen(wxCommandEvent &);
    void OnEditOpen(wxCommandEvent &);
    void OnDeleteOpen(wxCommandEvent &);

  private:
    static const char *  openHeaders[];

	  long	m_allEntries;
	  long	m_openEntries;

    CpRec cp;

    CComboBoxEx * cbCp;    // Liste der WB
    CListCtrlEx * completeList;  // Liste der Spieler / Teams
    CListCtrlEx * openList;

    wxNotebook     *notebook;
    wxNotebookPage *openPage;
    wxString        openTitle;

    void UpdateCounts();

    void OnAddPlayer();
    void OnAddTeam();

    void OnDeletePlayer();
    void OnDeleteTeam();
    
  DECLARE_DYNAMIC_CLASS(CCpEntries)
  DECLARE_EVENT_TABLE()  
};


#endif 
