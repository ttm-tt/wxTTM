/* Copyright (C) 2020 Christoph Theis */

#ifndef _MTTEAM_H_
#define _MTTEAM_H_


// -----------------------------------------------------------------------
// CMtTeam form view

#include  "FormViewEx.h"
#include  "ListCtrlEx.h"
#include  "ItemCtrl.h"

#include  "CpStore.h"
#include  "GrStore.h"
#include  "MtStore.h"
#include  "SyStore.h"
#include  "TmEntryStore.h"

class CMtTeam : public CFormViewEx
{
  public:
	  CMtTeam();           
	 ~CMtTeam(); 

    virtual bool Edit(va_list);

  protected:
    virtual void OnOK();
    virtual void OnEdit();

	protected:
	  virtual void OnInitialUpdate();
	  virtual void OnUpdate(CRequest *reqPtr);

  private:
	  void OnMtReverse(wxCommandEvent &);
	  void OnNominationA(wxMouseEvent &);
	  void OnNominationX(wxMouseEvent &);
	  void OnWalkOverA(wxCommandEvent &);
	  void OnWalkOverX(wxCommandEvent &);
    void OnScore(wxCommandEvent &);
    
    void OnNomination(long tmID, int ax);
    void OnWalkOver(int ax);
    
    virtual bool TransferDataToWindow();

  private:
	  short m_woa;
	  short m_wox;

  private:
    CItemCtrl * cpItem;
    CItemCtrl * grItem;
    CItemCtrl * tmAItem;
    CItemCtrl * tmXItem;
    CItemCtrl * tmWinnerItem;  
	  CListCtrlEx *  mtList;

    CpStore   cp;
    GrStore   gr;
    MtStore   mt;
    SyStore   sy;
    TmEntry   tmA;
    TmEntry   tmX;
    
  DECLARE_DYNAMIC_CLASS(CMtTeam)
  DECLARE_EVENT_TABLE()  
};

#endif 
