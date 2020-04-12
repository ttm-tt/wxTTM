/* Copyright (C) 2020 Christoph Theis */

#ifndef _MTSELECT_H_
#define _MTSELECT_H_

#include "FormViewEx.h"

// -----------------------------------------------------------------------
// CMtSelect dialog

class CMtSelect : public CFormViewEx
{
  public:
	  CMtSelect();   
	 ~CMtSelect(); 
	 
  protected:
	  virtual void OnInitialUpdate();

  private:
    void OnResult(wxCommandEvent &);
	  void OnScore(wxCommandEvent &);
	  
	private:	
	  long m_mtNr;
	  
	DECLARE_DYNAMIC_CLASS(CMtSelect)
	DECLARE_EVENT_TABLE()
};


#endif 
