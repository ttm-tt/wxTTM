/* Copyright (C) 2020 Christoph Theis */

#ifndef _LTDOUBLE_H_
#define _LTDOUBLE_H_


// -----------------------------------------------------------------------
// CLtDouble form view

#include "FormViewEx.h"
#include "ComboBoxEx.h"
#include "PlItem.h"
#include "ItemCtrl.h"
#include "LtStore.h"

class CLtDouble : public CFormViewEx
{
  public:
	  CLtDouble();           // protected constructor used by dynamic creation
	 ~CLtDouble(); 
  	
  public:    
    virtual  bool  Edit(va_list);

  public:
	  virtual void OnInitialUpdate();

  protected:
    virtual void OnOK();

  private:
  	void OnSelchangeLtDoublePartner(wxCommandEvent &);

  private:
    long          m_cpID;       // Wettbewerb
    long          m_plID;       // Spieler
    CItemCtrl *   m_plItem;     // Spielerdaten Spieler
    CComboBoxEx * m_bdCbBox;    // Combobox Partner
	  CComboBoxEx * m_naCbBox;    // Combobox der Nationen
    
  DECLARE_DYNAMIC_CLASS(CLtDouble)
  DECLARE_EVENT_TABLE()    
};

#endif 
