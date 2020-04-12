/* Copyright (C) 2020 Christoph Theis */

#ifndef _NMEDIT_H_
#define _NMEDIT_H_


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

class CNmEdit : public CFormViewEx
{
  public:
	  CNmEdit();           
	 ~CNmEdit(); 

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
    
  DECLARE_DYNAMIC_CLASS(CNmEdit)
  DECLARE_EVENT_TABLE()    
};


#endif 
