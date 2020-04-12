/* Copyright (C) 2020 Christoph Theis */

// drawrr.h
// Auslosung erste Stufe fuer Oldies (Round Robin)
// 21.09.94 (ChT)

#ifndef  DRAWRR_H
#define  DRAWRR_H

#include  "DrawInf.h"

#include  "CpStore.h"
#include  "GrStore.h"


class  DrawRR
{
  // Konstruktor
  public:
    DrawRR(Connection *ptr = NULL);
   ~DrawRR();

    bool Draw(const CpRec &, const wxString &fromStage, const wxString &toStage, int fromPos, int toPos, RankingChoice, int seed);     // Auslosung

// Abfolge der Auslosung
  private:
    bool DrawImpl();

    bool ReadGroups();            // Legt Gruppen per GRTMPL an

    // Hilfen fuer Draw
    bool  Shuffle();    
		bool  Mix(int *, int);
		bool  ClearGroups();  
    bool  ReadEntries();
		bool  ReadSeeding(int *);
    bool  ReadRanking();           // Ranking lesen
    bool  DrawRanking(int *);
		bool  DrawThem(int *);    
		bool  Distribute(int *);       

    int  Minimum(int *, int);
    int  NextNumber(const int *, const int *, const int *, std::map<int, int> &, int *, const int *, int);
  // Variablen
  private:
    Connection *connPtr;
    
    CpRec     cp;
    wxString  fromStage;
    wxString  toStage;
    int       fromPos;
    int       toPos;
    RankingChoice rkChoice;

    int  groups;                       // Anzahl Gruppen in Stage 'stage'
    GrStore *pgr;                      // Array der Gruppen (oder der IDs)
		DrawListNation listNA;             // Liste der Nationen
    DrawListTeam   listIRK;            // Liste der Spieler mit Int'l Ranking

    std::list<wxString> warnings;      // Warnungen im Verlaufe der Auslosung
};



#endif