/* Copyright (C) 2020 Christoph Theis */


#ifndef _FORMVIEWEX_H_
#define _FORMVIEWEX_H_

#include "Validators.h"  

/////////////////////////////////////////////////////////////////////////////
// CFormViewEx form view

class  CRequest;

// Some default events to be used in direct call of event handler
// Don't use for PostEvent or other calls, only to call event handler directly
extern wxCommandEvent     wxCommandEvent_; 
extern wxFocusEvent       wxFocusEvent_;
extern wxListEvent        wxListEvent_;
// Don't initialize wxInitDialogEvent, it seems an instance can be used only once
// and not more dialogs / frames are initialized after that
// extern wxInitDialogEvent  wxInitDialogEvent_;

class CFormViewEx : public wxPanel
{
  public:
    CFormViewEx(wxWindow *parent = NULL);
	 ~CFormViewEx();
  	

  // Implementation
  public:
    // Edit Record with id...
    virtual bool Edit(va_list vaList);

    virtual void SaveSettings();
    virtual void RestoreSettings();
    
    void  Close();

  protected:
    virtual void  SaveSize();
    virtual void  RestoreSize();

    wxTopLevelWindow * GetTopLevelWindow() const;

  protected:
    virtual void OnOK();
	  virtual void OnCancel();

    virtual void OnAdd();
    virtual void OnEdit();
    virtual void OnDelete();

    virtual void OnRefresh();
    virtual void OnPrint();

    virtual void OnUpdate(CRequest *);
    virtual void OnInitialUpdate();

  protected:
    // Event handler
    void OnInitDialog(wxInitDialogEvent &);
    void OnCommand(wxCommandEvent &);
    void OnCharHook(wxKeyEvent &);
    void OnClose(wxCloseEvent &);    

  protected:
    unsigned  cW;
    
  private:
    int  origX;
    int  origY;
    int  origW;
    int  origH;
    
  DECLARE_DYNAMIC_CLASS(CFormViewEx)
  DECLARE_EVENT_TABLE()
};

#endif 
