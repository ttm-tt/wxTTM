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
    void OnMarkPrinted(wxCommandEvent &);
    void OnKillFocus(wxFocusEvent &);
    void OnCombined(wxCommandEvent &);
  
  private:
    short  m_matchOption = 0;
    bool   m_notPrinted = false;
    bool   m_inclTeam = false;
    bool   m_combined = false;
    bool   m_consolation = false;

  private:
    struct PrintScheduledScoreStruct
    {
      std::vector<MtListRec *> *mtList = nullptr;
      Printer *printer = nullptr;
      bool     isPreview = false;
      bool     isCombined = false;
	    bool     rrConsolation = false;
    };

    static unsigned PrintScheduledThread(void *);

    void  DoPrint();

    void  DoPrintMatch();
    void  DoPrintRound();
    void  DoPrintGroup();
    void  DoPrintScheduled();

    void  DoSetPrinted(boolean set);

    Printer *m_printer = nullptr;

    MtStore::MtPlace fromPlace;
    MtStore::MtPlace toPlace;

    MtRec  mt;
    CpRec  cp;
    GrRec  gr;
    
  DECLARE_DYNAMIC_CLASS(CScore)
  DECLARE_EVENT_TABLE()
};


#endif 
