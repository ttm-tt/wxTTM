/* Copyright (C) 2020 Christoph Theis */

#ifndef _MTRES_H_
#define _MTRES_H_


// -----------------------------------------------------------------------
// CMtRes form view

#include  "FormViewEx.h"
#include  "ItemCtrl.h"

#include  "CpStore.h"
#include  "GrStore.h"
#include  "TmEntryStore.h"
#include  "MtStore.h"

class CMtRes : public CFormViewEx
{
  public:
	  CMtRes();           
	 ~CMtRes(); 

    virtual bool Edit(va_list);

  protected:
    virtual void OnOK();

  private:
	  virtual void OnInitialUpdate();

  private:
	  void OnKillFocusSet(wxFocusEvent &);
	  void OnSetFocusSet(wxFocusEvent &);
	  void OnWalkOverA(wxCommandEvent &);
    void OnInjuredA(wxCommandEvent &);
	  void OnDisqualifiedA(wxCommandEvent &);
	  void OnWalkOverX(wxCommandEvent &);
    void OnInjuredX(wxCommandEvent &);
	  void OnDisqualifiedX(wxCommandEvent &);
	  void OnKillFocusBestof(wxFocusEvent &);
    void OnCharHookSet(wxKeyEvent &);
    void OnSwap(wxCommandEvent &);
	  
	private:
    void OnWalkOver();
    void OnDisqualified();
    void OnInjured();

  private:
    static long idcList[];
    
    CItemCtrl * cpItem = nullptr;
    CItemCtrl * grItem = nullptr;
    CItemCtrl * tmAItem = nullptr;
    CItemCtrl * tmXItem = nullptr;
    CItemCtrl * tmWinnerItem = nullptr;
    CItemCtrl * mtSetsItem = nullptr;
    
    CpStore   cp;
    GrStore   gr;
    MtStore   mt;
    TmEntry   tmA;
    TmEntry   tmX;

    MtSet *   mtSetList = nullptr;
    MtMatch   mtMatch;
    
  DECLARE_DYNAMIC_CLASS(CMtRes)
  DECLARE_EVENT_TABLE()
};


#endif 
