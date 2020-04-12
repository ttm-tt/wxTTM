/* Copyright (C) 2020 Christoph Theis */

// drawcp.h
// Auslosung zweite Stufe fuer Oldies (Championship)
// 09.10.94 (ChT)

#ifndef  DRAWCP_H
#define  DRAWCP_H

#include  "DrawInf.h"

#include  "CpStore.h"
#include  "GrStore.h"

struct DLPack;
struct DLMeta;


class  DrawCP
{
  // Konstruktor
  public:
    DrawCP(bool champs, Connection *ptr = NULL);
   ~DrawCP();

    bool Draw(const CpRec &, const wxString &from, const wxString &to, int fromPos, int toPos, int seed);  // Auslosung

  // Variablen
  private:
    Connection *connPtr;
    
    bool  champs;                      // Flag Auslosung fuer Championship
    CpRec cp;                          // WB der Auslosung
    wxString frStage;                  // Stufe, aus der die Teilnehmer stammen
    wxString toStage;                  // Gruppe, in die gelost wird
    int   fromPos;                     // Position in Quali, ab der beruecksichtigt wird
    int   toPos;                       // Position in Quali, bis zu der beruecksichtigt wird

    int  totalFirst;                   // Anzahl erster
    int  totalNA;                      // Anzahl echter Spieler
    int  totalBY;                      // Anzahl Freilose
    int  lastStage;                    // letzte 'Stufe' == Anzahl der 'Stufen'

    DrawListTeam   listBY;             // Liste der Freilose
    DrawListNation listNA;             // Liste der Nationen

    std::map<long, NaRec> idMapping;   // Map Verband zu Regionalverband

    std::list<wxString> warnings;      // Warnungen im Verlaufe der Auslosung

  // Hilfen
  private:
    bool  ReadGroups();                // Einlesen der Gruppen der ersten Stufe 
    bool  ReadSeeding(GrStore &);      // Einlesen der Setzung
    bool  Distribute(GrStore &);       // Setzen der gelosten Spieler in der Gruppe
    bool  BuildPacklist(int, int);     // Aufbau der Packliste
    bool  BuildPackList(int, int, DrawListNation &, DLPack &, DLMeta &, int &, int &);
    bool  DrawPacklist(int, int);      // Auslosen der Packliste
    bool  Shuffle();
    bool  DrawSection(int, int);       // Auslosen einer Section
    bool  DrawStage(int);              // Auslosen einer 'Stufe'
    bool  DrawThem(GrStore &);         // Auslosen ansich
    bool  DrawImpl(long);              // Eigentliche Auslosung
};



#endif