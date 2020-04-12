/* Copyright (C) 2020 Christoph Theis */

#ifndef _MTTIME_H_
#define _MTTIME_H_


// -----------------------------------------------------------------------
// CMtTime form view

#include  "FormViewEx.h"

#include  "CpStore.h"
#include  "GrStore.h"
#include  "MtStore.h"

#include  "ItemCtrl.h"
// #include  "DateTimeEdit.h"

class CDateTimeEdit;

class CMtTime : public CFormViewEx
{
  public:
	  CMtTime();           
	 ~CMtTime(); 

    virtual bool Edit(va_list);

  protected:
    virtual void OnOK();

  private:
    short m_applyTo;
	  bool	m_assignPlayer;
	  short	m_availTables;
	  bool	m_decTables;

    bool  m_noSchedule;

    short m_sequence;

  protected:
	  virtual void OnInitialUpdate();

  private:
    void OnBnClickedApplyTo(wxCommandEvent &);
    void OnBnClickedAssign(wxCommandEvent &);

    void OnSetFocusTable(wxFocusEvent &);
    void OnKeyDownTable(wxKeyEvent &);

  private:
    void  CalculateNextTable();
    void  UpdateMatch();
    void  UpdateRound();
    void  UpdateRoundExclude();
    void  UpdateRoundsGroup();
    void  UpdateRoundsGroupExclude();
    void  UpdateMatchesGroup();
    void  UpdateGroup();

    CItemCtrl *   cpItem;
    CItemCtrl *   grItem;
    CDateTimeEdit * matchDate;
    CDateTimeEdit * matchTime;
    
    CpStore   cp;
    GrStore   gr;
    MtStore   mt;
    
  DECLARE_DYNAMIC_CLASS(CMtTime)
  DECLARE_EVENT_TABLE()
};


// -----------------------------------------------------------------------
#endif 
