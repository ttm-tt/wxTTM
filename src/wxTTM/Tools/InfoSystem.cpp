/* Copyright (C) 2020 Christoph Theis */

// Information und Error System

#include  "stdafx.h"

#include  "InfoSystem.h"
#include  "StrUtils.h"
#include  "res.h"
#include  "SQLException.h"

#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <stdarg.h>

#include  <string>


InfoSystem::InfoSystem()
{
};


InfoSystem::~InfoSystem()
{
};


// -----------------------------------------------------------------------
// Bestaetigung: id, Buttons, Titel
bool  InfoSystem::Confirmation(const wxChar *fmt, ...)
{
  wxString text;
  va_list   varg;

  va_start(varg, fmt);
  
  text.PrintfV(fmt, varg);

  return DisplayMessageBox(_("Confirmation"), text, wxOK | wxCANCEL | wxICON_QUESTION | wxCENTER);
}


// -----------------------------------------------------------------------
// Frage, Buttons, Titel
bool  InfoSystem::Question(const wxChar *fmt, ...)
{
  wxString text;
  va_list   varg;

  va_start(varg, fmt);
  
  text.PrintfV(fmt, varg);

  return DisplayMessageBox(_("Question"), text, wxYES_NO | wxICON_QUESTION | wxCENTER);
}


// -----------------------------------------------------------------------
// Information
bool  InfoSystem::Information(const wxChar *fmt, ...)
{
  wxString text;
  va_list   varg;

  va_start(varg, fmt);
  
  text.PrintfV(fmt, varg);

  return DisplayMessageBox(_("Information"), text, wxOK | wxICON_INFORMATION | wxCENTER);
}


// -----------------------------------------------------------------------
// Warnung
bool  InfoSystem::Warning(const wxChar *fmt, ...)
{
  wxString text;
  va_list   varg;

  va_start(varg, fmt);
  
  text.PrintfV(fmt, varg);

  return DisplayMessageBox(_("Warning"), text, wxOK | wxICON_WARNING | wxCENTER);
}


// -----------------------------------------------------------------------
// Fehler
bool  InfoSystem::Error(const wxChar *fmt, ...)
{
  wxString text;
  va_list   varg;

  va_start(varg, fmt);
  
  text.PrintfV(fmt, varg);

  return DisplayMessageBox(_("Error"), text, wxOK | wxICON_EXCLAMATION | wxCENTER);
}


// -----------------------------------------------------------------------
// toedlicher Fehler
bool  InfoSystem::Fatal(const wxChar *fmt, ...)
{
  wxString text;
  va_list   varg;

  va_start(varg, fmt);
  
  text.PrintfV(fmt, varg);

  DisplayMessageBox(_("Fatal Error"), text, wxOK | wxICON_EXCLAMATION | wxCENTER);
  
  ::exit(1);

  return true;
}


// -----------------------------------------------------------------------
// SQL-Exception, -Error, -Warning, -Info
bool  InfoSystem::Exception(const wxString &sql, const SQLException &e)
{
  wxString  title;  // Soll auf jeden Fall ausreichen
  wxString  message;

  title = wxString::Format("SQL Error [%s]", e.GetSQLState());

  if (wxSnprintf(wxStringBuffer(message, 1024), 1024-4, "%s\nLocation: %s %d\nStatement: %s", 
                 e.GetMessage(), e.GetFileName(), e.GetLineNr(), sql.data()) < 0)
    message.Append("...");

  return DisplayMessageBox(title, message, wxOK | wxICON_EXCLAMATION | wxCENTER);
}



// -----------------------------------------------------------------------
// Messagebox ausgeben
bool  InfoSystem::DisplayMessageBox(const wxString &title, const wxString &text, int type)
{
  wxWindow *parent = wxTheApp->GetTopWindow();

  switch (wxMessageBox(text, title, type, parent))
  {
    case wxOK :
    case wxYES :
      return true;
    default :
      return false;
  }
}