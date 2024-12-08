/* Copyright (C) 2020 Christoph Theis */

#ifndef _SCORE_H_
#define _SCORE_H_


// -----------------------------------------------------------------------
// CTossSheet form view

class  Printer;


#include  "FormViewEx.h"

// #include  "DateTimeEdit.h"

#include  "MtStore.h"
#include  "GrStore.h"
#include  "CpStore.h"

#include <vector>

struct MtListRec;


class CTossSheet : public CFormViewEx
{
  public:
	  CTossSheet(); 
	 ~CTossSheet(); 

    virtual bool  Edit(va_list);

  protected:
	  virtual void OnInitialUpdate();
    virtual void OnPrint();

  private:
    void OnSelectMatch(wxCommandEvent &);
    void OnKillFocus(wxFocusEvent &);
  
  private:
    short  m_matchOption = 0;

  private:
    struct PrintScheduledTossStruct
    {
      std::vector<MtListRec *> *mtList = nullptr;
      Printer *printer = nullptr;
      bool     isPreview = false;
    };

    static unsigned PrintScheduledThread(void *);

    void  DoPrint();

    void  DoPrintMatch();
    void  DoPrintScheduled();

    Printer *m_printer;

    MtStore::MtPlace fromPlace;
    MtStore::MtPlace toPlace;

    MtRec  mt;
    CpRec  cp;
    GrRec  gr;
    
  DECLARE_DYNAMIC_CLASS(CTossSheet)
  DECLARE_EVENT_TABLE()
};


#endif 
