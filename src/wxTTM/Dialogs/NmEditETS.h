/* Copyright (C) 2020 Christoph Theis */

#ifndef _NMEDITETS_H_
#define _NMEDITETS_H_


// -----------------------------------------------------------------------
// CNmEditETS form view

#include  "FormViewEx.h"
#include  "ListCtrlEx.h"
#include  "ItemCtrl.h"

#include  "CpStore.h"
#include  "GrStore.h"
#include  "MtStore.h"
#include  "NmStore.h"
#include  "SyStore.h"
#include  "TmEntryStore.h"

class CNmEditETS : public CFormViewEx
{
  public:
	  CNmEditETS();           
	 ~CNmEditETS(); 

    virtual bool Edit(va_list);


  protected:
	  virtual void OnInitialUpdate();
    virtual  void OnAdd();
    virtual  void OnDelete();
    virtual void OnOK();

  private:
    void OnSelChanged(wxListEvent &);

  private:
    CItemCtrl *  cpItem = nullptr;
    CItemCtrl *  grItem = nullptr;
    CItemCtrl *  tmItem = nullptr;
	  CListCtrlEx * nmList = nullptr;
	  CListCtrlEx * plList = nullptr;
    wxChoice *   plFourReplace = nullptr;

    CpStore   cp;
    GrStore   gr;
    SyStore   sy;
    MtStore   mt;
    NmStore   nm;
    TmEntry   tm;
    
  DECLARE_DYNAMIC_CLASS(CNmEditETS)
  DECLARE_EVENT_TABLE()    
};


#endif 
