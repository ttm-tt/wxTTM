/* Copyright (C) 2020 Christoph Theis */

// Profile Management

#ifndef  PROFILE_H
#define  PROFILE_H

class  Profile
{
  // Konstruktor
  public:
    Profile();
    Profile(const wxString &);  // mit Dateiname
   ~Profile();

  // Methoden
  public:
    int    Open(const wxString &);
    wxString GetFirstSection();
    wxString GetNextSection();
    int    DeleteSection(const wxString &section);
    int    GetInt(const wxString &section, const wxString &key, int def = -1);
  	bool   GetBool(const wxString &section, const wxString &key, bool def = false);
	  wxString GetString(const wxString &section, const wxString &key, const wxString &def = wxEmptyString);
    wxString GetFirstKey(const wxString &section);
    wxString GetNextKey();
    int    AddString(const wxString &section, const wxString &key, const wxString &);
    int    AddInt(const wxString &section, const wxString &key, long);
    int    AddBool(const wxString &section, const wxString &key, bool);
    int    DeleteString(const wxString &section, const wxString &key);

  // Variablen
  private:
    wxString fileName;      // Filename
    wxChar * key;           // Liste aller Eintraege
    wxChar * buffer;        // Buffer fuer Get...String
    int    keyPointer;      // Pointer in entry fuer GetFirst, GetNext
};



#endif

// EOF  Profile.h