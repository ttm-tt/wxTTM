/* Copyright (C) 2020 Christoph Theis */

#ifndef _MDEDIT_H_
#define _MDEDIT_H_


// -----------------------------------------------------------------------
// CMdEdit form view

#include  "FormViewEx.h"
#include  "MdStore.h"

class CComboBoxEx;


class CMdEdit : public CFormViewEx
{
  public:
	  CMdEdit();    
	 ~CMdEdit(); 

    virtual bool  Edit(va_list);

  protected:
	  virtual void OnInitialUpdate();
    virtual void OnOK();

  private:
    void OnSelChangeMp(wxCommandEvent &);
	  void OnChangeMdEditSize(wxFocusEvent &);
	  void OnMdEditAuto(wxCommandEvent &);
	  void OnGridCellChanged(wxGridEvent &);
	  
  private:  
    void WriteGridCtrl();   // Daten in GridCtrl ausgeben

    CComboBoxEx * m_cbMtPts;
    wxGrid  * m_gridCtrl;
    MdStore   md;
    
  DECLARE_DYNAMIC_CLASS(CMdEdit)
  DECLARE_EVENT_TABLE()  
};


#endif 
