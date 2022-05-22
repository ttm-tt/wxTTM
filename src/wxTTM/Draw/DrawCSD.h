/* Copyright (C) 2020 Christoph Theis */

// Auslosung a'la CSD
#ifndef DRAWCSD_H
#define DRAWCSD_H

#include "DrawInf.h"

#include "GrStore.h"
#include "CpStore.h"

struct CpRec;

class  DrawCSD
{
  public:
    DrawCSD(Connection *ptr = NULL);
   ~DrawCSD();
    
  public:
    bool Draw(const CpRec &cp, wxString deStage, wxString quStage, int seed);
    
  private:
    bool ReadRanking(const CpRec &cp);
    bool ReadGroups(const CpRec &cp, const wxString &stage);
    bool ReadSeeding(const CpRec &cp);
    
    bool DrawThem(int startStage, int lastDrawStage, int lastStage);
    bool DrawStage(int stg, int lastStage);
    bool DrawSection(int stg, int sec, int lastStage);
    
    bool DrawSeededPacks(int stg, int sec);
    bool DistributeByes(int stg, int sec, int lastStage);
    bool DistributeQGroups(int stg, int sec, int lastStage);
    
    bool BuildPackList(int stg, int sec);
    DrawItemPack * GetPack(DrawListTeam &list, int stg, int sec);
    bool DrawPack(DrawItemPack *, int stg, int sec);
    bool ActualizeDrawParameters(int op);
    bool SolutionExists(int op);
    int  GetOperator(DrawItemPack *);
    
    bool CreateGroups(const CpRec &cp, wxString deStage, wxString quStage);
    bool DistributeEntries();
    
  private:    
    DrawListNation listNA;  // Liste der Verbaende
    DrawListTeam   listBY;  // Liste der Freilose
    DrawListTeam   listQG;  // Liste der Q-Gruppen
    DrawListPack   listPK;  // Liste der Packs
    
    int opCounter[5];
    
    Connection *connPtr;
    GrStore *pgr;
    
    int de = 0;     // Number direct entries
    int qu= 0;      // Number qualifiers
    int dde = 0;    // Diff direct entrices upper - lower half
    int dtn = 0;    // Diff all entries upper - lower half
    int deSec = 0;  // Number direct entries in stage
    int quSec = 0;  // Number qualifiers in stage
    
    int sizeDGroup = 0;  // Entries in competition proper
    int nofQGroups = 0;  // Number of Q-Groups    
};


#endif
