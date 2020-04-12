/* Copyright (C) 2020 Christoph Theis */

# ifndef  STENTRYSTORE_H
# define  STENTRYSTORE_H

#include  "TmEntryStore.h"

#include  "StStore.h"
#include  "XxStore.h"

#include  "Rec.h"

struct  StEntry : public TmEntry
{
  StRec  st;

  StEntry() {memset(this, 0, sizeof(StEntry));}

  void  Init() {memset(this, 0, sizeof(StEntry));}

  bool  IsBye() const {return (st.stID != 0) && (tmID == 0) && (team.cpType != CP_GROUP);}
};


class  StEntryStore : public StoreObj, public StEntry
{
  public:
    static  bool  CreateView();
    static  bool  RemoveView();

  public:
    StEntryStore(Connection *connPtr = 0);

    virtual void  Init();

    bool  Next();

  public:
    bool  SelectBySt(const StRec &st, short cpType);
    bool  SelectByGr(const GrRec &gr, short cpType);
    bool  SelectById(long id, short cpType);
    bool  SelectById(const std::set<long> &ids, short cpType);
    bool  SelectByNr(const GrRec &gr, short nr, short cpType);

    bool  SelectAll(const CpRec &cp, const wxString &stage);

  private:
    short cpType;

    wxString  SelectString() const;
    bool  BindRec();
};

# endif
