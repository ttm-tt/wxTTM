/* Copyright (C) 2020 Christoph Theis */

// Liste der Ergebnisse
#ifndef  MTLISTSTORE_H
#define  MTLISTSTORE_H

#include  "StoreObj.h"

#include  "MtStore.h"

#include  <list>

struct  GrRec;
struct  TmRec;

struct  MtListRec : public MtRec
{
  short  mtSet;

  short    cpType;
  short    grModus;
  short    grSize;
  short    grWinner;
  short    grQualRounds;
  short    grNofRounds;
  short    grNofMatches;
  short    syComplete;
  wxChar   cpName[9];
  wxChar   grName[9];
  
  MtListRec()  {Init();}
 ~MtListRec()  {}

  void  Init() {memset(this, 0, sizeof(MtListRec));}
};



class  MtListStore : public StoreObj, public MtListRec
{
  public:
    enum Finished
    {
      Event,
      Stage,
      Group,
      Round
    };

  // View in DB erzeugen
  public:
    static  bool  CreateView();
    static  bool  RemoveView();

  public:
    MtListStore(Connection *connPtr = 0);

    virtual void  Init();
    virtual bool  Next();

  public:
    bool  SelectByGr(const GrRec &gr);
    bool  SelectByGrTm(const GrRec &gr, const TmRec &tm);
    bool  SelectByRound(const GrRec &gr, short round);
    bool  SelectByEvent(const MtEvent &);
    bool  SelectByTime(const timestamp &fromTime, short fromTable,
                       const timestamp &toTime, short toTable,
                       bool includeNoTable = false);
    bool  SelectById(long id);
    bool  SelectByNr(long nr);
    bool  SelectByTimestamp(const timestamp &ts);
    
    std::list<timestamp>  ListVenueDays();

    timestamp GetLastUpdateTime();

    // Return tuples of grID and mtRound, and mtDateTime
    std::list<std::tuple<long, short, timestamp>> GetFinishedRounds(Finished what);
    
  private:
    wxString  SelectString() const;
    bool  BindRec();
};

#endif