/* Copyright (C) 2020 Christoph Theis */

#ifndef DRAWLP_H
#define DRAWLP_H

#include  "DrawInf.h"

#include  "CpStore.h"
#include  "GrStore.h"

struct DLPack;
struct DLMeta;

class wxZipOutputStream;


class DrawLP
{
  // Konstruktor
  public:
    DrawLP(bool champs, bool koCons, Connection *ptr = NULL);
   ~DrawLP();

    bool Draw(const CpRec &, const wxString &from, const wxString &to, int fromPos, int toPos, long toGrID, RankingChoice rkChoice, int seed = 0);  // Auslosung

  private:
    wxZipOutputStream * ziplp;
    wxZipOutputStream * ziplog;

  private:
    Connection *connPtr;
    
    bool  champs;                      // Flag Auslosung fuer Championship
    bool  koCons;                      // Flag KO Consolation
    CpRec cp;                          // WB der Auslosung
    wxString fromStage;                // Stufe, aus der die Teilnehmer stammen
    wxString toStage;                  // Stufe, in die gelost wird
    long toGrID;                       // Gruppe, in die gelost wird
    int  fromPos;                      // Position in Quali, ab der beruecksichtigt wird
    int  toPos;                        // Position in Quali, bis zu der beruecksichtigt wird
    int  totalFirst;                   // Anzahl erster
    int  totalNA;                      // Anzahl echter Spieler
    int  totalBY;                      // Anzahl Freilose
    int  totalDE;                      // Anzahl DE
    int  lastStage;                    // letzte 'Stufe' == Anzahl der 'Stufen'
    RankingChoice rkChoice;

    DrawListTeam   listBY;             // Liste der Freilose
    DrawListNation listNA;             // Liste der Nationen
    DrawListTeam   listIRK;            // Liste der Spieler mit Int'l Ranking
    DrawListNation listNRK;            // Liste der Spieler mit Nat'l Ranking

    std::map<long, NaRec> idMapping;   // Map Verband zu Regionalverband

    std::list<wxString> warnings;      // Warnungen im Verlaufe der Auslosung

  // Hilfen
  private:
    bool  ReadGroups();                // Einlesen der Gruppen der ersten Stufe 
    bool  ReadMatches();               // Alternativ einlesen der Verlierer der ersten Stufe KO
    bool  ReadRegions();
    bool  ReadRanking();               // Einlesen des Ranking
    bool  ReadSeeding(GrStore &);      // Einlesen der Setzung
    bool  Distribute(GrStore &);       // Setzen der gelosten Spieler in der Gruppe
    bool  DrawSection(int, int);       // Auslosen einer Section
    bool  DrawStage(int);              // Auslosen einer 'Stufe'
    bool  DrawThem(GrStore &);         // Auslosen ansich
    bool  DrawImpl(long);              // Eigentliche Auslosung

    bool  Shuffle();
};

#endif