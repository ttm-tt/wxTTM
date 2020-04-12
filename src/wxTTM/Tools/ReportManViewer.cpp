/* Copyright (C) 2020 Christoph Theis */

#include "stdafx.h"

#include "ReportManViewer.h"

#include "TT32App.h"
#include "TTDbse.h"

#include "InfoSystem.h"

#include <process.h>


// -----------------------------------------------------------------------
void CReportManViewer::DoReport(const wxString &name, const std::map<wxString, wxString> &params, wxWindow *parent)
{
  // Turnierverzeichnis (hier liegen spezialisierte Versionen)
  wxString fname = CTT32App::instance()->GetPath() + "\\" + name + ".rep";

  // Wenn nicht, dann im aktuellen Verzeichnis (hier liegt das .ini-File und damit auch die installierten Reports)
  if (_access(fname.data(), 00) == -1)
    fname = name + ".rep";

  // Beim .exe, warum auch immer
  if (_access(fname.data(), 00) == -1)
  {
    wxChar path[_MAX_PATH];
    GetModuleFileName(NULL, path, _MAX_PATH);
    *(wxStrrchr(path, wxT('\\')) +1) = wxT('\0');
    fname = path + name + wxT(".rep");
  }

  // Im workspace schauen, relativ zum .exe
  if (_access(fname.data(), 00) == -1)
  {
    wxFileName exe = wxFileName(wxStandardPaths::Get().GetExecutablePath());
    exe.RemoveLastDir();
    exe.RemoveLastDir();
    exe.RemoveLastDir();
    exe.AppendDir("Reports");
    wxString path = exe.GetPath();

    fname = path + wxT("\\") + name + wxT(".rep");
  }

  // Falls nicht gefunden, Fehlermeldung
  if (_access(fname.data(), 00) == -1)
  {
    infoSystem.Error(_("Cannot find report %s"), name.t_str());
    return;
  }

  // CReportView  dlg(itemPtr->GetFName().data());
  // dlg.DoModal();
  
  // Sprache auswaehlen
  long  lang = 0;
	const wxString resDLL = CTT32App::instance()->GetPrintLanguage();
	if (resDLL.size() == 0)
	  lang = 0;  // Default: English
	else if (resDLL.compare(0, 3, "en_") == 0)
	  lang = 0;  // English
	else if (resDLL.compare(0, 3, "de_") == 0)	
	  lang = 5;  // Deutsch
	else
	  lang = 0;	
	  
  wxString connectionString = 
      "Provider=MSDASQL.1;Persist Security Info=True;Extended Properties=\"" +
      TTDbse::instance()->GetConnectionString() + "\";";
  
  CReportManViewer *reportManPtr = new CReportManViewer();
  
  reportManPtr->SetFilename(fname);
  reportManPtr->SetDatabaseConnectionString(wxT("TT32"), connectionString);
  reportManPtr->SetLanguage(lang);
  reportManPtr->SetParamValue(wxT("PTITLE"), CTT32App::instance()->GetReportTitle());
  reportManPtr->SetParamValue(wxT("PSUBTITLE"), CTT32App::instance()->GetReportSubtitle());

  for (std::map<wxString, wxString>::const_iterator it = params.begin(); it != params.end(); it++)
  {
    if ( !(*it).first.IsEmpty() )
      reportManPtr->SetParamValue((*it).first, (*it).second);
  }

  reportManPtr->ShowParams();

  reportManPtr->SetPreview(CTT32App::instance()->GetPrintPreview() ? true : false);

  if (CTT32App::instance()->GetPrintPdf())
  {
    wxFileDialog fileDlg(
      parent, wxFileSelectorPromptStr, CTT32App::instance()->GetPath(), wxString::Format("%s.pdf", name), 
      wxT("PDF Files (*.pdf)|*.pdf|All Files (*.*)|*.*||"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (fileDlg.ShowModal() != wxID_OK)
      return;

    reportManPtr->SaveToPDF(fileDlg.GetPath());
  }
  else 
    reportManPtr->ExecuteAsync();

  return;
}


// -----------------------------------------------------------------------
// Das ActiveX Objekt, um ein einfaches InvokeHelper zu haben
CReportManViewer::CReportManViewer()
{
  CreateInstance(wxT("ReportMan.ReportManX"));
}

void CReportManViewer::SetDatabaseConnectionString(const wxString &databasename, 
	                                const wxString &connectionstring)
{
  CallMethod(wxT("SetDatabaseConnectionString"), wxVariant(databasename), wxVariant(connectionstring));
}	           
    
    
wxString CReportManViewer::GetDatabaseConnectionString(const wxString &databasename)
{
  wxVariant res = CallMethod(wxT("GetDatabaseConnectionString"), wxVariant(databasename));
      
  return res.GetString();
}                        
	  
void CReportManViewer::Execute()
{
	CallMethod(wxT("Execute"));
}
	  
void CReportManViewer::ShowParams()
{
	CallMethod(wxT("ShowParams"));
}
	  
void CReportManViewer::SetFilename(const wxString &fname)
{
	PutProperty(wxT("Filename"), wxVariant(fname));
}
	  
bool CReportManViewer::GetPreview()
{
	return GetProperty(wxT("Preview")).GetBool();
}
	  
	  
void CReportManViewer::SetPreview(bool val)
{
	PutProperty(wxT("Preview"), wxVariant(val));
}
	  
	  
void CReportManViewer::SetParamValue(const wxString & name, const wxString & value)
{
	CallMethod(wxT("SetParamValue"), wxVariant(name), wxVariant(value));
}
	  
	  
void CReportManViewer::SetLanguage(long val)
{
	PutProperty(wxT("Language"), wxVariant(val));
}
	  
void CReportManViewer::AboutBox()
{
	CallMethod(wxT("AboutBox"));
}	  
	  
void CReportManViewer::SaveToPDF(const wxString &fileName, bool compressed)
{
  CallMethod(wxT("SaveToPDF"), wxVariant(fileName), wxVariant(compressed));
}
	  
void CReportManViewer::ExecuteAsync()
{
	_beginthread(ExecuteThread, 0, this);
}

	  
void CReportManViewer::ExecuteThread(void *ptr)
{
  ::CoInitialize(NULL);
  try
  {
    ((CReportManViewer *) ptr)->Execute();
  }
  catch(...)
  {
    // Z.B. "No data to print"
    // infoSystem.Error(pEx->m_strDescription);
  }
      
      
  ::CoUninitialize();
  delete ((CReportManViewer *) ptr);
}	  


