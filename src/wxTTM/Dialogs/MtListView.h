/* Copyright (C) 2020 Christoph Theis */

#ifndef _MTLISTVIEW_H_
#define _MTLISTVIEW_H_


// -----------------------------------------------------------------------
// CMtListView form view

#include  "FormViewEx.h"
#include  "ComboBoxEx.h"
#include  "ListCtrlEx.h"

#include  "CpListStore.h"
#include  "GrListStore.h"
#include  "MtListStore.h"
#include  "StEntryStore.h"

#include  <map>


class CMtListView : public CFormViewEx
{
  typedef  std::map<long, StEntry, std::less<long> >  StEntryMap;

  public:
	  CMtListView();           
	 ~CMtListView();

    virtual bool Edit(va_list);

	private:
	  virtual void OnInitialUpdate();
	  virtual void OnUpdate(CRequest *);

  private:
	  void OnSelChangeCp(wxCommandEvent &);
	  void OnSelChangeGr(wxCommandEvent &);
	  void OnRoundFirst(wxCommandEvent &);
	  void OnRoundNext(wxCommandEvent &);
	  void OnRoundPrev(wxCommandEvent &);
	  void OnRoundLast(wxCommandEvent &);
	  void OnResultTime(wxCommandEvent &);
    void OnScore(wxCommandEvent &);
    void OnTable(wxCommandEvent &);
    void OnConsolation(wxCommandEvent &);
    void OnGroupView(wxCommandEvent &);

    void  OnChangeRound();
    
    virtual void OnEdit();

  protected:
    bool  showTimes;

  private:
    CComboBoxEx *  m_cbCp;
    CComboBoxEx *  m_cbGr;
	  CListCtrlEx *  m_listCtrl;
  	
	  int    m_chance;

    CpRec  cp;
    GrRec  gr;
    MtRec  mt;

    short  round;

    StEntryMap  stMap;

    StEntry GetTeamA(const MtRec &);
    StEntry GetTeamX(const MtRec &);
    StEntry GetTeam(long id);
    
  DECLARE_DYNAMIC_CLASS(CMtListView)
  DECLARE_EVENT_TABLE()
};


#endif 
