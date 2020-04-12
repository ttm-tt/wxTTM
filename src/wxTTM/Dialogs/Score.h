/* Copyright (C) 2020 Christoph Theis */

#ifndef _SCORE_H_
#define _SCORE_H_


// -----------------------------------------------------------------------
// CScore form view

class  Printer;


#include  "FormViewEx.h"

// #include  "DateTimeEdit.h"

#include  "MtStore.h"
#include  "GrStore.h"
#include  "CpStore.h"

#include <vector>

struct MtListRec;


class CScore : public CFormViewEx
{
  public:
	  CScore(); 
	 ~CScore(); 

    virtual bool  Edit(va_list);

  protected:
	  virtual void OnInitialUpdate();
    virtual void OnPrint();

  private:
    void OnSelectMatch(wxCommandEvent &);
    void OnResetPrinted(wxCommandEvent &);
    void OnKillFocus(wxFocusEvent &);
    void OnCombined(wxCommandEvent &);
  
  private:
    short  m_matchOption;
    bool   m_notPrinted;
    bool   m_inclTeam;
    bool   m_combined;
    bool   m_consolation;

  private:
    struct PrintScheduledStruct
    {
      std::vector<MtListRec *> *mtList;
      Printer *printer;
      bool     isPreview;
      bool     isCombined;
	  bool     rrConsolation;
    };

    static unsigned PrintScheduledThread(void *);

    void  DoPrint();

    void  DoPrintMatch();
    void  DoPrintRound();
    void  DoPrintGroup();
    void  DoPrintScheduled();

    Printer *m_printer;

    MtStore::MtPlace fromPlace;
    MtStore::MtPlace toPlace;

    MtRec  mt;
    CpRec  cp;
    GrRec  gr;
    
  DECLARE_DYNAMIC_CLASS(CScore)
  DECLARE_EVENT_TABLE()
};


#endif 
