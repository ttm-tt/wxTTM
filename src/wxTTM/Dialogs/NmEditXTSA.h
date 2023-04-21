/* Copyright (C) 2022 Christoph Theis */

#pragma once

// -----------------------------------------------------------------------
// CNmEdit form view

#include  "FormViewEx.h"
#include  "ListCtrlEx.h"
#include  "ItemCtrl.h"

#include  "CpStore.h"
#include  "GrStore.h"
#include  "MtStore.h"
#include  "NmStore.h"
#include  "SyStore.h"
#include  "TmEntryStore.h"

class CNmEditXTSA : public CFormViewEx
{
  public:
	  CNmEditXTSA();           
	 ~CNmEditXTSA(); 

    virtual bool Edit(va_list);


  protected:
	  virtual void OnInitialUpdate();
    virtual  void OnAdd();
    virtual  void OnDelete();
    virtual void OnOK();

  private:
    void OnSelChanged(wxListEvent &);

  private:
    CItemCtrl *  cpItem;
    CItemCtrl *  grItem;
    CItemCtrl *  tmItem;  
	  CListCtrlEx * nmList;
	  CListCtrlEx * plList;

    CpStore   cp;
    GrStore   gr;
    SyStore   sy;
    MtStore   mt;
    NmStore   nm;
    TmEntry   tm;
    
  DECLARE_DYNAMIC_CLASS(CNmEditXTSA)
  DECLARE_EVENT_TABLE()    
};


