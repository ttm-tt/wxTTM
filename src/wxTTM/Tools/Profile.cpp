/* Copyright (C) 2020 Christoph Theis */

// Methoden zur Verwaltung von Profiles

#include  "stdafx.h"
#include  "Profile.h"   // Module Header

#include  <string.h>
#include  <malloc.h>

#define  BUFFER_SIZE 1024
#define  KEY_SIZE    1024

// Konstruktor
Profile::Profile()
{
  key = NULL;
  buffer = NULL;
}


Profile::Profile(const wxString &fn)
{
  fileName = fn;
  key = NULL;
  buffer = NULL;
}


Profile::~Profile()
{
  if (key)
    free(key);

  if (buffer)
    free(buffer);
}


// Methoden
int  Profile::Open(const wxString & fn)
{
  fileName = fn;

  return 0;
}


int  Profile::GetInt(const wxString &section, const wxString &key, int def)
{
  if (fileName.IsEmpty())
    return def;

  return ::GetPrivateProfileInt(section.t_str(), key.t_str(), def, fileName.t_str());
}


bool Profile::GetBool(const wxString &section, const wxString &key, bool def)
{
  return GetInt(section, key, def ? 1 : 0) != 0;
}


wxString Profile::GetString(const wxString &section, const wxString &key, const wxString &def)
{
  if (fileName.IsEmpty())       // interner Fehler
    return def;

  if (section.IsEmpty() || key.IsEmpty())        // ungueltige Parameter
    return def;

  unsigned len = 0;    
  
  do    
  {
    len += BUFFER_SIZE;
    buffer = (wxChar *) realloc(buffer, (len + 1) * sizeof(wxChar));
  } while ( ::GetPrivateProfileString(section.t_str(), key.t_str(), def.t_str(), buffer, len, fileName.t_str()) == len - 1 );
  
  return (buffer && *buffer ? wxString(buffer) : def);
}


wxString Profile::GetFirstKey(const wxString &section)
{
  if (fileName.IsEmpty() || section.IsEmpty())
    return wxEmptyString;

	keyPointer = 0;
	
	unsigned len = 0;
	
  do
  {
    len += KEY_SIZE;
    key = (wxChar *) realloc(key, (len + 1) * sizeof(wxChar));
  } while ( ::GetPrivateProfileString(section.t_str(), NULL, wxT(""), key, len, fileName.t_str()) == len - 2);
  
  return (*key ? wxString(key) : wxEmptyString);
}


wxString Profile::GetNextKey()
{
  if (!key || !key[keyPointer])  // Kein Eintrag oder Ende erreicht
    return wxEmptyString;

  wxChar *ptr = key + keyPointer;  // Liste von '\0' terminierten Strings
  keyPointer += wxStrlen(ptr) + 1;

  return (*(key + keyPointer) ? wxString(key + keyPointer) : wxEmptyString);
}


int  Profile::AddString(const wxString &section, const wxString &key, const wxString &value)
{
  if (fileName.IsEmpty() || section.IsEmpty())
    return -1;

  if(value.IsEmpty())          // Bedeutet loeschen
    return DeleteString(section, key);

  int rc = ::WritePrivateProfileString(section.t_str(), key.t_str(), value.t_str(), fileName.t_str());
  return (rc ? 0 : 1);
}


int  Profile::AddInt(const wxString &section, const wxString &key, long val)
{
	// if (!val)
	// 	return DeleteString(section, key);

	wxChar  str[32];
	_ltot(val, str, 10);

	return AddString(section, key, str);
}


int  Profile::AddBool(const wxString &section, const wxString &key, bool val)
{
  return AddInt(section, key, val ? 1 : 0);
}


int  Profile::DeleteString(const wxString &section, const wxString &key)
{
  if (fileName.IsEmpty() || section.IsEmpty())
    return -1;

  int rc = ::WritePrivateProfileString(section.t_str(), key.t_str(), NULL, fileName.t_str());
  return (rc ? 0 : 1);
}


int  Profile::DeleteSection(const wxString &section)
{
  if (fileName.IsEmpty() || section.IsEmpty())
    return -1;

  int rc = ::WritePrivateProfileString(section.t_str(), NULL, NULL, fileName.t_str());
  return (rc ? 0 : 1);
}


wxString Profile::GetFirstSection()
{
  if (fileName.IsEmpty())
    return wxEmptyString;

  keyPointer = 0;

  unsigned len = 0;

  do
  {
    len += KEY_SIZE;
    key = (wxChar *)realloc(key, (len + 1) * sizeof(wxChar));
  } while (::GetPrivateProfileSectionNames(key, len, fileName.t_str()) == len - 2);

  return (*key ? wxString(key) : wxEmptyString);
}


wxString Profile::GetNextSection()
{
  if (!key || !key[keyPointer])  // Kein Eintrag oder Ende erreicht
    return wxEmptyString;

  wxChar *ptr = key + keyPointer;  // Liste von '\0' terminierten Strings
  keyPointer += wxStrlen(ptr) + 1;

  return (*(key + keyPointer) ? wxString(key + keyPointer) : wxEmptyString);
}


// EOF  Profile.cpp