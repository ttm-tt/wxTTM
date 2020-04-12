/* Copyright (C) 2020 Christoph Theis */

#ifndef _DRAWQUALIFICATION_H_
#define _DRAWQUALIFICATION_H_


// -----------------------------------------------------------------------
// CDrawQualification form view

#include  "FormViewEx.h"
#include  "ComboBoxEx.h"

class CDrawQualification : public CFormViewEx
{
  private:
    static unsigned int DrawThread(void *);

  public:
	  CDrawQualification();      
	 ~CDrawQualification();

  protected:
	  virtual void OnInitialUpdate();
    virtual void OnOK();

  private:
    CComboBoxEx * cbCP;
    wxComboBox  * cbStage;

    wxString stage;

    int seed;

    void OnSelChangeCP(wxCommandEvent &);
    
  DECLARE_DYNAMIC_CLASS(CDrawQualification)
  DECLARE_EVENT_TABLE()
};


#endif 
