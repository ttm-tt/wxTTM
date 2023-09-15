/* Copyright (C) 2020 Christoph Theis */

# ifndef  MTENTRYSTORE_H
# define  MTENTRYSTORE_H

#include  "TmEntryStore.h"
#include  "MtListStore.h"

struct  MtEntry
{
  MtListRec  mt;  
  
  TmEntry    tmA;
  short      nmAnmNr;
  
  TmEntry    tmX;
  short      nmXnmNr;

  MtEntry()    {Init();}
  MtEntry(const MtListRec &mt_, const TmEntry &tmA_, const TmEntry &tmX_, short ms = 0)
    : mt(mt_), tmA(tmA_), nmAnmNr(0), tmX(tmX_), nmXnmNr(0) 
  {
    mt.mtEvent.mtMS = ms; 
    if (tmA.team.cpType == CP_GROUP)
      mt.xxAstID = tmA.team.gr.xxStID;
    if (tmX.team.cpType == CP_GROUP)
      mt.xxXstID = tmX.team.gr.xxStID;
  }
    
  void  Init();
  bool  IsABye() const {return mt.IsABye();}
  bool  IsXBye() const {return mt.IsXBye();}
  bool  IsBye() const  {return IsBye();}
  bool  IsAByeOrXBye() const {return mt.IsAByeOrXBye();}
  
  bool  IsFinished() const {return mt.IsFinished();}

  short QryWinnerAX() const  {return mt.QryWinnerAX();}
};


inline  void MtEntry::Init()
{
  mt.Init();
  tmA.Init();
  tmX.Init();

  nmAnmNr = 0;
  nmXnmNr = 0;
}


class  MtEntryStore : public StoreObj, public MtEntry
{
  public:
    static  bool  CreateView();
    static  bool  RemoveView();

  public:
    MtEntryStore(Connection *connPtr = 0);

    virtual void  Init();

    // Next liest auch die Ergebnisse aus
    bool  Next();

  public:
    bool  SelectByGr(const GrRec &gr, short round, short cpType);
    bool  SelectByMS(const MtRec &mt, short ms = 0);
    bool  SelectById(long id, short cpType);
    bool  SelectByTime(const timestamp &fromTime, short fromTable,
                       const timestamp &toTime, short toTable, short cpType = CP_UNKNOWN);
    bool  SelectByTime(const MtStore::MtPlace &fromEvent, const MtStore::MtPlace &toEvent, short cpType = CP_UNKNOWN)
    { 
      return SelectByTime(fromEvent.mtDateTime, fromEvent.mtTable, 
                          toEvent.mtDateTime, toEvent.mtTable, cpType); 
    }

    bool  SelectUnscheduled(short cpType, long cpID = 0, long grID = 0);

  private:
      // temp fields to setup TmEntry
    wxChar  xxAName[9];
    wxChar  xxADesc[65];
    short   xxAPos;
    long    xxAID;       // grID of Qualification

    // temp fields to setup TmEntry
    wxChar  xxXName[9];
    wxChar  xxXDesc[65];
    short   xxXPos;
    long    xxXID;       // grID of Qualification

    wxString  SelectString() const;
    bool  BindRec();
};

# endif
