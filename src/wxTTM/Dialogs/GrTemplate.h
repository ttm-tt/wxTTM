/* Copyright (C) 2020 Christoph Theis */

#ifndef _GRTEMPLATE_H_
#define _GRTEMPLATE_H_

#include  "GrStore.h"


// -----------------------------------------------------------------------
// CGrTemplate dialog

class CGrTemplate : public wxDialog
{
  // Construction
  public:
	  CGrTemplate(GrStore::CreateGroupStruct *ptr = NULL, wxWindow *parent = NULL);
	 ~CGrTemplate();
	  
	  void OnInitDialog(wxInitDialogEvent &);

  private:
    void OnKillFocus(wxFocusEvent &);

  private:
    void UpdateDialog();
    
    GrStore::CreateGroupStruct *cgs;
    
  DECLARE_DYNAMIC_CLASS(CGrTemplate)
  DECLARE_EVENT_TABLE()  
};


#endif 
