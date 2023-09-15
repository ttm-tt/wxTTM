/* Copyright (C) 2020 Christoph Theis */

#ifndef _NMEDITETS_H_
#define _NMEDITETS_H_


// -----------------------------------------------------------------------
// NmEditYSTA form view

#include  "FormViewEx.h"
#include  "ListCtrlEx.h"
#include  "ItemCtrl.h"

#include  "CpStore.h"
#include  "GrStore.h"
#include  "MtStore.h"
#include  "NmStore.h"
#include  "SyStore.h"
#include  "TmEntryStore.h"

class NmEditYSTA : public CFormViewEx
{
  public:
	  NmEditYSTA();           
	 ~NmEditYSTA(); 

    virtual bool Edit(va_list);


  protected:
	  virtual void OnInitialUpdate();
    virtual void OnAdd();
    virtual void OnDelete();
    virtual void OnOK();

  private:
    void OnSelChanged(wxListEvent &);

  private:
    void ResetPlReplaces();

  private:
    int ax = false;
    wxString lblPlReplaces[6];

    CItemCtrl *  cpItem = nullptr;
    CItemCtrl *  grItem = nullptr;
    CItemCtrl *  tmItem = nullptr;
	  CListCtrlEx * nmList = nullptr;
	  CListCtrlEx * plList = nullptr;
    wxChoice *   plReplace = nullptr;

    CpStore   cp;
    GrStore   gr;
    SyStore   sy;
    MtStore   mt;
    NmStore   nm;
    TmEntry   tm;
    
  DECLARE_DYNAMIC_CLASS(NmEditYSTA)
  DECLARE_EVENT_TABLE()    
};


#endif 
