/* Copyright (C) 2020 Christoph Theis */

#pragma once

// Persistenz von PrintRasterOptions

#include  "StoreObj.h"

#include  <list>

class Profile;
class wxJSONValue;

struct  PrintRasterOptions
{
  // Seite RR
  bool  rrResults;     // Flag Ergebnisse ausdrucken
  bool  rrSignature;   // Flag Unterschrift des Siegers
  bool  rrTeamDetails; // Auch die einzelnen Spiele bei Mannschaften (RR)
  bool  rrIgnoreByes;  // Byes wie Spieler behandelnt (aber nicht drucken)
  bool  rrSchedule;    // Schedule in Raster drucken
  bool  rrNewPage;     // Flag Jede Gruppe auf neuer Seite
  bool  rrSlctRound;   // Flag Ausgewaehlte Runden
  short rrFromRound;   // Von Runde
  short rrToRound;     // Bis Runde
  bool  rrCombined;    // Combined Scoresheet
  bool  rrConsolation; // Print participate consolation
  bool  rrLastResults; // Ergebnisse nur von der letzten gespielten Runde
  bool  rrPrintNotes;  // Anmerkungen unter die Gruppe drucken

  // Seite KO
  bool  koSlctRound;   // Flag Ausgewaehlte Runden
  short koFromRound;   // Von Runde
  short koToRound;     // Bis Runde
  bool  koLastRounds;  // Die letzten Runden
  bool  koNoQuRounds;  // Keine Qualifikation
  bool  koSlctMatch;   // Flag Ausgewaehlte Spiele
  short koFromMatch;   // Von Spiel
  short koToMatch;     // Bis Spiel
  bool  koLastMatches; // Die letzten Spiele
  bool  koNr;          // Flag: auch Matchnr
  bool  koTeamDetails; // Auch die einzelnen Spiele bei Mannschaften (KO)
  bool  koIgnoreByes;  // Byes wie Spieler behandeln (aber nicht drucken)
  bool  koNewPage;     // Jede Gruppe auf neuer Seite drucken
  bool  koLastResults; // Ergebnisse nur von der letzten Runde
  bool  koInbox;       // Spieler innerhalb des Rasters
  bool  koPrintPosNrs; // Drucke Position am Anfang
  bool  koPrintNotes;  // Anmerkungen unter die Gruppe drucken
  bool  koPrintOnlyScheduled;  // Nur angesetzte Spiele drucken
};

struct PoRec : public PrintRasterOptions
{
  long   poID;
  wxChar poName[64];

  PoRec() { Init(); }
  PoRec(const PrintRasterOptions &option) { Init(); PrintRasterOptions::operator=(option); }
  void  Init() { memset(this, 0, sizeof(PoRec)); }
};


class PoStore : public StoreObj, public PoRec
{
public:
  static  bool  CreateTable();
  static  bool  UpdateTable(long version);

public:
  PoStore(Connection * = 0);
  PoStore(const PrintRasterOptions &options, Connection *connPtr = 0);
  ~PoStore();

  virtual void Init();
  virtual bool Next();

public:
  bool Read(const wxString &name = wxEmptyString);
  bool Write(const wxString &name = wxEmptyString, bool isPrivate = true);
  bool Delete(const wxString &);

  std::list<wxString> List() const;

  bool Exists(const wxString &);

  bool Publish(const wxString &name);

public:
  bool  Insert();
  bool  Update();
  bool  InsertOrUpdate();
  bool  Remove(long id);

  bool  SelectAll();
  bool  SelectById(long id);
  bool  SelectByName(const wxString &name);

public:
  long  NameToID(const wxString &name);

private:
  bool  ReadFromProfile(Profile &profile, const wxString &section);
  bool  WriteToProfile(Profile &profile, const wxString &section) const;
  bool  DeleteFromProfile(Profile &profile, const wxString &section);

  bool  ReadFromJson(const wxString &);
  bool  WriteToJson(wxString &) const;

  bool  ReadFromDatabase(const wxString &name);
  bool  WriteToDatabse(const wxString &name);
  bool  DeleteFromDatabase(const wxString &name);

private:
  wxString  SelectString() const;
  short GetShort(wxJSONValue &, const wxString &);
};