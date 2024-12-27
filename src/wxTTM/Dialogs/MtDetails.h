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

class CMtDetails : public CFormViewEx
{
  public:
	  CMtDetails();           
	 ~CMtDetails(); 

    virtual bool Edit(va_list);

	protected:
	  virtual void OnInitialUpdate();

  private:
    CItemCtrl * cpItem = nullptr;
    CItemCtrl * grItem = nullptr;
    CItemCtrl * tmAItem = nullptr;
    CItemCtrl * tmXItem = nullptr;

    CpStore   cp;
    GrStore   gr;
    MtStore   mt;
    TmEntry   tmA;
    TmEntry   tmX;
    
  DECLARE_DYNAMIC_CLASS(CMtDetails)
  DECLARE_EVENT_TABLE()  
};

#endif 
