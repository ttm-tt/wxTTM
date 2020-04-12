/* Copyright (C) 2020 Christoph Theis */

#ifndef _DRAWCHAMPIONSHIP_H_
#define _DRAWCHAMPIONSHIP_H_


// -----------------------------------------------------------------------
// CDrawChampionship form view

#include "FormViewEx.h"
#include "ComboBoxEx.h"

#include "DrawInf.h"


class CDrawChampionship : public CFormViewEx  
{
  private:
    typedef enum
    {
      Championship = 0,
      Consolation = 1,
      CSD = 2,
      FromQualification = 3,
      FromKO = 4,
      GroupsFromQualification = 5,
    } DrawType;

  private:
    static unsigned int DrawThread(void *);
    
  public:
	  CDrawChampionship();
	 ~CDrawChampionship(); 
	  
	  bool Edit(va_list vaList);

  protected:
	  virtual void OnInitialUpdate();
    virtual void OnOK();

    void OnSelChangeCP(wxCommandEvent &);
    void OnSelChangeToStage(wxCommandEvent &);

  protected:
    CComboBoxEx * cbCP;
    wxComboBox  * cbFromStage;
    wxComboBox  * cbToStage;
    CComboBoxEx  * cbToGroup;

    wxString      frStage;
    wxString      toStage;
    long          toGroup;

    int fromPos;
    int toPos;

    int seed;
    
    DrawType what;
    RankingChoice rkChoice;

    bool lpsolve;

  DECLARE_DYNAMIC_CLASS(CDrawChampionship)  
  DECLARE_EVENT_TABLE()
};


#endif 
