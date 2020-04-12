/* Copyright (C) 2020 Christoph Theis */

#include <wx/msw/ole/automtn.h>

#include <map>

// -----------------------------------------------------------------------
// Das ActiveX Objekt, um ein einfaches InvokeHelper zu haben
class CReportManViewer : public wxAutomationObject 
{
  public :
    static void DoReport(const wxString &name, const std::map<wxString, wxString> &params, wxWindow *parent = NULL);

  public :
    CReportManViewer();

	public:
	  void SetDatabaseConnectionString(const wxString &databasename, const wxString &connectionstring);  
    wxString GetDatabaseConnectionString(const wxString &databasename);
	  
	  void Execute();
	  
	  void ShowParams();
	  
	  void SetFilename(const wxString &fname);
	  
	  bool GetPreview();	    
	  void SetPreview(bool val);
	  
	  void SetParamValue(const wxString & name, const wxString & value);
	  
	  void SetLanguage(long val);
	  
	  void AboutBox();
	  
    void SaveToPDF(const wxString &fileName, bool compressed = true);
	  
	  void ExecuteAsync();
	  
  private:
    static void __cdecl ExecuteThread(void *ptr);
};


