/* Copyright (C) 2020 Christoph Theis */

// Information und Error System

#ifndef  INFOSYSTEM_H
#define  INFOSYSTEM_H

#include <string>

class  SQLException;

class  InfoSystem
{
  // Konstruktor
  public:
    InfoSystem();  
    ~InfoSystem();          

  // Aufruf
  public:
    // Bestaetigung: id, Buttons, Titel
	  bool Confirmation(const wxChar *fmt, ...);
    bool Question(const wxChar *fmt, ...);

    // Information 
    bool Information(const wxChar *fmt, ...);

    // Warnung, Fehler
    bool Warning(const wxChar *fmt, ...);
    bool Error(const wxChar *fmt, ...);
    bool Fatal(const wxChar *fmt, ...);

    // SQL-Fehler
    bool Exception(const wxString &hint, const SQLException &e);

  private:
    bool  DisplayMessageBox(const wxString &title, const wxString &text, int type);

  // Defaults
  private:
};



#endif