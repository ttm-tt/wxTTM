/* Copyright (C) 2020 Christoph Theis */

#ifndef _GROPTIONS_H_
#define _GROPTIONS_H_


#include  "Raster.h"


// -----------------------------------------------------------------------
// CGrOptions dialog

class CGrOptions : public wxDialog
{
  // Construction
  public:
	  CGrOptions();   // standard constructor
	 ~CGrOptions(); 
	 
	public:
	  void SetPrintRasterOptions(PrintRasterOptions *ptr);

  protected:
    virtual bool TransferDataFromWindow();

  private:
    void OnInitDialog(wxInitDialogEvent &);
    void OnCheckBoxSelected(wxCommandEvent &);
    void OnSaveAs(wxCommandEvent &);
      
  private:
    PrintRasterOptions *m_poPtr;

    int   koRounds;
    int   koMatches;
    bool  koLastRounds;
    bool  koLastMatches;
    
  DECLARE_DYNAMIC_CLASS(CGrOptions)
  DECLARE_EVENT_TABLE()  
};

#endif 
